#include "PrecompileHeader.h"
#include "Nero_ShopLevel.h"
#include "ShopBG.h"
#include "FadeEffect.h"
#include <GameEngineCore/GameEngineFont.h>
#include <GameEngineCore/GameEngineTexture.h>
#include <GameEnginePlatform/GameEngineInput.h>
Nero_ShopLevel::Nero_ShopLevel()
{
}

Nero_ShopLevel::~Nero_ShopLevel()
{

}

void Nero_ShopLevel::Start()
{

	GameEngineDirectory NewDir;
	NewDir.MoveParentToDirectory("ContentResources");
	NewDir.Move("ContentResources");
	NewDir.Move("Texture");
	NewDir.Move("UI");
	NewDir.Move("ShopTexture");

	std::vector<GameEngineFile> File = NewDir.GetAllFile({ ".Png", });


	for (size_t i = 0; i < File.size(); i++)
	{
		GameEngineTexture::Load(File[i].GetFullPath());
	}
	GetMainCamera()->SetProjectionType(CameraType::Orthogonal);
	GetMainCamera()->GetTransform()->SetLocalPosition({ 0, 0, -1000.0f });
	GetMainCamera()->GetCamTarget()->DepthSettingOff();

	FEffect = GetLastTarget()->CreateEffect<FadeEffect>();
	std::shared_ptr<ShopBG> ShopBGPtr = CreateActor<ShopBG>();
	ShopBGPtr->GetTransform()->SetLocalPosition({ 0.0f,0.0f,0.0f });

}

void Nero_ShopLevel::Update(float _DeltaTime)
{
	if (GameEngineInput::IsDown("UIDEBUGMODE"))
	{
		IsDebugSwitch();
	}
	BaseLevel::Update(_DeltaTime);
}

void Nero_ShopLevel::LevelChangeStart()
{
	FEffect->FadeOut();
}

void Nero_ShopLevel::LevelChangeEnd()
{
}
