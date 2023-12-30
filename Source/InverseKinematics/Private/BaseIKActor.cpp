#include "BaseIKActor.h"
#include "Library.h"

ABaseIKActor::ABaseIKActor() { PrimaryActorTick.bCanEverTick = true; }

void ABaseIKActor::BeginPlay() { Super::BeginPlay(); }

void ABaseIKActor::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void ABaseIKActor::ChangePose(USceneComponent* Root, USceneComponent* Controller, FVector Target)
{
	FVector LocalTarget = ULibrary::ToActorLocalSpace(this, Target);
	FVector ProjLocalTarget(LocalTarget.X, LocalTarget.Y, 0);
	ProjLocalTarget.Normalize();
	float YawAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ProjLocalTarget, FVector(0, 1, 0))));
	if (ProjLocalTarget.Y < 0.0f)
		YawAngle = YawAngle - 180;
	if (ProjLocalTarget.X > 0.0f)
		YawAngle = -YawAngle;
	Root->SetRelativeRotation(FRotator(0, YawAngle, 0));
	FVector RootLocation = ULibrary::ToActorLocalSpace(this, Root->GetComponentLocation());
	FVector RootToTarget = LocalTarget - RootLocation;
	float DistanceToTarget = RootToTarget.Length();
	if (DistanceToTarget <= GetIKSystemLength())
		BackTrackFromLeafArm(Root, Controller, LocalTarget);
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
	FVector ParentOgLocation = ULibrary::ToActorLocalSpace(this, GetOriginalWorldLocationOfComponent(Parent));
	FVector ParentOgToTarget = Target - ParentOgLocation;
	float ParentOgToTargetLen = ParentOgToTarget.Length();
	FVector CompOgLocation = ULibrary::ToActorLocalSpace(this, GetOriginalWorldLocationOfComponent(Component));
	FVector ParentOgToComponentOg = CompOgLocation - ParentOgLocation;
	float ParentOgToComponentOgLen = ParentOgToComponentOg.Length();

	FVector TargetToParentOg = -ParentOgToTarget;
	TargetToParentOg.Normalize();
	TargetToParentOg *= ParentOgToComponentOgLen;

	FVector RootLocation = ULibrary::ToActorLocalSpace(this, Root->GetComponentLocation());
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
	float TargetY = FMath::Abs(Target.Y);
	float RootY = FMath::Abs(RootLocation.Y);
	float Diff = TargetY - RootY;
	if (Diff < 0.0f || Diff > 0.0f)
		Psi = (Target.Y - RootLocation.Y) < 0.0f ? -Psi : Psi;
	ArmData.Add(FArm2DData{ Psi, Parent });
	BackTrackFromLeafArm(Root, Parent, ParentLocation);
}

void ABaseIKActor::ChangePoseFor2ndArm(USceneComponent* Component, FVector Target)
{
	USceneComponent* Parent = Component->GetAttachParent();
	USceneComponent* Root = Parent->GetAttachParent();
	FVector ComponentLocation = ULibrary::ToActorLocalSpace(this, Component->GetComponentLocation());
	FVector ParentLocation = ULibrary::ToActorLocalSpace(this, Parent->GetComponentLocation());
	FVector RootLocation = ULibrary::ToActorLocalSpace(this, Root->GetComponentLocation());

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

	FRotator RootRotation = Root->GetRelativeRotation();
	RootRotation.Roll = Theta1;
	Root->SetRelativeRotation(RootRotation);
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