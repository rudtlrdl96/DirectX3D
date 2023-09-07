#include "PrecompileHeader.h"
#include "NetworkManager.h"

#include "NetworkGUI.h"
#include "ContentsEnum.h"
#include "PacketEnum.h"

#include "ConnectIDPacket.h"
#include "ObjectUpdatePacket.h"
#include "MessageChatPacket.h"
#include "LinkObjectPacket.h"
#include "FsmChangePacket.h"

////////
//		클라 패킷 초기화
////////


void NetworkManager::ClientPacketInit()
{
	//----------------------------------------------------------------------------------------------------------------------------------------------------
	//ConnectIDPacket 처리
	//----------------------------------------------------------------------------------------------------------------------------------------------------
	NetInst->Dispatcher.AddHandler<ConnectIDPacket>
		([=](std::shared_ptr<ConnectIDPacket> _Packet)
	{
		//이 클라이언트의 네트워크 아이디 지정
		NetID = _Packet->GetObjectID();

		if (nullptr != ConnectCallBack)
		{
			ConnectCallBack(NetID);
			ConnectCallBack = nullptr;
		}
		
		//연결 성공 메세지 보내기
		const std::string& ClientName = NetworkGUI::GetInst()->GetNickName();
		std::string ChatMsg = ClientName + " Connect Server";
		PushChatPacket(ChatMsg);
	});



	//----------------------------------------------------------------------------------------------------------------------------------------------------
	//ObjectUpdatePacket 처리
	//----------------------------------------------------------------------------------------------------------------------------------------------------
	NetInst->Dispatcher.AddHandler<ObjectUpdatePacket>(
		[=](std::shared_ptr<ObjectUpdatePacket> _Packet)
	{
		//서버로 부터 받은 패킷이 현재 레벨과 다른 경우
		if (CurLevelType != _Packet->LevelType)
			return;

		unsigned int ObjID = _Packet->GetObjectID();

		//해당 NetObejctID의 객체가 존재하지 않다면 만든다면 여기서 만들어버리기
		if (false == GameEngineNetObject::IsNetObject(ObjID))
		{
			std::shared_ptr<GameEngineNetObject> NewNetObj = nullptr;
			NewNetObj = NetworkManager::CreateNetActor(_Packet->ActorType, ObjID);
			NewNetObj->SetControll(NetControllType::NetControll);
		}

		//패킷이 Death처리 된 경우
		if (true == _Packet->IsDeath)
		{
			//네트워크와 연결 끊기
			GameEngineNetObject* NetObject = nullptr;
			NetObject = GameEngineNetObject::GetNetObject(ObjID);
			NetObject->NetDisconnect();

			GameEngineActor* ActorPtr = dynamic_cast<GameEngineActor*>(NetObject);
			if (nullptr == ActorPtr)
			{
				MsgAssert("해당 NetObejct가 GameEngineActor타입으로 캐스팅하지 못해서 Death처리를 할 수 없습니다.");
				return;
			}
			ActorPtr->Death();
		}

		//패킷이 Death처리 되지 않은 경우에만 엑터쪽에 패킷 전달
		else
		{
			//Player가 스스로 처리할 수 있게 자료구조에 저장
			GameEngineNetObject::PushNetObjectPacket(_Packet);
		}
	});



	//----------------------------------------------------------------------------------------------------------------------------------------------------
	//MessageChatPacket 처리
	//----------------------------------------------------------------------------------------------------------------------------------------------------
	NetInst->Dispatcher.AddHandler<MessageChatPacket>(
		[](std::shared_ptr<MessageChatPacket> _Packet)
	{
		NetworkGUI::GetInst()->PrintLog(_Packet->Message);
	});
	


	//----------------------------------------------------------------------------------------------------------------------------------------------------
	//LinkObjectPacket 처리
	//----------------------------------------------------------------------------------------------------------------------------------------------------
	NetInst->Dispatcher.AddHandler<LinkObjectPacket>(
		[](std::shared_ptr<LinkObjectPacket> _Packet)
	{
		GameEngineNetObject* ObjPtr = reinterpret_cast<GameEngineNetObject*>(_Packet->Ptr);
		ObjPtr->InitNetObject(_Packet->GetObjectID(), NetInst);
		ObjPtr->SetUserControllType();
	});



	//----------------------------------------------------------------------------------------------------------------------------------------------------
	//FsmChangePacket 처리
	//----------------------------------------------------------------------------------------------------------------------------------------------------

	NetInst->Dispatcher.AddHandler<FsmChangePacket>(
		[](std::shared_ptr<FsmChangePacket> _Packet)
	{
		unsigned int ObjID = _Packet->GetObjectID();

		//해당 NetObejctID의 객체가 존재하지 않는 수행하지 않음
		if (false == GameEngineNetObject::IsNetObject(ObjID))
			return;


		//클라로 부터 받은 패킷이 현재 레벨과 다른 경우
		if (CurLevelType != _Packet->LevelType)
			return;

		//네트워크 연결이 끊긴 경우에도 수행하지 않음
		GameEngineNetObject* NetObj = nullptr;
		NetObj = GameEngineNetObject::GetNetObject(ObjID);
		if (true == NetObj->IsNetDisconnected())
			return;

		//각자 스스로 처리할 수 있게 자료구조에 저장
		GameEngineNetObject::PushNetObjectPacket(_Packet);
	});
}