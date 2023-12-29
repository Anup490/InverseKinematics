#pragma once
#include "CoreMinimal.h"

USTRUCT(NoExport)
struct FArm2DData
{
	UPROPERTY() float Psi;
	UPROPERTY() USceneComponent* Component;
};
