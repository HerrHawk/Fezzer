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

	


	UPROPERTY()
	bool bCanCameraRotate = true;

	UFUNCTION()
	void CameraRight();

	UFUNCTION()
	void CameraLeft();

	UFUNCTION()
	void CameraRightStop();

	UFUNCTION()
	void CameraLeftStop();

	UFUNCTION()
	void CameraRotation(float DeltaSeconds);

	UPROPERTY()
	bool bCanFall = true;

	UFUNCTION()
	void Fall();

	UFUNCTION()
	void FallStop();

	UFUNCTION()
	void DepthCorrection();
	
	UFUNCTION()
	void SetNewPositionDepth(FVector & impactPoint, FVector & FrwdVec);

	UPROPERTY()
	float CameraRotationSpeed;
	
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

public:
	AFEZ2DCharacter();

	/** Returns SideViewCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetSideViewCameraComponent() const { return SideViewCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	
};
