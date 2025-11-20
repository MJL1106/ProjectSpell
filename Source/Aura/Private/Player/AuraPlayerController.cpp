// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Actor/MagicCircle.h"
#include "Aura/Aura.h"
#include "Components/DecalComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "Interaction/HighlightInterface.h"
#include "UI/Widget/DamageTextComponent.h"


AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

// Update PlayerTick to conditionally call CursorTrace
void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
    
	// Only trace cursor on PC
#if !PLATFORM_ANDROID && !PLATFORM_IOS
	CursorTrace();
#endif
    
	AutoRun();
	UpdateMagicCircleLocation();
}

void AAuraPlayerController::ShowMagicCircle(UMaterialInterface* DecalMaterial)
{
	if (!IsValid(MagicCircle))
	{
		MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass);
		if (DecalMaterial)
		{
			MagicCircle->MagicCircleDecal->SetMaterial(0, DecalMaterial);
		}
	}
}

void AAuraPlayerController::HideMagicCircle()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->Destroy();
	}
}

void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}

void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

void AAuraPlayerController::UpdateMagicCircleLocation()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->SetActorLocation(CursorHit.ImpactPoint);
	}
}


void AAuraPlayerController::HighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_HighlightActor(InActor);
	}
}

void AAuraPlayerController::UnHighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_UnHighlightActor(InActor);
	}
}

// Modify CursorTrace to only run on PC
void AAuraPlayerController::CursorTrace()
{
	// Only do cursor trace on PC platforms
#if PLATFORM_ANDROID || PLATFORM_IOS
	return; // Skip cursor trace on mobile
#endif
    
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_CursorTrace))
	{
		UnHighlightActor(LastActor);
		UnHighlightActor(ThisActor);
        
		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}
    
	const ECollisionChannel TraceChannel = IsValid(MagicCircle) ? ECC_ExcludePlayers : ECC_Visibility;
	GetHitResultUnderCursor(TraceChannel, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	if (IsValid(CursorHit.GetActor()) && CursorHit.GetActor()->Implements<UHighlightInterface>())
	{
		ThisActor = CursorHit.GetActor();
	}
	else
	{
		ThisActor = nullptr;
	}
    
	if (LastActor != ThisActor)
	{
		UnHighlightActor(LastActor);
		HighlightActor(ThisActor);
	}
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed)) return;
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_MDM))
	{
		if (IsValid(ThisActor))
		{
			TargetingStatus = ThisActor->Implements<UEnemyInterface>() ? ETargetingStatus::TargetingEnemy : ETargetingStatus::TargetingNonEnemy;
		}
		else
		{
			TargetingStatus = ETargetingStatus::NotTargeting;
		}
		bAutoRunning = false;
	}
	if (GetASC())
	{
		GetASC()->AbilityInputTagPressed(InputTag);
	}
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputReleased)) return;
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_MDM))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
		return;
	}

	// Check if the mouse is targeting something e.g. an enemy
	if (GetASC())
	{
		GetASC()->AbilityInputTagReleased(InputTag);
	}
	
	if (TargetingStatus != ETargetingStatus::TargetingEnemy && !bShiftKeyDown)
	{
		APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())
			{
				IHighlightInterface::Execute_SetMoveToLocation(ThisActor, CachedDestination);
			}
			else if (GetASC() && !GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
			}
			if(UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
				}
				if (NavPath->PathPoints.Num()>0)
				{
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
					bAutoRunning = true;
				}
			}
		}
		FollowTime = 0.f;
		TargetingStatus = ETargetingStatus::NotTargeting;
	}
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputHeld)) return;
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_MDM))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;
	}

	// Check if the mouse is targeting something e.g. an enemy
	if (TargetingStatus == ETargetingStatus::TargetingEnemy || bShiftKeyDown)
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
	}
	else // Not Targeting so find location to move to
	{
		FollowTime += GetWorld()->GetDeltaSeconds();
		
		if (CursorHit.bBlockingHit)
		{
			CachedDestination = CursorHit.ImpactPoint;
		}

		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}

void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	UE_LOG(LogTemp, Warning, TEXT("AuraPlayerController BeginPlay"));
    
	if (TouchInputAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("Touch Input Action is SET"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Touch Input Action is NULL - Set it in Blueprint!"));
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	// Platform-specific setup
#if PLATFORM_ANDROID || PLATFORM_IOS
	// Mobile setup
	bShowMouseCursor = false;
	DefaultMouseCursor = EMouseCursor::None;
        
	FInputModeGameOnly InputModeData;
	SetInputMode(InputModeData);
#else
	// PC setup
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
#endif
}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
    
	// Existing PC bindings
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);
    
	// Touch/Click bindings - simplified
	if (TouchInputAction)
	{
		AuraInputComponent->BindAction(TouchInputAction, ETriggerEvent::Started, this, &AAuraPlayerController::OnTouchStarted);
		AuraInputComponent->BindAction(TouchInputAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::OnTouchTriggered);
		AuraInputComponent->BindAction(TouchInputAction, ETriggerEvent::Completed, this, &AAuraPlayerController::OnTouchCompleted);
	}
    
	// Ability bindings
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, 
		&ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed)) return;
	
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	
	const FVector ForwardDirection = FVector(1.0f, 0.0f, 0.0f);
	const FVector RightDirection = FVector(0.0f, 1.0f, 0.0f);
	
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}
void AAuraPlayerController::OnTouchStarted()
{
    // Don't process if blocked
    if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed)) 
        return;
    
    TouchStartTime = GetWorld()->GetTimeSeconds();
    bIsTouchHeld = false;
    bAutoRunning = false; // Stop any current auto-run
    
    // Get mouse/touch position
    float MouseX, MouseY;
    GetMousePosition(MouseX, MouseY);
    FVector2D ScreenPosition(MouseX, MouseY);
    LastTouchLocation = ScreenPosition;
    
    // Get hit result
    FHitResult HitResult;
    GetHitResultAtScreenPosition(ScreenPosition, ECC_Visibility, false, HitResult);
    
    if (!HitResult.bBlockingHit) return;
    
    // Store for other systems
    CursorHit = HitResult;
    
    // Check what we hit
    if (HitResult.GetActor() && HitResult.GetActor()->Implements<UEnemyInterface>())
    {
        // Enemy - start attacking
        ThisActor = HitResult.GetActor();
        TargetingStatus = ETargetingStatus::TargetingEnemy;
        
        // Highlight enemy
        HighlightActor(ThisActor);
        
        // Start attack
        if (GetASC())
        {
            GetASC()->AbilityInputTagPressed(FAuraGameplayTags::Get().InputTag_LMB);
        }
    }
    else
    {
        // Ground - store the destination for when we release
        TargetingStatus = ETargetingStatus::NotTargeting;
        CachedDestination = HitResult.ImpactPoint;
    }
}

void AAuraPlayerController::OnTouchTriggered()
{
    // Don't process if blocked
    if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputHeld)) 
        return;
    
    float HoldTime = GetWorld()->GetTimeSeconds() - TouchStartTime;
    
    // Get current mouse/touch position
    float MouseX, MouseY;
    GetMousePosition(MouseX, MouseY);
    FVector2D ScreenPosition(MouseX, MouseY);
    
    if (TargetingStatus == ETargetingStatus::TargetingEnemy)
    {
        // Continue attacking while held
        if (GetASC())
        {
            GetASC()->AbilityInputTagHeld(FAuraGameplayTags::Get().InputTag_LMB);
        }
    }
    else if (HoldTime > 0.3f) // Only activate hold-to-move after a delay
    {
        // This is optional - hold and drag to continuously update movement
        bIsTouchHeld = true;
        
        FHitResult HitResult;
        GetHitResultAtScreenPosition(ScreenPosition, ECC_Visibility, false, HitResult);
        
        if (HitResult.bBlockingHit)
        {
            CachedDestination = HitResult.ImpactPoint;
            
            // Direct movement for drag
            if (APawn* ControlledPawn = GetPawn())
            {
                const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
                ControlledPawn->AddMovementInput(WorldDirection);
            }
        }
    }
}

void AAuraPlayerController::OnTouchCompleted()
{
    // Don't process if blocked
    if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputReleased)) 
        return;
    
    float TouchDuration = GetWorld()->GetTimeSeconds() - TouchStartTime;
    
    if (TargetingStatus == ETargetingStatus::TargetingEnemy)
    {
        // Stop attacking
        if (GetASC())
        {
            GetASC()->AbilityInputTagReleased(FAuraGameplayTags::Get().InputTag_LMB);
        }
        
        // Unhighlight
        UnHighlightActor(ThisActor);
        ThisActor = nullptr;
        TargetingStatus = ETargetingStatus::NotTargeting;
    }
    else if (!bIsTouchHeld) // If we didn't hold and drag
    {
        // TAP TO MOVE - This is the key part!
        // Get the final touch position
        float MouseX, MouseY;
        GetMousePosition(MouseX, MouseY);
        FVector2D ScreenPosition(MouseX, MouseY);
        
        FHitResult HitResult;
        GetHitResultAtScreenPosition(ScreenPosition, ECC_Visibility, false, HitResult);
        
        if (HitResult.bBlockingHit)
        {
            FVector Destination = HitResult.ImpactPoint;
            APawn* ControlledPawn = GetPawn();
            
            if (ControlledPawn)
            {
                // Set up pathfinding for tap-to-move
                if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(
                    this, ControlledPawn->GetActorLocation(), Destination))
                {
                    Spline->ClearSplinePoints();
                    for (const FVector& PointLoc : NavPath->PathPoints)
                    {
                        Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
                    }
                    
                    if (NavPath->PathPoints.Num() > 0)
                    {
                        CachedDestination = NavPath->PathPoints.Last();
                        bAutoRunning = true; // This enables tap-to-move!
                        
                        // Visual feedback at tap location
                        if (ClickNiagaraSystem)
                        {
                            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                                this, ClickNiagaraSystem, Destination);
                        }
                    }
                }
            }
        }
    }
    
    // Reset state
    bIsTouchHeld = false;
    FollowTime = 0.f;
}