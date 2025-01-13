// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerChar.h"

// Sets default values
APlayerChar::APlayerChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	//Creates camera component. Attaches to "head" to give a first person POV
	PlayerCanComp = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));

	PlayerCanComp->SetupAttachment(GetMesh(), "head");

	PlayerCanComp->bUsePawnControlRotation = true;

	

}

// Called when the game starts or when spawned
void APlayerChar::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle StatsTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(StatsTimerHandle, this, &APlayerChar::DecreaseStats, 2.0f, true);
	
}

// Called every frame
void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//This binds the input axis we set up to the functions corresponding to each one.
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerChar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerChar::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerChar::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APlayerChar::AddControllerYawInput);

	PlayerInputComponent->BindAction("JumpEvent", IE_Pressed, this, &APlayerChar::StartJump);
	PlayerInputComponent->BindAction("JumpEvent", IE_Released, this, &APlayerChar::StopJump);


}

void APlayerChar::MoveForward(float axisValue)
{
//Gets the vector using the current rotation on the X axis
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);
	AddMovementInput(Direction, axisValue);
	
}

void APlayerChar::MoveRight(float axisValue)
{
	//Gets the vector using the current rotation on the Y axis
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::StartJump()
{
	bPressedJump = true;
}

void APlayerChar::StopJump()
{
	bPressedJump = false;
}

void APlayerChar::FindObject()
{
	//not implemented
}

void APlayerChar::SetHealth(float newHealth)
{
	if (Health + newHealth < 100)
	{
		Health = Health + newHealth;
	}
}

void APlayerChar::SetHunger(float newHunger)
{
	if (Hunger + newHunger < 100)
	{
		Hunger = Hunger + newHunger;
	}
}

void APlayerChar::SetStamina(float newStamina)
{
	if (Stamina + newStamina < 100)
	{
		Stamina = Stamina + newStamina;
	}
}

void APlayerChar::DecreaseStats()
{
	if (Hunger > 0)
	{
		SetHunger(-1.0f);
	}
	
	SetStamina(10.0f);

	if (Hunger <= 0)
	{
		SetHealth(-3.0f);
	}
}

