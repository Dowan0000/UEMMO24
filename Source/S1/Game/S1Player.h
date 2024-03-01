// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Protocol.pb.h"
#include "S1Player.generated.h"

UCLASS()
class S1_API AS1Player : public ACharacter
{
	GENERATED_BODY()

public:
	AS1Player();
	virtual ~AS1Player();

protected:
	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds) override;

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:
	bool IsMyPlayer();

	Protocol::MoveState GetMoveState() { return PlayerInfo->state(); }
	void SetMoveState(Protocol::MoveState State);

public:
	void SetObjectInfo(const Protocol::ObjectInfo& Info);
	void SetPlayerInfo(const Protocol::PosInfo& Info);
	void SetDestInfo(const Protocol::PosInfo& Info);
	Protocol::PosInfo* GetPlayerInfo() { return PlayerInfo; }

protected:
	class Protocol::ObjectInfo* ObjectInfo;
	
	class Protocol::PosInfo* PlayerInfo; // ���� ��ġ
	class Protocol::PosInfo* DestInfo; // ������

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Health;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Damage;

};
