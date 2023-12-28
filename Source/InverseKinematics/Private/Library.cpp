#include "Library.h"

FVector ULibrary::ToActorLocalSpace(AActor* Actor, FVector WorldPoint)
{
	FTransform Transform = Actor->GetTransform();
	FMatrix LocalToWorld = Transform.ToMatrixWithScale();
	FMatrix WorldToLocal = LocalToWorld.Inverse();
	return WorldToLocal.TransformVector(WorldPoint);
}

