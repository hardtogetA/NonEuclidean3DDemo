// Copyright Epic Games, Inc. All Rights Reserved.

#include "HyperbolicCameraPawn.h"

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

AHyperbolicCameraPawn::AHyperbolicCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Root);
	Camera->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
	Camera->bUsePawnControlRotation = false;

	HyperbolicMPC = TSoftObjectPtr<UMaterialParameterCollection>(
		FSoftObjectPath(TEXT("/Game/Hyperbolic/MPC_Hyperbolic.MPC_Hyperbolic")));

	// --- Enhanced Input, built at runtime so no editor assets are needed ---
	MoveForwardAction = NewObject<UInputAction>(this, TEXT("MoveForward"));
	MoveForwardAction->ValueType = EInputActionValueType::Axis1D;

	MoveRightAction = NewObject<UInputAction>(this, TEXT("MoveRight"));
	MoveRightAction->ValueType = EInputActionValueType::Axis1D;

	LookAction = NewObject<UInputAction>(this, TEXT("Look"));
	LookAction->ValueType = EInputActionValueType::Axis2D;

	ResetAction = NewObject<UInputAction>(this, TEXT("Reset"));
	ResetAction->ValueType = EInputActionValueType::Boolean;

	MappingContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Hyperbolic"));

	MappingContext->MapKey(MoveForwardAction, EKeys::W);
	FEnhancedActionKeyMapping& S = MappingContext->MapKey(MoveForwardAction, EKeys::S);
	S.Modifiers.Add(NewObject<UInputModifierNegate>(this, TEXT("NegateForward")));

	MappingContext->MapKey(MoveRightAction, EKeys::D);
	FEnhancedActionKeyMapping& A = MappingContext->MapKey(MoveRightAction, EKeys::A);
	A.Modifiers.Add(NewObject<UInputModifierNegate>(this, TEXT("NegateRight")));

	MappingContext->MapKey(LookAction, EKeys::Mouse2D);
	MappingContext->MapKey(ResetAction, EKeys::R);

	Pose = FHyperbolicPose::Identity();
}

void AHyperbolicCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialParameterCollection* ResolvedMPC = HyperbolicMPC.LoadSynchronous())
	{
		MPCInstance = GetWorld()->GetParameterCollectionInstance(ResolvedMPC);
	}
	if (!MPCInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("HyperbolicCameraPawn: no MPC at '%s'. Create it or set HyperbolicMPC."), *HyperbolicMPC.ToString());
	}

	// Push identity once so the first rendered frame is already correct.
	Tick(0.f);
}

void AHyperbolicCameraPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MPCInstance)
	{
		MPCInstance->SetVectorParameterValue(FName(TEXT("HLorentzRow0")), Pose.Row(0));
		MPCInstance->SetVectorParameterValue(FName(TEXT("HLorentzRow1")), Pose.Row(1));
		MPCInstance->SetVectorParameterValue(FName(TEXT("HLorentzRow2")), Pose.Row(2));
		MPCInstance->SetVectorParameterValue(FName(TEXT("HLorentzRow3")), Pose.Row(3));
	}
}

void AHyperbolicCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AHyperbolicCameraPawn::MoveForward);
		EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AHyperbolicCameraPawn::MoveRight);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHyperbolicCameraPawn::Look);
		EIC->BindAction(ResetAction, ETriggerEvent::Started, this, &AHyperbolicCameraPawn::ResetPose);

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

void AHyperbolicCameraPawn::MoveForward(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	const float Delta = (MoveSpeed / CurvatureRadius) * Axis * GetWorld()->GetDeltaSeconds();
	if (!FMath::IsNearlyZero(Delta))
	{
		// Moving forward = boosting the world backwards along local +X.
		Pose.ComposeLeft(FHyperbolicPose::Boost(FVector(-1.f, 0.f, 0.f), Delta));
	}
}

void AHyperbolicCameraPawn::MoveRight(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	const float Delta = (MoveSpeed / CurvatureRadius) * Axis * GetWorld()->GetDeltaSeconds();
	if (!FMath::IsNearlyZero(Delta))
	{
		Pose.ComposeLeft(FHyperbolicPose::Boost(FVector(0.f, -1.f, 0.f), Delta));
	}
}

void AHyperbolicCameraPawn::Look(const FInputActionValue& Value)
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
		Pose.ComposeLeft(FHyperbolicPose::Rotation(FQuat(FVector(0.f, 1.f, 0.f), Pitch)));
	}
	if (!FMath::IsNearlyZero(Yaw))
	{
		Pose.ComposeLeft(FHyperbolicPose::Rotation(FQuat(FVector(0.f, 0.f, 1.f), -Yaw)));
	}
}

void AHyperbolicCameraPawn::ResetPose(const FInputActionValue& Value)
{
	Pose = FHyperbolicPose::Identity();
}
