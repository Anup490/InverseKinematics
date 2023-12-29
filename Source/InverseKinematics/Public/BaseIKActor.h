#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Arm2DData.h"
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
	UFUNCTION(BlueprintImplementableEvent) FVector GetOriginalLocationOfComponent(USceneComponent* Component);
private:
	TArray<FArm2DData> ArmData;
	void BackTrackFromLeafArm(USceneComponent* Root, USceneComponent* Component, FVector Target);
	void CalculatePsiAngle(USceneComponent* Root, USceneComponent* Component, FVector Target);
	void ChangePoseFor2ndArm(USceneComponent* Component, FVector Target);
	void ChangePoseForNthArm(float ThetaSum, int32 Index);
};