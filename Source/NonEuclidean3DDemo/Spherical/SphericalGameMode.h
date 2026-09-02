// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SphericalGameMode.generated.h"

/**
 * Game mode for the spherical demo: spawns the spherical camera pawn and a
 * single spherical world actor.
 */
UCLASS()
class NONEUCLIDEAN3DDEMO_API ASphericalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASphericalGameMode();

	virtual void StartPlay() override;
};
