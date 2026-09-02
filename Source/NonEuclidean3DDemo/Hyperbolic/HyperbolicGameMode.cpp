// Copyright Epic Games, Inc. All Rights Reserved.

#include "HyperbolicGameMode.h"

#include "EngineUtils.h"
#include "HyperbolicCameraPawn.h"
#include "HyperbolicWorldActor.h"

AHyperbolicGameMode::AHyperbolicGameMode()
{
	DefaultPawnClass = AHyperbolicCameraPawn::StaticClass();
}

void AHyperbolicGameMode::StartPlay()
{
	Super::StartPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AHyperbolicWorldActor> It(World); It; ++It)
	{
		return; // already present
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World->SpawnActor<AHyperbolicWorldActor>(AHyperbolicWorldActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
}
