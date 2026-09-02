// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Math/Color.h"

/**
 * SO(4) rotation pose of the player inside spherical (positive-curvature) space.
 *
 * Spherical 3-space is the 3-sphere S^3 embedded in R^4:
 *     (x, y, z, w)  with  x^2 + y^2 + z^2 + w^2 = 1.
 *
 * The gnomonic (authoring) coordinates are
 *     k = (x, y, z) / w,          |k| unbounded,
 * and the stereographic (render) coordinates are
 *     p = (x, y, z) / (1 + w).
 *
 * This is the positive-curvature analogue of the hyperbolic (Poincare ball)
 * demo: the isometry group is SO(4) instead of the Lorentz group SO(1,3), and a
 * "move" is a rotation in the (direction, w) plane instead of a hyperbolic boost.
 */
struct FSphericalPose
{
	float M[4][4]; // row-major [row][col]

	static FSphericalPose Identity()
	{
		FSphericalPose P;
		FMemory::Memzero(P.M, sizeof(P.M));
		P.M[0][0] = P.M[1][1] = P.M[2][2] = P.M[3][3] = 1.0f;
		return P;
	}

	static FSphericalPose Multiply(const FSphericalPose& A, const FSphericalPose& B)
	{
		FSphericalPose C;
		for (int32 R = 0; R < 4; ++R)
		{
			for (int32 Col = 0; Col < 4; ++Col)
			{
				C.M[R][Col] = A.M[R][0] * B.M[0][Col]
							+ A.M[R][1] * B.M[1][Col]
							+ A.M[R][2] * B.M[2][Col]
							+ A.M[R][3] * B.M[3][Col];
			}
		}
		return C;
	}

	/** Compose a local-frame delta on the left: Pose = Delta * Pose. */
	void ComposeLeft(const FSphericalPose& Delta)
	{
		*this = Multiply(Delta, *this);
	}

	/** Rotation in the (U, W) plane by Angle radians. "Moving" along U. */
	static FSphericalPose MoveAlong(const FVector& U, float Angle)
	{
		const float C = FMath::Cos(Angle);
		const float S = FMath::Sin(Angle);
		const float C1 = C - 1.0f;
		const FVector u = U.GetSafeNormal();

		FSphericalPose P;
		P.M[0][0] = 1.0f + C1 * u.X * u.X; P.M[0][1] = C1 * u.X * u.Y;      P.M[0][2] = C1 * u.X * u.Z;      P.M[0][3] = S * u.X;
		P.M[1][0] = C1 * u.Y * u.X;      P.M[1][1] = 1.0f + C1 * u.Y * u.Y; P.M[1][2] = C1 * u.Y * u.Z;      P.M[1][3] = S * u.Y;
		P.M[2][0] = C1 * u.Z * u.X;      P.M[2][1] = C1 * u.Z * u.Y;        P.M[2][2] = 1.0f + C1 * u.Z * u.Z; P.M[2][3] = S * u.Z;
		P.M[3][0] = -S * u.X;            P.M[3][1] = -S * u.Y;              P.M[3][2] = -S * u.Z;              P.M[3][3] = C;
		return P;
	}

	/** Spatial rotation (a plain 3D rotation that leaves the w component fixed). */
	static FSphericalPose Rotation(const FQuat& Q)
	{
		const FVector X = Q.RotateVector(FVector(1.f, 0.f, 0.f));
		const FVector Y = Q.RotateVector(FVector(0.f, 1.f, 0.f));
		const FVector Z = Q.RotateVector(FVector(0.f, 0.f, 1.f));

		FSphericalPose P;
		P.M[0][0] = X.X; P.M[0][1] = Y.X; P.M[0][2] = Z.X; P.M[0][3] = 0.f;
		P.M[1][0] = X.Y; P.M[1][1] = Y.Y; P.M[1][2] = Z.Y; P.M[1][3] = 0.f;
		P.M[2][0] = X.Z; P.M[2][1] = Y.Z; P.M[2][2] = Z.Z; P.M[2][3] = 0.f;
		P.M[3][0] = 0.f; P.M[3][1] = 0.f; P.M[3][2] = 0.f; P.M[3][3] = 1.f;
		return P;
	}

	/** Matrix row as a float4, ready for a Material Parameter Collection vector. */
	FLinearColor Row(int32 Index) const
	{
		return FLinearColor(M[Index][0], M[Index][1], M[Index][2], M[Index][3]);
	}
};
