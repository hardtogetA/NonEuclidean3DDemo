// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HyperbolicMath.h"
#include "HyperbolicCameraPawn.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UMaterialParameterCollection;
class UMaterialParameterCollectionInstance;
struct FInputActionValue;

/**
 * First-person camera for a hyperbolic (Poincare ball) world.
 *
 * The camera physically stays at the world origin; all motion is expressed as
 * a Lorentz transform (see FHyperbolicPose) that is written into a Material
 * Parameter Collection every tick. The material's World Position Offset uses
 * that transform to warp the Beltrami-Klein geometry into view.
 */
UCLASS()
class NONEUCLIDEAN3DDEMO_API AHyperbolicCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	AHyperbolicCameraPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** MPC that receives the Lorentz transform rows (HLorentzRow0..3). */
	UPROPERTY(EditAnywhere, Category = "Hyperbolic")
	TSoftObjectPtr<UMaterialParameterCollection> HyperbolicMPC;

	/** Hyperbolic distance travelled per second (cm/s). */
	UPROPERTY(EditAnywhere, Category = "Hyperbolic|Movement", meta = (ClampMin = "0.0"))
	float MoveSpeed = 40.0f;

	/** Curvature radius of the hyperbolic space in cm. Must match the material's BallRadius. */
	UPROPERTY(EditAnywhere, Category = "Hyperbolic|Movement", meta = (ClampMin = "1.0"))
	float CurvatureRadius = 100.0f;

	/** Mouse look sensitivity (radians per pixel). */
	UPROPERTY(EditAnywhere, Category = "Hyperbolic|Look")
	float LookSensitivity = 0.003f;

	UPROPERTY(EditAnywhere, Category = "Hyperbolic|Look")
	bool bInvertYaw = false;

	UPROPERTY(EditAnywhere, Category = "Hyperbolic|Look")
	bool bInvertPitch = false;

private:
	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ResetPose(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, Category = "Hyperbolic")
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

	FHyperbolicPose Pose;
};
