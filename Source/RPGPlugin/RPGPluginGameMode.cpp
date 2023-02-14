// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGPluginGameMode.h"
#include "RPGPluginCharacter.h"
#include "RPGPluginGameInstance.h"
#include "UObject/ConstructorHelpers.h"
//#include "UObject/ConstructorHelpers.h"

ARPGPluginGameMode::ARPGPluginGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}


}

void ARPGPluginGameMode::BeginPlay()
{
	Super::BeginPlay();
}




FItem ARPGPluginGameMode::FindItem_Implementation(FName ItemID, bool& Success)
{
	Success = false;

	FItem Item;
	if (ItemDatabase == nullptr) { return Item; }

	for (int i = 0; i < ItemDatabase->Data.Num(); i++)
	{
		if (ItemDatabase->Data[i].ItemID == ItemID)
		{
			Success = true;
			return ItemDatabase->Data[i];

		}
	}

	return Item;
}


FQuest ARPGPluginGameMode::FindQuest_Implementation(FName QuestID, bool& Success)
{
	Success = false;

	FQuest Quest;
	if (QuestDatabase == nullptr) { return Quest; }

	for (int i = 0; i < QuestDatabase->QuestData.Num(); i++)
	{
		if (QuestDatabase->QuestData[i].QuestID == QuestID)
		{
			Success = true;
			return QuestDatabase->QuestData[i];

		}
	}

	return Quest;
}

