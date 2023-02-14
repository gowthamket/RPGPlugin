// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ItemData.h"
#include "RPGPluginGameMode.generated.h"

UCLASS(minimalapi)
class ARPGPluginGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARPGPluginGameMode();

protected:

	virtual void BeginPlay() override;


protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest System")
		class UQuestData* QuestDatabase;


public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Quest System")
		FQuest FindQuest(FName QuestID, bool& Success);

	FQuest FindQuest_Implementation(FName QuestID, bool& Success);


	///////////////////////// Inventory ////////////////////////////////
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item System")
		class UItemData* ItemDatabase;

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Item System")
		FItem FindItem(FName ItemID, bool& Success);

	FItem FindItem_Implementation(FName ItemID, bool& Success);


	///////////////////////// Inventory ////////////////////////////////
};







