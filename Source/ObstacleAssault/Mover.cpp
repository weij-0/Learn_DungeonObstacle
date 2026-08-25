// Fill out your copyright notice in the Description page of Project Settings.


#include "Mover.h"
#include "Math/UnrealMathUtility.h"

// Sets default values for this component's properties
UMover::UMover()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMover::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetOwner()->GetActorLocation();
	//UE_LOG(LogTemp, Display, TEXT("%s at location %s"), *GetOwner()->GetActorNameOrLabel(), *StartLocation.ToCompactString());

	SetShouldMove(false);
}


// Called every frame
void UMover::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Move

	FVector CurrentLocation = GetOwner()->GetActorLocation();

	if (!CurrentLocation.Equals(TargetLocation)) // if CurrentLocation not TargetLocation
	{
		float Speed = MoveOffset.Length() / MoveTime;

		FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, Speed);

		GetOwner()->SetActorLocation(NewLocation);
	}
}

//Get and Set MoveOffset
FVector UMover::GetMoveOffset()
{
	return MoveOffset;
}

void UMover::SetMoveOffset(FVector NewMoveOffset)
{
	MoveOffset = NewMoveOffset;
}

//Get and Set MoveTime
float UMover::GetMoveTime()
{
	return MoveTime;
}

void UMover::SetMoveTime(float NewMoveTime)
{
	MoveTime = NewMoveTime;
}

//Get and Set ShouldMove
bool UMover::GetShouldMove()
{
	return ShouldMove;
}

void UMover::SetShouldMove(bool NewShouldMove)
{
	ShouldMove = NewShouldMove;

	// Calculate and Set TargetLocation
	if (ShouldMove)
	{
		TargetLocation = StartLocation + MoveOffset;
	}
	else
	{
		TargetLocation = StartLocation;
	}
}

