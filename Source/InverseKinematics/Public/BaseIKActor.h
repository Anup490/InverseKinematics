#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseIKActor.generated.h"

UCLASS()
class INVERSEKINEMATICS_API ABaseIKActor : public AActor
{
	GENERATED_BODY()
public:	
	ABaseIKActor();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;
	UFUNCTION(BlueprintCallable) void ChangePose(USceneComponent* Root, USceneComponent* Controller, FVector Target);
	UFUNCTION(BlueprintImplementableEvent) float GetIKSystemLength();
private:
	void ChangePoseFor2ndArm(USceneComponent* Controller, FVector Target);
	void BackTrackFromLeafArm(USceneComponent* Component, USceneComponent* Child, FVector Target);
};
