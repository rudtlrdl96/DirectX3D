#include "PrecompileHeader.h"
#include "BasePlayerActor.h"
#include <GameEngineBase/GameEngineNet.h>
#include <GameEngineCore/GameEngineFBXRenderer.h>
#include <GameEngineCore/GameEngineCollision.h>
#include "PlayerController.h"
#include "PlayerCamera.h"
#include "ServerWindow.h"
#include "ObjectUpdatePacket.h"
#include "ContentsEnum.h"
#include "BaseLog.h"
#include "AnimationEvent.h"
#include "NetworkManager.h"
#include <GameEngineCore/PhysXCapsuleComponent.h>
#include "FsmChangePacket.h"

std::vector<BasePlayerActor*> BasePlayerActor::Players = std::vector<BasePlayerActor*>();

BasePlayerActor::BasePlayerActor()
{
	Players.push_back(this);
}

BasePlayerActor::~BasePlayerActor()
{
}

void BasePlayerActor::LookDir(const float4& _LookDir)
{
 	float4 LocalForward = GetTransform()->GetWorldForwardVector().NormalizeReturn();
	float4 LookDir = _LookDir.NormalizeReturn();
 	float Dot = float4::DotProduct3D(LocalForward, LookDir); 
	if (1.0f < Dot || Dot == 0.0f)
	{
		return;
	}
	float Angle = acosf(Dot) * GameEngineMath::RadToDeg;
	float4 Cross = float4::Cross3DReturnNormal(LocalForward, LookDir);

	if (Cross.y < 0.0f)
	{
		Rot.y -= Angle;
	}
	else
	{
		Rot.y += Angle;
	}
	PhysXCapsule->SetWorldRotation(Rot);
	return;
}

void BasePlayerActor::LookTarget()
{
	if (nullptr == LockOnEnemyTransform)
	{
		return;
	}

	float4 LocalForward = GetTransform()->GetWorldLeftVector();
	float4 LookDir = (LockOnEnemyTransform->GetWorldPosition() - GetTransform()->GetWorldPosition()).NormalizeReturn();
	float4 Cross = float4::Cross3DReturnNormal(LocalForward, LookDir);
	float Dot = float4::DotProduct3D(LocalForward, LookDir);

	if (Cross.y <= 0.0f)
	{
		Rot.y += 180;
		PhysXCapsule->SetWorldRotation(Rot);
	}
	else
	{
		Rot.y += -GameEngineMath::RadToDeg * Dot;
		PhysXCapsule->SetWorldRotation(Rot);
	}
	return;
}

void BasePlayerActor::RotationToDir(const float4& _Dir, float _MaxValue)
{
	float4 LocalForward = GetTransform()->GetWorldForwardVector().NormalizeReturn();
	float4 LookDir = _Dir.NormalizeReturn();
	float Dot = float4::DotProduct3D(LocalForward, LookDir);
	if (1.0f < Dot || Dot == 0.0f)
	{
		return;
	}
	float Angle = acosf(Dot) * GameEngineMath::RadToDeg;
	float4 Cross = float4::Cross3DReturnNormal(LocalForward, LookDir);

	if (Cross.y < 0.0f)
	{
		Rot.y -= std::clamp(Angle, -_MaxValue, _MaxValue);
	}
	else
	{
		Rot.y += std::clamp(Angle, -_MaxValue, _MaxValue);
	}
	PhysXCapsule->SetWorldRotation(Rot);
	return;
}

void BasePlayerActor::RotationToTarget(float _MaxValue /* = 360.0f */)
{
	if (nullptr == LockOnEnemyTransform)
	{
		return;
	}
	RotationToDir((LockOnEnemyTransform->GetWorldPosition() - GetTransform()->GetWorldPosition()).NormalizeReturn(), _MaxValue);
}

void BasePlayerActor::RotationToMoveVector(float _MaxValue)
{
	const float4& MoveVector = Controller->GetMoveVector();
	if (MoveVector == float4::ZERO)
	{
		return;
	}
	RotationToDir(MoveVector, _MaxValue);
}

void BasePlayerActor::Start()
{
	// PhysX 콜리전
	PhysXCapsule = CreateComponent<PhysXCapsuleComponent>();
	PhysXCapsule->SetPhysxMaterial(0.0f, 0.0f, 0.0f);
	PhysXCapsule->CreatePhysXActors({ 150, 100, 150 });
	PhysXCapsule->GetDynamic()->setMass(5.0f);
}

void BasePlayerActor::NetControllLoad()
{
	PhysXCapsule->TurnOffGravity();
}

void BasePlayerActor::UserControllLoad()
{
	// 유저 컨트롤 엑터인 경우 실행
	PhysXCapsule->SetMainPlayer();
	CustomCallback::SetMainPlayer(PhysXCapsule.get());

	Camera = GetLevel()->CreateActor<PlayerCamera>();
	Camera->SetPlayerTranform(GetTransform());

	// 플레이어 컨트롤러 (조작 체계)
	Controller = CreateComponent<PlayerController>();
	Controller->SetCameraTransform(Camera->GetTransform());
	Controller->CallBack_LockOnDown = std::bind(&BasePlayerActor::LockOn, this);
	Controller->CallBack_LockOnUp = std::bind(&BasePlayerActor::LockOff, this);

	// 플레이어 충돌체
	PlayerCollision = CreateComponent<GameEngineCollision>(CollisionOrder::Player);
	PlayerCollision->GetTransform()->SetLocalScale({ 100, 100, 100 });
	PlayerCollision->SetColType(ColType::OBBBOX3D);

	// 록온 용 충돌체
	LockOnCollision = CreateComponent<GameEngineCollision>(CollisionOrder::PlayerLockOn);
	LockOnCollision->GetTransform()->SetParent(Camera->GetCameraTarget());
	LockOnCollision->GetTransform()->SetLocalScale({ 2000, 5000, 5000 });
	LockOnCollision->GetTransform()->SetLocalPosition({ 0, 0, 2500 });
	LockOnCollision->SetColType(ColType::OBBBOX3D);
}

void BasePlayerActor::Update_ProcessPacket()
{
	//패킷을 다 처리할 때 까지
	while (GameEngineNetObject::IsPacket())
	{
		//지금 처리할 패킷의 타입을 알아옵니다
		PacketEnum Type = GameEngineNetObject::GetFirstPacketType<PacketEnum>();

		switch (Type)
		{
			//업데이트 패킷의 경우엔
		case PacketEnum::ObjectUpdatePacket:
		{
			//패킷을 템플릿 포인터로 꺼내옵니다(Enum값과 포인터값을 맞게 해주셔야 하는 부분 유의부탁드려요)
			std::shared_ptr<ObjectUpdatePacket> ObjectUpdate = PopFirstPacket<ObjectUpdatePacket>();

			//패킷의 정보에 따라 자신의 값 수정
			Server_PrevPos = GetTransform()->GetWorldPosition();
			Server_NextPos = ObjectUpdate->Position;
			Server_Timer = 0.0f;
			PhysXCapsule->SetWorldRotation(ObjectUpdate->Rotation);
			ObjectUpdate->TimeScale;

			float TimeScale = ObjectUpdate->TimeScale;
			//unsigned int FsmState = ObjectUpdate->FsmState;
			//bool IsFsmForce = ObjectUpdate->IsFsmForce;
			break;
		}
		case PacketEnum::FsmChangePacket:
		{
			std::shared_ptr<FsmChangePacket> FsmChange = PopFirstPacket<FsmChangePacket>();
			SetFSMStateValue(FsmChange->FsmState);
			break;
		}
		default:
		{
			MsgAssert("처리하지 못하는 패킷이 플레이어로 날아왔습니다.");
			return;
		}
		}
	}
}

void BasePlayerActor::Update(float _DeltaTime)
{
	Update_Character(_DeltaTime);

	if (NetControllType::NetControll == GameEngineNetObject::GetControllType())
	{
		Server_Timer += _DeltaTime;
		float Ratio = Server_Timer / Server_UpdateTime;
		PhysXCapsule->SetWorldPosition(float4::LerpClamp(Server_PrevPos, Server_NextPos, Ratio));
	}
	Update_SendPacket(_DeltaTime);
}

void BasePlayerActor::Update_SendPacket(float _DeltaTime)
{
	NetworkManager::PushUpdatePacket({ .ObjPtr = this, .TimeScale = 1.0f });
}

void BasePlayerActor::LockOn()
{
	std::vector<std::shared_ptr<GameEngineCollision>> Cols;
	std::shared_ptr<GameEngineCollision> MinCol = nullptr;
	if (true == LockOnCollision->CollisionAll(CollisionOrder::Enemy, Cols))
	{
		float Min = 9999;
		for (std::shared_ptr<GameEngineCollision> Col : Cols)
		{
			float Length = (GetTransform()->GetWorldPosition() - Col->GetTransform()->GetWorldPosition()).Size();
			if (Length < Min)
			{
				Min = Length;
				MinCol = Col;
			}
		}
		if (MinCol == nullptr)
		{
			return;
		}
		LockOnEnemyTransform = MinCol->GetActor()->GetTransform();
		Camera->SetTargetTranform(LockOnEnemyTransform);
	}
}

void BasePlayerActor::LockOff()
{
	LockOnEnemyTransform = nullptr;
	//Camera->SetTargetTranform(nullptr);
}

bool BasePlayerActor::FloorCheck()
{
	float4 Point;
	return GetLevel()->RayCast(GetTransform()->GetWorldPosition(), float4::DOWN, Point, 100.0f) || PhysXCapsule->GetIsPlayerGroundTouch();
}

void BasePlayerActor::SetForce(float4 _Value)
{
	float4 re = _Value.RotaitonYDegReturn(Rot.y);
	PhysXCapsule->SetForce(re);
}

void BasePlayerActor::SetPush(float4 _Value)
{
	PhysXCapsule->SetPush(_Value.RotaitonYDegReturn(Rot.y));
}

void BasePlayerActor::SetMove(float4 _Value)
{
	PhysXCapsule->SetMove(_Value.RotaitonYDegReturn(Rot.y));
}

void BasePlayerActor::SetWorldPosition(float4 _Value)
{
	PhysXCapsule->SetWorldPosition(_Value);
}