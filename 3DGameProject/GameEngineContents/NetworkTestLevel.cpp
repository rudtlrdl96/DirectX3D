#include "PrecompileHeader.h"
#include "NetworkTestLevel.h"

#include <GameEngineCore/PhysXCapsuleComponent.h>

#include "ContentsEnum.h"

#include "NetworkManager.h"
#include "NetworkGUI.h"

#include "PlayerActor_Nero.h"
#include "PlayerActor_Vergil.h"
#include "Plane.h"
#include "Enemy_HellCaina.h"

NetworkTestLevel* NetworkTestLevel::Inst = nullptr;

NetworkTestLevel::NetworkTestLevel()
{
	Inst = this;
}

NetworkTestLevel::~NetworkTestLevel()
{
	Inst = nullptr;
}


void NetworkTestLevel::Start()
{
	CreateScene(GetName());

	BaseLevel::Start();
	BaseLevel::SetNetLevelType(Net_LevelType::NetTestLevel);

	GetCamera(0)->SetProjectionType(CameraType::Perspective);

	GameEngineInput::CreateKey("Test_BackMainLevel", VK_ESCAPE);
	GameEngineInput::CreateKey("Test_EnemyDeath", 'C');
	GameEngineInput::CreateKey("Test_EnemyOnOff", 'V');
}


void NetworkTestLevel::LevelChangeStart()
{
	BaseLevel::LevelChangeStart();

	
	if (/*네로만*//*false*/NetworkManager::IsServer())
	{
		std::shared_ptr<PlayerActor_Nero> Nero = CreateActor<PlayerActor_Nero>(ActorOrder::Player);
		Nero->GetPhysXComponent()->SetWorldPosition({ 0, 100, 0 });
		NetworkManager::LinkNetwork(Nero.get(), this);
	}

	if (/*버질*//*true*/NetworkManager::IsClient())
	{
		std::shared_ptr<PlayerActor_Vergil> Nero = CreateActor<PlayerActor_Vergil>(ActorOrder::Player);
		Nero->GetPhysXComponent()->SetWorldPosition({ 0, 100, 0 });
		NetworkManager::LinkNetwork(Nero.get(), this);
	}

	std::shared_ptr<Plane> Flat = CreateActor<Plane>();
}


void NetworkTestLevel::Update(float _DeltaTime)
{
	BaseLevel::Update(_DeltaTime);

	//0번
	if (true == GameEngineInput::IsDown("Test_BackMainLevel"))
	{
		GameEngineCore::ChangeLevel("MainLevel");
		return;
	}


	if (true == GameEngineInput::IsDown("Test_EnemyDeath"))
	{
		if (nullptr == Enemy)
		{
			Enemy = NetworkManager::CreateNetworkActor<Enemy_HellCaina>(this);
		}
		else
		{
			Enemy->Death();
			Enemy = nullptr;
		}
	}

	if (true == GameEngineInput::IsDown("Test_EnemyOnOff") && nullptr != Enemy)
	{
		if (Enemy->IsUpdate())
		{
			Enemy->Off();
		}
		else
		{
			Enemy->On();
		}
	}

}


void NetworkTestLevel::LevelChangeEnd()
{
	BaseLevel::LevelChangeEnd();

	const std::list<std::shared_ptr<GameEngineActor>>& Players = GetActorGroup(ActorOrder::Player);
	for (std::shared_ptr<GameEngineActor> Player : Players)
	{
		PlayerActor_Nero* NetPlayer = nullptr;
		NetPlayer = dynamic_cast<PlayerActor_Nero*>(Player.get());
		if (nullptr == NetPlayer)
			continue;

		
		NetPlayer->NetDisconnect();
		NetPlayer->Death();
	}
}

