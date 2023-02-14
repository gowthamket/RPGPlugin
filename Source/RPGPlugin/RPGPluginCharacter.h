// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DefaultWeapon.h"
#include "ItemData.h"
#include "Interactable.h"
#include "RPGPluginCharacter.generated.h"

UCLASS(config = Game)
class ARPGPluginCharacter : public ACharacter
{
	GENERATED_BODY()

		/** Camera boom positioning the camera behind the character */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USceneComponent* CarryItemPoint;


public:
	ARPGPluginCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input)
		float TurnRateGamepad;

	//public:
		//void StartLookAt(AActor* ActorTarget);

		//void StopLookAt();

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	UPROPERTY(BlueprintReadOnly, Category = "Player")
		class UComplexAnimInstance* Animator = nullptr;

	TArray<FQuestItem> QuestList;

	//Allows the character to start sprinting
	void Sprint();

	//Allows the character to stop sprinting
	void StopSprinting();

	//Allows the character to punch
	void Punch();

	UFUNCTION(BlueprintCallable)
		void TakeDamage(float _damageAmount);

	UFUNCTION(BlueprintCallable)
		void Heal(float _healAmount);

	//This function calls Heal()
	UFUNCTION(BlueprintCallable)
		void StartHealing();

	UFUNCTION(BlueprintCallable)
		void HealArmor(float _healAmount);

	//The character gains experience
	UFUNCTION(BlueprintCallable, Category = "Stats")
		void GainExperience(float _expAmount);

	UFUNCTION(BlueprintCallable)
		void ZoomIn();

	UFUNCTION(BlueprintCallable)
		void StopZoom();

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	//The amount of armor the character currently has
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
		float playerArmor;

protected:
	void BeginPlay();
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	void UpdateAndShowQuestList();

	//Determines when the character is sprinting
	bool isSprinting;

	//Determines if the character is currently zoomed into their weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		bool isZoomedIn;

	//The amount of health the character currently has
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
		float playerHealth;

	//Determines if the character has armor
	bool hasArmor;

	//The character's current experience points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float experiencePoints;

	//The total amount of experience points required to level up
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float experienceToLevel;

	//The character's current level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int currentLevel;

	//The amount of available upgrade points the character currently has
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int upgradePoints;

	//The amount of strength the character currently has
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int strengthValue;

	//The amount of dexterity the character currently has
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int dexterityValue;

	//The amount of intellect the character currently has
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		int intellectValue;

	//Determines if the character is currently zoomed in to their weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
		bool hasPunched;

	//The weapon the character is currently using
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		ADefaultWeapon* currentWeapon;

	//The attack speed our character currently has
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
		float attackSpeed;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	bool FindQuest(FName QuestID, FQuestItem& Quest);

	void AcceptQuest(FName QuestID);

	void MarkQuestCompleted(FName QuestID);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Events")
		void OnShowQuestInfo(FQuest Quest);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Events")
		void OnShowQuestCompleted(const FText& Message);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Events")
		void OnShowUpdatedQuestList(const TArray<FText>& QuestTextList);

protected:

	// Equipment inventory on UI and hands
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
		TArray<FItem> EquipmentInventory;

	// Maximun slots 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
		int32 TotalEquipmentSlots = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
		bool bHasItemOnHands = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
		int32 IndexItemOnHands;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
		FName ItemIDOnHands;

public:
	void AddItem(FName ItemID);

	void RemoveItem(FName ItemID, bool RemoveItemFromHands);

	bool HasFreeInventorySlots();

	bool HasItemOnHands(FName ItemID);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Events")
		void OnRefreshInventory();

public:

	void SwitchItem();

	void FreeHands();

	//// Interactives ///////

protected:

	AActor* CurrentInteractiveActor;

	IInteractable* CurrentInteractive;


public:

	void OnEnterActor(AActor* InteractiveActor);

	void OnLeaveActor();

	//// Interactives ///////

public:
	void StartLookAt(AActor* ActorTarget);

	void StopLookAt();

public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Events")
		void OnShowUI(FName Character);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Events")
		void OnHideUI();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Events")
		void OnSpawnItem(TSubclassOf<class AActor> ItemToGrab);


public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Game Events")
		void TriggerCheckPoint();

	void TriggerCheckPoint_Implementation();

};
