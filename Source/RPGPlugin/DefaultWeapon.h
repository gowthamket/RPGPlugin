// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DefaultWeapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	E_Default		UMETA(DisplayName = "DEFAULT"),
	E_Sword			UMETA(DisplayName = "SWORD"),
	E_Dagger		UMETA(DisplayName = "DAGGER"),
	E_Axe			UMETA(DisplayName = "AXE")
};

UCLASS()
class RPGPLUGIN_API ADefaultWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADefaultWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// The level requirement to use the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		int levelReq;

	//The base damage of the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		float baseDamage;

	//The base speed of the weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		float baseSpeed;

	//The type of weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
		EWeaponType weaponType;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
