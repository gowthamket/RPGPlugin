// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "BasicInteractive.generated.h"

UCLASS()
class RPGPLUGIN_API ABasicInteractive : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABasicInteractive();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactive")
		class USceneComponent* RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interactive")
		class UBoxComponent* Trigger;

	class ARPGPluginCharacter* PlayerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive")
		FName InteractiveName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive")
		FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interactive")
		bool QuestActivated;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UFUNCTION()
		virtual void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);

	virtual void OnPlayerBeginOverlap();

	UFUNCTION()
		virtual void EndOverlap(UPrimitiveComponent* OverlappedComp,
			AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void OnPlayerEndOverlap();

};
