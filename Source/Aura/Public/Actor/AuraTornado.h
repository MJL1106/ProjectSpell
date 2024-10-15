// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/AuraProjectile.h"
#include "AuraTornado.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraTornado : public AAuraProjectile
{
	GENERATED_BODY()

public:
	AAuraTornado();
	
	virtual void Tick(float DeltaSeconds) override;
	

protected:

	virtual void OnHit() override;
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	
	void ApplyBobbingEffectToActor(AActor* AffectedActor, float DeltaTime);
	void ApplyDamageToActor(AActor* ActorToDamage, UAbilitySystemComponent* TargetASC);
	void CauseRepeatingDamage(AActor* ActorToDamage, UAbilitySystemComponent* TargetASC);
	void PrepareDestroyTornado();

	UPROPERTY()
	TArray<AActor*> AffectedActors;

	UPROPERTY()
	TSet<AActor*> DamagedActors;

	UPROPERTY()
	AActor* CurrentAffectedActor = nullptr;

	UPROPERTY()
	TMap<AActor*, FVector> InitialLocations;

	UPROPERTY()
	TMap<AActor*, float> RunningTimeMap;

	UPROPERTY()
	FTimerHandle TornadoDestroyTimerHandle;

	UPROPERTY()
	FTimerHandle DamageTimerHandle;

	UPROPERTY()
	FTimerHandle BobbingTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SineAmplitude = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SinePeriodConstant = 2.0f;
};
