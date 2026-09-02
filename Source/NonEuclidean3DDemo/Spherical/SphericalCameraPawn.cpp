// Copyright Epic Games, Inc. All Rights Reserved.

#include "SphericalCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

ASphericalCameraPawn::ASphericalCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Root);
	Camera->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
	Camera->bUsePawnControlRotation = false;

	SphericalMPC = TSoftObjectPtr<UMaterialParameterCollection>(
		FSoftObjectPath(TEXT("/Game/Spherical/MPC_Spherical.MPC_Spherical")));

	// --- Enhanced Input, built at runtime so no editor assets are needed ---
	MoveForwardAction = NewObject<UInputAction>(this, TEXT("MoveForward"));
	MoveForwardAction->ValueType = EInputActionValueType::Axis1D;

	MoveRightAction = NewObject<UInputAction>(this, TEXT("MoveRight"));
	MoveRightAction->ValueType = EInputActionValueType::Axis1D;

	LookAction = NewObject<UInputAction>(this, TEXT("Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;

	ResetAction = NewObject<UInputAction>(this, TEXT("Reset"));
	ResetAction->ValueType = EInputActionValueType::Boolean;

	MappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Spherical"));

	MappingContext->MapKey(MoveForwardAction, EKeys::W);
	FEnhancedActionKeyMapping& S = MappingContext->MapKey(MoveForwardAction, EKeys::S);
	S.Modifiers.Add(NewObject<UInputModifierNegate>(this, TEXT("NegateForward")));

	MappingContext->MapKey(MoveRightAction, EKeys::D);
	FEnhancedActionKeyMapping& A = MappingContext->MapKey(MoveRightAction, EKeys::A);
	A.Modifiers.Add(NewObject<UInputModifierNegate>(this, TEXT("NegateRight")));

	MappingContext->MapKey(LookAction, EKeys::Mouse2D);
	MappingContext->MapKey(ResetAction, EKeys::R);

	Pose = FSphericalPose::Identity();
}

void ASphericalCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialParameterCollection* ResolvedMPC = SphericalMPC.LoadSynchronous())
	{
		MPCInstance = GetWorld()->GetParameterCollectionInstance(ResolvedMPC);
	}
	if (!MPCInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("SphericalCameraPawn: no MPC at '%s'. Create it or set SphericalMPC."), *SphericalMPC.ToString());
	}

	// Push identity once so the first rendered frame is already correct.
	Tick(0.f);
}

void ASphericalCameraPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MPCInstance)
	{
		MPCInstance->SetVectorParameterValue(FName(TEXT("SLorentzRow0")), Pose.Row(0));
		MPCInstance->SetVectorParameterValue(FName(TEXT("SLorentzRow1")), Pose.Row(1));
		MPCInstance->SetVectorParameterValue(FName(TEXT("SLorentzRow2")), Pose.Row(2));
		MPCInstance->SetVectorParameterValue(FName(TEXT("SLorentzRow3")), Pose.Row(3));
	}
}

void ASphericalCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &ASphericalCameraPawn::MoveForward);
		EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &ASphericalCameraPawn::MoveRight);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASphericalCameraPawn::Look);
		EIC->BindAction(ResetAction, ETriggerEvent::Started, this, &ASphericalCameraPawn::ResetPose);

		if (const APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (const ULocalPlayer* LP = PC->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				{
					Subsystem->AddMappingContext(MappingContext, 0);
				}
			}
		}
	}
}

void ASphericalCameraPawn::MoveForward(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	const float Delta = (MoveSpeed / CurvatureRadius) * Axis * GetWorld()->GetDeltaSeconds();
	if (!FMath::IsNearlyZero(Delta))
	{
		// Moving forward = rotating the world backwards in the (X, W) plane.
		Pose.ComposeLeft(FSphericalPose::MoveAlong(FVector(-1.f, 0.f, 0.f), Delta));
	}
}

void ASphericalCameraPawn::MoveRight(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	const float Delta = (MoveSpeed / CurvatureRadius) * Axis * GetWorld()->GetDeltaSeconds();
	if (!FMath::IsNearlyZero(Delta))
	{
		Pose.ComposeLeft(FSphericalPose::MoveAlong(FVector(0.f, -1.f, 0.f), Delta));
	}
}

void ASphericalCameraPawn::Look(const FInputActionValue& Value)
{
	const FVector2D V = Value.Get<FVector2D>();

	float Yaw = V.X;
	float Pitch = V.Y;

	if (bInvertYaw)
	{
		Yaw = -Yaw;
	}
	if (bInvertPitch)
	{
		Pitch = -Pitch;
	}

	Yaw *= LookSensitivity;
	Pitch *= LookSensitivity;

	if (!FMath::IsNearlyZero(Pitch))
	{
		Pose.ComposeLeft(FSphericalPose::Rotation(FQuat(FVector(0.f, 1.f, 0.f), Pitch)));
	}
	if (!FMath::IsNearlyZero(Yaw))
	{
		Pose.ComposeLeft(FSphericalPose::Rotation(FQuat(FVector(0.f, 0.f, 1.f), -Yaw)));
	}
}

void ASphericalCameraPawn::ResetPose(const FInputActionValue& Value)
{
	Pose = FSphericalPose::Identity();
}
