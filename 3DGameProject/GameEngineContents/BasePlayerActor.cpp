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
BasePlayerActor* BasePlayerActor::Instance = nullptr;

BasePlayerActor::BasePlayerActor()
{
	if (GameEngineNetObject::GetControllType() == NetControllType::UserControll)
	{
		Instance = this;
	}
}

BasePlayerActor::~BasePlayerActor()
{
}

void BasePlayerActor::LookDir(const float4& _LookDir)
{
	float4 LocalForward = GetTransform()->GetWorldForwardVector();
	//float4 LocalForward = Controller->GetMoveVector();

	float4 Cross = float4::Cross3DReturnNormal(LocalForward,  _LookDir);
	//float4 Cross = float4::Cross3DReturnNormal(_LookDir, LocalForward);
	float Dot = float4::DotProduct3D(LocalForward, _LookDir);
	//float Dot = float4::DotProduct3D(_LookDir, LocalForward);

	if (Cross.y == -1.0f)
	{
		//GetTransform()->AddLocalRotation({ 0, 180, 0 });
		physx::PxTransform PhyTF = PhysXCapsule->GetDynamic()->getGlobalPose();
		float4 Q = float4(0, 180, 0).EulerDegToQuaternion();
		PhyTF.q.x += Q.x;
		PhyTF.q.y += Q.y;
		PhyTF.q.z += Q.z;
		PhysXCapsule->GetDynamic()->setGlobalPose(PhyTF);
	}
	else
	{
		//GetTransform()->AddLocalRotation({ 0, -GameEngineMath::RadToDeg * Dot, 0 });
		physx::PxTransform PhyTF = PhysXCapsule->GetDynamic()->getGlobalPose();
		float4 Q = float4(0, -GameEngineMath::RadToDeg * Dot, 0).EulerDegToQuaternion();
		PhyTF.q.x += Q.x;
		PhyTF.q.y += Q.y;
		PhyTF.q.z += Q.z;
		PhysXCapsule->GetDynamic()->setGlobalPose(PhyTF);
	}
	return;

}

void BasePlayerActor::Start()
{
	Camera = GetLevel()->CreateActor<PlayerCamera>();
	Camera->SetPlayerTranform(GetTransform());

	Controller = CreateComponent<PlayerController>();
	Controller->SetCameraTransform(Camera->GetTransform());
	Controller->CallBack_LockOnDown = std::bind(&BasePlayerActor::LockOn, this);
	Controller->CallBack_LockOnUp = std::bind(&BasePlayerActor::LockOff, this);

	PlayerCollision = CreateComponent<GameEngineCollision>(CollisionOrder::Player);
	PlayerCollision->GetTransform()->SetLocalScale({ 100, 100, 100 });
	PlayerCollision->SetColType(ColType::OBBBOX3D);

	physx::PxVec3 VecSclae = { 150, 100, 150 };

	PhysXCapsule = CreateComponent<PhysXCapsuleComponent>();
	PhysXCapsule->SetPhysxMaterial(0.0f, 1.0f, 0.0f);
	PhysXCapsule->CreatePhysXActors(GetLevel()->GetLevelScene(), GetLevel()->GetLevelPhysics(), VecSclae);
	PhysXCapsule->GetDynamic()->setMass(1.0f);
	PhysXCapsule->GetDynamic()->setMaxAngularVelocity(physx::PxReal(10.0f));

	LockOnCollision = CreateComponent<GameEngineCollision>(CollisionOrder::Player);
	LockOnCollision->GetTransform()->SetLocalScale({ 3000, 500, 1000 });
	LockOnCollision->GetTransform()->SetLocalPosition({ 1500, 0, 0 });
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
			if (true == ObjectUpdate->IsDeath)
			{
				Death();
				break;
			}

			//패킷의 정보에 따라 자신의 값 수정
			GetTransform()->SetLocalPosition(ObjectUpdate->Position);
			GetTransform()->SetLocalRotation(ObjectUpdate->Rotation);
			ObjectUpdate->TimeScale;
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
	Update_ProcessPacket();
	Update_Character(_DeltaTime);

	if (NetControllType::UserControll == GameEngineNetObject::GetControllType())
	{
		//Update_Character(_DeltaTime);
	}
	Update_SendPacket(_DeltaTime);


	
}

void BasePlayerActor::Update_SendPacket(float _DeltaTime)
{
	//NetworkManager::SendUpdatePacket(this, this, 1.f);
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
			int a = 0;
			return;
		}
		LockOnEnemyTransform = MinCol->GetActor()->GetTransform();
	}
}

void BasePlayerActor::LockOff()
{
	LockOnEnemyTransform = nullptr;
}

