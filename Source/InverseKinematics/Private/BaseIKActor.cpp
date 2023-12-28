#include "BaseIKActor.h"
#include "Library.h"

ABaseIKActor::ABaseIKActor() { PrimaryActorTick.bCanEverTick = true; }

void ABaseIKActor::BeginPlay() { Super::BeginPlay(); }

void ABaseIKActor::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void ABaseIKActor::ChangePose(USceneComponent* Root, USceneComponent* Controller, FVector Target)
{
	FVector CompLocation = Root->GetComponentLocation();
	FVector CompToTarget = Target - CompLocation;
	float DistanceToTarget = CompToTarget.Length();
	if (DistanceToTarget <= GetIKSystemLength())
		BackTrackFromLeafArm(Controller, NULL, Target);
	else
	{
		CompToTarget.Normalize();
		CompToTarget *= DistanceToTarget;
		FVector NewTarget = CompLocation + CompToTarget;
		BackTrackFromLeafArm(Controller, NULL, NewTarget);
	}
}

void ABaseIKActor::ChangePoseFor2ndArm(USceneComponent* Controller, FVector Target)
{
	USceneComponent* Component = Controller->GetAttachParent();
	USceneComponent* Root = Component->GetAttachParent();
	FVector ControlLocation = Controller->GetComponentLocation();
	FVector CompLocation = Component->GetComponentLocation();
	FVector RootLocation = Root->GetComponentLocation();

	FVector CompToControl = ControlLocation - CompLocation;
	float CompToControlLen = CompToControl.Length();
	float CompToControlLenSquared = CompToControlLen * CompToControlLen;

	FVector RootToComp = CompLocation - RootLocation;
	float RootToCompLen = RootToComp.Length();
	float RootToCompLenSquared = RootToCompLen * RootToCompLen;

	FVector RootToTarget = Target - RootLocation;
	float RootToTargetLen = RootToTarget.Length();
	float RootToTargetLenSquared = RootToTargetLen * RootToTargetLen;

	float Theta1ATanNumerator = Target.Z - RootLocation.Z;
	float Theta1ATanDenominator = Target.Y - RootLocation.Y;
	bool IsYNegative = Theta1ATanDenominator < 0.0f;
	float ArcTangent = FMath::RadiansToDegrees(FMath::Atan(Theta1ATanNumerator / Theta1ATanDenominator));

	float Theta1ACosNumerator = RootToCompLenSquared + RootToTargetLenSquared - CompToControlLenSquared;
	float Theta1ACosDenominator = 2 * RootToCompLen * RootToTargetLen;
	float ArcCosine = FMath::RadiansToDegrees(FMath::Acos(Theta1ACosNumerator / Theta1ACosDenominator));
	
	if (IsYNegative)
	{
		ArcCosine = -ArcCosine;
		ArcTangent = -ArcTangent;
	}

	float Theta1 = 90 - ArcCosine - ArcTangent;
	if (IsYNegative)
		Theta1 = -Theta1;

	Root->SetRelativeRotation(FRotator(0, 0, Theta1));
	float Theta2Numerator = CompToControlLenSquared + RootToCompLenSquared - RootToTargetLenSquared;
	float Theta2Denominator = 2 * CompToControlLen * RootToCompLen;
	float ArcCosineTheta2 = FMath::RadiansToDegrees(FMath::Acos(Theta2Numerator / Theta2Denominator));

	if (IsYNegative)
		ArcCosineTheta2 = -ArcCosineTheta2;
	float Theta2 = 180.0 - ArcCosineTheta2;
	if (IsYNegative)
		Theta2 = -Theta2;
	Component->SetRelativeRotation(FRotator(0, 0, Theta2));
}

void ABaseIKActor::BackTrackFromLeafArm(USceneComponent* Component, USceneComponent* Child, FVector Target)
{
	USceneComponent* Parent = Component->GetAttachParent();
	USceneComponent* GrandParent = Parent->GetAttachParent();
	USceneComponent* GreatGrandParent = GrandParent->GetAttachParent();
	USceneComponent* Root = GreatGrandParent->GetAttachParent();
	if (Parent && GrandParent && GreatGrandParent && !Root)
		ChangePoseFor2ndArm(Component, Target);
}

