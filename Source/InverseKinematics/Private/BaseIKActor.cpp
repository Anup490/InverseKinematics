#include "BaseIKActor.h"
#include "Library.h"

ABaseIKActor::ABaseIKActor() { PrimaryActorTick.bCanEverTick = true; }

void ABaseIKActor::BeginPlay() { Super::BeginPlay(); }

void ABaseIKActor::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void ABaseIKActor::ChangePose(USceneComponent* Root, USceneComponent* Controller, FVector Target)
{
	FVector RootLocation = Root->GetComponentLocation();
	Target.X = RootLocation.X;
	FVector RootToTarget = Target - RootLocation;
	float DistanceToTarget = RootToTarget.Length();
	if (DistanceToTarget <= GetIKSystemLength())
		BackTrackFromLeafArm(Root, Controller, Target);
	else
	{
		RootToTarget.Normalize();
		RootToTarget *= DistanceToTarget;
		FVector NewTarget = RootLocation + RootToTarget;
		BackTrackFromLeafArm(Root, Controller, NewTarget);
	}
}

void ABaseIKActor::BackTrackFromLeafArm(USceneComponent* Root, USceneComponent* Component, FVector Target)
{
	USceneComponent* Parent = Component->GetAttachParent();
	USceneComponent* GrandParent = Parent->GetAttachParent();
	USceneComponent* GreatGrandParent = GrandParent->GetAttachParent();
	USceneComponent* Great2GrandParent = GreatGrandParent->GetAttachParent();
	if (Parent && GrandParent && GreatGrandParent && Great2GrandParent)
		CalculatePsiAngle(Root, Component, Target);
	else if (Parent && GrandParent && GreatGrandParent && !Great2GrandParent)
		ChangePoseFor2ndArm(Component, Target);
}

void ABaseIKActor::CalculatePsiAngle(USceneComponent* Root, USceneComponent* Component, FVector Target)
{
	USceneComponent* Parent = Component->GetAttachParent();
	FVector ParentOgLocation = GetOriginalLocationOfComponent(Parent);
	FVector ParentOgToTarget = Target - ParentOgLocation;
	float ParentOgToTargetLen = ParentOgToTarget.Length();
	FVector CompOgLocation = GetOriginalLocationOfComponent(Component);
	FVector ParentOgToComponentOg = CompOgLocation - ParentOgLocation;
	float ParentOgToComponentOgLen = ParentOgToComponentOg.Length();

	FVector TargetToParentOg = -ParentOgToTarget;
	TargetToParentOg.Normalize();
	TargetToParentOg *= ParentOgToComponentOgLen;

	FVector RootLocation = Root->GetComponentLocation();
	FVector RootToTarget = Target - RootLocation;
	FVector RootToParent = RootToTarget + TargetToParentOg;
	float RootToParentLen = RootToParent.Length();
	FVector RootToParentOg = ParentOgLocation - RootLocation;
	float RootToParentOgLen = RootToParentOg.Length();

	float Psi = 0;
	FVector ParentLocation;
	if (RootToParentLen > RootToParentOgLen)
	{
		FVector TargetToRoot = -RootToTarget;
		TargetToRoot.Normalize();
		TargetToRoot *= ParentOgToComponentOgLen;
		ParentLocation = Target + TargetToRoot;

		FVector ParentToTarget = Target - ParentLocation;
		float ParentToTargetLen = ParentToTarget.Length();
		float Dot = FVector::DotProduct(ParentToTarget, ParentOgToComponentOg);
		Psi = FMath::RadiansToDegrees(FMath::Acos(Dot / (ParentToTargetLen * ParentOgToComponentOgLen)));
	}
	else
	{
		ParentLocation = Target + TargetToParentOg;
		float Dot = FVector::DotProduct(ParentOgToTarget, ParentOgToComponentOg);
		Psi = FMath::RadiansToDegrees(FMath::Acos(Dot / (ParentOgToTargetLen * ParentOgToComponentOgLen)));
	}
	Psi = (Target.Y - RootLocation.Y) < 0 ? -Psi : Psi;
	ArmData.Add(FArm2DData{ Psi, Parent });
	BackTrackFromLeafArm(Root, Parent, ParentLocation);
}

void ABaseIKActor::ChangePoseFor2ndArm(USceneComponent* Component, FVector Target)
{
	USceneComponent* Parent = Component->GetAttachParent();
	USceneComponent* Root = Parent->GetAttachParent();
	FVector ComponentLocation = Component->GetComponentLocation();
	FVector ParentLocation = Parent->GetComponentLocation();
	FVector RootLocation = Root->GetComponentLocation();

	FVector ParentToControl = ComponentLocation - ParentLocation;
	float ParentToControlLen = ParentToControl.Length();
	float ParentToControlLenSquared = ParentToControlLen * ParentToControlLen;

	FVector RootToParent = ParentLocation - RootLocation;
	float RootToParentLen = RootToParent.Length();
	float RootToParentLenSquared = RootToParentLen * RootToParentLen;

	FVector RootToTarget = Target - RootLocation;
	float RootToTargetLen = RootToTarget.Length();
	float RootToTargetLenSquared = RootToTargetLen * RootToTargetLen;

	float Theta1ATanNumerator = Target.Z - RootLocation.Z;
	float Theta1ATanDenominator = Target.Y - RootLocation.Y;
	bool IsYNegative = Theta1ATanDenominator < 0.0f;
	float ArcTangent = FMath::RadiansToDegrees(FMath::Atan(Theta1ATanNumerator / Theta1ATanDenominator));

	float Theta1ACosNumerator = RootToParentLenSquared + RootToTargetLenSquared - ParentToControlLenSquared;
	float Theta1ACosDenominator = 2 * RootToParentLen * RootToTargetLen;
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
	float Theta2Numerator = ParentToControlLenSquared + RootToParentLenSquared - RootToTargetLenSquared;
	float Theta2Denominator = 2 * ParentToControlLen * RootToParentLen;
	float ArcCosineTheta2 = FMath::RadiansToDegrees(FMath::Acos(Theta2Numerator / Theta2Denominator));

	if (IsYNegative)
		ArcCosineTheta2 = -ArcCosineTheta2;
	float Theta2 = 180.0 - ArcCosineTheta2;
	if (IsYNegative)
		Theta2 = -Theta2;
	Parent->SetRelativeRotation(FRotator(0, 0, Theta2));
	ChangePoseForNthArm(Theta1 + Theta2, ArmData.Num() - 1);
}

void ABaseIKActor::ChangePoseForNthArm(float ThetaSum, int32 Index)
{
	if (Index > -1)
	{
		FArm2DData Arm = ArmData[Index];
		USceneComponent* Component = Arm.Component;
		float Theta = Arm.Psi - ThetaSum;
		Component->SetRelativeRotation(FRotator(0, 0, Theta));
		ChangePoseForNthArm(Theta + ThetaSum, --Index);
	}
	else
		ArmData.Empty();
}