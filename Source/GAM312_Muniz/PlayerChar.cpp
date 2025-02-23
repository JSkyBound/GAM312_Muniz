// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerChar.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
APlayerChar::APlayerChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	//Creates camera component. Attaches to "head" to give a first person POV
	PlayerCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));

	PlayerCamComp->SetupAttachment(GetMesh(), "head");

	PlayerCamComp->bUsePawnControlRotation = true;

	BuildingArray.SetNum(4);
	ResourcesArray.SetNum(3);
	ResourcesNameArray.Add(TEXT("Wood"));
	ResourcesNameArray.Add(TEXT("Stone"));
	ResourcesNameArray.Add(TEXT("Berry"));
	ResourcesNameArray.Add(TEXT("Tree"));
	ResourcesNameArray.Add(TEXT("Rock"));
	ResourcesNameArray.Add(TEXT("Bush"));

	
}

// Called when the game starts or when spawned
void APlayerChar::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle StatsTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(StatsTimerHandle, this, &APlayerChar::DecreaseStats, 2.0f, true);

	if (objWidget)
	{
		objWidget->UpdateBuildObj(0.0f);
		objWidget->UpdateMatOBJ(0.0f);
	}
}

// Called every frame
void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	playerWidget->UpdateBars(Health, Hunger, Stamina);

	if (isBuilding)
	{
		if (spawnedPart)
		{
			FVector StartLocation = PlayerCamComp->GetComponentLocation();
			FVector Direction = PlayerCamComp->GetForwardVector() * 400.0f;
			FVector EndLocation = StartLocation + Direction;
			spawnedPart->SetActorLocation(EndLocation);
		}
	}
	 

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
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerChar::FindObject);
	PlayerInputComponent->BindAction("RotatePart", IE_Pressed, this, &APlayerChar::RotateBuilding);



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

//Defining what happens when you interact with a resource object
void APlayerChar::FindObject()
{
	FHitResult HitResult;
	FVector StartLocation = PlayerCamComp->GetComponentLocation();
	FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f;
	FVector EndLocation = StartLocation + Direction;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnFaceIndex = true;

	if (!isBuilding)
	{
		//This if-else checks if there is an object in the player's line of sight by using line tracing
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
		{
			AResource_M* HitResource = Cast<AResource_M>(HitResult.GetActor());

			//Checks if player has sufficient stamina to get resource
			if (Stamina > 5.0f)
			{
				//Checks the type of resource and destroys it once it is exhausted
				if (HitResource)
				{
					FString HitName = HitResource->resourceName;
					int resourceValue = HitResource->resourceAmount;

					HitResource->totalResource = HitResource->totalResource - resourceValue;

					if (HitResource->totalResource > resourceValue)
					{
						GiveResource(resourceValue, HitName);

						matsCollected = matsCollected + resourceValue;
						objWidget->UpdateMatOBJ(matsCollected);
						
						check(GEngine != nullptr);
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Resource Collected"));

						UGameplayStatics::SpawnDecalAtLocation(GetWorld(), hitDecal, FVector(10.0f, 10.0f, 10.0f), HitResult.Location, FRotator(-90, 0, 0), 2.0f);

						SetStamina(-5.0f);
					}
					else
					{
						HitResource->Destroy();
						check(GEngine != nullptr);
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Resource Depleted"));
				
					}
				}
			}
		
		}
	}

	else
	{
		isBuilding = false;
		objectsBuilt = objectsBuilt + 1.0f;
		objWidget->UpdateBuildObj(objectsBuilt);
	}
	
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
	if (Stamina + newStamina == 100)
	{
		Stamina = 100;
	}
	else if (Stamina + newStamina < 100)
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

//Allocates amount of each resource to a specific index in an array.
//Needs a better implementation where we can pull what item type it is alongside the amount!
//Idea: Dictionary? Two-dimensional array?
//Or just an array of objects? We did create the BPs in Unreal.
void APlayerChar::GiveResource(float amount, FString resourceType)
{
	if (resourceType == "Wood")
	{
		ResourcesArray[0] = ResourcesArray[0] + amount;
	}

	if (resourceType == "Stone")
	{
		ResourcesArray[1] = ResourcesArray[1] + amount;
	}

	if (resourceType == "Berry")
	{
		ResourcesArray[2] = ResourcesArray[2] + amount;
	}

	if (resourceType == "Tree")
	{
		ResourcesArray[0] = ResourcesArray[0] + amount;
	}

	if (resourceType == "Rock")
	{
		ResourcesArray[1] = ResourcesArray[1] + amount;
	}

	if (resourceType == "Bush")
	{
		ResourcesArray[2] = ResourcesArray[2] + amount;
	}
}

//Uses up the corresponding resources being used and allocates the building object to its corresponding array index
void APlayerChar::UpdateResource(float woodAmount, float stoneAmount, FString buildingObject)
{
	if (woodAmount <= ResourcesArray[0])
	{
		ResourcesArray[0] = ResourcesArray[0] - woodAmount;
		ResourcesArray[1] = ResourcesArray[1] - stoneAmount;

		if (buildingObject == "Wall")
		{
			BuildingArray[0] = BuildingArray[0] + 1;
		}

		if (buildingObject == "Floor")
		{
			BuildingArray[1] = BuildingArray[1] + 1;
		}

		if (buildingObject == "Ceiling")
		{
			BuildingArray[2] = BuildingArray[2] + 1;
		}

		if (buildingObject == "Shelter")
		{
			BuildingArray[3] = BuildingArray[3] + 1;
		}
	}
	
}

//Spawns the corresponding BuildPartClass object if it is a building
void APlayerChar::SpawnBuilding(int buildingID, bool& isSuccess)
{
	if (!isBuilding)
	{
		if (BuildingArray[buildingID] >= 1)
		{
			isBuilding = true;
			FActorSpawnParameters SpawnParams;
			FVector StartLocation = PlayerCamComp->GetComponentLocation();
			FVector Direction = PlayerCamComp->GetForwardVector() * 400.0f;
			FVector EndLocation = StartLocation + Direction;
			FRotator myRot(0, 0, 0	);

			BuildingArray[buildingID] = BuildingArray[buildingID] - 1;

			spawnedPart = GetWorld()->SpawnActor<ABuildingPart>(BuildPartClass, EndLocation, myRot, SpawnParams);

			isSuccess = true;
		}

		isSuccess = false;
	}
}

//Allows player to rotate the building in 90 degree increments.
void APlayerChar::RotateBuilding()
{
	if (isBuilding)
	{
		spawnedPart->AddActorWorldRotation(FRotator(0, 90, 0));
	}
}

int APlayerChar::GetWood()
{
	return this->Wood;
}

int APlayerChar::GetStone()
{
	return this->Stone;
}

void APlayerChar::AddResource_Debug(float woodAmount, float stoneAmount)
{
	ResourcesArray[0] = ResourcesArray[0] + woodAmount;
	ResourcesArray[1] = ResourcesArray[1] + stoneAmount;
}

