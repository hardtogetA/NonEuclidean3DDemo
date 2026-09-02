// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SphericalWorldActor.generated.h"

class UProceduralMeshComponent;
class UMaterial;

/**
 * Procedurally builds a lattice of cubes authored in gnomonic coordinates
 * (the spherical analogue of Beltrami-Klein) centred on the world origin and
 * applies the spherical material to it.
 */
UCLASS()
class NONEUCLIDEAN3DDEMO_API ASphericalWorldActor : public AActor
{
	GENERATED_BODY()

public:
	ASphericalWorldActor();

	virtual void BeginPlay() override;

	/** The spherical material (with the gnomonic -> stereographic WPO custom node). */
	UPROPERTY(EditAnywhere, Category = "Spherical")
	TSoftObjectPtr<UMaterial> BaseMaterial;

	/** Number of cubes per axis (lattice is GridSize x GridSize x GridSize). */
	UPROPERTY(EditAnywhere, Category = "Spherical|Tiling", meta = (ClampMin = "1"))
	int32 GridSize = 7;

	/** Distance between cube centres (cm). */
	UPROPERTY(EditAnywhere, Category = "Spherical|Tiling", meta = (ClampMin = "0.0"))
	float Spacing = 25.0f;

	/** Cube edge length (cm). */
	UPROPERTY(EditAnywhere, Category = "Spherical|Tiling", meta = (ClampMin = "0.0"))
	float CubeSize = 15.0f;

	/** Largest gnomonic radius (cm) for a cube centre. */
	UPROPERTY(EditAnywhere, Category = "Spherical|Tiling", meta = (ClampMin = "0.0"))
	float MaxRadius = 120.0f;

private:
	void BuildGeometry();

	UPROPERTY(VisibleAnywhere, Category = "Spherical")
	UProceduralMeshComponent* ProcMesh;
};
