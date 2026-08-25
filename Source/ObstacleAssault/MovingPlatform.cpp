// Fill out your copyright notice in the Description page of Project Settings.


#include "MovingPlatform.h"

// Sets default values
AMovingPlatform::AMovingPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMovingPlatform::BeginPlay()
{
	Super::BeginPlay();

	SetActorLocation(StartLocation);
}

// Called every frame
void AMovingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Call Move the platform ea frame
	MovePlatform(DeltaTime);
	RotatePlatform(DeltaTime);

}



// Move the platform back and forth
void AMovingPlatform::MovePlatform(float DeltaTime) {

	FVector CurrentLocation = GetActorLocation();

	CurrentLocation = CurrentLocation + (MoveVelocity * DeltaTime);

	SetActorLocation(CurrentLocation);

	DistMoved = FVector::Dist(StartLocation, CurrentLocation);

	if (DistMoved >= MoveDist) {
		
		FVector MoveDirection = MoveVelocity.GetSafeNormal();
		//UE_LOG(LogTemp, Display, TEXT("MoveDirection = MoveVelocity.GetSafeNormal = (%f, %f, %f)"), MoveDirection.X, MoveDirection.Y, MoveDirection.Z);

		StartLocation = StartLocation + MoveDirection * MoveDist;
		SetActorLocation(StartLocation);

		MoveVelocity = -MoveVelocity;
	}
}


// Rotate the platform
void AMovingPlatform::RotatePlatform(float DeltaTime) {

	AddActorLocalRotation(RotationVelocity * DeltaTime);
}
