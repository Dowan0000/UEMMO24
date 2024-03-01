// Fill out your copyright notice in the Description page of Project Settings.


#include "S1GameInstance.h"
#include "Sockets.h"
#include "Common/TcpSocketBuilder.h"
#include "Serialization/ArrayWriter.h"
#include "SocketSubsystem.h"
#include "PacketSession.h"
#include "Protocol.pb.h"
#include "ClientPacketHandler.h"
#include "S1MyPlayer.h"
#include "S1Monster.h"

void US1GameInstance::ConnectToGameServer()
{
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(TEXT("Stream"), TEXT("Client Socket"));
	
	FIPv4Address Ip;
	FIPv4Address::Parse(IpAddress, Ip);

	TSharedRef<FInternetAddr> InternetAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	InternetAddr->SetIp(Ip.Value);
	InternetAddr->SetPort(Port);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Connecting To Server...")));

	bool Connected = Socket->Connect(*InternetAddr);

	if (Connected)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Connection Success")));

		// Session
		GameServerSession = MakeShared<PacketSession>(Socket);
		GameServerSession->Run();

		// TEMP : Lobby에서 캐릭터 선택창 등
		{
			Protocol::C_LOGIN Pkt;
			SendBufferRef SendBuffer = ClientPacketHandler::MakeSendBuffer(Pkt);
			SendPacket(SendBuffer);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Connection Failed")));
	}
}

void US1GameInstance::DisconnectFromGameServer()
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	Protocol::C_LEAVE_GAME LeavePkt;
	SEND_PACKET(LeavePkt);

	/*if (Socket)
	{
		ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get();
		SocketSubsystem->DestroySocket(Socket);
		Socket = nullptr;
	}*/
}

void US1GameInstance::HandleRecvPackets()
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	GameServerSession->HandleRecvPackets();
}

void US1GameInstance::SendPacket(SendBufferRef SendBuffer)
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	GameServerSession->SendPacket(SendBuffer);
}

void US1GameInstance::HandleSpawn(const Protocol::ObjectInfo& ObjectInfo, bool IsMine)
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	auto* World = GetWorld();
	if (World == nullptr)
		return;

	// 중복 처리 체크
	const uint64 ObjectId = ObjectInfo.object_id();
	if (Players.Find(ObjectId) != nullptr)
		return;

	FVector SpawnLocation(ObjectInfo.pos_info().x(), ObjectInfo.pos_info().y(), ObjectInfo.pos_info().z());

	if (IsMine)
	{
		auto* PC = UGameplayStatics::GetPlayerController(this, 0);
		AS1Player* Player = Cast<AS1Player>(PC->GetPawn());
		if (Player == nullptr)
			return;

		Player->SetObjectInfo(ObjectInfo);
		Player->SetPlayerInfo(ObjectInfo.pos_info());
		MyPlayer = Player;
		Players.Add(ObjectInfo.object_id(), Player);
	}
	else
	{
		AS1Player* Player = Cast<AS1Player>(World->SpawnActor(OtherPlayerClass, &SpawnLocation));
		Player->SetObjectInfo(ObjectInfo);
		Player->SetPlayerInfo(ObjectInfo.pos_info());
		Players.Add(ObjectInfo.object_id(), Player);
	}
}

void US1GameInstance::HandleSpawn(const Protocol::S_ENTER_GAME& EnterGamePkt)
{
	HandleSpawn(EnterGamePkt.player(), true);
}

void US1GameInstance::HandleSpawn(const Protocol::S_SPAWN& SpawnPkt)
{
	for (auto& Object : SpawnPkt.objects())
	{
		switch (Object.object_type())
		{
		case ObjectType::CREATURE_TYPE_PLAYER:
			HandleSpawn(Object, false);
		case ObjectType::CREATURE_TYPE_MONSTER:
			HandleSpawn_Mon(Object);
		}
	}
}

void US1GameInstance::HandleSpawn_Mon(const Protocol::ObjectInfo& MonsterInfo)
{
	auto* World = GetWorld();
	if (World == nullptr)
		return;

	const uint64 ObjectId = MonsterInfo.object_id();
	if (Players.Find(ObjectId) != nullptr)
		return;

	FVector SpawnLocation(MonsterInfo.pos_info().x(), MonsterInfo.pos_info().y(), MonsterInfo.pos_info().z());
	
	AS1Monster* Monster = Cast<AS1Monster>(World->SpawnActor(MonstersClass, &SpawnLocation));
	Monster->SetObjectInfo(MonsterInfo);
	Monster->SetMonsterInfo(MonsterInfo.pos_info());
	Monsters.Add(MonsterInfo.object_id(), Monster);

}

void US1GameInstance::HandleDespawn(uint64 ObjectId)
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	auto* World = GetWorld();
	if (World == nullptr)
		return;

	AS1Player** FindActor = Players.Find(ObjectId);
	if (FindActor == nullptr)
		return;

	World->DestroyActor(*FindActor);
}

void US1GameInstance::HandleDespawn(const Protocol::S_DESPAWN& DespawnPkt)
{
	for (auto& ObjectId : DespawnPkt.object_ids())
	{
		HandleDespawn(ObjectId);
	}
}

void US1GameInstance::HandleMove(const Protocol::S_MOVE& MovePkt)
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	auto* World = GetWorld();
	if (World == nullptr)
		return;

	// 플레이어 이동
	const uint64 ObjectId = MovePkt.info().object_id();
	AS1Player** FindActor = Players.Find(ObjectId);
	if (FindActor)
	{
		AS1Player* Player = *FindActor;
		if (Player->IsMyPlayer())
			return;

		const Protocol::PosInfo& Info = MovePkt.info();
		//Player->SetPlayerInfo(Info);
		Player->SetDestInfo(Info);
	}


	// 몬스터 이동
	AS1Monster** FindMonster = Monsters.Find(ObjectId);
	if (FindMonster)
	{
		AS1Monster* Monster = *FindMonster;
		if (Monster == nullptr)
			return;

		const Protocol::PosInfo& Info = MovePkt.info();
		
		if(Info.state() == Protocol::MoveState::MOVE_STATE_RUN)
			Monster->SetDestInfo(Info);
		else if (Info.state() == Protocol::MoveState::MOVE_STATE_SKILL)
		{
			//Monster->Jump();
		}
	}

}

void US1GameInstance::HandleAttack(const Protocol::S_ATTACK& AttackPkt)
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	auto* World = GetWorld();
	if (World == nullptr)
		return;

	const uint64 ObjectId = AttackPkt.info().object_id();
	AS1Player** FindPlayer = Players.Find(ObjectId);
	AS1Player* Player = *FindPlayer;
	if (!Player->IsMyPlayer())
		Player->AttackAnim();
	
	bool hit = AttackPkt.hit();

	if (hit)
	{
		// AttackPkt에서 몬스터 id와 rest_hp를 가져와서 적용
		AS1Monster* monster = *Monsters.Find(AttackPkt.target());
		monster->SetHealth(AttackPkt.rest_hp());

		UE_LOG(LogTemp, Warning, TEXT("MonsterID : %d, Rest_HP : %f"), AttackPkt.target(), AttackPkt.rest_hp());
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("MonsterID : %d, Rest_HP : %f"), AttackPkt.target(), AttackPkt.rest_hp()));
	}
	
}

void US1GameInstance::HandleDead(const Protocol::S_DEAD& DeadPkt)
{
	if (Socket == nullptr || GameServerSession == nullptr)
		return;

	auto* World = GetWorld();
	if (World == nullptr)
		return;

	const uint64 ObjectId = DeadPkt.info().object_id();
	
	// 일단 몬스터 죽는거...
	AS1Monster** FindMonster = Monsters.Find(ObjectId);


	// 여기서 Crash...
	//Monsters.Remove(ObjectId);

	AS1Monster* Monster = *FindMonster;
	if(Monster)
		Monster->DeadAnim();
}

