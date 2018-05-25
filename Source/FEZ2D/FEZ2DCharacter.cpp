// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "FEZ2DCharacter.h"
#include "PaperFlipbookComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Camera/CameraComponent.h"
#include "Engine.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(SideScrollerCharacter, Log, All);

//////////////////////////////////////////////////////////////////////////
// AFEZ2DCharacter

AFEZ2DCharacter::AFEZ2DCharacter()
{
	// Use only Yaw from the controller and ignore the rest of the rotation.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Set the size of our collision capsule.
	GetCapsuleComponent()->SetCapsuleHalfHeight(96.0f);
	GetCapsuleComponent()->SetCapsuleRadius(40.0f);

	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetCapsuleComponent());
	CameraBoom->TargetArmLength = 500.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 75.0f);
	CameraBoom->bAbsoluteRotation = true;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->RelativeRotation = FRotator(0.0f, -90.0f, 0.0f);

	RootComponent->bEditableWhenInherited = true;
	GetCapsuleComponent()->bEditableWhenInherited = true;
	CameraBoom->bEditableWhenInherited = true;
	

	// Create an orthographic camera (no perspective) and attach it to the boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	SideViewCameraComponent->OrthoWidth = 2048.0f;
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Prevent all automatic rotation behavior on the camera, character, and camera component
	CameraBoom->bAbsoluteRotation = true;
	SideViewCameraComponent->bUsePawnControlRotation = false;
	SideViewCameraComponent->bAutoActivate = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.0f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.0f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxFlySpeed = 600.0f;

	// Lock character motion onto the XZ plane, so the character can't move in or out of the screen
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

	// Behave like a traditional 2D platformer character, with a flat bottom instead of a curved capsule bottom
	// Note: This can cause a little floating when going up inclines; you can choose the tradeoff between better
	// behavior on the edge of a ledge versus inclines by setting this to true or false
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;


	//LineTrace for DepthCorrection;


    // 	TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("IncarGear"));
    // 	TextComponent->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f));
    // 	TextComponent->SetRelativeLocation(FVector(35.0f, 5.0f, 20.0f));
    // 	TextComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    // 	TextComponent->SetupAttachment(RootComponent);

	// Enable replication on the Sprite component so animations show up when networked
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;
}

//////////////////////////////////////////////////////////////////////////
// Animation

void AFEZ2DCharacter::UpdateAnimation()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();

	UPaperFlipbook* DesiredAnimation = IdleAnimation;

	if (PlayerVelocity.Z > 0 || PlayerVelocity.Z < 0) {
		DesiredAnimation = JumpAnimation;
	}
	else if (PlayerVelocity.X > 0 || PlayerVelocity.X < 0) {
		DesiredAnimation = RunningAnimation;
	}
	else {
		DesiredAnimation = IdleAnimation;
	}



	// Are we moving or standing still?
	if( GetSprite()->GetFlipbook() != DesiredAnimation 	)
	{
		GetSprite()->SetFlipbook(DesiredAnimation);
	}
}

void AFEZ2DCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	UpdateCharacter();	
}


//////////////////////////////////////////////////////////////////////////
// Input

void AFEZ2DCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Note: the 'Jump' action and the 'MoveRight' axis are bound to actual keys/buttons/sticks in DefaultInput.ini (editable from Project Settings..Input)
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFEZ2DCharacter::MoveRight);

	PlayerInputComponent->BindTouch(IE_Pressed, this, &AFEZ2DCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AFEZ2DCharacter::TouchStopped);



	PlayerInputComponent->BindAction("CameraRight", IE_Pressed, this, &AFEZ2DCharacter::CameraRight);
	PlayerInputComponent->BindAction("CameraRight", IE_Released, this, &AFEZ2DCharacter::CameraRightStop);

	PlayerInputComponent->BindAction("CameraLeft", IE_Pressed, this, &AFEZ2DCharacter::CameraLeft);
	PlayerInputComponent->BindAction("CameraLeft", IE_Released, this, &AFEZ2DCharacter::CameraLeftStop);

	PlayerInputComponent->BindAction("Fall", IE_Pressed, this, &AFEZ2DCharacter::Fall);
	PlayerInputComponent->BindAction("Fall", IE_Released, this, &AFEZ2DCharacter::FallStop);

}

void AFEZ2DCharacter::MoveRight(float Value)
{
	/*UpdateChar();*/

	// Apply the input to the character motion
	FVector vec = CameraBoom->GetForwardVector();
	//AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Value);
	vec.RotateAngleAxis(90.f, FVector(0, 0, 1));
	//GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, vec.ToString());
	
	AddMovementInput(FVector(vec.Y * -1, 0, 0), Value);
}

void AFEZ2DCharacter::TouchStarted(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Jump on any touch
	Jump();
}

void AFEZ2DCharacter::TouchStopped(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	// Cease jumping once touch stopped
	StopJumping();
}

void AFEZ2DCharacter::UpdateCharacter()
{
	// Update animation to match the motion
	UpdateAnimation();

	// Now setup the rotation of the controller based on the direction we are travelling
	const FVector PlayerVelocity = GetVelocity();	
	float TravelDirection = PlayerVelocity.X;
	// Set the rotation so that the character faces his direction of travel.
	if (Controller != nullptr)
	{
		if (TravelDirection < 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0, 180.0f, 0.0f));
		}
		else if (TravelDirection > 0.0f)
		{
			Controller->SetControlRotation(FRotator(0.0f, 0.0f, 0.0f));
		}
	}

	DepthCorrection();
}


void AFEZ2DCharacter::CameraRight()
{
	if (bCanCameraRotate) {
		FRotator NewCapsuleRotation = GetCapsuleComponent()->RelativeRotation;
		NewCapsuleRotation.Yaw = NewCapsuleRotation.Yaw + -90.f;
		GetCapsuleComponent()->SetRelativeRotation(NewCapsuleRotation, true);
		
		
		FRotator NewCameraRotation = CameraBoom->RelativeRotation;
		NewCameraRotation.Yaw = NewCameraRotation.Yaw + -90.f;
		CameraBoom->RelativeRotation = NewCameraRotation;

		//CameraBoom->SetRelativeRotation(NewRotator, true);
		//CameraBoom->SetRelativeRotation(FMath::Lerp(CameraBoom->RelativeRotation, NewRotator, 0.05f));



		GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, TEXT("CameraRight"));
		MoveRight(1.f);
	}
	bCanCameraRotate = false;
}

void AFEZ2DCharacter::CameraLeft()
{
	if (bCanCameraRotate) {
		FRotator NewCapsuleRotation = GetCapsuleComponent()->RelativeRotation;
		NewCapsuleRotation.Yaw = NewCapsuleRotation.Yaw + +90.f;
		GetCapsuleComponent()->SetRelativeRotation(NewCapsuleRotation, true);


		FRotator NewCameraRotation = CameraBoom->RelativeRotation;
		NewCameraRotation.Yaw = NewCameraRotation.Yaw + +90.f;
		CameraBoom->RelativeRotation = NewCameraRotation;
		GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, TEXT("CameraLeft"));
		MoveRight(1.f);
	}
	bCanCameraRotate = false;
}

void AFEZ2DCharacter::CameraRightStop()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, TEXT("CameraRightStop"));
	bCanCameraRotate = true;
}

void AFEZ2DCharacter::CameraLeftStop()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, TEXT("CameraLeftStop"));
	bCanCameraRotate = true;
}

void AFEZ2DCharacter::Fall()
{
	if (bCanFall) {
		FVector ActorLocation = GetActorLocation();
		ActorLocation.Z -= 10.f;
		SetActorLocation(ActorLocation);
	}
	bCanFall = false;
}

void AFEZ2DCharacter::FallStop()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, TEXT("Fall"));
	bCanFall = true;
}


void AFEZ2DCharacter::DepthCorrection()
{
	FHitResult OutHit;

	FVector Start = SideViewCameraComponent->GetComponentLocation()+FVector(0.0,0.0,-100);
	FVector FrwdVec =  SideViewCameraComponent->GetForwardVector();
	FVector End = ((FrwdVec *1000.0f) + Start);
	
	FCollisionQueryParams CollisionParams;

	DrawDebugLine(GetWorld(), Start, End, FColor::Emerald, false, 1, 0, 1);
	

	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			if (GEngine) {
				SetNewPositionDepth(OutHit.ImpactPoint, FrwdVec);
				DrawDebugPoint(GetWorld(), OutHit.ImpactPoint, 50, FColor::Red, false, 1, 0);
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Emerald, FString::Printf(TEXT("You are hitting: %s Location: %s"), *OutHit.GetActor()->GetName(), *OutHit.ImpactPoint.ToCompactString()));
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("ForwardVector: %s "), *FrwdVec.ToCompactString()));
				
				
			}
		}
	}
}


void AFEZ2DCharacter::SetNewPositionDepth(FVector & impactPoint, FVector & FrwdVec) 
{
	FVector location = GetActorLocation();
	if (FrwdVec.X != 0) {
		GetCapsuleComponent()->SetWorldLocation(FVector(impactPoint.X, location.Y, location.Z), false);
	}
	else if (FrwdVec.Y != 0) {
		GetCapsuleComponent()->SetWorldLocation(FVector(location.X, impactPoint.Y, location.Z), false);
	}
	
}