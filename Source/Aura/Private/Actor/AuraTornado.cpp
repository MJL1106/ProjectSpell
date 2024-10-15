// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraTornado.h"

#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

AAuraTornado::AAuraTornado()
{
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);

	Sphere->SetupAttachment(Root);
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
