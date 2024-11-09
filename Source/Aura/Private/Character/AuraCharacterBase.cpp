// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"
#include "Aura/Aura.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interaction/EnemyInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


AAuraCharacterBase::AAuraCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	
	BurnDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>("BurnDebuffComponent");
	BurnDebuffComponent->SetupAttachment(GetRootComponent());
	BurnDebuffComponent->DebuffTag = GameplayTags.Debuff_Burn;

	StunDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>("StunDebuffComponent");
	StunDebuffComponent->SetupAttachment(GetRootComponent());
	StunDebuffComponent->DebuffTag = GameplayTags.Debuff_Stun;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EffectAttachComponent = CreateDefaultSubobject<USceneComponent>("EffectAttachPoint");
	EffectAttachComponent->SetupAttachment(GetRootComponent());

	HaloOfProtectionNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>("HaloOfProtectionComponent");
	HaloOfProtectionNiagaraComponent->SetupAttachment(EffectAttachComponent);

	HaloOfProtectionSuccessNiagara = CreateDefaultSubobject<UNiagaraComponent>("HaloOfProtectionSuccessNiagaraComponent");
	HaloOfProtectionSuccessNiagara->SetupAttachment(EffectAttachComponent);

	LifeSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>("LifeSiphonNiagaraComponent");
	LifeSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);

	LifeSiphonSuccessNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("LifeSiphonSuccessNiagaraComponent");
	LifeSiphonSuccessNiagaraComponent->SetupAttachment(EffectAttachComponent);

	ManaSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>("ManaSiphonNiagaraComponent");
	ManaSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);

	ManaSiphonSuccessNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("ManaSiphonSuccessNiagaraComponent");
	ManaSiphonSuccessNiagaraComponent->SetupAttachment(EffectAttachComponent);
}

void AAuraCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	EffectAttachComponent->SetWorldRotation(FRotator::ZeroRotator);
}

void AAuraCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAuraCharacterBase, bIsStunned);
	DOREPLIFETIME(AAuraCharacterBase, bIsBurned);
	DOREPLIFETIME(AAuraCharacterBase, bIsBeingShocked);
}

float AAuraCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float DamageTaken = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	OnDamageDelegate.Broadcast(DamageTaken);
	return DamageTaken;
	
}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


UAnimMontage* AAuraCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

void AAuraCharacterBase::Die(const FVector& DeathImpulse)
{
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	MulticastHandleDeath(DeathImpulse);
}

void AAuraCharacterBase::SiphonAttribute(AActor* SourceAvatar, AActor* TargetAvatar,const FGameplayTag& AbilityTag, const FGameplayTag& DataEventTag)
{
	if (const AAuraCharacterBase* AuraSourceCharacter = Cast<AAuraCharacterBase>(SourceAvatar))
	{
		if (UAuraAbilitySystemComponent* SourceASC = Cast<UAuraAbilitySystemComponent>(AuraSourceCharacter->GetAbilitySystemComponent()))
		{
			if (SourceASC->GetStatusFromAbilityTag(AbilityTag) == FAuraGameplayTags::Get().Abilities_Status_Equipped)
			{
				if (TargetAvatar->GetClass()->ImplementsInterface(UEnemyInterface::StaticClass()))
				{
					if (const AAuraCharacterBase* EnemyCharacter = Cast<AAuraCharacterBase>(TargetAvatar))
					{
						if (const UAuraAttributeSet* EnemyAttributes = Cast<UAuraAttributeSet>(EnemyCharacter->GetAttributeSet()))
						{
							const float EnemyMaxHealth = EnemyAttributes->GetMaxHealth();
							const int32 AbilityLevel = UAuraAbilitySystemLibrary::GetAbilityLevelByTag(SourceASC, AbilityTag);

							if (AbilityTag == FAuraGameplayTags::Get().Abilities_Passive_LifeSiphon)
							{
								const float SiphonPercentage = LifeSiphonCurve->GetFloatValue(AbilityLevel);
								const float SiphonAmount = SiphonPercentage * EnemyMaxHealth;
								
								FGameplayEffectSpecHandle SiphonSpecHandle = SourceASC->MakeOutgoingSpec(LifeSiphonGameplayEffect, AbilityLevel, SourceASC->MakeEffectContext());
								if (SiphonSpecHandle.IsValid())
								{
									SiphonSpecHandle.Data->SetSetByCallerMagnitude(DataEventTag, SiphonAmount);
									SourceASC->ApplyGameplayEffectSpecToSelf(*SiphonSpecHandle.Data);
							
									LifeSiphonSuccessNiagaraComponent->Activate();
									FTimerHandle TimerHandle;
									GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
									{
										LifeSiphonSuccessNiagaraComponent->Deactivate();
									}, 1.0f, false);
								}
							}
							else if (AbilityTag == FAuraGameplayTags::Get().Abilities_Passive_ManaSiphon)
							{
								const float SiphonPercentage = ManaSiphonCurve->GetFloatValue(AbilityLevel);
								const float SiphonAmount = SiphonPercentage * EnemyMaxHealth;
								
								FGameplayEffectSpecHandle SiphonSpecHandle = SourceASC->MakeOutgoingSpec(ManaSiphonGameplayEffect, AbilityLevel, SourceASC->MakeEffectContext());
								if (SiphonSpecHandle.IsValid())
								{
									SiphonSpecHandle.Data->SetSetByCallerMagnitude(DataEventTag, SiphonAmount);
									SourceASC->ApplyGameplayEffectSpecToSelf(*SiphonSpecHandle.Data);
							
									ManaSiphonSuccessNiagaraComponent->Activate();
									FTimerHandle TimerHandle;
									GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
									{
										ManaSiphonSuccessNiagaraComponent->Deactivate();
									}, 1.0f, false);
								}
							}
						}
					}
				}
			}
		}
	}
}

bool AAuraCharacterBase::IsSuccessfulHaloProtection(AActor* TargetAvatar)
{
	const FGameplayTag AbilityTag = FAuraGameplayTags::Get().Abilities_Passive_HaloOfProtection;
	
	if (const AAuraCharacterBase* AuraTargetCharacter = Cast<AAuraCharacterBase>(TargetAvatar))
	{
		if (UAuraAbilitySystemComponent* TargetASC = Cast<UAuraAbilitySystemComponent>(AuraTargetCharacter->GetAbilitySystemComponent()))
		{
			if (TargetASC->GetStatusFromAbilityTag(AbilityTag) == FAuraGameplayTags::Get().Abilities_Status_Equipped)
			{
				const int32 AbilityLevel = UAuraAbilitySystemLibrary::GetAbilityLevelByTag(TargetASC, AbilityTag);
				const float ActivationChance = HaloOfProtectionCurve->GetFloatValue(AbilityLevel);
				
				if (FMath::RandRange(1, 100) < ActivationChance * 100)
				{
					if (const UAuraAttributeSet* TargetAttributes = Cast<UAuraAttributeSet>(AuraTargetCharacter->GetAttributeSet()))
					{
						const float PlayerMaxHealth = TargetAttributes->GetMaxHealth();
						const float SiphonAmount = ActivationChance * PlayerMaxHealth;

						FGameplayEffectSpecHandle SiphonSpecHandle = TargetASC->MakeOutgoingSpec(LifeSiphonGameplayEffect, AbilityLevel, TargetASC->MakeEffectContext());
						if (SiphonSpecHandle.IsValid())
						{
							SiphonSpecHandle.Data->SetSetByCallerMagnitude(FAuraGameplayTags::Get().Event_Data_HealthAmount, SiphonAmount);
							TargetASC->ApplyGameplayEffectSpecToSelf(*SiphonSpecHandle.Data);
							
							UGameplayStatics::PlaySoundAtLocation(this, HaloProtectionSuccessfulSound, GetActorLocation());
							
							HaloOfProtectionSuccessNiagara->Activate();
							FTimerHandle TimerHandle;
							GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
							{
								HaloOfProtectionSuccessNiagara->Deactivate();
							}, 1.0f, false);
							
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

FOnDeathSignature& AAuraCharacterBase::GetOnDeathDelegate()
{
	return OnDeathDelegate;
}

void AAuraCharacterBase::MulticastHandleDeath_Implementation(const FVector& DeathImpulse)
{
	UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation(), GetActorRotation());
	
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Weapon->AddImpulse(DeathImpulse * 0.1f, NAME_None, true);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Dissolve();
	bDead = true;
	BurnDebuffComponent->Deactivate();
	StunDebuffComponent->Deactivate();
	OnDeathDelegate.Broadcast(this);
}

void AAuraCharacterBase::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bIsStunned = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bIsStunned ? 0.f : BaseWalkSpeed;
}

void AAuraCharacterBase::OnRep_Stunned()
{
	
}

void AAuraCharacterBase::OnRep_Burned()
{
	
}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag)
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_Weapon) && IsValid(Weapon))
	{
		return Weapon->GetSocketLocation(WeaponTipSocketName);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_LeftHand))
	{
		return GetMesh()->GetSocketLocation(LeftHandSocketName);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_RightHand))
	{
		return GetMesh()->GetSocketLocation(RightHandSocketName);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_Tail))
	{
		return GetMesh()->GetSocketLocation(TailSocketName);
	}
	return FVector();
}

bool AAuraCharacterBase::IsDead_Implementation() const
{
	return bDead;
}

AActor* AAuraCharacterBase::GetAvatar_Implementation()
{
	return this;
}

TArray<FTaggedMontage> AAuraCharacterBase::GetAttackMontages_Implementation()
{
	return AttackMontages;
}

UNiagaraSystem* AAuraCharacterBase::GetBloodEffect_Implementation()
{
	return  BloodEffect;
}

FTaggedMontage AAuraCharacterBase::GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag)
{
	for (FTaggedMontage TaggedMontage : AttackMontages)
	{
		if (TaggedMontage.MontageTag == MontageTag)
		{
			return TaggedMontage;
		}
	}
	return FTaggedMontage();
}

int32 AAuraCharacterBase::GetMinionCount_Implementation()
{
	return MinionCount;
}

void AAuraCharacterBase::IncrementMinionCount_Implementation(int32 Amount)
{
	MinionCount += Amount;
}

ECharacterClass AAuraCharacterBase::GetCharacterClass_Implementation()
{
	return CharacterClass;
}

FOnASCRegistered& AAuraCharacterBase::GetOnASCRegisteredDelegate()
{
	return OnAscRegistered;
}

USkeletalMeshComponent* AAuraCharacterBase::GetWeapon_Implementation()
{
	return Weapon;
}

void AAuraCharacterBase::SetIsBeingShocked_Implementation(bool bIsInShock)
{
	bIsBeingShocked = bIsInShock;
}

bool AAuraCharacterBase::IsBeingShocked_Implementation() const
{
	return bIsBeingShocked;
}

FOnDamageSignature& AAuraCharacterBase::GetOnDamageSignature()
{
	return OnDamageDelegate;
}

void AAuraCharacterBase::InitAbilityActorInfo()
{
}

void AAuraCharacterBase::ApplyEffectToSelf(const TSubclassOf<UGameplayEffect> GameplayEffectClass, const float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AAuraCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}

void AAuraCharacterBase::AddCharacterAbilities()
{
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	if (!HasAuthority()) return;

	AuraASC->AddCharacterAbilities(StartupAbilities);
	AuraASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
}

void AAuraCharacterBase::Dissolve()
{
	if (IsValid(DissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicMatInst);
		StartDissolveTimeline(DynamicMatInst);
	}
	if (IsValid(WeaponDissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
		Weapon->SetMaterial(0, DynamicMatInst);
		StartWeaponDissolveTimeline(DynamicMatInst);
	}
}


