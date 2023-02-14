// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameFramework/SaveGame.h"
#include "ItemData.h"
#include "RPGPluginGameInstance.generated.h"

UCLASS()
class RPGPLUGIN_API UMainSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString SaveGameName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FDateTime CreationTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FQuestItem> QuestStatus;


	void CreateSlot(const FString& SlotName)
	{
		SaveGameName = SlotName;
		CreationTime = FDateTime::Now();
		QuestStatus.Empty();
	};

};

UCLASS()
class RPGPLUGIN_API URPGPluginGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:

	static const FString UNIQUE_SAVE_SLOT;

public:

	UPROPERTY()
		UMainSaveGame* CurrentSaveGame;

public:

	bool IsNewGame();

	bool CreateNewSaveGame();

public:

	bool LoadGame();

	bool SaveGame();


};

