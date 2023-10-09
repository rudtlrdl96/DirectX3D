#include "PrecompileHeader.h"
#include "PlayerActor_Nero.h"
#include <GameEngineCore/GameEngineFBXRenderer.h>
#include <GameEngineCore/PhysXCapsuleComponent.h>
#include <GameEngineCore/GameEngineFBXAnimation.h>
#include <GameEngineCore/GameEngineCollision.h>
#include "AnimationEvent.h"
#include "EffectRenderer.h"
#include "PlayerController.h"
#include "NetworkManager.h"
#include "AttackCollision.h"
#include "BaseEnemyActor.h"
#include "NeroItemGlass.h"
#include "NeroHPUI.h"
#include "FXSystem.h"
#include "PlayerCamera.h"
std::list<DevilBreaker> PlayerActor_Nero::BreakerList;
PlayerActor_Nero::~PlayerActor_Nero()
{
}

void PlayerActor_Nero::Start()
{
	BasePlayerActor::Start();
	BreakerList.push_back(DevilBreaker::None);

	SetNetObjectType(Net_ActorType::Nero);

	//NetControllType::NetControll으로 변경될 때 아래 콜백이 실행됩니다. 
	SetControllCallBack(NetControllType::PassiveControll, [=]()
		{
			NetControllLoad();
			NetLoad();
			LoadCheck = true;
		});

	//NetControllType::UserControll으로 변경될 때 아래 콜백이 실행됩니다.
	SetControllCallBack(NetControllType::ActiveControll, [=]()
		{
			UserControllLoad();
			PlayerLoad();
			LoadCheck = true;
		});

	LinkData_UpdatePacket<int>(ArmValue, [this](int _BeforeData)
	{
		SetArm(ArmValue);
	});
	LinkData_UpdatePacket<int>(ExceedLevel);
}

void PlayerActor_Nero::PlayerLoad()
{
	GetLevel()->CreateActor<NeroItemGlass>();
	HPRender = GetLevel()->CreateActor<NeroHPUI>();

	// Effect 생성
	{
		EffectSystem = CreateComponent<FXSystem>();

		GameEngineDirectory NewDir;
		NewDir.MoveParentToDirectory("ContentResources");
		NewDir.Move("ContentResources");
		NewDir.Move("Effect");
		NewDir.Move("Mesh");
		if (nullptr == GameEngineFBXMesh::Find("Effect_Mesh_01.FBX"))
		{
			std::vector<GameEngineFile> Files = NewDir.GetAllFile({ ".fbx" });
			for (GameEngineFile File : Files)
			{
				GameEngineFBXMesh::Load(File.GetFullPath());
			}
		}
		NewDir.MoveParent();
		NewDir.Move("Texture");
		if (nullptr == GameEngineTexture::Find("Effect_Texture_01.tga"))
		{
			std::vector<GameEngineFile> Files = NewDir.GetAllFile({ ".tga" });
			for (GameEngineFile File : Files)
			{
				GameEngineTexture::Load(File.GetFullPath());
			}
		}

		// 이펙트 Sprite 로드
		if (nullptr == GameEngineSprite::Find("Effect_Impact.tga"))
		{
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Impact.tga").GetFullPath(), 8, 8);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Impact_01.tga").GetFullPath(), 8, 4);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Muzzle_03.tga").GetFullPath(), 2, 1);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Spark_02.tga").GetFullPath(), 8, 8);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Magic_01.tga").GetFullPath(), 8, 8);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Fire_01.tga").GetFullPath(), 8, 4);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Fire_02.tga").GetFullPath(), 8, 4);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Fire_03.tga").GetFullPath(), 8, 4);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Fire_04.tga").GetFullPath(), 8, 8);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Fire_05.tga").GetFullPath(), 8, 8);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Explosion_01.tga").GetFullPath(), 8, 8);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Explosion_02.tga").GetFullPath(), 8, 8);
		}
		NewDir.MoveParent();
		NewDir.Move("Nero");

		std::vector<GameEngineFile> Files = NewDir.GetAllFile({ ".effect" });
		for (GameEngineFile File : Files)
		{
			if (nullptr == FXData::Find(File.GetFileName()))
			{
				FXData::Load(File.GetFullPath());
			}
			EffectSystem->CreateFX(FXData::Find(File.GetFileName()));
		}
	}

	// Renderer 생성
	{
		GameEngineDirectory NewDir;
		NewDir.MoveParentToDirectory("ContentResources");
		NewDir.Move("ContentResources");
		NewDir.Move("Character");
		NewDir.Move("Player");
		NewDir.Move("Nero");
		NewDir.Move("Mesh");
		if (nullptr == GameEngineFBXMesh::Find("Nero.FBX"))
		{
			GameEngineFBXMesh::Load(NewDir.GetPlusFileName("Nero.fbx").GetFullPath());
			GameEngineTexture::Load(NewDir.GetPlusFileName("pl0010_03_atos.texout.png").GetFullPath());
			GameEngineTexture::Load(NewDir.GetPlusFileName("pl0000_03_atos.texout.png").GetFullPath());
		}
		NewDir.MoveParent();
		NewDir.Move("Animation");

		Renderer = CreateComponent<GameEngineFBXRenderer>();
		Renderer->GetTransform()->SetLocalRotation({ 0, 0, 0 });
		Renderer->GetTransform()->SetLocalPosition({ 0, -75, 0 });

		//Renderer->SetBaseColor(float4(2, 0, 0));

		switch (GameEngineOption::GetOption("Shader"))
		{
		case GameEngineOptionValue::Low:
		{
			Renderer->SetFBXMesh("Nero.FBX", "AniFBX_Low");
		}
		break;
		case GameEngineOptionValue::High:
		{
			Renderer->SetFBXMesh("Nero.FBX", "AniFBX");
		}
		break;
		default:
			break;
		}

		Renderer->ShadowOn();
		Renderer->SetDynamic();

		//Renderer->SetSpecularTexture("pl0000_03_albm.texout.png", "pl0000_03_atos.texout.png");
		//Renderer->SetSpecularTexture("pl0010_03_albm.texout.png", "pl0010_03_atos.texout.png");

		Renderer->SetMaterial("pl0000_03_albm.texout.png", "AniFBX_Alpha", nullptr);

		AnimationEvent::LoadAll({ .Dir = NewDir.GetFullPath().c_str(), .Renderer = Renderer, .RendererLocalPos = {0, -75, 0},
			.Objects = { (GameEngineObject*)Col_Attack.get() },
			.CallBacks_void = {
				std::bind([=] {InputCheck = true; }),
				std::bind(&PlayerActor_Nero::RedQueenOn, this),
				std::bind(&PlayerActor_Nero::RedQueenOff, this),
				std::bind(&PlayerActor_Nero::BlueRoseOn, this),
				std::bind(&PlayerActor_Nero::WeaponIdle, this),
				std::bind(&PhysXCapsuleComponent::SetLinearVelocityZero, PhysXCapsule),
				std::bind([=] {MoveCheck = true; }),
				std::bind([=] {DelayCheck = true; }),
				std::bind(&PhysXCapsuleComponent::TurnOnGravity, PhysXCapsule),
				std::bind(&PlayerActor_Nero::SetOverture, this),
				std::bind(&PlayerActor_Nero::SetHuman, this),
				std::bind(&PlayerActor_Nero::SetDemon, this),
				std::bind(&PlayerActor_Nero::DestroyBreaker, this),
				std::bind(&PlayerActor_Nero::SetExActTiming, this), //13
				std::bind(&PlayerCamera::ShakeLight),	//14
				std::bind(&PlayerCamera::ShakeMiddle),	//15
				std::bind(&PlayerCamera::ShakeHeavy),	//16
			},
			.CallBacks_int = {
				std::bind(&PlayerActor_Nero::ChangeState, this, std::placeholders::_1),
			},
			.CallBacks_float = {
				std::bind(&BasePlayerActor::RotationToTarget, this, std::placeholders::_1),
				std::bind(&BasePlayerActor::RotationToMoveVector, this, std::placeholders::_1),
				std::bind(&BasePlayerActor::SetInvincibility, this, std::placeholders::_1)
			},
			.CallBacks_float4 = {
				std::bind(&BasePlayerActor::SetForce, this, std::placeholders::_1),
				std::bind(&BasePlayerActor::SetPush, this, std::placeholders::_1),
				std::bind(&BasePlayerActor::SetMove, this, std::placeholders::_1)
			}
			});

		//Object = 0 : 공격 충돌체
		//
		//콜백void = 0 : 입력체크시작
		//콜백void = 1 : 손에 레드퀸 들려줌
		//콜백void = 2 : 손에 레드퀸 뺌
		//콜백void = 3 : 손에 블루로즈
		//콜백void = 4 : WeaponIdle (빈손, 칼 등에)
		//콜백void = 5 : SetLinearVelocityZero
		//콜백void = 6 : 이동체크 시작
		//콜백void = 7 : 딜레이체크 시작k
		//콜백void = 8 : 중력 적용
		//콜백void = 9 : SetOverture
		//콜백void = 10 : SetHuman
		//콜백void = 11 : SetDemon
		//콜백void = 12 : 데빌브레이커 파괴
		//
		//콜백 int = 0 : FSM변경
		// 
		//콜백 float4 = 0 : SetForce
		//콜백 float4 = 0 : SetPush
		//콜백 float4 = 0 : SetMove

	}

	// OvertureRenderer 생성
	{}
	{
		GameEngineDirectory NewDir;
		NewDir.MoveParentToDirectory("ContentResources");
		NewDir.Move("ContentResources");
		NewDir.Move("Character");
		NewDir.Move("Player");
		NewDir.Move("Nero");
		NewDir.Move("Overture");
		NewDir.Move("Mesh");
		if (nullptr == GameEngineFBXMesh::Find("Overture.FBX"))
		{
			GameEngineFBXMesh::Load(NewDir.GetPlusFileName("Overture.fbx").GetFullPath());
		}
		Renderer_Overture = CreateComponent<GameEngineFBXRenderer>();
		Renderer_Overture->GetTransform()->SetLocalPosition({ 0, -75, 0 });
		switch (GameEngineOption::GetOption("Shader"))
		{
		case GameEngineOptionValue::Low:
		{
			Renderer_Overture->SetFBXMesh("Overture.FBX", "AniFBX_Low");
		}
		break;
		case GameEngineOptionValue::High:
		{
			Renderer_Overture->SetFBXMesh("Overture.FBX", "AniFBX");
		}
		break;
		default:
			break;
		}
		Renderer_Overture->ShadowOn();
		Renderer_Overture->SetDynamic();

		NewDir.MoveParent();
		NewDir.Move("Animation");
		std::vector<GameEngineFile> Files = NewDir.GetAllFile({ ".fbx" });
		for (GameEngineFile File : Files)
		{
			if (nullptr == GameEngineFBXAnimation::Find(File.GetFileName()))
			{
				GameEngineFBXAnimation::Load(File.GetFullPath());
			}
			Renderer_Overture->CreateFBXAnimation(File.GetFileName(), { .Inter = 0.0166f, .Loop = false });
		}
		Renderer_Overture->ChangeAnimation("wp00_010_Shoot.fbx");
		Renderer_Overture->Off();
	}

	SetHuman();
	WeaponIdle();

	// 기본 움직임
	{
		// Idle
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Idle,
			.Start = [=] {
				PhysXCapsule->TurnOffGravity();
				WeaponIdle();
				PhysXCapsule->SetLinearVelocityZero();
				Renderer->ChangeAnimation("pl0000_Idle_Normal");
			},
			.Update = [=](float _DeltaTime) {
				PhysXCapsule->SetLinearVelocityZero();

				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
				if (true == Controller->GetIsLockOn())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Switch_Idle_to_Lockon);
					return;
				}
			},
			.End = [=] {

			}
			});
		// Run Start
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RunStart,
			.Start = [=] {
				PhysXCapsule->TurnOnGravity();
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Run_Start");
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
				if (true == Controller->GetIsLockOn())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Switch_Idle_to_Lockon);
					return;
				}
				if (Controller->GetMoveVector() == float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStop);
					return;
				}

				if (true == Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Run);
					return;
				}
				LookDir(Controller->GetMoveVector());
				float4 MoveDir = Controller->GetMoveVector() * RunSpeed;
				PhysXCapsule->SetMove(MoveDir);
			},
			.End = [=] {
			}
			});
		// Run
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Run,
			.Start = [=] {
				DashTimer = 0;
				Renderer->ChangeAnimation("pl0000_Run_Loop");
			},
			.Update = [=](float _DeltaTime) {
				DashTimer += _DeltaTime;

				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
				if (true == Controller->GetIsLockOn())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Switch_Idle_to_Lockon);
					return;
				}

				if (Controller->GetMoveVector() == float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStop);
					return;
				}
				if (1 < DashTimer)
				{
					ChangeState(FSM_State_Nero::Nero_Dash);
					return;
				}

				LookDir(Controller->GetMoveVector());
				float4 MoveDir = Controller->GetMoveVector() * RunSpeed;
				PhysXCapsule->SetMove(MoveDir);
			},
			.End = [=] {
			}
			});
		// RunStop
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RunStop,
			.Start = [=] {
				PhysXCapsule->SetLinearVelocityZero();
				Renderer->ChangeAnimation("pl0000_Run_Stop");
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
				if (true == Controller->GetIsLockOn())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Switch_Idle_to_Lockon);
					return;
				}
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
			}
			});
		// Dash
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Dash,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Dash_Loop");
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
				if (true == Controller->GetIsLockOn())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Switch_Idle_to_Lockon);
					return;
				}
				if (Controller->GetMoveVector() == float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_DashStop);
					return;
				}

				LookDir(Controller->GetMoveVector());
				float4 MoveDir = Controller->GetMoveVector() * DashSpeed;
				PhysXCapsule->SetMove(MoveDir);
			},
			.End = [=] {

			}
			});
		// DashStop
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_DashStop,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Dash_Stop");
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
				if (true == Controller->GetIsLockOn())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Switch_Idle_to_Lockon);
					return;
				}
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				PhysXCapsule->SetLinearVelocityZero();
			}
			});
		// Jump_Vertical
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Jump_Vertical,
			.Start = [=] {
				RotationToMoveVector();
				PhysXCapsule->TurnOnGravity();
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->SetMove(Controller->GetMoveVector() * 500);
				InputCheck = false;
				MoveCheck = false;
				SetInvincibility(0.5f);
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_Vertical");
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }

				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }

				PhysXCapsule->SetForce(Controller->GetMoveVector() * JumpMoveForce);

				if (false == MoveCheck) { return; }
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
			},
			.End = [=] {
			}
			});
		// Jump_Back
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Jump_Back,
			.Start = [=] {
				PhysXCapsule->TurnOnGravity();
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->SetMove(Controller->GetMoveVector() * 500);
				InputCheck = false;
				MoveCheck = false;
				SetInvincibility(0.5f);
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_Back");
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }

				PhysXCapsule->SetForce(Controller->GetMoveVector() * JumpMoveForce);

				if (false == MoveCheck) { return; }
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
			},
			.End = [=] {
			}
			});
		// Jump Fly
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Jump_Fly,
			.Start = [=] {
				PhysXCapsule->SetPush(float4::DOWN * FlyDownForce);
				PhysXCapsule->TurnOnGravity();
				Renderer->ChangeAnimation("pl0000_Jump_Fly_loop");
			},
			.Update = [=](float _DeltaTime) {
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
				PhysXCapsule->SetForce(Controller->GetMoveVector() * JumpMoveForce);
			},
			.End = [=] {
				PhysXCapsule->TurnOffGravity();
			}
			});
		// Landing
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Landing,
			.Start = [=] {
				MoveCheck = false;
				PhysXCapsule->TurnOnGravity();
				UseDoubleJump = false;
				PhysXCapsule->SetLinearVelocityZero();
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_Landing");
			},
			.Update = [=](float _DeltaTime) {
				PhysXCapsule->SetLinearVelocityZero();
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
				if (false == MoveCheck) { return; }
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Idle);
				}
			},
			.End = [=] {

			}
			});
		// 2nd_Jump
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_2nd_Jump,
			.Start = [=] {
				InputCheck = false;
				MoveCheck = false;
				RotationToMoveVector();
				PhysXCapsule->TurnOnGravity();
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->SetMove(Controller->GetMoveVector() * 250);
				SetInvincibility(0.5f);
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_2ndJump", true);
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }

				if (true == InputCheck)
				{
					if (true == Input_SpecialCheckFly()) { return; }
				}
				PhysXCapsule->SetForce(Controller->GetMoveVector() * JumpMoveForce);

				if (false == MoveCheck) { return; }
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
			},
			.End = [=] {
			}
			});
		// 2nd_Jump_Back
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_2nd_Jump_Back,
			.Start = [=] {
				InputCheck = false;
				MoveCheck = false;
				PhysXCapsule->TurnOnGravity();
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->SetMove(Controller->GetMoveVector() * 250);
				SetInvincibility(0.5f);
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_Back_2ndJump", true);
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }

				if (true == InputCheck)
				{
					if (true == Input_SpecialCheckFly()) { return; }
				}
				PhysXCapsule->SetForce(Controller->GetMoveVector() * JumpMoveForce);

				if (false == MoveCheck) { return; }
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
			},
			.End = [=] {
			}
			});

		// EnemyStep Jump
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_EnemyStep_Jump,
			.Start = [=] {
				InputCheck = false;
				MoveCheck = false;
				PhysXCapsule->TurnOnGravity();
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->SetMove(Controller->GetMoveVector() * 250);
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_EnemyStep", true);
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == InputCheck)
				{
					if (true == Input_SpecialCheckFly()) { return; }
				}
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }

				PhysXCapsule->SetForce(Controller->GetMoveVector() * JumpMoveForce);

				if (false == MoveCheck) { return; }
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
			},
			.End = [=] {
			}
			});
	}
	{}
	// 레드 퀸
	{
		// RedQueen ComboA1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboA_1,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(120));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboA1_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager){ExceedLevel--;});
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboA1.effect");
				}
				PhysXCapsule->TurnOffGravity();
				PhysXCapsule->SetLinearVelocityZero();
				Renderer->ChangeAnimation("pl0000_RQ_ComboA_1");
				RotationToTarget(30.0f);
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck(1)) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboA2
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboA_2,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(90));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboA2_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager){ExceedLevel--;});
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboA2.effect");
				}
				PhysXCapsule->SetLinearVelocityZero();
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboA_2");
				RotationToTarget(30.0f);
				InputCheck = false;
				MoveCheck = false;
				DelayCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (true == DelayCheck)
				{
					if (true == Input_SwordCheck(4)) { return; }
				}
				else if (true == Input_SwordCheck(2)) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboA3
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboA_3,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(60));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboA3_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager){ExceedLevel--;});
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboA3.effect");
				}
				PhysXCapsule->SetLinearVelocityZero();
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboA_3");
				RotationToTarget(30.0f);
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck(3)) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboA4
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboA_4,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Heavy, DamageCalculate(150));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboA4_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager){ExceedLevel--;});
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboA4.effect");
				}
				PhysXCapsule->SetLinearVelocityZero();
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboA_4");
				RotationToTarget(30.0f);
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboD1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboD_1,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(60));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboD1_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager){ExceedLevel--;});
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboD1.effect");
				}
				PhysXCapsule->SetLinearVelocityZero();
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboD_1");
				RotationToTarget(30.0f);
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck(5)) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboD2
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboD_2,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(90));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboD2_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager){ExceedLevel--;});
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboD2.effect");
				}
				PhysXCapsule->SetLinearVelocityZero();
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboD_2");
				RotationToTarget(30.0f);
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck(6)) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboD3
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboD_3,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(60));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboD3_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager){ExceedLevel--;});
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboD3.effect");
				}
				PhysXCapsule->SetLinearVelocityZero();
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboD_3");
				RotationToTarget(30.0f);
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck(7)) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboD4
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboD_4,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Heavy, DamageCalculate(150));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboD4_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager){ExceedLevel--;});
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboD4.effect");
				}
				PhysXCapsule->SetLinearVelocityZero();
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboD_4");
				RotationToTarget(30.0f);
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen HR
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_HR,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Air, DamageCalculate(135, true));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_HR_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager) {ExceedLevel -= 3; });
				}
				else
				{
					EffectSystem->PlayFX("RQ_HR.effect");
				}
				PhysXCapsule->TurnOnGravity();
				PhysXCapsule->SetLinearVelocityZero();
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_HR-EX");
				RotationToTarget();
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (true == Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
				RedQueenOff();
				WeaponIdle();
			}
			});
		// RedQueen Shuffle
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Shuffle,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Heavy, DamageCalculate(180, true));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_Shuffle_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager) {ExceedLevel -= 3; });
				}
				else
				{
					EffectSystem->PlayFX("RQ_Shuffle.effect");
				}
				InputCheck = false;
				DelayCheck = false;
				PhysXCapsule->SetLinearVelocityZero();
				RotationToTarget();
				SetInvincibility(0.5f);
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Shuffle", true);
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Idle);
				}
				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (DelayCheck == false) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				PhysXCapsule->SetLinearVelocityZero();
				WeaponIdle();
			}
			});
		// RedQueen Stleak1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Stleak1,
			.Start = [=] {
				EffectSystem->PlayFX("RQ_Stleak_1.effect");
				PhysXCapsule->SetLinearVelocityZero();
				RotationToTarget();
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Stleak_1");
			},
			.Update = [=](float _DeltaTime) {
				if (true == Input_SpecialCheck()) { return; }
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_RQ_Skill_Stleak2);
				}
			},
			.End = [=] {
				RedQueenOff();
			}
			});
		static float StreakTimer = 0;
		// RedQueen Stleak2
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Stleak2,
			.Start = [=] {
				RedQueenOn();
				StreakTimer = 0;
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Stleak_2_Loop");
			},
			.Update = [=](float _DeltaTime) {
				if (true == Input_SpecialCheck()) { return; }
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				std::vector<std::shared_ptr<GameEngineCollision>> Cols;
				if (true == Col_EnemyStepCheck->CollisionAll(CollisionOrder::Enemy, Cols))
				{
					ChangeState(FSM_State_Nero::Nero_RQ_Skill_Stleak3);
					return;
				}
				StreakTimer += _DeltaTime;
				if (0.5f < StreakTimer)
				{
					ChangeState(FSM_State_Nero::Nero_RQ_Skill_Stleak3);
					return;
				}
				PhysXCapsule->SetMove(GetTransform()->GetWorldForwardVector() * 1300);
			},
			.End = [=] {
				RedQueenOff();
				PhysXCapsule->SetLinearVelocityZero();
			}
			});
		// RedQueen Stleak3
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Stleak3,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Heavy, DamageCalculate(180, true));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_Stleak_3_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager) {ExceedLevel -= 3; });
				}
				else
				{
					EffectSystem->PlayFX("RQ_Stleak_3.effect");
				}
				PhysXCapsule->TurnOffGravity();
				PhysXCapsule->SetLinearVelocityZero();
				InputCheck = false;
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Stleak_3");
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Idle);
				}
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				RedQueenOff();
			}
			});
		// RedQueen AirComboA1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_AirComboA_1,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(150));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_AirComboA1_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager){ExceedLevel--;});
				}
				else
				{
					EffectSystem->PlayFX("RQ_AirComboA1.effect");
				}
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				Renderer->ChangeAnimation("pl0000_RQ_AT-Jump_1");
				RotationToTarget();
				InputCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly(1)) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }

			},
			.End = [=] {
				WeaponIdle();
				PhysXCapsule->TurnOnGravity();
			}
			});
		// RedQueen AirComboA2
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_AirComboA_2,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(120));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_AirComboA2_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager){ExceedLevel--;});
				}
				else
				{
					EffectSystem->PlayFX("RQ_AirComboA2.effect");
				}
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_AT-Jump_2");
				RotationToTarget();
				InputCheck = false;
				DelayCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }
				if (DelayCheck == true)
				{
					if (true == Input_SwordCheckFly(3)) { return; }
				}
				else
				{
					if (true == Input_SwordCheckFly(2)) { return; }
				}
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
				WeaponIdle();
				PhysXCapsule->TurnOnGravity();
			}
			});
		// RedQueen AirComboA3
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_AirComboA_3,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Slam, DamageCalculate(180));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_AirComboA3_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager){ExceedLevel--;});
				}
				else
				{
					EffectSystem->PlayFX("RQ_AirComboA3.effect");
				}
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_AT-Jump_3");
				RotationToTarget();
				InputCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
				WeaponIdle();
				PhysXCapsule->TurnOnGravity();
			}
			});
		// RedQueen AirComboB
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_AirComboB,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Air, DamageCalculate(405));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_AirComboB_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager){ExceedLevel--;});
				}
				else
				{
					EffectSystem->PlayFX("RQ_AirComboB.effect");
				}
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_AirComboB");
				RotationToTarget(30.0f);
				InputCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
				WeaponIdle();
				PhysXCapsule->TurnOnGravity();
			}
			});
		// RedQueen Split_1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Split_1,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Slam, DamageCalculate(22, true));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_Split_1_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_Split_1.effect");
				}
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				RotationToTarget();
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Split_1");
			},
			.Update = [=](float _DeltaTime) {
				if (true == Input_SpecialCheckFly()) { return; }
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_RQ_Skill_Split_2);
				}
			},
			.End = [=] {
			}
			});
		// RedQueen Split_2
		static float4 SplitTargetPos;
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Split_2,
			.Start = [=] {
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_Split_2_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_Split_2.effect");
				}
				EffectSystem->Loop = true;
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Split_2_Loop");
				GetLevel()->RayCast(GetTransform()->GetWorldPosition(), float4::DOWN, SplitTargetPos, 9999.0f);
				SplitTargetPos += float4::UP * 100;
			},
			.Update = [=](float _DeltaTime) {
				PhysXCapsule->SetMove(float4::DOWN * 500);
				if (GetTransform()->GetWorldPosition().y < SplitTargetPos.y)
				{
					PhysXCapsule->SetWorldPosition(SplitTargetPos);
					ChangeState(FSM_State_Nero::Nero_RQ_Skill_Split_3);
					return;
				}
				if (true == Input_SpecialCheckFly()) { return; }

				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_RQ_Skill_Split_3);
					return;
				}
			},
			.End = [=] {
				EffectSystem->Loop = false;
			}
			});
		// RedQueen Split_3
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Split_3,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Heavy, DamageCalculate(90, true));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_Split_3_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager) {ExceedLevel -= 3; });
				}
				else
				{
					EffectSystem->PlayFX("RQ_Split_3.effect");
				}
				UseDoubleJump = false;
				InputCheck = false;
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Split_3");
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen Caliber_1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Caliber_1,
			.Start = [=] {
				EffectSystem->PlayFX("RQ_Calibur_1.effect");
				RedQueenOn();
				RotationToTarget();
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Caliber_1");
			},
			.Update = [=](float _DeltaTime) {
				if (true == Input_SpecialCheckFly()) { return; }
				std::vector<std::shared_ptr<GameEngineCollision>> Cols;
				if (true == Col_EnemyStepCheck->CollisionAll(CollisionOrder::Enemy, Cols))
				{
					ChangeState(FSM_State_Nero::Nero_RQ_Skill_Caliber_2);
					return;
				}
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_RQ_Skill_Caliber_2);
				}
			},
			.End = [=] {
			}
			});
		// RedQueen Caliber_2
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Caliber_2,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Heavy, DamageCalculate(180, true));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_Calibur_2_EX.effect");
					TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager) {ExceedLevel -= 3; });
				}
				else
				{
					EffectSystem->PlayFX("RQ_Calibur_2.effect");
				}
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Caliber_2");
				InputCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
				PhysXCapsule->TurnOnGravity();
				RedQueenOff();
			}
			});


	}
	{}
	// 블루 로즈 (락온)
	{
		// Idle To LockOn
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Switch_Idle_to_Lockon,
			.Start = [=] {
				PhysXCapsule->TurnOffGravity();
				PhysXCapsule->SetLinearVelocityZero();
				Renderer->ChangeAnimation("pl0000_BR_Switch_Idle_to_Lockon");
			},
			.Update = [=](float _DeltaTime) {

				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
				if (false == Controller->GetIsLockOn())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Switch_Lockon_to_Idle);
					return;

				}
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_BR_Strafe);
					return;
				}
				if (true == Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Lockon_Front);
					return;
				}
				LookTarget();
			},
			.End = [=] {

			}
			});
		// LockOnFront
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Lockon_Front,
			.Start = [=] {
				PhysXCapsule->TurnOffGravity();
				BlueRoseOn();
				PhysXCapsule->SetLinearVelocityZero();
				Renderer->ChangeAnimation("pl0000_BR_Lockon_Front");
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
				if (false == Controller->GetIsLockOn())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Switch_Lockon_to_Idle);
					return;

				}
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_BR_Strafe);
					return;
				}

				LookTarget();
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// Strafe
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Strafe,
			.Start = [=] {
				PhysXCapsule->TurnOnGravity();
				BlueRoseOn();
			},
			.Update = [=](float _DeltaTime) {

				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
				if (false == Controller->GetIsLockOn())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Switch_Lockon_to_Idle);
					return;

				}
				if (Controller->GetMoveVector() == float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_BR_Lockon_Front);
					return;
				}

				LookTarget();

				float4 MoveDir = Controller->GetMoveVector() * WalkSpeed;
				PhysXCapsule->SetMove(MoveDir);

				switch (Controller->MoveVectorToChar(Controller->GetMoveVector()))
				{
				case '8':
					Renderer->ChangeAnimation("pl0000_Strafe_F_Loop");
					FSMValue = FSM_State_Nero::Nero_BR_Strafe_F;
					break;
				case '7':
					Renderer->ChangeAnimation("pl0000_Strafe_FL_Loop");
					FSMValue = FSM_State_Nero::Nero_BR_Strafe_FL;
					break;
				case '4':
					Renderer->ChangeAnimation("pl0000_Strafe_L_Loop");
					FSMValue = FSM_State_Nero::Nero_BR_Strafe_L;
					break;
				case '1':
					Renderer->ChangeAnimation("pl0000_Strafe_BL_Loop");
					FSMValue = FSM_State_Nero::Nero_BR_Strafe_BL;
					break;
				case '2':
					Renderer->ChangeAnimation("pl0000_Strafe_B_Loop");
					FSMValue = FSM_State_Nero::Nero_BR_Strafe_B;
					break;
				case '3':
					Renderer->ChangeAnimation("pl0000_Strafe_BR_Loop");
					FSMValue = FSM_State_Nero::Nero_BR_Strafe_BR;
					break;
				case '6':
				case '9':
					Renderer->ChangeAnimation("pl0000_Strafe_R_Loop");
					FSMValue = FSM_State_Nero::Nero_BR_Strafe_R;
					break;
				}
				NetworkManager::SendFsmChangePacket(this, FSMValue);

			},
			.End = [=] {
				WeaponIdle();
				PhysXCapsule->SetLinearVelocityZero();
			} });
		// LockOn To Idle
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Switch_Lockon_to_Idle,
			.Start = [=] {
				PhysXCapsule->TurnOffGravity();
				BlueRoseOn();
				PhysXCapsule->SetLinearVelocityZero();
				Renderer->ChangeAnimation("pl0000_BR_Switch_Lockon_to_Idle");
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
				if (true == Controller->GetIsLockOn())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Switch_Idle_to_Lockon);
					return;
				}
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
				if (true == Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Idle);
					return;
				}
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// Evade
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Evade_Left,
			.Start = [=] {
				RotationToTarget();
				PhysXCapsule->TurnOffGravity();
				PhysXCapsule->SetLinearVelocityZero();
				InputCheck = false;
				MoveCheck = false;
				BlueRoseOn();
				SetInvincibility(0.5f);
				Renderer->ChangeAnimation("pl0000_Evade_Left", true);
			},
			.Update = [=](float _DeltaTime) {
				if (true == Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Lockon_Front);
					return;
				}
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}
				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_BR_Strafe);
					return;
				}

			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// Evade
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Evade_Right,
			.Start = [=] {
				RotationToTarget();
				PhysXCapsule->TurnOffGravity();
				PhysXCapsule->SetLinearVelocityZero();
				InputCheck = false;
				MoveCheck = false;
				BlueRoseOn();
				SetInvincibility(0.5f);
				Renderer->ChangeAnimation("pl0000_Evade_Right", true);
			},
			.Update = [=](float _DeltaTime) {
				if (true == Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Lockon_Front);
					return;
				}
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_BR_Strafe);
					return;
				}
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// Shoot
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Shoot,
			.Start = [=] {
				EffectSystem->PlayFX("BR_Shoot.effect");
				Col_Attack->SetAttackData(DamageType::Light, 54, std::bind(&GameEngineObjectBase::Off, Col_Attack));
				if (nullptr == LockOnEnemyTransform)
				{
					Col_Attack->GetTransform()->SetLocalPosition({ 0, 100, 1000 });
					Col_Attack->GetTransform()->SetLocalScale({ 50, 50, 2000 });
				}
				else
				{
					Col_Attack->GetTransform()->SetWorldPosition(LockOnEnemyTransform->GetWorldPosition());
					Col_Attack->GetTransform()->SetLocalScale({ 50, 50, 50 });
				}
				Col_Attack->On();
				PhysXCapsule->TurnOffGravity();
				PhysXCapsule->SetLinearVelocityZero();
				RotationToTarget();
				BlueRoseOn();
				InputCheck = false;
				MoveCheck = false;
				Renderer->ChangeAnimation("pl0000_BR_Shoot", true);
			},
			.Update = [=](float _DeltaTime) {
				if (true == Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_BR_Lockon_Front);
					return;
				}
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }

				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_BR_Strafe);
					return;
				}			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// Air Shoot
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_AirShoot,
			.Start = [=] {
				EffectSystem->PlayFX("BR_Shoot.effect");
				Col_Attack->SetAttackData(DamageType::Light, 54, std::bind(&GameEngineObjectBase::Off, Col_Attack));
				if (nullptr == LockOnEnemyTransform)
				{
					Col_Attack->GetTransform()->SetLocalPosition({ 0, 100, 1000 });
					Col_Attack->GetTransform()->SetLocalScale({ 50, 50, 2000 });
				}
				else
				{
					Col_Attack->GetTransform()->SetWorldPosition(LockOnEnemyTransform->GetWorldPosition());
					Col_Attack->GetTransform()->SetLocalScale({ 50, 50, 50 });
				}
				Col_Attack->On();
				BlueRoseOn();
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				RotationToTarget();
				InputCheck = false;
				MoveCheck = false;
				Renderer->ChangeAnimation("pl0000_BR_Air_Shoot", true);
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
				WeaponIdle();
			}
			});
	}
	// 오버추어
	{}
	{
		// Nero_Overture_Shoot
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Overture_Shoot,
			.Start = [=] {
				WeaponIdle();
				SetOvertureAnimation();
				PhysXCapsule->SetLinearVelocityZero();
				Col_Attack->SetAttackData(DamageType::Heavy, 420);
				EffectSystem->PlayFX("Overture_Shoot.effect");
				Renderer->ChangeAnimation("pl0000_Overture_Shoot", true);
				Renderer_Overture->ChangeAnimation("wp00_010_Shoot.fbx", true);
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					LookDir(Controller->GetMoveVector());
				}
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				SetOverture();
				WeaponIdle();
			}
			});
		// Nero_Overture_AirShoot
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Overture_AirShoot,
			.Start = [=] {
				WeaponIdle();
				SetOvertureAnimation();
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				Col_Attack->SetAttackData(DamageType::Heavy, 420);
				EffectSystem->PlayFX("Overture_Shoot.effect");
				Renderer->ChangeAnimation("pl0000_Overture_Air_Shoot", true);
				Renderer_Overture->ChangeAnimation("wp00_010_Air_Shoot.fbx", true);
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					LookDir(Controller->GetMoveVector());
				}
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
				SetOverture();
				WeaponIdle();
			}
			});
	}
	// 거베라
	{}
	{
		// Nero_Gerbera_Back
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Gerbera_Back,
			.Start = [=] {
				WeaponIdle();
				PhysXCapsule->TurnOffGravity();
				PhysXCapsule->SetLinearVelocityZero();
				InputCheck = false;
				SetInvincibility(0.5f);
				Col_Attack->SetAttackData(DamageType::Heavy, 75);
				EffectSystem->PlayFX("Gerbera_Jump_Back.effect");
				Renderer->ChangeAnimation("pl0000_Gerbera_Jump_Back", true);
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
			}
			});

		// Nero_Gerbera_Front
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Gerbera_Front,
			.Start = [=] {
				PhysXCapsule->TurnOffGravity();
				PhysXCapsule->SetLinearVelocityZero();
				InputCheck = false;
				WeaponIdle();
				SetInvincibility(0.5f);
				Col_Attack->SetAttackData(DamageType::Heavy, 75);
				EffectSystem->PlayFX("Gerbera_Jump_Back.effect");
				EffectSystem->GetTransform()->SetLocalRotation({ 0, 180, 0 });
				Renderer->ChangeAnimation("pl0000_Gerbera_Jump_Front", true);
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
				EffectSystem->GetTransform()->SetLocalRotation(float4::ZERO);
			}
			});

		// Nero_Gerbera_Left
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Gerbera_Left,
			.Start = [=] {
				PhysXCapsule->TurnOffGravity();
				PhysXCapsule->SetLinearVelocityZero();
				InputCheck = false;
				WeaponIdle();
				SetInvincibility(0.5f);
				Col_Attack->SetAttackData(DamageType::Heavy, 75);
				EffectSystem->PlayFX("Gerbera_Jump_Back.effect");
				EffectSystem->GetTransform()->SetLocalRotation({ 0, 90, 0 });
				Renderer->ChangeAnimation("pl0000_Gerbera_Jump_Left", true);
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
				EffectSystem->GetTransform()->SetLocalRotation(float4::ZERO);
			}
			});

		// Nero_Gerbera_Right
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Gerbera_Right,
			.Start = [=] {
				PhysXCapsule->TurnOffGravity();
				PhysXCapsule->SetLinearVelocityZero();
				InputCheck = false;
				WeaponIdle();
				SetInvincibility(0.5f);
				Col_Attack->SetAttackData(DamageType::Heavy, 75);
				EffectSystem->PlayFX("Gerbera_Jump_Back.effect");
				EffectSystem->GetTransform()->SetLocalRotation({ 0, -90, 0 });
				Renderer->ChangeAnimation("pl0000_Gerbera_Jump_Right", true);
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == Input_SpecialCheckFly()) { return; }
				if (false == InputCheck) { return; }
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
				EffectSystem->GetTransform()->SetLocalRotation(float4::ZERO);
			}
			});
	}
	// 버스터암
	{}
	{
		// BusterArm Catch
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Buster_Catch,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Buster, 0, std::bind(&PlayerActor_Nero::ChangeState, this, Nero_Buster_Strike));
				WeaponIdle();
				PhysXCapsule->SetLinearVelocityZero();
				EffectSystem->PlayFX("Buster_Catch.effect");
				Renderer->ChangeAnimation("pl0000_Buster_Catch", true);
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					LookDir(Controller->GetMoveVector());
				}
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				Col_Attack->Off();
			}
			});

		// BusterArm Strike
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Buster_Strike,
			.Start = [=] {
				EffectSystem->PlayFX("Buster_Strike.effect");
				Renderer->ChangeAnimation("pl0000_Buster_Strike_Common");
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}


				if (InputCheck == false) { return; }

				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
			}
			});

		// BusterArm Repelled
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Buster_Repelled,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Buster_Catch_Repelled");
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}


				if (InputCheck == false) { return; }

				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
			}
			});

		// BusterArm Catch Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Buster_Catch_Air,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Buster, 50, std::bind(&PlayerActor_Nero::ChangeState, this, Nero_Buster_Strike_Air));
				WeaponIdle();
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				EffectSystem->PlayFX("Buster_Catch.effect");
				Renderer->ChangeAnimation("pl0000_Buster_Air_Catch", true);
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					LookDir(Controller->GetMoveVector());
				}
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}

				if (true == Input_SpecialCheckFly()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
				Col_Attack->Off();
			}
			});

		// BusterArm Strike Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Buster_Strike_Air,
			.Start = [=] {
				EffectSystem->PlayFX("Buster_Strike.effect");
				Renderer->ChangeAnimation("pl0000_Buster_Air_Strike_Common");
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}

				if (InputCheck == false) { return; }

				if (true == Input_SpecialCheckFly()) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
			}
			});

		// BusterArm Repelled Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Buster_Repelled_Air,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Buster_Air_Catch_Repelled");
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}

				if (true == Input_SpecialCheckFly()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
			}
			});
	}
	// 스내치
	{}
	{
		static float4 SnatchDir;
		const static float SnatchSpeed = 4000.0f;
		// Snatch Shoot
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Snatch_Shoot,
			.Start = [=] {
				PhysXCapsule->SetLinearVelocityZero();
				RotationToTarget();
				WeaponIdle();
				Col_Attack->On();
				Col_Attack->SetAttackData(DamageType::Snatch, 0, std::bind(&PlayerActor_Nero::ChangeState, this, FSM_State_Nero::Nero_Snatch_Pull));
				Col_Attack->GetTransform()->SetWorldPosition(GetTransform()->GetWorldPosition());
				Col_Attack->GetTransform()->SetLocalScale({100, 100, 100});
				if (nullptr == LockOnEnemyTransform)
				{
					SnatchDir = GetTransform()->GetWorldForwardVector();
				}
				else
				{
					SnatchDir = LockOnEnemyTransform->GetWorldPosition() - GetTransform()->GetWorldPosition();
					SnatchDir.Normalize();
				}
				Renderer->ChangeAnimation("pl0000_Snatch", true);
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				Col_Attack->GetTransform()->AddWorldPosition(SnatchDir * _DeltaTime * SnatchSpeed);

				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (InputCheck == false) { return; }

				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
				Col_Attack->Off();
			}
			});

		// Snatch Pull
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Snatch_Pull,
			.Start = [=] {
				RotationToTarget();
				Renderer->ChangeAnimation("pl0000_Snatch_Pull");
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
			}
			});

		// Snatch Repel
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Snatch_Repel,
			.Start = [=] {
				RotationToTarget();
				Renderer->ChangeAnimation("pl0000_Snatch_Repel");
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (InputCheck == false) { return; }

				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }

				if (MoveCheck == false) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
			}
			});

		// Snatch Shoot Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Snatch_Shoot_Air,
			.Start = [=] {
				PhysXCapsule->TurnOffGravity();
				PhysXCapsule->SetLinearVelocityZero();
				RotationToTarget();
				WeaponIdle();
				Col_Attack->On();
				Col_Attack->SetAttackData(DamageType::Snatch, 0, std::bind(&PlayerActor_Nero::ChangeState, this, FSM_State_Nero::Nero_Snatch_Pull_Air));
				Col_Attack->GetTransform()->SetWorldPosition(GetTransform()->GetWorldPosition());
				Col_Attack->GetTransform()->SetLocalScale({ 100, 100, 100 });
				if (nullptr == LockOnEnemyTransform)
				{
					SnatchDir = GetTransform()->GetWorldForwardVector();
				}
				else
				{
					SnatchDir = LockOnEnemyTransform->GetWorldPosition() - GetTransform()->GetWorldPosition();
					SnatchDir.Normalize();
				}
				Renderer->ChangeAnimation("pl0000_Air_Snatch", true);
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				Col_Attack->GetTransform()->AddWorldPosition(SnatchDir * _DeltaTime * SnatchSpeed);

				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}

				if (InputCheck == false) { return; }

				if (true == Input_SpecialCheckFly()) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
				Col_Attack->Off();
			}
			});

		// Snatch Pull Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Snatch_Pull_Air,
			.Start = [=] {
				RotationToTarget();
				Renderer->ChangeAnimation("pl0000_Air_Snatch_Pull", true);
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}

				if (true == Input_SpecialCheckFly()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
			}
			});

		// Snatch Repel Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Snatch_Repel_Air,
			.Start = [=] {
				RotationToTarget();
				Renderer->ChangeAnimation("pl0000_Air_Snatch_Repel", true);
				InputCheck = false;
				MoveCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}

				if (InputCheck == false) { return; }

				if (true == Input_SpecialCheckFly()) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
			}
			});
	}
	// 특수
	{}
	{
		// DT Start
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_DT_Start,
		.Start = [=] {
			WeaponIdle();
			PhysXCapsule->SetLinearVelocityZero();
			SetInvincibility(0.5f);
			Col_Attack->SetAttackData(DamageType::Air, 150);
			EffectSystem->PlayFX("Nero_DT_On.effect");
			Renderer->ChangeAnimation("pl0000_DT_Start", true);
			InputCheck = false;
			MoveCheck = false;
		},
		.Update = [=](float _DeltaTime) {
			if (true == Input_SpecialCheck()) { return; }
			if (InputCheck == false) { return; }
			if (false == FloorCheck())
			{
				ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				return;
			}


			if (true == Input_JumpCheck()) { return; }
			if (true == Input_SwordCheck()) { return; }
			if (true == Input_GunCheck()) { return; }
			if (true == Input_DevilBreakerCheck()) { return; }

			if (MoveCheck == false) { return; }

			if (Controller->GetMoveVector() != float4::ZERO)
			{
				ChangeState(FSM_State_Nero::Nero_RunStart);
				return;
			}
		},
		.End = [=] {
		}
			});
		// DT Air Start
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_DT_AirStart,
			.Start = [=] {
			WeaponIdle();
				InputCheck = false;
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				SetInvincibility(0.5f);
				Col_Attack->SetAttackData(DamageType::Air, 150);
				EffectSystem->PlayFX("Nero_DT_On.effect");
				Renderer->ChangeAnimation("pl0000_DT_AirStart", true);
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}

				if (true == Input_SpecialCheckFly()) { return; }
				if (InputCheck == false) { return; }
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
			}
			});

		// GT Bomb
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_GT_Bomb,
		.Start = [=] {
			WeaponIdle();
			PhysXCapsule->SetLinearVelocityZero();
			SetInvincibility(0.5f);
			Col_Attack->SetAttackData(DamageType::Heavy, 150);
			EffectSystem->PlayFX("GT_Bomb.effect");
			Renderer->ChangeAnimation("pl0000_GT_Bomb", true);
			InputCheck = false;
		},
		.Update = [=](float _DeltaTime) {
			if (false == FloorCheck())
			{
				ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				return;
			}
			if (InputCheck == false) { return; }
			if (true == Input_SpecialCheck()) { return; }


			if (true == Input_JumpCheck()) { return; }
			if (true == Input_SwordCheck()) { return; }
			if (true == Input_GunCheck()) { return; }
			if (true == Input_DevilBreakerCheck()) { return; }
			if (Controller->GetMoveVector() != float4::ZERO)
			{
				ChangeState(FSM_State_Nero::Nero_RunStart);
				return;
			}
		},
		.End = [=] {
		}
			});
		// GT Bomb Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_GT_AirBomb,
			.Start = [=] {
				WeaponIdle();
				InputCheck = false;
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				SetInvincibility(0.5f);
				Col_Attack->SetAttackData(DamageType::Heavy, 150);
				EffectSystem->PlayFX("GT_Bomb.effect");
				Renderer->ChangeAnimation("pl0000_GT_AirBomb", true);
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}

				if (InputCheck == false) { return; }
				if (true == Input_SpecialCheckFly()) { return; }
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
			}
			});

		// EnemyStep
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_EnemyStep,
			.Start = [=] {
				WeaponIdle();
				UseDoubleJump = false;
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				SetInvincibility(0.5f);
				Renderer->ChangeAnimation("pl0000_EnemyStep", true);
			},
			.Update = [=](float _DeltaTime) {
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_EnemyStep_Jump);
					return;
				}
			},
			.End = [=] {
			}
			});


		// Provocation 1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Provocation,
		.Start = [=] {
			WeaponIdle();
			PhysXCapsule->SetLinearVelocityZero();
			Renderer->ChangeAnimation("pl0000_Provocation", true);
			InputCheck = false;
			MoveCheck = false;
		},
		.Update = [=](float _DeltaTime) {
			if (InputCheck == false) { return; }
			if (false == FloorCheck())
			{
				ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				return;
			}


			if (true == Input_SpecialCheck()) { return; }
			if (true == Input_JumpCheck()) { return; }
			if (true == Input_SwordCheck()) { return; }
			if (true == Input_GunCheck()) { return; }
			if (true == Input_DevilBreakerCheck()) { return; }

			if (MoveCheck == false) { return; }

			if (Controller->GetMoveVector() != float4::ZERO)
			{
				ChangeState(FSM_State_Nero::Nero_RunStart);
				return;
			}
		},
		.End = [=] {
		} });

		// Provocation Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Provocation_Air,
			.Start = [=] {
				WeaponIdle();
				InputCheck = false;
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				RotationToMoveVector();
				PhysXCapsule->SetMove(Controller->GetMoveVector() * 350);
				Renderer->ChangeAnimation("pl0000_Air_Provocation", true);
			},
			.Update = [=](float _DeltaTime) {
				PhysXCapsule->SetForce(Controller->GetMoveVector() * JumpMoveForce);
				if (Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
				}

				if (true == Input_SpecialCheckFly()) { return; }
				if (InputCheck == false) { return; }
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Landing);
					return;
				}
				if (true == Input_JumpCheckFly()) { return; }
				if (true == Input_SwordCheckFly()) { return; }
				if (true == Input_GunCheckFly()) { return; }
				if (true == Input_DevilBreakerCheckFly()) { return; }
			},
			.End = [=] {
				WeaponIdle();
			}
			});

	}
	// 대미지
	{}
	{
		// Damage Light
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Light,
			.Start = [=] {
				WeaponIdle();
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				Renderer->ChangeAnimation("pl0000_Damage_Common", true);
				InputCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
			}
			});

		// Damage Heavy
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Heavy,
			.Start = [=] {
				WeaponIdle();
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				Renderer->ChangeAnimation("pl0000_Damage_Away_Fly", true);
				InputCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (true == Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Damage_Fall);
					return;
				}
				if (InputCheck == false) { return; }
				if (true == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Damage_Ground);
					return;
				}
			},
			.End = [=] {
			}
			});

		// Damage Fly
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Fly,
			.Start = [=] {
				WeaponIdle();
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				Renderer->ChangeAnimation("pl0000_Damage_AirCombo_Fly", true);
			},
			.Update = [=](float _DeltaTime) {
				if (true == Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Damage_Fall);
					return;
				}
				if (true == FloorCheck())
				{
					if (HP <= 0)
					{
						ChangeState(FSM_State_Nero::Nero_Damage_Fly_Death);
						return;
					}
					ChangeState(FSM_State_Nero::Nero_Damage_Ground);
					return;
				}
			},
			.End = [=] {
			}
			});

		// Damage Fall
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Fall,
			.Start = [=] {
				PhysXCapsule->SetPush(float4::DOWN * FlyDownForce);
				PhysXCapsule->TurnOnGravity();
				Renderer->ChangeAnimation("pl0000_Damage_Supine_Fall_Loop");
			},
			.Update = [=](float _DeltaTime) {
				if (true == FloorCheck())
				{
					if (HP <= 0)
					{
						ChangeState(FSM_State_Nero::Nero_Damage_Fly_Death);
						return;
					}
					ChangeState(FSM_State_Nero::Nero_Damage_Ground);
					return;
				}
			},
			.End = [=] {
			}
			});

		// Damage Ground
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Ground,
			.Start = [=] {
				UseDoubleJump = false;
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				Renderer->ChangeAnimation("pl0000_Damage_Away_Ground");
				InputCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (true == Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Damage_GetUp);
					return;
				}
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (InputCheck == false) { return; }

				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
			},
			.End = [=] {
			}
			});

		// Damage GetUp
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_GetUp,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Damage_Away_GetUp");
			},
			.Update = [=](float _DeltaTime) {
				if (false == FloorCheck())
				{
					ChangeState(FSM_State_Nero::Nero_Jump_Fly);
					return;
				}

				if (true == Input_SpecialCheck()) { return; }
				if (true == Input_JumpCheck()) { return; }
				if (true == Input_SwordCheck()) { return; }
				if (true == Input_GunCheck()) { return; }
				if (true == Input_DevilBreakerCheck()) { return; }
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					ChangeState(FSM_State_Nero::Nero_RunStart);
					return;
				}
			},
			.End = [=] {
			}
			});

		// Damage Death1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Death_1,
			.Start = [=] {
				WeaponIdle();
				PhysXCapsule->SetLinearVelocityZero();
				PhysXCapsule->TurnOffGravity();
				Renderer->ChangeAnimation("pl0000_Damage_Kneel_Down_1", true);
			},
			.Update = [=](float _DeltaTime) {
				if (true == Renderer->IsAnimationEnd())
				{
					ChangeState(FSM_State_Nero::Nero_Damage_Death_2);
					return;
				}
			},
			.End = [=] {
			}
			});

		// Damage Death2
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Death_2,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Damage_Kneel_Down_GameOver", true);
			},
			.Update = [=](float _DeltaTime) {
				if (true == Renderer->IsAnimationEnd())
				{
					return;
				}
			},
			.End = [=] {
			}
			});

		// Damage Fly Death
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Fly_Death,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Damage_Away_Death_Ground", true);
			},
			.Update = [=](float _DeltaTime) {
				if (true == Renderer->IsAnimationEnd())
				{
					return;
				}
			},
			.End = [=] {
			}
			});

	}

	ChangeState(FSM_State_Nero::Nero_Idle);
}

void PlayerActor_Nero::NetLoad()
{
	// Effect 생성
	{
		EffectSystem = CreateComponent<FXSystem>();

		GameEngineDirectory NewDir;
		NewDir.MoveParentToDirectory("ContentResources");
		NewDir.Move("ContentResources");
		NewDir.Move("Effect");
		NewDir.Move("Mesh");
		if (nullptr == GameEngineFBXMesh::Find("Effect_Mesh_01.FBX"))
		{
			std::vector<GameEngineFile> Files = NewDir.GetAllFile({ ".fbx" });
			for (GameEngineFile File : Files)
			{
				GameEngineFBXMesh::Load(File.GetFullPath());
			}
		}
		NewDir.MoveParent();
		NewDir.Move("Texture");
		if (nullptr == GameEngineTexture::Find("Effect_Texture_01.tga"))
		{
			std::vector<GameEngineFile> Files = NewDir.GetAllFile({ ".tga" });
			for (GameEngineFile File : Files)
			{
				GameEngineTexture::Load(File.GetFullPath());
			}
		}

		// 이펙트 Sprite 로드
		if (nullptr == GameEngineSprite::Find("Effect_Impact.tga"))
		{
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Impact.tga").GetFullPath(), 8, 8);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Impact_01.tga").GetFullPath(), 8, 4);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Muzzle_03.tga").GetFullPath(), 2, 1);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Spark_02.tga").GetFullPath(), 8, 8);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Magic_01.tga").GetFullPath(), 8, 8);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Fire_01.tga").GetFullPath(), 8, 4);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Fire_02.tga").GetFullPath(), 8, 4);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Fire_03.tga").GetFullPath(), 8, 4);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Fire_04.tga").GetFullPath(), 8, 8);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Fire_05.tga").GetFullPath(), 8, 8);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Explosion_01.tga").GetFullPath(), 8, 8);
			GameEngineSprite::LoadSheet(NewDir.GetPlusFileName("Effect_Explosion_02.tga").GetFullPath(), 8, 8);
		}
		NewDir.MoveParent();
		NewDir.Move("Nero");

		std::vector<GameEngineFile> Files = NewDir.GetAllFile({ ".effect" });
		for (GameEngineFile File : Files)
		{
			if (nullptr == FXData::Find(File.GetFileName()))
			{
				FXData::Load(File.GetFullPath());
			}
			EffectSystem->CreateFX(FXData::Find(File.GetFileName()));
		}
	}

	// Renderer 생성
	{
		GameEngineDirectory NewDir;
		NewDir.MoveParentToDirectory("ContentResources");
		NewDir.Move("ContentResources");
		NewDir.Move("Character");
		NewDir.Move("Player");
		NewDir.Move("Nero");
		NewDir.Move("Mesh");
		if (nullptr == GameEngineFBXMesh::Find("Nero.FBX"))
		{
			GameEngineFBXMesh::Load(NewDir.GetPlusFileName("Nero.fbx").GetFullPath());
			GameEngineTexture::Load(NewDir.GetPlusFileName("pl0010_03_atos.texout.png").GetFullPath());
			GameEngineTexture::Load(NewDir.GetPlusFileName("pl0000_03_atos.texout.png").GetFullPath());
		}
		NewDir.MoveParent();
		NewDir.Move("Animation");

		Renderer = CreateComponent<GameEngineFBXRenderer>();
		Renderer->GetTransform()->SetLocalRotation({ 0, 0, 0 });
		Renderer->GetTransform()->SetLocalPosition({ 0, -75, 0 });

		switch (GameEngineOption::GetOption("Shader"))
		{
		case GameEngineOptionValue::Low:
		{
			Renderer->SetFBXMesh("Nero.FBX", "AniFBX_Low");
		}
		break;
		case GameEngineOptionValue::High:
		{
			Renderer->SetFBXMesh("Nero.FBX", "AniFBX");
		}
		break;
		default:
			break;
		}

		Renderer->SetMaterial("pl0000_03_albm.texout.png", "AniFBX_Alpha", nullptr);

		Renderer->ShadowOn();
		Renderer->SetDynamic();

		Renderer->SetSpecularTexture("pl0000_03_albm.texout.png", "pl0000_03_atos.texout.png");
		Renderer->SetSpecularTexture("pl0010_03_albm.texout.png", "pl0010_03_atos.texout.png");

		AnimationEvent::LoadAll({ .Dir = NewDir.GetFullPath().c_str(), .Renderer = Renderer,
			.Objects = { (GameEngineObject*)Col_Attack.get() },
			.CallBacks_void = {
				nullptr,
				std::bind(&PlayerActor_Nero::RedQueenOn, this),
				std::bind(&PlayerActor_Nero::RedQueenOff, this),
				std::bind(&PlayerActor_Nero::BlueRoseOn, this),
				std::bind(&PlayerActor_Nero::WeaponIdle, this),
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				std::bind(&PlayerActor_Nero::SetOverture, this),
				std::bind(&PlayerActor_Nero::SetHuman, this),
				std::bind(&PlayerActor_Nero::SetDemon, this),
				nullptr,
			},
			.CallBacks_int = {
				std::bind(&GameEngineFSM::ChangeState, &FSM, std::placeholders::_1)
			},
			});

		//Object = 0 : 공격 충돌체
		//
		//콜백void = 0 : 입력체크시작
		//콜백void = 1 : 손에 레드퀸 들려줌
		//콜백void = 2 : 손에 레드퀸 뺌
		//콜백void = 3 : 손에 블루로즈
		//콜백void = 4 : WeaponIdle (빈손, 칼 등에)
		//콜백void = 5 : SetLinearVelocityZero
		//콜백void = 6 : 이동체크 시작
		//콜백void = 7 : 딜레이체크 시작k
		//콜백void = 8 : 중력 적용
		//콜백void = 9 : SetOverture
		//콜백void = 10 : SetHuman
		//콜백void = 11 : SetDemon
		//콜백void = 12 : 데빌브레이커 파괴
		//
		//콜백 int = 0 : FSM변경
		// 
		//콜백 float4 = 0 : SetForce
		//콜백 float4 = 0 : SetPush
		//콜백 float4 = 0 : SetMove

	}

	// OvertureRenderer 생성
	{
		GameEngineDirectory NewDir;
		NewDir.MoveParentToDirectory("ContentResources");
		NewDir.Move("ContentResources");
		NewDir.Move("Character");
		NewDir.Move("Player");
		NewDir.Move("Nero");
		NewDir.Move("Overture");
		NewDir.Move("Mesh");
		if (nullptr == GameEngineFBXMesh::Find("Overture.FBX"))
		{
			GameEngineFBXMesh::Load(NewDir.GetPlusFileName("Overture.fbx").GetFullPath());
		}
		Renderer_Overture = CreateComponent<GameEngineFBXRenderer>();
		Renderer_Overture->GetTransform()->SetLocalPosition({ 0, -75, 0 });
		switch (GameEngineOption::GetOption("Shader"))
		{
		case GameEngineOptionValue::Low:
		{
			Renderer_Overture->SetFBXMesh("Overture.FBX", "AniFBX_Low");
		}
		break;
		case GameEngineOptionValue::High:
		{
			Renderer_Overture->SetFBXMesh("Overture.FBX", "AniFBX");
		}
		break;
		default:
			break;
		}

		Renderer_Overture->ShadowOn();
		Renderer_Overture->SetDynamic();

		NewDir.MoveParent();
		NewDir.Move("Animation");

		std::vector<GameEngineFile> Files = NewDir.GetAllFile({ ".fbx" });
		for (GameEngineFile File : Files)
		{
			if (nullptr == GameEngineFBXAnimation::Find(File.GetFileName()))
			{
				GameEngineFBXAnimation::Load(File.GetFullPath());
			}
			Renderer_Overture->CreateFBXAnimation(File.GetFileName(), { .Inter = 0.0166f, .Loop = false });
		}
		Renderer_Overture->ChangeAnimation("wp00_010_Shoot.fbx");
		Renderer_Overture->Off();
	}

	SetHuman();
	WeaponIdle();

	// 기본 움직임
	{
		// Idle
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Idle,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Idle_Normal");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {

			}
			});
		// Run Start
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RunStart,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Run_Start");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
		// Run
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Run,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Run_Loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
		// RunStop
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RunStop,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Run_Stop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {

			}
			});
		// Dash
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Dash,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Dash_Loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {

			}
			});
		// DashStop
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_DashStop,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Dash_Stop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
		// Jump_Vertical
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Jump_Vertical,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_Vertical");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
		// Jump_Back
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Jump_Back,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_Back");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
		// Jump Fly
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Jump_Fly,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Jump_Fly_loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
		// Landing
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Landing,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_Landing");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {

			}
			});
		// 2nd_Jump
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_2nd_Jump,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_2ndJump", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
		// 2nd_Jump_Back
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_2nd_Jump_Back,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_Back_2ndJump", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
		// EnemyStep Jump
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_EnemyStep_Jump,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_EnemyStep", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
	}
	{}
	// 레드 퀸
	{
		// RedQueen ComboA1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboA_1,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(120));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboA1_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboA1.effect");
				}
				Renderer->ChangeAnimation("pl0000_RQ_ComboA_1");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboA2
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboA_2,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(90));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboA2_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboA2.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboA_2");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboA3
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboA_3,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(60));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboA3_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboA3.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboA_3");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboA4
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboA_4,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Heavy, DamageCalculate(150));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboA4_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboA4.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboA_4");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboD1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboD_1,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(60));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboD1_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboD1.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboD_1");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboD2
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboD_2,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(90));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboD2_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboD2.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboD_2");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboD3
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboD_3,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(60));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboD3_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboD3.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboD_3");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen ComboD4
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_ComboD_4,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Heavy, DamageCalculate(150));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_ComboD4_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_ComboD4.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboD_4");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen HR
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_HR,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Air, DamageCalculate(135, true));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_HR_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_HR.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_HR-EX");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				RedQueenOff();
				WeaponIdle();
			}
			});
		// RedQueen Shuffle
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Shuffle,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Heavy, DamageCalculate(180, true));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_Shuffle_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_Shuffle.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Shuffle", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen Stleak1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Stleak1,
			.Start = [=] {
				EffectSystem->PlayFX("RQ_Stleak_1.effect");
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Stleak_1");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				RedQueenOff();
			}
			});
		// RedQueen Stleak2
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Stleak2,
			.Start = [=] {
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Stleak_2_Loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				RedQueenOff();
			}
			});
		// RedQueen Stleak3
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Stleak3,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Heavy, DamageCalculate(180, true));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_Stleak_3_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_Stleak_3.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Stleak_3");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				RedQueenOff();
			}
			});
		// RedQueen AirComboA1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_AirComboA_1,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(150));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_AirComboA1_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_AirComboA1.effect");
				}
				Renderer->ChangeAnimation("pl0000_RQ_AT-Jump_1");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen AirComboA2
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_AirComboA_2,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Light, DamageCalculate(120));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_AirComboA2_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_AirComboA2.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_AT-Jump_2");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen AirComboA3
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_AirComboA_3,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Slam, DamageCalculate(180));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_AirComboA3_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_AirComboA3.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_AT-Jump_3");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen AirComboB
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_AirComboB,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Air, DamageCalculate(405));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_AirComboB_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_AirComboB.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_AirComboB");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen Split_1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Split_1,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Slam, DamageCalculate(22, true));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_Split_1_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_Split_1.effect");
				}
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Split_1");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
		// RedQueen Split_2
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Split_2,
			.Start = [=] {
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_Split_2_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_Split_2.effect");
				}
				EffectSystem->Loop = true;
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Split_2_Loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				EffectSystem->Loop = false;
			}
			});
		// RedQueen Split_3
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Split_3,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Heavy, DamageCalculate(90, true));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_Split_3_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_Split_3.effect");
				}
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Split_3");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// RedQueen Caliber_1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Caliber_1,
			.Start = [=] {
				EffectSystem->PlayFX("RQ_Calibur_1.effect");
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Caliber_1");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
		// RedQueen Caliber_2
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_RQ_Skill_Caliber_2,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Heavy, DamageCalculate(180, true));
				if (0 < ExceedLevel)
				{
					EffectSystem->PlayFX("RQ_Calibur_2_EX.effect");
				}
				else
				{
					EffectSystem->PlayFX("RQ_Calibur_2.effect");
				}
				Renderer->ChangeAnimation("pl0000_RQ_Skill_Caliber_2");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				RedQueenOff();
			}
			});


	}
	{}
	// 블루 로즈 (락온)
	{
		// Idle To LockOn
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Switch_Idle_to_Lockon,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_BR_Switch_Idle_to_Lockon");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {

			}
			});
		// LockOnFront
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Lockon_Front,
			.Start = [=] {
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_BR_Lockon_Front");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// Strafe
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Strafe,
			.Start = [=] {
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_Strafe_F_Loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			} });
		// StrafeF
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Strafe_F,
			.Start = [=] {
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_Strafe_F_Loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			} });
		// StrafeFL
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Strafe_FL,
			.Start = [=] {
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_Strafe_FL_Loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			} });
		// StrafeL
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Strafe_L,
			.Start = [=] {
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_Strafe_L_Loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			} });
		// StrafeBL
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Strafe_BL,
			.Start = [=] {
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_Strafe_BL_Loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			} });
		// StrafeB
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Strafe_B,
			.Start = [=] {
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_Strafe_B_Loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			} });
		// StrafeBR
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Strafe_BR,
			.Start = [=] {
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_Strafe_BR_Loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			} });
		// StrafeR
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Strafe_R,
			.Start = [=] {
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_Strafe_R_Loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			} });
		// LockOn To Idle
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Switch_Lockon_to_Idle,
			.Start = [=] {
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_BR_Switch_Lockon_to_Idle");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// Evade
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Evade_Left,
			.Start = [=] {
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_Evade_Left", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// Evade
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Evade_Right,
			.Start = [=] {
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_Evade_Right", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// Shoot
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_Shoot,
			.Start = [=] {
				// 아직 미완성
				EffectSystem->PlayFX("BR_Shoot.effect");
				Col_Attack->SetAttackData(DamageType::Light, 54, std::bind(&GameEngineObjectBase::Off, Col_Attack));
				Col_Attack->GetTransform()->SetLocalPosition({ 0, 100, 1000 });
				Col_Attack->GetTransform()->SetLocalScale({ 50, 50, 2000 });
				Col_Attack->On();
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_BR_Shoot", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
		// Air Shoot
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_BR_AirShoot,
			.Start = [=] {
				// 아직 미완성
				EffectSystem->PlayFX("BR_Shoot.effect");
				Col_Attack->SetAttackData(DamageType::Light, 54, std::bind(&GameEngineObjectBase::Off, Col_Attack));
				Col_Attack->GetTransform()->SetLocalPosition({ 0, 100, 1000 });
				Col_Attack->GetTransform()->SetLocalScale({ 50, 50, 2000 });
				Col_Attack->On();
				BlueRoseOn();
				Renderer->ChangeAnimation("pl0000_BR_Air_Shoot", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});
	}
	// 오버추어
	{}
	{
		// Nero_Overture_Shoot
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Overture_Shoot,
			.Start = [=] {
				WeaponIdle();
				SetOvertureAnimation();
				Col_Attack->SetAttackData(DamageType::Heavy, 420);
				EffectSystem->PlayFX("Overture_Shoot.effect");
				Renderer->ChangeAnimation("pl0000_Overture_Shoot", true);
				Renderer_Overture->ChangeAnimation("wp00_010_Shoot.fbx", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				SetOverture();
				WeaponIdle();
			}
			});
		// Nero_Overture_AirShoot
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Overture_AirShoot,
			.Start = [=] {
				WeaponIdle();
				SetOvertureAnimation();
				Col_Attack->SetAttackData(DamageType::Heavy, 420);
				EffectSystem->PlayFX("Overture_Shoot.effect");
				Renderer->ChangeAnimation("pl0000_Overture_Air_Shoot", true);
				Renderer_Overture->ChangeAnimation("wp00_010_Air_Shoot.fbx", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				SetOverture();
				WeaponIdle();
			}
			});
	}
	// 거베라
	{}
	{
		// Nero_Gerbera_Back
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Gerbera_Back,
			.Start = [=] {
				WeaponIdle();
				Col_Attack->SetAttackData(DamageType::Heavy, 75);
				EffectSystem->PlayFX("Gerbera_Jump_Back.effect");
				Renderer->ChangeAnimation("pl0000_Gerbera_Jump_Back", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// Nero_Gerbera_Front
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Gerbera_Front,
			.Start = [=] {
				WeaponIdle();
				Col_Attack->SetAttackData(DamageType::Heavy, 75);
				EffectSystem->PlayFX("Gerbera_Jump_Back.effect");
				EffectSystem->GetTransform()->SetLocalRotation({ 0, 180, 0 });
				Renderer->ChangeAnimation("pl0000_Gerbera_Jump_Front", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				EffectSystem->GetTransform()->SetLocalRotation(float4::ZERO);
			}
			});

		// Nero_Gerbera_Left
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Gerbera_Left,
			.Start = [=] {
				WeaponIdle();
				Col_Attack->SetAttackData(DamageType::Heavy, 75);
				EffectSystem->PlayFX("Gerbera_Jump_Back.effect");
				EffectSystem->GetTransform()->SetLocalRotation({ 0, 90, 0 });
				Renderer->ChangeAnimation("pl0000_Gerbera_Jump_Left", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				EffectSystem->GetTransform()->SetLocalRotation(float4::ZERO);
			}
			});

		// Nero_Gerbera_Right
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Gerbera_Right,
			.Start = [=] {
				WeaponIdle();
				Col_Attack->SetAttackData(DamageType::Heavy, 75);
				EffectSystem->PlayFX("Gerbera_Jump_Back.effect");
				EffectSystem->GetTransform()->SetLocalRotation({ 0, -90, 0 });
				Renderer->ChangeAnimation("pl0000_Gerbera_Jump_Right", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				EffectSystem->GetTransform()->SetLocalRotation(float4::ZERO);
			}
			});
	}
	// 버스터암
	{}
	{
		// BusterArm Catch
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Buster_Catch,
			.Start = [=] {
				// 아직 미완성
				Col_Attack->SetAttackData(DamageType::Buster, 0);
				WeaponIdle();
				EffectSystem->PlayFX("Buster_Catch.effect");
				Renderer->ChangeAnimation("pl0000_Buster_Catch", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				Col_Attack->Off();
			}
			});

		// BusterArm Strike
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Buster_Strike,
			.Start = [=] {
				EffectSystem->PlayFX("Buster_Strike.effect");
				Renderer->ChangeAnimation("pl0000_Buster_Strike_Common");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// BusterArm Repelled
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Buster_Repelled,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Buster_Catch_Repelled");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// BusterArm Catch Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Buster_Catch_Air,
			.Start = [=] {
				Col_Attack->SetAttackData(DamageType::Buster, 50);
				WeaponIdle();
				EffectSystem->PlayFX("Buster_Catch.effect");
				Renderer->ChangeAnimation("pl0000_Buster_Air_Catch", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				Col_Attack->Off();
			}
			});

		// BusterArm Strike Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Buster_Strike_Air,
			.Start = [=] {
				EffectSystem->PlayFX("Buster_Strike.effect");
				Renderer->ChangeAnimation("pl0000_Buster_Air_Strike_Common");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// BusterArm Repelled Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Buster_Repelled_Air,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Buster_Air_Catch_Repelled");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
	}
	// 스내치
	{}
	{
		static float4 SnatchDir;
		const static float SnatchSpeed = 4000.0f;
		// Snatch Shoot
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Snatch_Shoot,
			.Start = [=] {
				// 아직 미완성
				WeaponIdle();
				Col_Attack->On();
				Col_Attack->SetAttackData(DamageType::Snatch, 0);
				Col_Attack->GetTransform()->SetWorldPosition(GetTransform()->GetWorldPosition());
				Col_Attack->GetTransform()->SetLocalScale({100, 100, 100});
				SnatchDir = GetTransform()->GetWorldForwardVector();
				Renderer->ChangeAnimation("pl0000_Snatch", true);
			},
			.Update = [=](float _DeltaTime) {
				Col_Attack->GetTransform()->AddWorldPosition(SnatchDir * _DeltaTime * SnatchSpeed);
			},
			.End = [=] {
				Col_Attack->Off();
			}
			});

		// Snatch Pull
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Snatch_Pull,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Snatch_Pull");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// Snatch Repel
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Snatch_Repel,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Snatch_Repel");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// Snatch Shoot Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Snatch_Shoot_Air,
			.Start = [=] {
				// 아직 미완성
				WeaponIdle();
				Col_Attack->On();
				Col_Attack->SetAttackData(DamageType::Snatch, 0);
				Col_Attack->GetTransform()->SetWorldPosition(GetTransform()->GetWorldPosition());
				Col_Attack->GetTransform()->SetLocalScale({ 100, 100, 100 });
				SnatchDir = GetTransform()->GetWorldForwardVector();
				Renderer->ChangeAnimation("pl0000_Air_Snatch", true);
			},
			.Update = [=](float _DeltaTime) {
				Col_Attack->GetTransform()->AddWorldPosition(SnatchDir * _DeltaTime * SnatchSpeed);
			},
			.End = [=] {
				Col_Attack->Off();
			}
			});

		// Snatch Pull Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Snatch_Pull_Air,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Air_Snatch_Pull", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// Snatch Repel Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Snatch_Repel_Air,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Air_Snatch_Repel", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
	}
	// 특수
	{}
	{
		// DT Start
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_DT_Start,
		.Start = [=] {
			WeaponIdle();
			Col_Attack->SetAttackData(DamageType::Air, 150);
			EffectSystem->PlayFX("Nero_DT_On.effect");
			Renderer->ChangeAnimation("pl0000_DT_Start", true);
		},
		.Update = [=](float _DeltaTime) {
		},
		.End = [=] {
		}
			});
		// DT Air Start
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_DT_AirStart,
			.Start = [=] {
				WeaponIdle();
				Col_Attack->SetAttackData(DamageType::Air, 150);
				EffectSystem->PlayFX("Nero_DT_On.effect");
				Renderer->ChangeAnimation("pl0000_DT_AirStart", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// GT Bomb
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_GT_Bomb,
		.Start = [=] {
			WeaponIdle();
			Col_Attack->SetAttackData(DamageType::Heavy, 150);
			EffectSystem->PlayFX("GT_Bomb.effect");
			Renderer->ChangeAnimation("pl0000_GT_Bomb", true);
		},
		.Update = [=](float _DeltaTime) {
		},
		.End = [=] {
		}
			});
		// GT Bomb Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_GT_AirBomb,
			.Start = [=] {
				WeaponIdle();
				Col_Attack->SetAttackData(DamageType::Heavy, 150);
				EffectSystem->PlayFX("GT_Bomb.effect");
				Renderer->ChangeAnimation("pl0000_GT_AirBomb", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// EnemyStep
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_EnemyStep,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_EnemyStep", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});


		// Provocation 1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Provocation,
		.Start = [=] {
			WeaponIdle();
			Renderer->ChangeAnimation("pl0000_Provocation", true);
		},
		.Update = [=](float _DeltaTime) {
		},
		.End = [=] {
		} });

		// Provocation Air
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Provocation_Air,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Air_Provocation", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
				WeaponIdle();
			}
			});

	}
	// 대미지
	{}
	{
		// Damage Light
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Light,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Damage_Common", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// Damage Heavy
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Heavy,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Damage_Away_Fly", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// Damage Fly
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Fly,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Damage_AirCombo_Fly", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// Damage Fall
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Fall,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Damage_Supine_Fall_Loop");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// Damage Ground
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Ground,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Damage_Away_Ground");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// Damage GetUp
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_GetUp,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Damage_Away_GetUp");
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// Damage Death1
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Death_1,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Damage_Kneel_Down_1", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// Damage Death2
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Death_2,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Damage_Kneel_Down_GameOver", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});

		// Damage Fly Death
		FSM.CreateState({ .StateValue = FSM_State_Nero::Nero_Damage_Fly_Death,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_Damage_Away_Death_Ground", true);
			},
			.Update = [=](float _DeltaTime) {
			},
			.End = [=] {
			}
			});
	}

	FSM.ChangeState(FSM_State_Nero::Nero_Idle);
}

void PlayerActor_Nero::Update_Character(float _DeltaTime)
{
	if (LoadCheck == false) { return; }
	FSM.Update(_DeltaTime);

	if (NetControllType::ActiveControll == GameEngineNetObject::GetControllType())
	{
		if (GameEngineInput::IsDown("SelectLevel_01"))
		{
			AddBreaker(DevilBreaker::Overture);
		}
		if (GameEngineInput::IsDown("SelectLevel_02"))
		{
			AddBreaker(DevilBreaker::Gerbera);
		}
		if (GameEngineInput::IsDown("SelectLevel_03"))
		{
			AddBreaker(DevilBreaker::BusterArm);
		}
		if (GameEngineInput::IsDown("SelectLevel_04"))
		{
			HP -= 500;
			HPRender->SetPlayerHP(HP);
			LightDamage();
		}
		if (GameEngineInput::IsDown("SelectLevel_05"))
		{
			HP -= 500;
			HPRender->SetPlayerHP(HP);
			HeavyDamage();
		}
	}
}

void PlayerActor_Nero::LightDamage()
{
	if (true == FloorCheck())
	{
		if (HP <= 0)
		{
			ChangeState(FSM_State_Nero::Nero_Damage_Death_1);
			return;
		}
		ChangeState(FSM_State_Nero::Nero_Damage_Light);
		return;
	}
	else
	{
		ChangeState(FSM_State_Nero::Nero_Damage_Fly);
	}
}
void PlayerActor_Nero::HeavyDamage()
{
	if (true == FloorCheck())
	{
		if (HP <= 0)
		{
			ChangeState(FSM_State_Nero::Nero_Damage_Death_1);
			return;
		}
		ChangeState(FSM_State_Nero::Nero_Damage_Heavy);
	}
	else
	{
		ChangeState(FSM_State_Nero::Nero_Damage_Fly);
	}
}
bool PlayerActor_Nero::Input_SwordCheck(int AddState /*= 0*/)
{
	if (false == Controller->GetIsLockOn())
	{
		if (Controller->GetIsAnySword())
		{
			ChangeState(AddState + Nero_RQ_ComboA_1);
			return true;
		}
		return false;
	}

	if (Controller->GetIsBackFrontSword())
	{
		ChangeState(FSM_State_Nero::Nero_RQ_Skill_Shuffle);
		return true;
	}
	if (Controller->GetIsFrontSword())
	{
		ChangeState(FSM_State_Nero::Nero_RQ_Skill_Stleak1);
		return true;
	}
	if (Controller->GetIsBackSword())
	{
		ChangeState(FSM_State_Nero::Nero_RQ_Skill_HR);
		return true;
	}
	if (Controller->GetIsSword())
	{
		ChangeState(AddState + Nero_RQ_ComboA_1);
		return true;
	}
	return false;
}

bool PlayerActor_Nero::Input_SwordCheckFly(int AddState /*= 0*/)
{
	if (false == Controller->GetIsLockOn())
	{
		if (Controller->GetIsAnySword())
		{
			ChangeState(AddState + Nero_RQ_AirComboA_1);
			return true;
		}
		return false;
	}

	if (Controller->GetIsBackFrontSword())
	{
		ChangeState(FSM_State_Nero::Nero_RQ_Skill_Caliber_1);
		return true;
	}
	if (Controller->GetIsBackSword())
	{
		ChangeState(FSM_State_Nero::Nero_RQ_Skill_Split_1);
		return true;
	}
	if (Controller->GetIsSword())
	{
		ChangeState(AddState + Nero_RQ_AirComboA_1);
		return true;
	}
	return false;
}

bool PlayerActor_Nero::Input_GunCheck()
{
	if (Controller->GetIsGunUp())
	{
		ChangeState(FSM_State_Nero::Nero_BR_Shoot);
		return true;
	}
	return false;
}

bool PlayerActor_Nero::Input_GunCheckFly()
{
	if (Controller->GetIsGunUp())
	{
		ChangeState(FSM_State_Nero::Nero_BR_AirShoot);
		return true;
	}
	return false;
}

bool PlayerActor_Nero::Input_JumpCheck()
{
	if (Controller->GetIsProvocation())
	{
		ChangeState(FSM_State_Nero::Nero_Provocation);
		return true;
	}

	if (false == Controller->GetIsLockOn())
	{
		if (Controller->GetIsAnyJump())
		{
			if ('2' == Controller->MoveVectorToChar4(Controller->GetMoveVector()))
			{
				ChangeState(FSM_State_Nero::Nero_Jump_Back);
				return true;
			}
			ChangeState(FSM_State_Nero::Nero_Jump_Vertical);
			return true;
		}
		return false;
	}


	if (Controller->GetIsLeftJump())
	{
		ChangeState(FSM_State_Nero::Nero_Evade_Left);
		return true;
	}
	if (Controller->GetIsRightJump())
	{
		ChangeState(FSM_State_Nero::Nero_Evade_Right);
		return true;
	}
	if (Controller->GetIsJump())
	{
		if ('2' == Controller->MoveVectorToChar4(Controller->GetMoveVector()))
		{
			ChangeState(FSM_State_Nero::Nero_Jump_Back);
			return true;
		}
		ChangeState(FSM_State_Nero::Nero_Jump_Vertical);
		return true;
	}
	return false;
}

bool PlayerActor_Nero::Input_JumpCheckFly()
{
	if (Controller->GetIsProvocation())
	{
		ChangeState(FSM_State_Nero::Nero_Provocation_Air);
		return true;
	}

	if (true == UseDoubleJump)
	{
		return false;
	}
	if (Controller->GetIsAnyJump())
	{
		UseDoubleJump = true;
		if ('2' == Controller->MoveVectorToChar4(Controller->GetMoveVector()))
		{
			ChangeState(FSM_State_Nero::Nero_2nd_Jump_Back);
			return true;
		}
		ChangeState(FSM_State_Nero::Nero_2nd_Jump);
		return true;
	}
	return false;
}

bool PlayerActor_Nero::Input_DevilBreakerCheck()
{
	if (true == Controller->GetIsSkill())
	{
		if (CurDevilBreaker == DevilBreaker::None)
		{
			return false;
		}

		switch (CurDevilBreaker)
		{
		case DevilBreaker::Overture:
			ChangeState(FSM_State_Nero::Nero_Overture_Shoot);
			break;
		case DevilBreaker::Gerbera:
		{
			char Dir = Controller->MoveVectorToChar4(Controller->GetMoveVector());
			if (Dir == '4')
			{
				ChangeState(FSM_State_Nero::Nero_Gerbera_Right);
			}
			else if (Dir == '6')
			{
				ChangeState(FSM_State_Nero::Nero_Gerbera_Left);
			}
			else if (Dir == '2')
			{
				ChangeState(FSM_State_Nero::Nero_Gerbera_Front);
			}
			else
			{
				ChangeState(FSM_State_Nero::Nero_Gerbera_Back);
			}
			break;
		}
		case DevilBreaker::BusterArm:
			ChangeState(FSM_State_Nero::Nero_Buster_Catch);
			break;
		}
		return true;
	}

	if (true == Controller->GetIsLockOnSkill())
	{
		ChangeState(FSM_State_Nero::Nero_Snatch_Shoot);
		return true;
	}

	return false;
}

bool PlayerActor_Nero::Input_DevilBreakerCheckFly()
{
	if (true == Controller->GetIsSkill())
	{

		if (CurDevilBreaker == DevilBreaker::None)
		{
			return false;
		}

		switch (CurDevilBreaker)
		{
		case DevilBreaker::Overture:
			ChangeState(FSM_State_Nero::Nero_Overture_AirShoot);
			break;
		case DevilBreaker::Gerbera:
		{
			char Dir = Controller->MoveVectorToChar4(Controller->GetMoveVector());
			if (Dir == '4')
			{
				ChangeState(FSM_State_Nero::Nero_Gerbera_Right);
			}
			else if (Dir == '6')
			{
				ChangeState(FSM_State_Nero::Nero_Gerbera_Left);
			}
			else if (Dir == '2')
			{
				ChangeState(FSM_State_Nero::Nero_Gerbera_Front);
			}
			else
			{
				ChangeState(FSM_State_Nero::Nero_Gerbera_Back);
			}
			break;
		}
		case DevilBreaker::BusterArm:
			ChangeState(FSM_State_Nero::Nero_Buster_Catch_Air);
			break;
		}
		return true;
	}

	if (true == Controller->GetIsLockOnSkill())
	{
		ChangeState(FSM_State_Nero::Nero_Snatch_Shoot_Air);
		return true;
	}

	return false;
}

bool PlayerActor_Nero::Input_SpecialCheck()
{
	// 익시드 체크
	if (GameEngineInput::IsDown("Player_Exceed"))
	{
		if (true == IsMaxActTiming)
		{
			ExceedLevel = 3;
		}
		else if (true == IsExActTiming)
		{
			ExceedLevel = std::clamp(ExceedLevel + 1, 1, 3);
		}
		IsMaxActTiming = false;
		IsExActTiming = false;
	}


	if (Controller->GetIsDevilTrigger())
	{
		if (false == DTValue)
		{
			DTValue = true;
			ChangeState(FSM_State_Nero::Nero_DT_Start);
			return true;
		}
		DTValue = false;
		SetHuman();
		SetOverture();
		return true;
	}
	if (Controller->GetIsGTBomb() && CurDevilBreaker != DevilBreaker::None)
	{
		ChangeState(FSM_State_Nero::Nero_GT_Bomb);
		return true;
	}

	return false;
}

bool PlayerActor_Nero::Input_SpecialCheckFly()
{
	// 익시드 체크
	if (GameEngineInput::IsDown("Player_Exceed"))
	{
		if (true == IsMaxActTiming)
		{
			ExceedLevel = 3;
		}
		else if (true == IsExActTiming)
		{
			ExceedLevel = std::clamp(ExceedLevel + 1, 1, 3);
		}
		IsMaxActTiming = false;
		IsExActTiming = false;
	}

	if (Controller->GetIsAnyJump())
	{
		std::vector<std::shared_ptr<GameEngineCollision>> Cols;
		if (true == Col_EnemyStepCheck->CollisionAll(CollisionOrder::Enemy, Cols))
		{
			ChangeState(FSM_State_Nero::Nero_EnemyStep);
			return true;
		}
		if (false == UseDoubleJump && PhysXCapsule->GetIsPlayerWallTouch())
		{
			ChangeState(FSM_State_Nero::Nero_EnemyStep);
			UseDoubleJump = true;
			return true;
		}
	}
	if (Controller->GetIsDevilTrigger())
	{
		if (false == DTValue)
		{
			DTValue = true;
			ChangeState(FSM_State_Nero::Nero_DT_AirStart);
			return true;
		}
		DTValue = false;
		SetHuman();
		SetOverture();
		return true;
	}
	if (Controller->GetIsGTBomb() && CurDevilBreaker != DevilBreaker::None)
	{
		ChangeState(FSM_State_Nero::Nero_GT_AirBomb);
		return true;
	}

	return false;
}

//void PlayerActor_Nero::ChangeState(FSM_State_Nero _StateValue)
//{
//	FSM.ChangeState(_StateValue);
//	FSMValue = _StateValue;
//	NetworkManager::SendFsmChangePacket(this, _StateValue);
//}

void PlayerActor_Nero::ChangeState(int _StateValue)
{
	FSM.ChangeState(_StateValue);
	FSMValue = _StateValue;
	NetworkManager::SendFsmChangePacket(this, _StateValue);
}

void PlayerActor_Nero::RedQueenOn()
{
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_RQHand]->On();	// 손 레드퀸
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_RQBack]->Off();	// 등 레드퀸
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BR]->Off();	// 블루로즈
}

void PlayerActor_Nero::RedQueenOff()
{
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_RQHand]->Off();	// 손 레드퀸
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_RQBack]->On();	// 등 레드퀸
}

void PlayerActor_Nero::BlueRoseOn()
{
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_RQHand]->Off();	// 손 레드퀸
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_RQBack]->On();	// 등 레드퀸
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BR]->On();	// 블루로즈
}

void PlayerActor_Nero::WeaponIdle()
{
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_RQHand]->Off();	// 손 레드퀸
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_RQBack]->On();	// 등 레드퀸
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BR]->Off();	// 블루로즈
	Renderer_Overture->Off();

	if (nullptr != Col_Attack)
	{
		Col_Attack->Off();
	}
	if (nullptr != EffectSystem)
	{
		EffectSystem->Off();
	}

	if (true == DTValue)
	{
		return;
	}
	OnDevilBraeker();
}

void PlayerActor_Nero::SetHuman()
{
	DTValue = 0;
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human0]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human1]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human2]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human3]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human4]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human5]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human6]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human7]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human8]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human9]->On();

	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT0]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT1]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT2]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT3]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT4]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT5]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT6]->Off();
}

void PlayerActor_Nero::SetDemon()
{
	DTValue = 1;
	ArmValue = 0;

	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human0]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human1]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human2]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human3]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human4]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human5]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human6]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human7]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human8]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Human9]->Off();

	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT0]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT1]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT2]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT3]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT4]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT5]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_DT6]->On();

	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BusterArm0]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BusterArm1]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Gerbera0]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Gerbera1]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Overture]->Off();
}

void PlayerActor_Nero::SetOverture()
{
	if (true == DTValue)
	{
		return;
	}
	ArmValue = 1;
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BusterArm0]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BusterArm1]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Gerbera0]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Gerbera1]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Overture]->On();
	Renderer_Overture->Off();
}

void PlayerActor_Nero::SetGerbera()
{
	if (true == DTValue)
	{
		return;
	}
	ArmValue = 2;
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BusterArm0]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BusterArm1]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Gerbera0]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Gerbera1]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Overture]->Off();
	Renderer_Overture->Off();
}

void PlayerActor_Nero::SetBusterArm()
{
	if (true == DTValue)
	{
		return;
	}
	ArmValue = 3;
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BusterArm0]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BusterArm1]->On();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Gerbera0]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Gerbera1]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Overture]->Off();
	Renderer_Overture->Off();
}

void PlayerActor_Nero::SetOvertureAnimation()
{
	if (true == DTValue)
	{
		return;
	}
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BusterArm0]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BusterArm1]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Gerbera0]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Gerbera1]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Overture]->Off();
	Renderer_Overture->On();
}

void PlayerActor_Nero::OnDevilBraeker()
{
	switch (CurDevilBreaker)
	{
	case DevilBreaker::None:
		break;
	case DevilBreaker::Overture:
		SetOverture();
		break;
	case DevilBreaker::Gerbera:
		SetGerbera();
		break;
	case DevilBreaker::BusterArm:
		SetBusterArm();
		break;
	default:
		break;
	}
}

void PlayerActor_Nero::OffDevilBraeker()
{
	ArmValue = 0;
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BusterArm0]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_BusterArm1]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Gerbera0]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Gerbera1]->Off();
	Renderer->GetAllRenderUnit()[0][NeroRenderUnit_Overture]->Off();
}

void PlayerActor_Nero::AddBreaker(DevilBreaker _Breaker)
{
	NeroItemGlass::AddItemUI(true);
	CurDevilBreaker = _Breaker;
	switch (CurDevilBreaker)
	{
	case DevilBreaker::None:
		break;
	case DevilBreaker::Overture:
		BreakerList.push_back(DevilBreaker::Overture);
		SetOverture();
		break;
	case DevilBreaker::Gerbera:
		BreakerList.push_back(DevilBreaker::Gerbera);
		SetGerbera();
		break;
	case DevilBreaker::BusterArm:
		BreakerList.push_back(DevilBreaker::BusterArm);
		SetBusterArm();
		break;
	default:
		break;
	}
}

void PlayerActor_Nero::DestroyBreaker()
{
	if (BreakerList.back() == DevilBreaker::None) { return; }
	BreakerList.pop_back();
	CurDevilBreaker = BreakerList.back();
	NeroItemGlass::DestroyItemUI(true);
	switch (CurDevilBreaker)
	{
	case DevilBreaker::None:
		OffDevilBraeker();
		break;
	case DevilBreaker::Overture:
		SetOverture();
		break;
	case DevilBreaker::Gerbera:
		SetGerbera();
		break;
	case DevilBreaker::BusterArm:
		SetBusterArm();
		break;
	default:
		break;
	}
}

void PlayerActor_Nero::SetExActTiming()
{
	IsMaxActTiming = true;
	IsExActTiming = true;
	TimeEvent.AddEvent(0.08f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager)
		{
			IsMaxActTiming = false;
		});

	TimeEvent.AddEvent(0.2f, [=](GameEngineTimeEvent::TimeEvent& _Event, GameEngineTimeEvent* _Manager)
		{
			IsExActTiming = false;
		});
}

int PlayerActor_Nero::DamageCalculate(int _Damage, bool _IsSkill /* = false */)
{
	float Mul = 1.0f;

	if (false == _IsSkill)
	{
		if (0 < ExceedLevel) { Mul += 0.2f; }
		if (true == DTValue) { Mul *= 1.3f; }
		return static_cast<int>(_Damage * Mul);
	}

	switch (ExceedLevel)
	{
	case 1:
		Mul += 0.2f;
		break;
	case 2:
		Mul += 0.8f;
		break;
	case 3:
		Mul += 1.3f;
		break;
	}
	if (true == DTValue) { Mul *= 1.3f; }
	return static_cast<int>(_Damage * Mul);
}

//네로 렌더 유닛


//0 인간옷
//1 인간옷
//2 인간눈	
//3 버스터
//4 버스터
//5 손 레드퀸
//6 DT 손
//7 DT 날개
//8 인간 옷
//9 인간 속눈썹
//10 거베라
//11 거베라
//12 블루로즈
//13 인간머리
//14 오버추어
//15 DT 머리
//16 DT 눈
//17 등 레드퀸
//18 인간 팔
//19 인간 눈썹
//20 인간 머리카락
//21 인간 팔
//22 DT 몸
//23 DT 헤어
//24 DT 헤어
