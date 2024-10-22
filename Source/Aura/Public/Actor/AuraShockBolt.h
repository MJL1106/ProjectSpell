// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/AuraProjectile.h"
#include "AuraShockBolt.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraShockBolt : public AAuraProjectile
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintImplementableEvent)
	void StartOutgoingTimeline();
	
	UPROPERTY(BlueprintReadWrite)
	FDamageEffectParams ExplosionDamageParams;
protected:

	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void BeginPlay() override;
};
