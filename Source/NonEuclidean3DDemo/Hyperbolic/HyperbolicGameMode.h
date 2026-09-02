// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HyperbolicGameMode.generated.h"

/**
 * Game mode for the hyperbolic demo: spawns the hyperbolic camera pawn and a
 * single hyperbolic world actor.
 */
UCLASS()
class NONEUCLIDEAN3DDEMO_API AHyperbolicGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHyperbolicGameMode();

	virtual void StartPlay() override;
};
