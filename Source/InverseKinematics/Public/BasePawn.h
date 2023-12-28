#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BasePawn.generated.h"

UCLASS()
class INVERSEKINEMATICS_API ABasePawn : public APawn
{
	GENERATED_BODY()
public:
	ABasePawn();
protected:
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable) FVector ProjectInPlane(AActor* Plane, FVector WorldPoint);
public:	
	virtual void Tick(float DeltaTime) override;
};
