// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HyperbolicWorldActor.generated.h"

class UProceduralMeshComponent;
class UMaterial;

/**
 * Procedurally builds a lattice of cubes authored in Beltrami-Klein
 * coordinates (a ball of radius BallRadius centred on the world origin) and
 * applies the hyperbolic material to it.
 */
UCLASS()
class NONEUCLIDEAN3DDEMO_API AHyperbolicWorldActor : public AActor
{
	GENERATED_BODY()

public:
	AHyperbolicWorldActor();

	virtual void BeginPlay() override;

	/** The hyperbolic material (with the Klein->Poincare WPO custom node). */
	UPROPERTY(EditAnywhere, Category = "Hyperbolic")
	TSoftObjectPtr<UMaterial> BaseMaterial;

	/** Number of cubes per axis (lattice is GridSize x GridSize x GridSize). */
	UPROPERTY(EditAnywhere, Category = "Hyperbolic|Tiling", meta = (ClampMin = "1"))
	int32 GridSize = 7;

	/** Distance between cube centres (cm). */
	UPROPERTY(EditAnywhere, Category = "Hyperbolic|Tiling", meta = (ClampMin = "0.0"))
	float Spacing = 20.0f;

	/** Cube edge length (cm). */
	UPROPERTY(EditAnywhere, Category = "Hyperbolic|Tiling", meta = (ClampMin = "0.0"))
	float CubeSize = 12.0f;

	/** Largest Klein radius (cm) for a cube centre. Must stay below BallRadius. */
	UPROPERTY(EditAnywhere, Category = "Hyperbolic|Tiling", meta = (ClampMin = "0.0"))
	float MaxKleinRadius = 92.0f;

private:
	void BuildGeometry();

	UPROPERTY(VisibleAnywhere, Category = "Hyperbolic")
	UProceduralMeshComponent* ProcMesh;
};
