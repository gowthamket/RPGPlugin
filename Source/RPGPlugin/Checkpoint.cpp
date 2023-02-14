// Fill out your copyright notice in the Description page of Project Settings.


#include "Checkpoint.h"
#include "RPGPluginCharacter.h"


void ACheckpoint::OnInteract_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("OnInteract Checkpoint"));

	if (PlayerCharacter != nullptr)
	{
		PlayerCharacter->TriggerCheckPoint();
	}
}



FName ACheckpoint::GetName()
{
	return "";
}

FName ACheckpoint::GetQuestID()
{
	return "";
}




