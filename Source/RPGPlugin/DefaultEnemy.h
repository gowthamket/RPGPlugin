// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DefaultEnemy.generated.h"

UCLASS()
class RPGPLUGIN_API ADefaultEnemy : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADefaultEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
		void TakeDamage(float _damage);

	//The current health of the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enemy)
		float health;

	//Has the enemy been hit?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enemy)
		bool hasTakenDamage;

	//Is the enemy dead?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Enemy)
		bool isDead;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
