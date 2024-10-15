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

protected:

	virtual void OnHit() override;
	
};
