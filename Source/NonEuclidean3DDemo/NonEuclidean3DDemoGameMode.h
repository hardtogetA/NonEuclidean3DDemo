// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NonEuclidean3DDemoGameMode.generated.h"

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class ANonEuclidean3DDemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANonEuclidean3DDemoGameMode();
};



