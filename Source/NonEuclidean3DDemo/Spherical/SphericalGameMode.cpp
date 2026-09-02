// Copyright Epic Games, Inc. All Rights Reserved.

#include "SphericalGameMode.h"

#include "EngineUtils.h"
#include "SphericalCameraPawn.h"
#include "SphericalWorldActor.h"

ASphericalGameMode::ASphericalGameMode()
{
	DefaultPawnClass = ASphericalCameraPawn::StaticClass();
}

void ASphericalGameMode::StartPlay()
{
	Super::StartPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ASphericalWorldActor> It(World); It; ++It)
	{
		return; // already present
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<ASphericalWorldActor>(ASphericalWorldActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
}
