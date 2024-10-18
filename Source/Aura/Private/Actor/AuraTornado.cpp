// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraTornado.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interaction/EnemyInterface.h"
#include "Kismet/GameplayStatics.h"

AAuraTornado::AAuraTornado()
{
	PrimaryActorTick.bCanEverTick = true;
	
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);

	Sphere->SetupAttachment(Root);
}

void AAuraTornado::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	for (AActor* AffectedActor : AffectedActors)
	{
		if (IsValid(AffectedActor))
		{
			ApplyBobbingEffectToActor(AffectedActor, DeltaSeconds);
		}
	}
}

void AAuraTornado::OnHit()
{
	FVector SystemSpawnLocation = GetActorLocation();
	SystemSpawnLocation.Z += 50.f;
	FVector Scale = FVector(2.0f);
	
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, SystemSpawnLocation, FRotator::ZeroRotator, Scale);

	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	bHit = true;
}

void AAuraTornado::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(!IsValidOverlap(OtherActor)) return;
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	
	if (!bHit && !TargetASC)
	{
		OnHit();
	}
	
	if (HasAuthority())
	{
		if (TargetASC)
		{
			if (!AffectedActors.Contains(OtherActor))
			{
				ApplyDamageToActor(OtherActor, TargetASC);

				// Stop tornado moving when hits actor
				ProjectileMovement->Velocity = FVector::ZeroVector;

				if (UCharacterMovementComponent* CharacterMovement = OtherActor->FindComponentByClass<UCharacterMovementComponent>())
				{
					CharacterMovement->DisableMovement();
				}
				
				AffectedActors.Add(OtherActor);
				InitialLocations.Add(OtherActor, OtherActor->GetActorLocation());
				RunningTimeMap.Add(OtherActor, 0.0f);

				// Applies Damage every second
				GetWorldTimerManager().SetTimer(DamageTimerHandle, FTimerDelegate::CreateUObject(this, &AAuraTornado::CauseRepeatingDamage, OtherActor, TargetASC), 1.0f, true);

				// Destroys the tornado after 3 seconds
				GetWorldTimerManager().SetTimer(TornadoDestroyTimerHandle, this, &AAuraTornado::PrepareDestroyTornado, 3.0f, false);
			}
		}
		else
		{
			Destroy();
		}
	}
	else
	{
		bHit = true;
	}
}

void AAuraTornado::ApplyDamageToActor(AActor* ActorToDamage, UAbilitySystemComponent* TargetASC)
{
	if (!ActorToDamage) return;
	
	const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
	DamageEffectParams.DeathImpulse = DeathImpulse;

	DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
	UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
}

void AAuraTornado::CauseRepeatingDamage(AActor* ActorToDamage, UAbilitySystemComponent* TargetASC)
{
	ApplyDamageToActor(ActorToDamage, TargetASC);
}

void AAuraTornado::ApplyBobbingEffectToActor(AActor* AffectedActor, float DeltaTime)
{
	if (!AffectedActor) return;

	FVector InitialLocation = InitialLocations[AffectedActor];
	float& RunningTime = RunningTimeMap[AffectedActor];

	RunningTime += DeltaTime;

	const float Sine = SineAmplitude * (FMath::Sin(RunningTime * SinePeriodConstant) + 1.0f);
	FVector CalculatedLocation = InitialLocation + FVector(0.f, 0.f, Sine);

	AffectedActor->SetActorLocation(CalculatedLocation);
}

void AAuraTornado::PrepareDestroyTornado()
{
	for (AActor* AffectedActor : AffectedActors)
	{
		if (IsValid(AffectedActor))
		{
			if (UCharacterMovementComponent* CharacterMovement = AffectedActor->FindComponentByClass<UCharacterMovementComponent>())
			{
				CharacterMovement->SetMovementMode(EMovementMode::MOVE_Walking);
			}
		}
	}

	AffectedActors.Empty();
	InitialLocations.Empty();
	RunningTimeMap.Empty();

	OnHit();
	Destroy();
}


