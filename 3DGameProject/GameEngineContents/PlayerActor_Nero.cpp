#include "PrecompileHeader.h"
#include "PlayerActor_Nero.h"
#include <GameEngineCore/GameEngineFBXRenderer.h>
#include <GameEngineCore/PhysXCapsuleComponent.h>
#include "AnimationEvent.h"
#include "PlayerController.h"
PlayerActor_Nero::PlayerActor_Nero()
{
}

PlayerActor_Nero::~PlayerActor_Nero()
{
}

void PlayerActor_Nero::SetPush(const float4& _Value)
{
	PhysXCapsule->SetPush(_Value);
}

void PlayerActor_Nero::SetMass(float _Value)
{
	PhysXCapsule->GetDynamic()->setMass(_Value);
}
void PlayerActor_Nero::Start()
{
	BasePlayerActor::Start();
	PhysXCapsule->GetDynamic()->setMass(1.0f);
	PhysXCapsule->SetSpeedLimitValue(300.0f);
#ifdef _DEBUG
	TestLoad();
#else
	NeroLoad();
#endif

}

void PlayerActor_Nero::TestLoad()
{
	Renderer = CreateComponent<GameEngineFBXRenderer>();
	Renderer->SetFBXMesh("House1.fbx", "NoneAlphaMesh");

	{
		FSM.CreateState({ .StateValue = FSM_State_Nero::Idle,
			.Start = [=] {
			},
			.Update = [=](float _DeltaTime) {
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					FSM.ChangeState(FSM_State_Nero::Run);
					return;
				}
				if (true == IsLockOn)
				{
					FSM.ChangeState(BR_Switch_Idle_to_Lockon);
					return;
				}
			},
			.End = [=] {

			}
			});

		FSM.CreateState({ .StateValue = FSM_State_Nero::Run,
			.Start = [=] {
			},
			.Update = [=](float _DeltaTime) {
				if (Controller->GetMoveVector() == float4::ZERO)
				{
					FSM.ChangeState(FSM_State_Nero::Idle);
					return;
				}

				if (true == IsLockOn)
				{
					FSM.ChangeState(BR_Switch_Idle_to_Lockon);
					return;
				}

				LookDir(Controller->GetMoveVector());
				float4 MoveDir = Controller->GetMoveVector() * RunSpeed * _DeltaTime;
				PhysXCapsule->SetMove(MoveDir);
			},
			.End = [=] {

			}
			});

		FSM.CreateState({ .StateValue = FSM_State_Nero::BR_Switch_Idle_to_Lockon,
			.Start = [=] {
			},
			.Update = [=](float _DeltaTime) {
				if (false == IsLockOn)
				{
					FSM.ChangeState(Idle);
					return;
				}
				LookTarget(LockOnEnemyTransform->GetWorldPosition());
				float4 MoveDir = Controller->GetMoveVector() * RunSpeed * _DeltaTime;
				PhysXCapsule->SetMove(MoveDir);
			},
			.End = [=] {

			}
			});
	}

	FSM.ChangeState(FSM_State_Nero::Idle);
}

void PlayerActor_Nero::NeroLoad()
{
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
		}
		NewDir.MoveParent();
		NewDir.Move("Animation");

		Renderer = CreateComponent<GameEngineFBXRenderer>();
		Renderer->GetTransform()->SetLocalRotation({ 0, 90, 0 });
		Renderer->GetTransform()->SetLocalPosition({ 0, -75, 0 });
		Renderer->SetFBXMesh("Nero.FBX", "MeshAniTexture");
		AnimationEvent::LoadAll({ .Dir = NewDir.GetFullPath().c_str(), .Renderer = Renderer,
			.Objects = { (GameEngineObject*)AttackCollision.get() },
			.CallBacks_void = {
				std::bind([=] {InputCheck = true; }),
				std::bind(&PlayerActor_Nero::RedQueenOn, this),
				std::bind(&PlayerActor_Nero::RedQueenOff, this),
				std::bind(&PlayerActor_Nero::BlueRoseOn, this),
				std::bind(&PlayerActor_Nero::WeaponIdle, this)
			},
			.CallBacks_int = {
				std::bind(&GameEngineFSM::ChangeState, &FSM, std::placeholders::_1)
			}

			});


		//Object = 0 : 공격 충돌체
		//
		//콜백void = 0 : 입력체크시작
		//콜백void = 1 : 손에 레드퀸 들려줌
		//콜백void = 2 : 손에 레드퀸 뺌
		//콜백void = 3 : 손에 블루로즈
		//콜백void = 4 : WeaponIdle (빈손, 칼 등에)
		//
		//콜백 int = 0 : FSM변경



		Renderer->GetAllRenderUnit()[0][12]->Off();	// 버스터 암
		Renderer->GetAllRenderUnit()[0][13]->Off();	// 버스터 암
		Renderer->GetAllRenderUnit()[0][14]->On();	// 오버추어
		Renderer->GetAllRenderUnit()[0][15]->Off();	// 거베라
		Renderer->GetAllRenderUnit()[0][16]->Off();	// 거베라
		Renderer->GetAllRenderUnit()[0][17]->Off();	// 손 레드퀸
		Renderer->GetAllRenderUnit()[0][18]->On();	// 등 레드퀸
		Renderer->GetAllRenderUnit()[0][19]->Off();	// 블루로즈
	}

	/* 기본 움직임 */
	{
		// Idle
		FSM.CreateState({ .StateValue = FSM_State_Nero::Idle,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Idle_Normal");
			},
			.Update = [=](float _DeltaTime) {

				if (Controller->GetSwordDown())
				{
					FSM.ChangeState(FSM_State_Nero::RQ_ComboA_1);
					return;
				}
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					FSM.ChangeState(FSM_State_Nero::RunStart);
					return;
				}

				if (true == IsLockOn)
				{
					FSM.ChangeState(BR_Switch_Idle_to_Lockon);
					return;
				}
			},
			.End = [=] {

			}
			});

		// Run Start
		FSM.CreateState({ .StateValue = FSM_State_Nero::RunStart,
			.Start = [=] {
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Run_Start");
			},
			.Update = [=](float _DeltaTime) {
				if (Controller->GetSwordDown())
				{
					FSM.ChangeState(FSM_State_Nero::RQ_ComboA_1);
					return;
				}
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}
				if (Controller->GetMoveVector() == float4::ZERO)
				{
					FSM.ChangeState(FSM_State_Nero::RunStop);
					return;
				}
				if (true == IsLockOn)
				{
					FSM.ChangeState(BR_Switch_Idle_to_Lockon);
					return;
				}
				if (true == Renderer->IsAnimationEnd())
				{
					FSM.ChangeState(FSM_State_Nero::Run);
					return;
				}
				LookDir(Controller->GetMoveVector());
				float4 MoveDir = Controller->GetMoveVector() * RunSpeed * _DeltaTime;
				PhysXCapsule->SetMove(MoveDir);
			},
			.End = [=] {

			}
			});

		// Run
		FSM.CreateState({ .StateValue = FSM_State_Nero::Run,
			.Start = [=] {
				DashTimer = 0;
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Run_Loop");
			},
			.Update = [=](float _DeltaTime) {
				DashTimer += _DeltaTime;
				if (Controller->GetSwordDown())
				{
					FSM.ChangeState(FSM_State_Nero::RQ_ComboA_1);
					return;
				}
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}
				if (Controller->GetMoveVector() == float4::ZERO)
				{
					FSM.ChangeState(FSM_State_Nero::RunStop);
					return;
				}

				if (true == IsLockOn)
				{
					FSM.ChangeState(FSM_State_Nero::BR_Switch_Idle_to_Lockon);
					return;
				}

				if (1 < DashTimer)
				{
					FSM.ChangeState(FSM_State_Nero::Dash);
					return;
				}

				LookDir(Controller->GetMoveVector());
				float4 MoveDir = Controller->GetMoveVector() * RunSpeed * _DeltaTime;
				PhysXCapsule->SetMove(MoveDir);
			},
			.End = [=] {

			}
			});

		// RunStop
		FSM.CreateState({ .StateValue = FSM_State_Nero::RunStop,
			.Start = [=] {
				InputCheck = false;
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Run_Stop");
			},
			.Update = [=](float _DeltaTime) {
				if (Controller->GetSwordDown())
				{
					FSM.ChangeState(FSM_State_Nero::RQ_ComboA_1);
					return;
				}
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}

				if (false == InputCheck) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					FSM.ChangeState(FSM_State_Nero::RunStart);
					return;
				}
				if (true == IsLockOn)
				{
					FSM.ChangeState(FSM_State_Nero::BR_Switch_Idle_to_Lockon);
					return;
				}
			},
			.End = [=] {

			}
			});

		// Dash
		FSM.CreateState({ .StateValue = FSM_State_Nero::Dash,
			.Start = [=] {
				DashTimer = 0;
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Dash_Loop");
			},
			.Update = [=](float _DeltaTime) {
				DashTimer += _DeltaTime;
				if (Controller->GetSwordDown())
				{
					FSM.ChangeState(FSM_State_Nero::RQ_ComboA_1);
					return;
				}
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}
				if (Controller->GetMoveVector() == float4::ZERO)
				{
					FSM.ChangeState(FSM_State_Nero::DashStop);
					return;
				}
				if (true == IsLockOn)
				{
					FSM.ChangeState(FSM_State_Nero::BR_Switch_Idle_to_Lockon);
					return;
				}

				LookDir(Controller->GetMoveVector());
				float4 MoveDir = Controller->GetMoveVector() * DashSpeed * _DeltaTime;
				PhysXCapsule->SetMove(MoveDir);
			},
			.End = [=] {

			}
			});

		// DashStop
		FSM.CreateState({ .StateValue = FSM_State_Nero::DashStop,
			.Start = [=] {
				InputCheck = false;
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Dash_Stop");
			},
			.Update = [=](float _DeltaTime) {
				if (Controller->GetSwordDown())
				{
					FSM.ChangeState(FSM_State_Nero::RQ_ComboA_1);
					return;
				}
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}

				if (false == InputCheck) { return; }

				if (Controller->GetMoveVector() != float4::ZERO)
				{
					FSM.ChangeState(FSM_State_Nero::RunStart);
					return;
				}
				if (true == IsLockOn)
				{
					FSM.ChangeState(FSM_State_Nero::BR_Switch_Idle_to_Lockon);
					return;
				}
			},
			.End = [=] {

			}
			});

		// Jump_Vertical
		FSM.CreateState({ .StateValue = FSM_State_Nero::Jump_Vertical,
			.Start = [=] {
				PhysXCapsule->SetJump(JumpForce);
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_Vertical");
			},
			.Update = [=](float _DeltaTime) {
				if (Renderer->IsAnimationEnd())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Fly);
				}
				float4 MoveDir = Controller->GetMoveVector() * RunSpeed * _DeltaTime;
				PhysXCapsule->SetMove(MoveDir);
			},
			.End = [=] {

			}
			});

		static float Timer = 0;
		// Jump Fly
		FSM.CreateState({ .StateValue = FSM_State_Nero::Jump_Fly,
			.Start = [=] {
				Timer = 0;
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_Fly_loop");
			},
			.Update = [=](float _DeltaTime) {
				Timer += _DeltaTime;
				if (0.5f < Timer)
				{
					FSM.ChangeState(FSM_State_Nero::Landing);
				}
				float4 MoveDir = Controller->GetMoveVector() * RunSpeed * _DeltaTime;
				PhysXCapsule->SetMove(MoveDir);
			},
			.End = [=] {

			}
			});

		FSM.CreateState({ .StateValue = FSM_State_Nero::Landing,
			.Start = [=] {
				InputCheck = false;
				WeaponIdle();
				Renderer->ChangeAnimation("pl0000_Jump_Landing");
			},
			.Update = [=](float _DeltaTime) {
				if (true == InputCheck)
				{
					if (Controller->GetSwordDown())
					{
						FSM.ChangeState(FSM_State_Nero::RQ_ComboA_1);
						return;
					}
					if (Controller->GetJumpDown())
					{
						FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
						return;
					}
					if (Controller->GetMoveVector() != float4::ZERO)
					{
						FSM.ChangeState(FSM_State_Nero::RunStart);
						return;
					}
				}
				if (Renderer->IsAnimationEnd())
				{
					FSM.ChangeState(FSM_State_Nero::Idle);
				}
			},
			.End = [=] {

			}
			});
	}

	/* 레드 퀸 */
	{
		// RedQueen ComboA1
		FSM.CreateState({ .StateValue = FSM_State_Nero::RQ_ComboA_1,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_RQ_ComboA_1");
				InputCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (InputCheck == false) { return; }
				if (Controller->GetSwordDown())
				{
					FSM.ChangeState(FSM_State_Nero::RQ_ComboA_2);
					return;
				}
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}
			},
			.End = [=] {

			}
			});

		// RedQueen ComboA2
		FSM.CreateState({ .StateValue = FSM_State_Nero::RQ_ComboA_2,
			.Start = [=] {
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboA_2");
				InputCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (InputCheck == false) { return; }
				if (Controller->GetSwordDown())
				{
					FSM.ChangeState(FSM_State_Nero::RQ_ComboA_3);
					return;
				}
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}
			},
			.End = [=] {

			}
			});

		// RedQueen ComboA3
		FSM.CreateState({ .StateValue = FSM_State_Nero::RQ_ComboA_3,
			.Start = [=] {
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboA_3");
				InputCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (InputCheck == false) { return; }
				if (Controller->GetSwordDown())
				{
					FSM.ChangeState(FSM_State_Nero::RQ_ComboA_4);
					return;
				}
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}
			},
			.End = [=] {

			}
			});

		// RedQueen ComboA4
		FSM.CreateState({ .StateValue = FSM_State_Nero::RQ_ComboA_4,
			.Start = [=] {
				RedQueenOn();
				Renderer->ChangeAnimation("pl0000_RQ_ComboA_4");
				InputCheck = false;
			},
			.Update = [=](float _DeltaTime) {
				if (InputCheck == false) { return; }
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}
			},
			.End = [=] {

			}
			});

	}
	/* 블루 로즈 (락온) */
	{
		FSM.CreateState({ .StateValue = FSM_State_Nero::BR_Switch_Idle_to_Lockon,
			.Start = [=] {
				WeaponIdle();
				LookTarget(LockOnEnemyTransform->GetWorldPosition());
				Renderer->ChangeAnimation("pl0000_BR_Switch_Idle_to_Lockon");
			},
			.Update = [=](float _DeltaTime) {


				if (Controller->GetMoveVector() != float4::ZERO)
				{
					FSM.ChangeState(FSM_State_Nero::BR_Strafe);
					return;
				}
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}
				if (Controller->GetSwordDown())
				{
					FSM.ChangeState(FSM_State_Nero::RQ_ComboA_1);
					return;
				}
				if (true == Renderer->IsAnimationEnd())
				{
					FSM.ChangeState(FSM_State_Nero::BR_Lockon_Front);
					return;
				}
				LookTarget(LockOnEnemyTransform->GetWorldPosition());
			},
			.End = [=] {

			}
			});

		FSM.CreateState({ .StateValue = FSM_State_Nero::BR_Lockon_Front,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_BR_Lockon_Front");
			},
			.Update = [=](float _DeltaTime) {
				if (true == Controller->GetLockOnFree())
				{
					FSM.ChangeState(FSM_State_Nero::BR_Switch_Lockon_to_Idle);
					return;
				}
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					FSM.ChangeState(FSM_State_Nero::BR_Strafe);
					return;
				}
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}
				if (Controller->GetSwordDown())
				{
					FSM.ChangeState(FSM_State_Nero::RQ_ComboA_1);
					return;
				}
				LookTarget(LockOnEnemyTransform->GetWorldPosition());
			},
			.End = [=] {

			}
			});

		FSM.CreateState({ .StateValue = FSM_State_Nero::BR_Strafe,
			.Start = [=] {
				BlueRoseOn();
				CurDir = 'n';
			},
			.Update = [=](float _DeltaTime) {
				if (true == Controller->GetLockOnFree())
				{
					FSM.ChangeState(FSM_State_Nero::BR_Switch_Lockon_to_Idle);
					return;
				}
				if (Controller->GetMoveVector() == float4::ZERO)
				{
					FSM.ChangeState(FSM_State_Nero::BR_Lockon_Front);
					return;
				}
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}
				if (Controller->GetSwordDown())
				{
					FSM.ChangeState(FSM_State_Nero::RQ_ComboA_1);
					return;
				}

				LookTarget(LockOnEnemyTransform->GetWorldPosition());

				float4 MoveDir = Controller->GetMoveVector() * WalkSpeed * _DeltaTime;
				PhysXCapsule->SetMove(MoveDir);

				char NewDir = Controller->MoveVectorToChar(Controller->GetMoveVector());
				if (CurDir == NewDir) { return; }
				CurDir = NewDir;
				switch (CurDir)
				{
				case '8':
					Renderer->ChangeAnimation("pl0000_Strafe_F_Loop");
					break;
				case '7':
					Renderer->ChangeAnimation("pl0000_Strafe_FL_Loop");
					break;
				case '4':
					Renderer->ChangeAnimation("pl0000_Strafe_L_Loop");
					break;
				case '1':
					Renderer->ChangeAnimation("pl0000_Strafe_BL_Loop");
					break;
				case '2':
					Renderer->ChangeAnimation("pl0000_Strafe_B_Loop");
					break;
				case '3':
					Renderer->ChangeAnimation("pl0000_Strafe_BR_Loop");
					break;
				case '6':
					Renderer->ChangeAnimation("pl0000_Strafe_R_Loop");
					break;
				case '9':
					Renderer->ChangeAnimation("pl0000_Strafe_R_Loop");
					break;
				}

			},
			.End = [=] {



			} });

		FSM.CreateState({ .StateValue = FSM_State_Nero::BR_Switch_Lockon_to_Idle,
			.Start = [=] {
				Renderer->ChangeAnimation("pl0000_BR_Switch_Lockon_to_Idle");
			},
			.Update = [=](float _DeltaTime) {
				if (true == Renderer->IsAnimationEnd())
				{
					FSM.ChangeState(FSM_State_Nero::Idle);
					return;
				}
				if (Controller->GetMoveVector() != float4::ZERO)
				{
					FSM.ChangeState(FSM_State_Nero::RunStart);
					return;
				}
				if (Controller->GetJumpDown())
				{
					FSM.ChangeState(FSM_State_Nero::Jump_Vertical);
					return;
				}
				if (Controller->GetSwordDown())
				{
					FSM.ChangeState(FSM_State_Nero::RQ_ComboA_1);
					return;
				}
			},
			.End = [=] {

			}
			});
	}
	FSM.ChangeState(FSM_State_Nero::Idle);
}

void PlayerActor_Nero::Update_Character(float _DeltaTime)
{
	FSM.Update(_DeltaTime);
}

/*
0 12~13 버스트 암
0 14 오버추어
0 15~16 거베라
0 17 손 레드퀸
0 18 등 레드퀸
0 19 손 블루 로즈
*/
void PlayerActor_Nero::RedQueenOn()
{
	Renderer->GetAllRenderUnit()[0][17]->On();	// 손 레드퀸
	Renderer->GetAllRenderUnit()[0][18]->Off();	// 등 레드퀸
	Renderer->GetAllRenderUnit()[0][19]->Off();	// 블루로즈
}

void PlayerActor_Nero::RedQueenOff()
{
	Renderer->GetAllRenderUnit()[0][17]->Off();	// 손 레드퀸
	Renderer->GetAllRenderUnit()[0][18]->On();	// 등 레드퀸
}

void PlayerActor_Nero::BlueRoseOn()
{
	Renderer->GetAllRenderUnit()[0][17]->Off();	// 손 레드퀸
	Renderer->GetAllRenderUnit()[0][18]->On();	// 등 레드퀸
	Renderer->GetAllRenderUnit()[0][19]->On();	// 블루로즈
}

void PlayerActor_Nero::WeaponIdle()
{
	Renderer->GetAllRenderUnit()[0][17]->Off();	// 손 레드퀸
	Renderer->GetAllRenderUnit()[0][18]->On();	// 등 레드퀸
	Renderer->GetAllRenderUnit()[0][19]->Off();	// 블루로즈
}

//LookDir((LockOnEnemyTransform->GetWorldPosition() - GetTransform()->GetWorldPosition()).NormalizeReturn());
