// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Protocol.pb.h"
#include "S1Monster.generated.h"


class Protocol::ObjectInfo;
class Protocol::PosInfo;

UCLASS()
class S1_API AS1Monster : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AS1Monster();
	virtual ~AS1Monster();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:
	// set
	void SetObjectInfo(const Protocol::ObjectInfo& Info);
	void SetMonsterInfo(const Protocol::PosInfo& Info);

	void SetDestInfo(const Protocol::PosInfo& Info);

	void SetHealth(float newHealth) { Health = newHealth; };

	// get
	Protocol::ObjectInfo* GetObjectInfo() { return ObjectInfo; };


public:
	UFUNCTION(BlueprintNativeEvent)
	void DeadAnim();
	void DeadAnim_Implementation();

protected:
	Protocol::ObjectInfo* ObjectInfo;

	Protocol::PosInfo* MonsterInfo; // 현재 위치
	Protocol::PosInfo* DestInfo; // 목적지

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Health;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Damage;

};
