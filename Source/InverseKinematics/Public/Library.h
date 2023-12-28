#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Library.generated.h"

UCLASS()
class INVERSEKINEMATICS_API ULibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable) static FVector ToActorLocalSpace(AActor* Actor, FVector WorldPoint);
};
