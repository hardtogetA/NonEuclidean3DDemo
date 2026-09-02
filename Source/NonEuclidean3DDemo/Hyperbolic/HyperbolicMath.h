// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Math/Color.h"

/**
 * Lorentz pose of the player inside hyperbolic space.
 *
 * A point in hyperbolic space is represented on the hyperboloid sheet:
 *     (x, y, z, t)  with  t^2 - x^2 - y^2 - z^2 = 1,  t > 0.
 *
 * The Beltrami-Klein coordinates used for authored geometry are
 *     k = (x, y, z) / t,   |k| < 1,
 * and the Poincare-ball coordinates (what we rasterize) are
 *     p = (x, y, z) / (1 + t).
 *
 * The player's rigid motion is a Lorentz transformation. We store the
 * world->player transform M (column-vector convention: v' = M * v), so that
 * moving / looking is done by LEFT-multiplying M with a boost / rotation.
 * This automatically captures gyration (holonomy): walking a closed loop
 * rotates the view, exactly like the GyroVector.`gyr` in Hyperbolica.
 */
struct FHyperbolicPose
{
	float M[4][4]; // row-major [row][col]

	static FHyperbolicPose Identity()
	{
		FHyperbolicPose P;
		FMemory::Memzero(P.M, sizeof(P.M));
		P.M[0][0] = P.M[1][1] = P.M[2][2] = P.M[3][3] = 1.0f;
		return P;
	}

	static FHyperbolicPose Multiply(const FHyperbolicPose& A, const FHyperbolicPose& B)
	{
		FHyperbolicPose C;
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
	void ComposeLeft(const FHyperbolicPose& Delta)
	{
		*this = Multiply(Delta, *this);
	}

	/** Lorentz boost along unit direction U by rapidity Rho. */
	static FHyperbolicPose Boost(const FVector& U, float Rho)
	{
		const float Ch = FMath::Cosh(Rho);
		const float Sh = FMath::Sinh(Rho);
		const float C1 = Ch - 1.0f;
		const FVector u = U.GetSafeNormal();

		FHyperbolicPose P;
		P.M[0][0] = 1.0f + C1 * u.X * u.X; P.M[0][1] = C1 * u.X * u.Y;      P.M[0][2] = C1 * u.X * u.Z;      P.M[0][3] = Sh * u.X;
		P.M[1][0] = C1 * u.Y * u.X;      P.M[1][1] = 1.0f + C1 * u.Y * u.Y; P.M[1][2] = C1 * u.Y * u.Z;      P.M[1][3] = Sh * u.Y;
		P.M[2][0] = C1 * u.Z * u.X;      P.M[2][1] = C1 * u.Z * u.Y;        P.M[2][2] = 1.0f + C1 * u.Z * u.Z; P.M[2][3] = Sh * u.Z;
		P.M[3][0] = Sh * u.X;            P.M[3][1] = Sh * u.Y;              P.M[3][2] = Sh * u.Z;              P.M[3][3] = Ch;
		return P;
	}

	/** Lorentz rotation (a plain 3D rotation that leaves the time component fixed). */
	static FHyperbolicPose Rotation(const FQuat& Q)
	{
		const FVector X = Q.RotateVector(FVector(1.f, 0.f, 0.f));
		const FVector Y = Q.RotateVector(FVector(0.f, 1.f, 0.f));
		const FVector Z = Q.RotateVector(FVector(0.f, 0.f, 1.f));

		FHyperbolicPose P;
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
