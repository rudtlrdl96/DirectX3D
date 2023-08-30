#include "PrecompileHeader.h"
#include "NetTestPlayer.h"

#include <GameEngineBase/GameEngineNet.h>
#include <GameEngineCore/GameEngineFBXRenderer.h>

#include "NetworkManager.h"

#include "ObjectUpdatePacket.h"
#include "ServerWindow.h"
#include "ContentsEnum.h"

NetTestPlayer::NetTestPlayer()
{

}

NetTestPlayer::~NetTestPlayer()
{

}


void NetTestPlayer::Start() 
{
	//서버와 통신할 때 이 엑터가 어떤 종류인지 정의하는 부분입니다.
	//ObjectUpdatePacket을 사용하기 위해서는 반드시 이 부분을 호출해주셔야 합니다
	SetNetObjectType(Net_ActorType::Nero);

	//이건 그냥 예시 렌더링
	std::shared_ptr<GameEngineFBXRenderer> Renderer = CreateComponent<GameEngineFBXRenderer>();
	Renderer->SetFBXMesh("House1.FBX", "FBX");

	if (false == GameEngineInput::IsKey("NetTestLeft"))
	{
		GameEngineInput::CreateKey("NetTestLeft", 'A');
		GameEngineInput::CreateKey("NetTestRight", 'D');
		
		GameEngineInput::CreateKey("NetTestUp", 'W');
		GameEngineInput::CreateKey("NetTestDown", 'S');

		GameEngineInput::CreateKey("NetTestForward", 'E');
		GameEngineInput::CreateKey("NetTestBack", 'Q');

		GameEngineInput::CreateKey("NetTestDestroy", 'J');
	}
}

void NetTestPlayer::Update_ProcessPacket() 
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

void NetTestPlayer::Update(float _DeltaTime) 
{
	//플레이어의 경우 UserControll일때만 FSM이 동작되어야 합니다.
	if (NetControllType::UserControll != GameEngineNetObject::GetControllType())
		return;


	//일반적인 Update부분입니다.
	float4 MoveDir = float4::ZERO;
	if (GameEngineInput::IsPress("NetTestLeft"))
	{
		MoveDir += float4::LEFT;
	}
	if (GameEngineInput::IsPress("NetTestRight"))
	{
		MoveDir += float4::RIGHT;
	}
	if (GameEngineInput::IsPress("NetTestUp"))
	{
		MoveDir += float4::UP;
	}
	if (GameEngineInput::IsPress("NetTestDown"))
	{
		MoveDir += float4::DOWN;
	}
	if (GameEngineInput::IsPress("NetTestForward"))
	{
		MoveDir += float4::FORWARD;
	}
	if (GameEngineInput::IsPress("NetTestBack"))
	{
		MoveDir += float4::BACK;
	}
	
	GetTransform()->AddLocalPosition(MoveDir * 300.f * _DeltaTime);

	
	if (GameEngineInput::IsDown("NetTestDestroy"))
	{
		int ID = GetNetObjectID();
		Death();
	}
}

void NetTestPlayer::Update_SendPacket(float _DeltaTime) 
{
	int ID = GetNetObjectID();

	//NetworkManager를 통해서 업데이트 패킷을 보내면 됩니다.
	//그 외 패킷은 다른곳에서 보내도 상관없습니다.(아마도)
	NetworkManager::PushUpdatePacket(this, this, 1.f);

	//패킷을 보낼땐 모두 NetworkManager를 통해서 보낼 예정입니다.
	//추후 다양한 패킷 생성 예정
}

#include <GameEngineCore/imgui.h>

void NetTestPlayer::DrawEditor()
{
	ImGui::Text(GameEngineString::UniCodeToAnsi(L"Test Code").data());
}