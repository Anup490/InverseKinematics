#include "BasePawn.h"

ABasePawn::ABasePawn() { PrimaryActorTick.bCanEverTick = true; }

void ABasePawn::BeginPlay() { Super::BeginPlay(); }

void ABasePawn::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

FVector ABasePawn::ProjectInPlane(AActor* Plane, FVector WorldPoint)
{
	FTransform Transform = Plane->GetTransform();
	FMatrix LocalToWorld = Transform.ToMatrixWithScale();
	FMatrix WorldToLocal = LocalToWorld.Inverse();
	FVector LocalPoint = WorldToLocal.TransformVector(WorldPoint);
	return LocalToWorld.TransformVector(FVector(0, LocalPoint.Y, LocalPoint.Z));
}

