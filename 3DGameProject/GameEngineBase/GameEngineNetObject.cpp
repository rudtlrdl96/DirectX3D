#include "PrecompileHeader.h"
#include "GameEngineNetObject.h"

#include "GameEnginePacket.h"

std::atomic<int> GameEngineNetObject::AtomicObjectID;
std::mutex GameEngineNetObject::ObjectLock;
std::map<int, GameEngineNetObject*> GameEngineNetObject::AllNetObjects;
std::map<int, GameEngineNetObject*> GameEngineNetObject::MainThreadNetObjects;



void GameEngineNetObject::PushNetObjectPacket(std::shared_ptr<GameEnginePacket> _Packet)
{
	//패킷으로부터 오브젝트 아이디를 얻어온 후
	int Id = _Packet->GetObjectID();
	if (false == AllNetObjects.contains(Id))
	{
		MsgAssert("존재하지 않는 오브젝트에 패킷이 날아왔습니다.");
		return;
	}

	//해당 오브젝트의 자료구조에 패킷을 집어넣는다
	AllNetObjects[Id]->PushPacket(_Packet);
}

void GameEngineNetObject::Update_ProcessPackets()
{
	ObjectLock.lock();
	MainThreadNetObjects = AllNetObjects;
	ObjectLock.unlock();


	for (const std::pair<int, GameEngineNetObject*>& Pair : MainThreadNetObjects)
	{
		//연결된 적이 없는 경우
		if (false == Pair.second->IsNet())
			continue;

		//내가 조종하는 컨트롤 타입일 경우엔 패킷을 수신받지 않음
		if (NetControllType::UserControll == Pair.second->ControllType)
			continue;

		//연결이 끊긴 경우
		if (true == Pair.second->IsDisconnect)
			continue;

		//처리할 패킷이 없는 경우
		if (true == Pair.second->Packets.empty())
			continue;

		//패킷 처리
		Pair.second->Update_ProcessPacket();
	}

	MainThreadNetObjects.clear();
}


void GameEngineNetObject::Update_SendPackets(float _DeltaTime)
{
	ObjectLock.lock();
	MainThreadNetObjects = AllNetObjects;
	ObjectLock.unlock();

	
	for (const std::pair<int, GameEngineNetObject*>& Pair : MainThreadNetObjects)
	{
		//연결도 안된 경우
		if (false == Pair.second->IsNet())
			continue;

		//패킷을 수신받아 조종당하는 패킷의 경우엔 패킷을 전송하지 않음
		if (NetControllType::NetControll == Pair.second->ControllType)
			continue;

		//연결이 끊긴 경우
		if (true == Pair.second->IsDisconnect)
			continue;

		//패킷을 보내기
		Pair.second->Update_SendPacket(_DeltaTime);
	}
	MainThreadNetObjects.clear();
}

void GameEngineNetObject::ReleaseNetObject()
{
	//근데 이렇게 크게 Lock을 잡으면서 까지 릴리즈를 할 필요가 있을까?

	ObjectLock.lock();

	std::map<int, GameEngineNetObject*>::iterator ReleaseStartIter = AllNetObjects.begin();
	std::map<int, GameEngineNetObject*>::iterator ReleaseEndIter = AllNetObjects.end();

	while (ReleaseStartIter != ReleaseEndIter)
	{
		GameEngineNetObject* NetObjPtr = ReleaseStartIter->second;
		if (true == NetObjPtr->IsDisconnect || false == NetObjPtr->IsNet())
		{
			ReleaseStartIter = AllNetObjects.erase(ReleaseStartIter);
			continue;
		}

		++ReleaseStartIter;
	}

	ObjectLock.unlock();
}


GameEngineNetObject::GameEngineNetObject()
{

}

GameEngineNetObject::~GameEngineNetObject()
{

}

void GameEngineNetObject::InitNetObject(int _ObjectID, GameEngineNet* _Net)
{
	ObjectID = _ObjectID;

	ObjectLock.lock();
	AllNetObjects.insert(std::pair<int, GameEngineNetObject*>(ObjectID, this));
	ObjectLock.unlock();

	Net = _Net;
}

void GameEngineNetObject::PushPacket(std::shared_ptr<GameEnginePacket> _Packet)
{
	Packets.push_back(_Packet);
}