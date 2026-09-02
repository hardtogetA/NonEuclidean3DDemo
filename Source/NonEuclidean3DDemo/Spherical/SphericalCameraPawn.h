// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SphericalMath.h"
#include "SphericalCameraPawn.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UMaterialParameterCollection;
class UMaterialParameterCollectionInstance;
struct FInputActionValue;

/**
 * First-person camera for a spherical (S^3) world.
 *
 * The camera physically stays at the world origin; all motion is expressed as
 * an SO(4) rotation (see FSphericalPose) written into a Material Parameter
 * Collection every tick. The material's World Position Offset uses that
 * transform to map gnomonic geometry through stereographic projection.
 */
UCLASS()
class NONEUCLIDEAN3DDEMO_API ASphericalCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ASphericalCameraPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** MPC that receives the SO(4) transform rows (SLorentzRow0..3). */
	UPROPERTY(EditAnywhere, Category = "Spherical")
	TSoftObjectPtr<UMaterialParameterCollection> SphericalMPC;

	/** Distance travelled per second (cm/s). */
	UPROPERTY(EditAnywhere, Category = "Spherical|Movement", meta = (ClampMin = "0.0"))
	float MoveSpeed = 40.0f;

	/** Curvature radius of the sphere in cm. Must match the material's BallRadius. */
	UPROPERTY(EditAnywhere, Category = "Spherical|Movement", meta = (ClampMin = "1.0"))
	float CurvatureRadius = 100.0f;

	/** Mouse look sensitivity (radians per pixel). */
	UPROPERTY(EditAnywhere, Category = "Spherical|Look")
	float LookSensitivity = 0.003f;

	UPROPERTY(EditAnywhere, Category = "Spherical|Look")
	bool bInvertYaw = false;

	UPROPERTY(EditAnywhere, Category = "Spherical|Look")
	bool bInvertPitch = false;

private:
	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ResetPose(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, Category = "Spherical")
	UCameraComponent* Camera;

	UPROPERTY()
	UInputAction* MoveForwardAction;

	UPROPERTY()
	UInputAction* MoveRightAction;

	UPROPERTY()
	UInputAction* LookAction;

	UPROPERTY()
	UInputAction* ResetAction;

	UPROPERTY()
	UInputMappingContext* MappingContext;

	UPROPERTY(Transient)
	UMaterialParameterCollectionInstance* MPCInstance;

	FSphericalPose Pose;
};
