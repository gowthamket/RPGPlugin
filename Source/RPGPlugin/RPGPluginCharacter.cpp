// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGPluginCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "ComplexAnimInstance.h"
#include "RPGPluginGameMode.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RPGPluginGameInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include <Runtime/Engine/Classes/Kismet/GameplayStatics.h>

//////////////////////////////////////////////////////////////////////////
// ARPGPluginCharacter

ARPGPluginCharacter::ARPGPluginCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	CarryItemPoint = CreateDefaultSubobject<USceneComponent>(TEXT("CarryItemPoint"));
	CarryItemPoint->SetupAttachment(RootComponent);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	isSprinting = false;
	hasArmor = true;
	hasPunched = false;
	playerHealth = 1.00f;
	playerArmor = 1.00f;

	currentLevel = 1;
	upgradePoints = 5;

	strengthValue = 1;
	dexterityValue = 1;
	intellectValue = 1;

	experiencePoints = 0.0f;
	experienceToLevel = 2000.0f;

	attackSpeed = 1.0f;

	isZoomedIn = false;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARPGPluginCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetMesh() != nullptr)
	{
		Animator = Cast<UComplexAnimInstance>(GetMesh()->GetAnimInstance());
	}


	OnRefreshInventory();


	QuestList.Empty();
	// Load game
	URPGPluginGameInstance* GameInstance = Cast<URPGPluginGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	if ((GameInstance != nullptr) && (GameInstance->LoadGame()))
	{
		if (GameInstance->CurrentSaveGame != nullptr)
		{
			// Retrieve the quest list
			QuestList = GameInstance->CurrentSaveGame->QuestStatus;

			UpdateAndShowQuestList();
		}
	}

}

void ARPGPluginCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ARPGPluginCharacter::Sprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ARPGPluginCharacter::StopSprinting);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ARPGPluginCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ARPGPluginCharacter::MoveRight);

	PlayerInputComponent->BindAction("Punch", IE_Pressed, this, &ARPGPluginCharacter::Punch);

	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ARPGPluginCharacter::ZoomIn);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ARPGPluginCharacter::StopZoom);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ARPGPluginCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ARPGPluginCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ARPGPluginCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ARPGPluginCharacter::TouchStopped);
}

void ARPGPluginCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ARPGPluginCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ARPGPluginCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ARPGPluginCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ARPGPluginCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ARPGPluginCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ARPGPluginCharacter::Sprint()
{
	UE_LOG(LogTemp, Warning, TEXT("We have started sprinting."));
	isSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = 1500.0f;
}

void ARPGPluginCharacter::StopSprinting()
{
	UE_LOG(LogTemp, Warning, TEXT("We have stopped sprinting"));
	isSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
}

void ARPGPluginCharacter::ZoomIn()
{
	if (auto thirdPersonCamera = GetCameraBoom())
	{
		thirdPersonCamera->TargetArmLength = 150.0f;
		thirdPersonCamera->TargetOffset = FVector(0.0f, 0.0f, 70.0f);

		if (auto characterMovement = GetCharacterMovement())
		{
			characterMovement->MaxWalkSpeed = 300.0f;
		}

		isZoomedIn = false;
	}
}

void ARPGPluginCharacter::StopZoom()
{
	if (auto thirdPersonCamera = GetCameraBoom())
	{
		thirdPersonCamera->TargetArmLength = 300.0f;
		thirdPersonCamera->TargetOffset = FVector(0.0f, 0.0f, 0.0f);

		if (auto characterMovement = GetCharacterMovement())
		{
			characterMovement->MaxWalkSpeed = 600.0f;
		}

		isZoomedIn = true;
	}
}

void ARPGPluginCharacter::TakeDamage(float _damageAmount)
{
	UE_LOG(LogTemp, Warning, TEXT("We are taking damage for %f points."), _damageAmount);

	if (hasArmor)
	{
		playerArmor -= _damageAmount;

		if (playerArmor < 0.00f)
		{
			hasArmor = false;
			playerHealth += playerArmor;
			playerArmor = 0.00f;
		}
	}
	else
	{
		playerHealth -= _damageAmount;

		if (playerHealth < 0.00f)
		{
			playerHealth = 0.00f;
		}
	}
}

void ARPGPluginCharacter::StartHealing()
{
	Heal(0.02f);
}

void ARPGPluginCharacter::Heal(float _healAmount)
{
	UE_LOG(LogTemp, Warning, TEXT("We are healing for %f points."), _healAmount);
	playerHealth += _healAmount;

	if (playerHealth > 1.00f)
	{
		playerHealth = 1.00f;
	}
}

void ARPGPluginCharacter::HealArmor(float _healAmount)
{
	playerArmor += _healAmount;
	hasArmor = true;

	if (playerArmor > 1.00f)
	{
		playerArmor = 1.00f;
	}
}

void ARPGPluginCharacter::GainExperience(float _expAmount)
{
	experiencePoints += _expAmount;

	if (experiencePoints >= experienceToLevel)
	{
		++currentLevel;

		experiencePoints -= experienceToLevel;
		experiencePoints += 500.0f;
	}
}

void ARPGPluginCharacter::Punch()
{
	hasPunched = true;
}

void ARPGPluginCharacter::OnEnterActor(AActor* InteractiveActor)
{
	if (InteractiveActor != nullptr)
	{
		bool IsInterface = UKismetSystemLibrary::DoesImplementInterface(InteractiveActor, UInteractable::StaticClass());
		if (IsInterface)
		{
			CurrentInteractiveActor = InteractiveActor;

			CurrentInteractive = Cast<IInteractable>(InteractiveActor);
		}
	}
}


void ARPGPluginCharacter::OnLeaveActor()
{
	CurrentInteractive = nullptr;
	CurrentInteractiveActor = nullptr;
}

//void ANewRPGCharacter::Action()
//{
	//if (CurrentInteractive != nullptr)
	//{
		// Execute the interact event
		//IInteractable::Execute_OnInteract(CurrentInteractiveActor);
	//}
//}

bool ARPGPluginCharacter::FindQuest(FName QuestID, FQuestItem& Quest)
{
	for (int i = 0; i < QuestList.Num(); i++)
	{
		if (QuestList[i].QuestID == QuestID)
		{
			Quest = QuestList[i];
			return true;
		}
	}
	return false;
}


void ARPGPluginCharacter::AcceptQuest(FName QuestID)
{
	bool QuestFound = false;
	for (int i = 0; i < QuestList.Num(); i++)
	{
		if (QuestList[i].QuestID == QuestID)
		{
			QuestFound = true;
			break;
		}
	}

	if (!QuestFound)
	{
		FQuestItem NewQuest;
		NewQuest.QuestID = QuestID;
		NewQuest.IsCompleted = false;
		QuestList.Add(NewQuest);

		UpdateAndShowQuestList();
	}
}

void ARPGPluginCharacter::MarkQuestCompleted(FName QuestID)
{
	for (int i = 0; i < QuestList.Num(); i++)
	{
		if ((QuestList[i].QuestID == QuestID) && (!QuestList[i].IsCompleted))
		{
			QuestList[i].IsCompleted = true;
			break;
		}
	}

	UpdateAndShowQuestList();
}

void ARPGPluginCharacter::UpdateAndShowQuestList()
{
	// Prepare list of quest, to show on the UI
	ARPGPluginGameMode* GameMode = Cast<ARPGPluginGameMode>(GetWorld()->GetAuthGameMode());

	if (GameMode != nullptr)
	{
		TArray<FText> QuestTextList;
		for (int i = 0; i < QuestList.Num(); i++)
		{
			if (!QuestList[i].IsCompleted)
			{
				bool Success = false;
				// Find the quest on the game mode
				FQuest Quest = GameMode->FindQuest(QuestList[i].QuestID, Success);

				if (Success)
				{
					QuestTextList.Add(Quest.SortDescription);
				}
			}
		}

		OnShowUpdatedQuestList(QuestTextList);
	}
}

//void ANewRPGCharacter::StartLookAt(AActor* ActorTarget)
//{
	//if (Animator != nullptr)
	//{
		//Animator->StartLookAtActor(ActorTarget, GetMesh());
	//}
//}

//void ANewRPGCharacter::StopLookAt()
//{
	//if (Animator != nullptr)
	//{
		//Animator->StopLookAt();
	//}
//}


void ARPGPluginCharacter::TriggerCheckPoint_Implementation()
{
	// Save current game
	URPGPluginGameInstance* GameInstance = Cast<URPGPluginGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	if ((GameInstance != nullptr) && (GameInstance->CurrentSaveGame != nullptr))
	{
		GameInstance->CurrentSaveGame->QuestStatus.Empty();
		GameInstance->CurrentSaveGame->QuestStatus = QuestList;

		if (GameInstance->SaveGame())
		{
			UE_LOG(LogTemp, Warning, TEXT("[AHowToCharacter::TriggerCheckPoint] Success saving game"));

			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[AHowToCharacter::TriggerCheckPoint] Fail saving game"));
}



//// Inventory ///////

void ARPGPluginCharacter::AddItem(FName ItemID)
{
	// Find the item on the inventory
	for (int i = 0; i < EquipmentInventory.Num(); i++)
	{
		if (EquipmentInventory[i].ItemID == ItemID)
		{
			EquipmentInventory[i].Quantity += 1;

			if (!bHasItemOnHands)
			{
				IndexItemOnHands = i;
				ItemIDOnHands = ItemID;
				bHasItemOnHands = true;

				EquipmentInventory[i].SpawnedItem->SetActorHiddenInGame(false);

				Animator->SetAlphaRightArm(true);
				Animator->SetAlphaLeftArm(true);
			}

			OnRefreshInventory();
			return;
		}
	}

	if (EquipmentInventory.Num() == TotalEquipmentSlots)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AHowToCharacter::AddItem] EquipmentInventory full: %d / %d "), EquipmentInventory.Num(), TotalEquipmentSlots);
		return;
	}

	// Find the item on the table on the game mode to get the information	
	ARPGPluginGameMode* GameMode = Cast<ARPGPluginGameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode != nullptr)
	{
		bool Found = false;
		FItem ItemFound = GameMode->FindItem(ItemID, Found);

		if (Found && (ItemFound.ItemActor != nullptr))
		{
			AActor* SpawnItem = GetWorld()->SpawnActor<AActor>(ItemFound.ItemActor, FVector::ZeroVector, FRotator::ZeroRotator);

			// Spawn the item and add it to the list of elements
			if (SpawnItem != nullptr)
			{
				FItem NewItem;
				NewItem.ItemID = ItemID;
				NewItem.Name = ItemFound.Name;
				NewItem.Description = ItemFound.Description;
				NewItem.Quantity = 1;
				NewItem.ItemIcon = ItemFound.ItemIcon;
				NewItem.SpawnedItem = SpawnItem;

				SpawnItem->AttachToComponent(CarryItemPoint, FAttachmentTransformRules::KeepWorldTransform);
				SpawnItem->SetActorLocation(CarryItemPoint->GetComponentLocation());
				SpawnItem->SetActorRotation(CarryItemPoint->GetComponentRotation());

				EquipmentInventory.Add(NewItem);

				// Nothing on hands, we add this item on hands
				if (!bHasItemOnHands)
				{
					Animator->SetAlphaRightArm(true);
					Animator->SetAlphaLeftArm(true);

					// THe index is the last one on the list
					IndexItemOnHands = EquipmentInventory.Num() - 1;
					ItemIDOnHands = ItemID;
					bHasItemOnHands = true;
					SpawnItem->SetActorHiddenInGame(false);
				}
				else
				{
					// Hide item if exists
					SpawnItem->SetActorHiddenInGame(true);
				}
			}
		}
	}

	OnRefreshInventory();
}


void ARPGPluginCharacter::RemoveItem(FName ItemID, bool RemoveItemFromHands)
{
	// Find the item on the inventory
	int32 ItemIndexToRemove = -1;
	for (int i = 0; i < EquipmentInventory.Num(); i++)
	{
		if (EquipmentInventory[i].ItemID == ItemID)
		{
			EquipmentInventory[i].Quantity -= 1;

			if (EquipmentInventory[i].Quantity <= 0) // No more units
			{
				ItemIndexToRemove = i;

				// Remove item from hands
				if (bHasItemOnHands && (ItemIDOnHands == ItemID))
				{
					IndexItemOnHands = -1;
					bHasItemOnHands = false;
					ItemIDOnHands = "";

					Animator->SetAlphaRightArm(false);
					Animator->SetAlphaLeftArm(false);
				}
			}
			else if (RemoveItemFromHands) // If we hare more units, we hide it from hands
			{
				FreeHands();
			}

			break;
		}
	}

	// If we remove all units from the item, we remove it from the list
	if (ItemIndexToRemove >= 0)
	{
		EquipmentInventory[ItemIndexToRemove].SpawnedItem->Destroy();
		EquipmentInventory.RemoveAt(ItemIndexToRemove);
	}

	OnRefreshInventory();

}

bool ARPGPluginCharacter::HasFreeInventorySlots()
{
	return (EquipmentInventory.Num() < TotalEquipmentSlots);
}

bool ARPGPluginCharacter::HasItemOnHands(FName ItemID)
{
	if (bHasItemOnHands)
	{
		return (ItemID == ItemIDOnHands);
	}

	return false;
}


void ARPGPluginCharacter::SwitchItem()
{
	if (EquipmentInventory.Num() == 0) return;

	if (!bHasItemOnHands) // No items on hands
	{
		IndexItemOnHands = 0;
		EquipmentInventory[IndexItemOnHands].SpawnedItem->SetActorHiddenInGame(false);

		bHasItemOnHands = true;
		ItemIDOnHands = EquipmentInventory[IndexItemOnHands].ItemID;

		Animator->SetAlphaRightArm(true);
		Animator->SetAlphaLeftArm(true);

		return;
	}

	// Items on hands, we switch we have at least 1
	if (EquipmentInventory.Num() > 1)
	{
		// Hide the current one
		EquipmentInventory[IndexItemOnHands].SpawnedItem->SetActorHiddenInGame(true);

		IndexItemOnHands += 1;
		if (IndexItemOnHands >= EquipmentInventory.Num()) // Last item on the inventory
		{
			IndexItemOnHands = 0;
		}
		EquipmentInventory[IndexItemOnHands].SpawnedItem->SetActorHiddenInGame(false);
		ItemIDOnHands = EquipmentInventory[IndexItemOnHands].ItemID;
	}
}

void ARPGPluginCharacter::FreeHands()
{
	if (bHasItemOnHands && (IndexItemOnHands >= 0) && (IndexItemOnHands < EquipmentInventory.Num()))
	{
		EquipmentInventory[IndexItemOnHands].SpawnedItem->SetActorHiddenInGame(true);
	}

	IndexItemOnHands = -1;
	bHasItemOnHands = false;
	ItemIDOnHands = "";

	Animator->SetAlphaRightArm(false);
	Animator->SetAlphaLeftArm(false);
}

//// Inventory ///////


