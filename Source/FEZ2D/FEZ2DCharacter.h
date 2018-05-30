// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "FEZ2DCharacter.generated.h"


class UTextRenderComponent;

/**
 * This class is the default character for FEZ2D, and it is responsible for all
 * physical interaction between the player and the world.
 *
 * The capsule component (inherited from ACharacter) handles collision with the world
 * The CharacterMovementComponent (inherited from ACharacter) handles movement of the collision capsule
 * The Sprite component (inherited from APaperCharacter) handles the visuals
 */
UCLASS(config=Game)
class AFEZ2DCharacter : public APaperCharacter
{
	GENERATED_BODY()

	/** Side view camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera, meta=(AllowPrivateAccess="true"))
	class UCameraComponent* SideViewCameraComponent;

	/** Camera boom positioning the camera beside the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;


	UTextRenderComponent* TextComponent;
	virtual void Tick(float DeltaSeconds) override;

protected:
	// The animation to play while running around
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Animations)
	class UPaperFlipbook* RunningAnimation;

	// The animation to play while idle (standing still)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	class UPaperFlipbook* IdleAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UPaperFlipbook* ClimbAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UPaperFlipbook* DieAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UPaperFlipbook* JumpAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
		class UPaperFlipbook* TurnAnimation;

	/** Called to choose the correct animation to play based on the character's movement state */
	void UpdateAnimation();

	/** Called for side to side input */
	void MoveRight(float Value);

	void Falling();

	

	/** Is set to false when the camera rotates, so the rotate buttons can't be spammed. Is set to true when the rotaion finishes. */
	UPROPERTY()
	bool bCanCameraRotate = true;

	/** If bCanCameraRotate is allowd, starts rotating the capsule component to the right, therefore it looks like the camera rotates to the right. Also freezes the location, velocity and sprite animation. */
	UFUNCTION()
	void CameraRight();

	/** If bCanCameraRotate is allowd, starts rotating the capsule component to the left, therefore it looks like the camera rotates to the left. Also freezes the location, velocity and sprite animation. */
	UFUNCTION()
	void CameraLeft();

	/** Smoothly rotates the capsule component. If finished, sets bCanCameraRotate to true and unfreezes location, velocity and sprite animation. */
	UFUNCTION()
	void CameraRotation(float DeltaSeconds);

	/** Checks if the depth location of the character needs to be corrected. */
	UFUNCTION()
	void DepthCorrection();
	
	/** Corrects the depth location of the character. */
	UFUNCTION()
	void SetNewPositionDepth(FVector & impactPoint, FVector & FrwdVec);

	/** */
	UPROPERTY()
	float CameraRotationSpeed;
	
	UPROPERTY()
	FRotator CapsuleRotationDifference;

	UPROPERTY()
	FRotator NewCapsuleRotation;
	
	UPROPERTY()
	FVector FreezeLocation;
	
	UPROPERTY()
	FVector FreezeVelocity;

	UFUNCTION()
	void UpdateCharacter();


	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface


	float starttime;
	float endtime;



public:
	AFEZ2DCharacter();

	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	
};
