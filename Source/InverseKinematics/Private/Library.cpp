#include "Library.h"

FVector ULibrary::ToActorLocalSpace(AActor* Actor, FVector WorldPoint)
{
	FTransform Transform = Actor->GetTransform();
	FMatrix LocalToWorld = Transform.ToMatrixWithScale();
	FMatrix WorldToLocal = LocalToWorld.Inverse();
	FVector4 LocalPoint4D = WorldToLocal.TransformFVector4(FVector4(WorldPoint.X, WorldPoint.Y, WorldPoint.Z, 1));
	return FVector(LocalPoint4D.X, LocalPoint4D.Y, LocalPoint4D.Z);
}

FVector ULibrary::ProjectInActorYZPlane(AActor* Actor, FVector WorldPoint)
{
	FTransform Transform = Actor->GetTransform();
	FMatrix LocalToWorld = Transform.ToMatrixWithScale();
	FMatrix WorldToLocal = LocalToWorld.Inverse();
	FVector4 LocalPoint4D = WorldToLocal.TransformFVector4(FVector4(WorldPoint.X, WorldPoint.Y, WorldPoint.Z, 1));
	FVector4 ProjWorldPoint4D = LocalToWorld.TransformFVector4(FVector4(0, LocalPoint4D.Y, LocalPoint4D.Z, 1));
	return FVector(ProjWorldPoint4D.X, ProjWorldPoint4D.Y, ProjWorldPoint4D.Z);
}

