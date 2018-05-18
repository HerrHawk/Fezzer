// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "FEZ2DGameMode.h"
#include "FEZ2DCharacter.h"

AFEZ2DGameMode::AFEZ2DGameMode()
{
	// Set default pawn class to our character
	DefaultPawnClass = AFEZ2DCharacter::StaticClass();	
}
