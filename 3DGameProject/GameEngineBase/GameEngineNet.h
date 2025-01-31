#pragma once

#include <atomic>
#include <mutex>
#include "GameEngineDispatcher.h"

//서버와 클라가 공통으로 사용하는 클래스
class GameEngineNet
{
public:
	// constrcuter destructer
	GameEngineNet();
	~GameEngineNet();

	// delete Function
	GameEngineNet(const GameEngineNet& _Other) = delete;
	GameEngineNet(GameEngineNet&& _Other) noexcept = delete;
	GameEngineNet& operator=(const GameEngineNet& _Other) = delete;
	GameEngineNet& operator=(GameEngineNet&& _Other) noexcept = delete;

	//특정 상대에만 데이터를 전송하는 함수(서버가 Accept스레드에서 클라와 처음 연결될 때 사용)
	static void Send(SOCKET _Socket, const char* Data, unsigned int _Size);

	virtual void Send(const char* Data, unsigned int _Size, int _IgnoreID = -1) = 0;

	//인자로 받은 패킷을 직렬화하여서 위의 Send 멤버함수를 통해 전송
	void SendPacket(std::shared_ptr<class GameEnginePacket> _Packet, int _IgnoreID = -1);



	//서버에 연결되었는지 여부
	inline bool IsNet()
	{
		return IsNetValue;
	}

	//패킷을 어떻게 처리할 지에 대한 객체
	GameEngineDispatcher Dispatcher;

	//Recv스레드에서 수신받은 패킷을  메인스레드에서 처리하는 부분(모든 레벨 맨 처음 호출시켜주어야 함)
	void UpdatePacket();

protected:
	//상대방과 연결되어 있는 소켓, 자기 자신의 Net포인터(서버면 서버의 포인터, 클라면 클라의 포인터)
	static void RecvThreadFunction(SOCKET _Socket, GameEngineNet* _Net);

private:
	std::atomic<bool> IsNetValue = true;


	//아래 자료구조 RecvPacket를 위한 뮤텍스
	std::mutex RecvPacketLock;

	//Recv스레드에서 수신받은 패킷을 바로 처리할 순 없다(메인스레드는 열심히 렌더링을 하고 있는 중일수도 있기 때문에)
	//그래서 Recv스레드에서 수신받은 패킷은 아래 자료구조에 모았다가,
	//메인스레드에서 처리한다
	std::list<std::shared_ptr<GameEnginePacket>> RecvPacket;

	//메인스레드에서 패킷 처리를 위한 별도 자료구조(Lock걸고 ProcessPackets = RecvPacket 할 예정)
	std::list<std::shared_ptr<GameEnginePacket>> ProcessPackets;


	static bool SearchPacketData(GameEngineSerializer& _Ser, unsigned int& _PacketType, unsigned int& _PacketSize);
};

