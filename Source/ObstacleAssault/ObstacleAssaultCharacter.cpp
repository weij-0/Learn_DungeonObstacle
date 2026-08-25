// Copyright Epic Games, Inc. All Rights Reserved.

#include "ObstacleAssaultCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ObstacleAssault.h"
#include "CollectableItem.h"
#include "Lock.h"

AObstacleAssaultCharacter::AObstacleAssaultCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a first person camera
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position at head level
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a camera boom (pulls in towards the player if there is a collision)
	//CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	//CameraBoom->SetupAttachment(RootComponent);
	//CameraBoom->TargetArmLength = 400.0f;
	//CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	//FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	//FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AObstacleAssaultCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping (Safely mapping your custom C++ functions)
		if (JumpAction) {
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AObstacleAssaultCharacter::DoJumpStart);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AObstacleAssaultCharacter::DoJumpEnd);
		}

		// Moving & Looking
		if (MoveAction) EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AObstacleAssaultCharacter::Move);
		if (LookAction) EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AObstacleAssaultCharacter::Look);
		if (MouseLookAction) EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AObstacleAssaultCharacter::Look);

		// Interact
		if (InteractAction) {
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AObstacleAssaultCharacter::Interact);
		}
	}
	else
	{
		UE_LOG(LogObstacleAssault, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AObstacleAssaultCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AObstacleAssaultCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AObstacleAssaultCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AObstacleAssaultCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AObstacleAssaultCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
	UE_LOG(LogTemp, Display, TEXT("Space Jump"));
}

void AObstacleAssaultCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AObstacleAssaultCharacter::Interact()
{
	FVector Start = FirstPersonCameraComponent->GetComponentLocation();
	FVector End = Start + (FirstPersonCameraComponent->GetForwardVector() * MaxInteractionDistance);

	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 5.0f);

	FCollisionShape InteractionSphere = FCollisionShape::MakeSphere(InteractionSphereRadius);
	DrawDebugSphere(GetWorld(), End, InteractionSphereRadius, 20, FColor::Blue, false, 5.0f);

	FHitResult HitResult;
	bool HasHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_GameTraceChannel2,
		InteractionSphere
	);

	if (HasHit)
	{
		AActor* HitActor = HitResult.GetActor();

		if (HitActor->ActorHasTag("CollectableItem"))
		{

			ACollectableItem* CollectableItem = Cast<ACollectableItem>(HitActor);

			if (CollectableItem)
			{
				// Add item into inventory
				ItemList.Add(CollectableItem->ItemName);
				
				// Remove item from world
				CollectableItem->Destroy();
			}

		}
		else if (HitActor->ActorHasTag("Lock"))
		{

			ALock* Lock = Cast<ALock>(HitActor);

			if (Lock)
			{
				if (!Lock->GetIsKeyPlaced())
				{
					// If "lock" is empty (not on table), remove from inventory
					int32 ItemRemoved = ItemList.RemoveSingle(Lock->KeyItemName);

					if (ItemRemoved) {
						// Add item back to world
						Lock->SetIsKeyPlaced(true);
					}
					else 
					{
						UE_LOG(LogTemp, Display, TEXT("Lock not in inventory."));
					}
				}
				else
				{
					// If "lock" on table,

					// Add item into inventory, remove item from world
					ItemList.Add(Lock->KeyItemName);
					Lock->SetIsKeyPlaced(false);
					
				}
				
			}

		}
	}
	else
	{

	}
}