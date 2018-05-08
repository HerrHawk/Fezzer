// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "FEZ1GameMode.h"
#include "FEZ1Character.h"
#include "UObject/ConstructorHelpers.h"

AFEZ1GameMode::AFEZ1GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SideScrollerCPP/Blueprints/SideScrollerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
