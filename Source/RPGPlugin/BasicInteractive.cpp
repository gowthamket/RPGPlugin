// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicInteractive.h"
#include "Components/BoxComponent.h"
#include "RPGPluginCharacter.h"

// Sets default values
ABasicInteractive::ABasicInteractive()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootScene;

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(RootComponent);

	Trigger->OnComponentBeginOverlap.AddUniqueDynamic(this, &ABasicInteractive::BeginOverlap);
	Trigger->OnComponentEndOverlap.AddUniqueDynamic(this, &ABasicInteractive::EndOverlap);

}

// Called when the game starts or when spawned
void ABasicInteractive::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABasicInteractive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABasicInteractive::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	PlayerCharacter = Cast<ARPGPluginCharacter>(OtherActor);

	if (PlayerCharacter != nullptr)
	{
		PlayerCharacter->OnEnterActor(this);

		OnPlayerBeginOverlap();
	}
}

void ABasicInteractive::OnPlayerBeginOverlap()
{

}


void ABasicInteractive::EndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (PlayerCharacter != nullptr)
	{
		PlayerCharacter->OnLeaveActor();

		OnPlayerEndOverlap();
	}
}

void ABasicInteractive::OnPlayerEndOverlap()
{

}




