#pragma once
#include <GameEngineCore/GameEngineFBXRenderer.h>

// ���� :
class ShaderTestActor : public GameEngineActor
{
public:
	// constrcuter destructer
	ShaderTestActor();
	~ShaderTestActor();

	// delete Function
	ShaderTestActor(const ShaderTestActor& _Other) = delete;
	ShaderTestActor(ShaderTestActor&& _Other) noexcept = delete;
	ShaderTestActor& operator=(const ShaderTestActor& _Other) = delete;
	ShaderTestActor& operator=(ShaderTestActor&& _Other) noexcept = delete;

	void Start() override;

protected:

private:
	std::shared_ptr<GameEngineFBXRenderer> TestRenderer = nullptr;


};

