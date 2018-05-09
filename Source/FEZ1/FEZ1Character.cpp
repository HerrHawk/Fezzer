// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "FEZ1Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine.h"

AFEZ1Character::AFEZ1Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Rotation of the character should not affect rotation of boom
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->SocketOffset = FVector(0.f,0.f,75.f);
	CameraBoom->RelativeRotation = FRotator(0.f,180.f,0.f);

	// Create a camera and attach to boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	SideViewCameraComponent->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Face in the direction we are moving..
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->GravityScale = 2.f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.f;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxFlySpeed = 600.f;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

void AFEZ1Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFEZ1Character::MoveRight);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &AFEZ1Character::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AFEZ1Character::TouchStopped);


	PlayerInputComponent->BindAction("CameraRight", IE_Pressed, this, &AFEZ1Character::CameraRight);
	PlayerInputComponent->BindAction("CameraRight", IE_Released, this, &AFEZ1Character::CameraRightStop);

	PlayerInputComponent->BindAction("CameraLeft", IE_Pressed, this, &AFEZ1Character::CameraLeft);
	PlayerInputComponent->BindAction("CameraLeft", IE_Released, this, &AFEZ1Character::CameraLeftStop);

	PlayerInputComponent->BindAction("Fall", IE_Pressed, this, &AFEZ1Character::Fall);
	PlayerInputComponent->BindAction("Fall", IE_Released, this, &AFEZ1Character::FallStop);

}

void AFEZ1Character::CameraRight()
{
	if (bCanCameraRotate) {
		FRotator NewRotator = CameraBoom->RelativeRotation;
		NewRotator.Yaw = NewRotator.Yaw + -90.f;
		//CameraBoom->RelativeRotation = NewRotator;

		CameraBoom->SetRelativeRotation(NewRotator, true);
		//CameraBoom->SetRelativeRotation(FMath::Lerp(CameraBoom->RelativeRotation, NewRotator, 0.05f));
		


		GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, TEXT("CameraRight"));
		MoveRight(1.f);
	}
	bCanCameraRotate = false;
}

void AFEZ1Character::CameraLeft()
{
	if (bCanCameraRotate) {
		FRotator NewRotator = CameraBoom->RelativeRotation;
		NewRotator.Yaw = NewRotator.Yaw + 90.f;
		//CameraBoom->RelativeRotation = NewRotator;
		CameraBoom->SetRelativeRotation(NewRotator, true);
		GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, TEXT("CameraLeft"));
		MoveRight(1.f);
	}
	bCanCameraRotate = false;
}

void AFEZ1Character::CameraRightStop()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, TEXT("CameraRightStop"));
	bCanCameraRotate = true;
}

void AFEZ1Character::CameraLeftStop()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, TEXT("CameraLeftStop"));
	bCanCameraRotate = true;
}


void AFEZ1Character::Fall()
{
	if (bCanFall) {
		FVector ActorLocation = GetActorLocation();
		ActorLocation.Z -= 10.f;
		SetActorLocation(ActorLocation);
	}
	bCanFall = false;
}

void AFEZ1Character::FallStop()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, TEXT("Fall"));
	bCanFall = true;
}

























void AFEZ1Character::MoveRight(float Value)
{
	// add movement in that direction
	//AddMovementInput(FVector(0.f, -1.f, 0.f), Value);
	AddMovementInput(CameraBoom->GetRightVector(), Value);
}



void AFEZ1Character::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// jump on any touch
	Jump();
}

void AFEZ1Character::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	StopJumping();
}

