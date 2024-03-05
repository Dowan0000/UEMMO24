// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/S1Monster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AS1Monster::AS1Monster() : 
	Health(100.f), MaxHealth(100.f), Damage(10.f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	GetCharacterMovement()->bRunPhysicsWithNoController = true;

	ObjectInfo = new Protocol::ObjectInfo();
	MonsterInfo = new Protocol::PosInfo();
	DestInfo = new Protocol::PosInfo();
}

AS1Monster::~AS1Monster()
{
	delete ObjectInfo;
	delete MonsterInfo;
	delete DestInfo;
	ObjectInfo = nullptr;
	MonsterInfo = nullptr;
	DestInfo = nullptr;
}

// Called when the game starts or when spawned
void AS1Monster::BeginPlay()
{
	Super::BeginPlay();

	{
		FVector Location = GetActorLocation();
		DestInfo->set_x(Location.X);
		DestInfo->set_y(Location.Y);
		DestInfo->set_z(Location.Z);
		DestInfo->set_yaw(GetControlRotation().Yaw);

		//SetMoveState(Protocol::MOVE_STATE_IDLE);
	}
}

// Called every frame
void AS1Monster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	{
		FVector Location = GetActorLocation();
		MonsterInfo->set_x(Location.X);
		MonsterInfo->set_y(Location.Y);
		MonsterInfo->set_z(Location.Z);
		MonsterInfo->set_yaw(GetControlRotation().Yaw);
	}

	FVector CurrentLocation = GetActorLocation();
	FVector DestLocation = { DestInfo->x(), DestInfo->y(), DestInfo->z() };

	if (50.f <= UKismetMathLibrary::Vector_Distance(CurrentLocation, DestLocation))
	{
		FRotator LookRotator = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, DestLocation);

		SetActorRotation(FRotator(0.f, LookRotator.Yaw, 0.f));
		AddMovementInput(GetActorForwardVector());
	}
	
	//FVector Location = GetActorLocation();
	//FVector DestLocation = FVector(DestInfo->x(), DestInfo->y(), DestInfo->z());

	//FVector MoveDir = (DestLocation - Location);
	//const float DistToDest = MoveDir.Length();
	//MoveDir.Normalize();

	//float MoveDist = (MoveDir * 500.f * DeltaTime).Length();
	//MoveDist = FMath::Min(MoveDist, DistToDest);
	//FVector NextLocation = Location + MoveDir * MoveDist;

	////SetActorLocation(NextLocation);

	//const Protocol::MoveState State = MonsterInfo->state();

	//if (State == Protocol::MOVE_STATE_RUN)
	//{
	//	MoveState = EMoveState::MSI_Run;
	//	SetActorRotation(FRotator(0, DestInfo->yaw(), 0));

	//	AddMovementInput(GetActorForwardVector());

	//	/*if (50.f <= (Location - DestLocation).Length())
	//		SetActorLocation(NextLocation);*/
	//	if (100.f <= (Location - DestLocation).Length() || TooFar)
	//	{
	//		TooFar = true;
	//		SetActorLocation(NextLocation);
	//		//UE_LOG(LogTemp, Warning, TEXT("TooFar"));
	//	}
	//	if ((Location - DestLocation).Length() <= 10.f)
	//	{
	//		TooFar = false;
	//		//UE_LOG(LogTemp, Warning, TEXT("Dist under 10.f"));
	//	}

	//}
	//else
	//{
	//	//MoveState = EMoveState::MSI_Idle;
	//}
}

float AS1Monster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	return AppliedDamage;
}

void AS1Monster::SetObjectInfo(const Protocol::ObjectInfo& Info)
{
	ObjectInfo->CopyFrom(Info);
}

void AS1Monster::SetMonsterInfo(const Protocol::PosInfo& Info)
{
	/*if (PlayerInfo->object_id() != 0)
	{
		assert(PlayerInfo->object_id() == Info.object_id());
	}*/

	//PlayerInfo->CopyFrom(Info);

	FVector Location(Info.x(), Info.y(), Info.z());
	SetActorLocation(Location);
}

void AS1Monster::SetDestInfo(const Protocol::PosInfo& Info)
{
	if (MonsterInfo->object_id() != 0)
	{
		assert(PlayerInfo->object_id() == Info.object_id());
	}

	// Dest에 최종 상태 복사.
	DestInfo->CopyFrom(Info);

	// 상태만 바로 적용하자.
	//SetMoveState(Info.state());
}

void AS1Monster::AttackAnim_Implementation()
{
}

void AS1Monster::DeadAnim_Implementation()
{
}
