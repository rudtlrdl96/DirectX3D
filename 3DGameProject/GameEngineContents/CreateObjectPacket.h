#pragma once
#include <GameEngineBase/GameEnginePacket.h>
#include <GameEngineCore/GameEngineTransform.h>

#include "PacketEnum.h"

class CreateObjectPacket : public GameEnginePacket
{
public:
	static const PacketEnum Type = PacketEnum::CreateObjectPacket;

public:
	CreateObjectPacket()
	{
		SetPacketID(Type);
	}

	~CreateObjectPacket() override
	{

	}

	unsigned int ActorType = -1;
	float4 Rotation;
	float4 Position;

protected:
	void Serialize(GameEngineSerializer& _Ser) override
	{
		GameEnginePacket::Serialize(_Ser);

		_Ser << ActorType;
		_Ser << Rotation;
		_Ser << Position;
	}

	void DeSeralize(GameEngineSerializer& _Ser) override
	{
		GameEnginePacket::DeSeralize(_Ser);

		_Ser >> ActorType;
		_Ser >> Rotation;
		_Ser >> Position;
	}


private:

};