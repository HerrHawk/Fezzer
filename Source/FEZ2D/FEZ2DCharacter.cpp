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
	CameraBoom->TargetArmLength = 0.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 0.0f);
	CameraBoom->bAbsoluteRotation = true;
	CameraBoom->bDoCollisionTest = false;
	//CameraBoom->RelativeRotation = FRotator(0.0f, -90.0f, 0.0f);

	RootComponent->bEditableWhenInherited = true;
	GetCapsuleComponent()->bEditableWhenInherited = true;
	CameraBoom->bEditableWhenInherited = true;
	
	

	// Create an orthographic camera (no perspective) and attach it to the boom
	SideViewCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->ProjectionMode = ECameraProjectionMode::Orthographic;
	SideViewCameraComponent->OrthoWidth = 4096.0f;
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Prevent all automatic rotation behavior on the camera, character, and camera component
	CameraBoom->bAbsoluteRotation = true;
	SideViewCameraComponent->bUsePawnControlRotation = false;
	SideViewCameraComponent->bAutoActivate = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;


	CameraBoom->bUsePawnControlRotation = true;
	SideViewCameraComponent->SetRelativeLocationAndRotation(FVector(0.f, 4000.f, 0.f), FRotator(0.f, -90.f, 0.f));


	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.0f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.0f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MaxFlySpeed = 600.0f;

	// Lock character motion onto the XZ plane, so the character can't move in or out of the screen
	GetCharacterMovement()->bConstrainToPlane = false;
	//GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

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

	CameraRotationSpeed = 90.f;
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
	else if (PlayerVelocity.X > 0 || PlayerVelocity.X < 0 || PlayerVelocity.Y < 0 || PlayerVelocity.Y > 0) {
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

	
	if (bCanCameraRotate) {
		UpdateCharacter();
	} 
	else
	{
		CameraRotation(DeltaSeconds);
		//GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, FString::Printf(TEXT("Delta Seconds %f"), DeltaSeconds));
	}

}


//////////////////////////////////////////////////////////////////////////
// Input

void AFEZ2DCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Note: the 'Jump' action and the 'MoveRight' axis are bound to actual keys/buttons/sticks in DefaultInput.ini (editable from Project Settings..Input)
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFEZ2DCharacter::MoveRight);

	PlayerInputComponent->BindAction("CameraRight", IE_Pressed, this, &AFEZ2DCharacter::CameraRight);

	PlayerInputComponent->BindAction("CameraLeft", IE_Pressed, this, &AFEZ2DCharacter::CameraLeft);

}

void AFEZ2DCharacter::MoveRight(float Value)
{
	
	if (Value<-0.01 || Value>0.01) {
		DepthCorrection();
	}
	
	FVector vec = CameraBoom->GetForwardVector();
	vec = vec.RotateAngleAxis(90.f, FVector(0, 0, 1));
	
	AddMovementInput(GetCapsuleComponent()->GetForwardVector(), Value);
	
	FRotator SpriteRotation = GetCapsuleComponent()->GetComponentRotation();
	if (Value > 0.f)
	{
		GetSprite()->SetWorldRotation(SpriteRotation);
	}
	else if (Value < 0.f)
	{
		SpriteRotation.Yaw = SpriteRotation.Yaw - 180.f;
		GetSprite()->SetWorldRotation(SpriteRotation);
	}

}


void AFEZ2DCharacter::UpdateCharacter()
{

	UpdateAnimation();
	

}


void AFEZ2DCharacter::CameraRight()
{
	if (bCanCameraRotate) {		

		
		NewCapsuleRotation = GetCapsuleComponent()->GetComponentRotation();
		NewCapsuleRotation.Yaw = NewCapsuleRotation.Yaw + -90.f;
		CapsuleRotationDifference = NewCapsuleRotation - GetCapsuleComponent()->GetComponentRotation();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("new %s"), *NewCapsuleRotation.ToString()));

		FreezeLocation = GetCapsuleComponent()->GetComponentLocation();
		FreezeVelocity = GetCharacterMovement()->Velocity;
		FreezeVelocity = FreezeVelocity.RotateAngleAxis(-90.f, FVector(0.f, 0.f, 1.f));
		GetSprite()->Stop();


		GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, TEXT("CameraRight"));
		starttime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
		
	}
	bCanCameraRotate = false;
}

void AFEZ2DCharacter::CameraLeft()
{
	if (bCanCameraRotate) {

		NewCapsuleRotation = GetCapsuleComponent()->GetComponentRotation();
		NewCapsuleRotation.Yaw = NewCapsuleRotation.Yaw + 90.f;
		CapsuleRotationDifference = NewCapsuleRotation - GetCapsuleComponent()->GetComponentRotation();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("new %s"), *NewCapsuleRotation.ToString()));

		FreezeLocation = GetCapsuleComponent()->GetComponentLocation();
		FreezeVelocity = GetCharacterMovement()->Velocity;
		FreezeVelocity = FreezeVelocity.RotateAngleAxis(90.f, FVector(0.f, 0.f, 1.f));
		GetSprite()->Stop();

		GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, TEXT("CameraLeft"));
		
	}
	bCanCameraRotate = false;
}



void AFEZ2DCharacter::CameraRotation(float DeltaSeconds)
{
	Controller->SetControlRotation(GetCapsuleComponent()->GetComponentRotation() + (CapsuleRotationDifference * ((CameraRotationSpeed * DeltaSeconds)/90.f)));

	GetCapsuleComponent()->SetWorldLocation(FreezeLocation);
	GetCharacterMovement()->Velocity = FreezeVelocity;

	float RotationDifference = NewCapsuleRotation.Yaw - GetCapsuleComponent()->GetComponentRotation().Yaw;
	if (RotationDifference >= -0.5f && RotationDifference <= 0.5f || RotationDifference <= -359.5f && RotationDifference >= -360.5f)
	{
		bCanCameraRotate = true;
		NewCapsuleRotation.Yaw = roundf(NewCapsuleRotation.Yaw);
		Controller->SetControlRotation(NewCapsuleRotation);
		GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, FString::Printf(TEXT("Finished Roting")));
		GetSprite()->Play();
		endtime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
		GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, FString::Printf(TEXT("time %f"), endtime-starttime));
	}
	else
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Red, FString::Printf(TEXT("Difference %f"), RotationDifference));
	}
}

void AFEZ2DCharacter::Falling() {
	ACharacter::Falling();
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("I don't feel so good")));
	//DepthCorrection();
}


void AFEZ2DCharacter::DepthCorrection()
{
	

	FHitResult OutHit;

	FVector Start = SideViewCameraComponent->GetComponentLocation()+FVector(0.0,0.0,-120);
	FVector FrwdVec =  SideViewCameraComponent->GetForwardVector();
	FVector End = ((FrwdVec *7000.0f) + Start);
	
	FCollisionObjectQueryParams CollisionParams(ECC_TO_BITFIELD(ECC_GameTraceChannel1)); //FZ_Platform

	//DrawDebugLine(GetWorld(), Start, End, FColor::Emerald, false, 1, 0, 1);
	

	if (GetWorld()->LineTraceSingleByObjectType(OutHit, Start, End, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			if (GEngine) {
				SetNewPositionDepth(OutHit.ImpactPoint, FrwdVec);
				//DrawDebugPoint(GetWorld(), OutHit.ImpactPoint, 50, FColor::Red, false,1,0);
				//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Emerald, FString::Printf(TEXT("You are hitting: %s Location: %s"), *OutHit.GetActor()->GetName(), *OutHit.ImpactPoint.ToCompactString()));
				//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("ForwardVector: %s "), *FrwdVec.ToCompactString()));
				
				
			}
		}
	}
}


void AFEZ2DCharacter::SetNewPositionDepth(FVector & impactPoint, FVector & FrwdVec) 
{
	float axis_offset = 70.0;

	FVector location = GetActorLocation();
	if (FrwdVec.X > 0.1) {
		GetCapsuleComponent()->SetWorldLocation(FVector(impactPoint.X+axis_offset, location.Y, location.Z), false);
	}
	else if (FrwdVec.X < -0.1) {
		GetCapsuleComponent()->SetWorldLocation(FVector(impactPoint.X-axis_offset, location.Y, location.Z), false);
	}
	else if (FrwdVec.Y > 0.1) {
		GetCapsuleComponent()->SetWorldLocation(FVector(location.X, impactPoint.Y+axis_offset, location.Z), false);
	}
	else if (FrwdVec.Y < -0.1) {
		GetCapsuleComponent()->SetWorldLocation(FVector(location.X, impactPoint.Y-axis_offset, location.Z), false);
	}
	
}