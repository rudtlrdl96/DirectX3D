#include "PrecompileHeader.h"
#include "ShopLevel.h"
#include "ShopBG.h"
#include "Nero_InvenToryUI.h"
#include "FadeEffect.h"
#include "Nero_ShopUI.h"
#include "Vergil_ShopUI.h"
#include "Char_ChoiceUI.h"
#include <GameEngineCore/GameEngineFont.h>
#include <GameEngineCore/GameEngineTexture.h>
#include <GameEnginePlatform/GameEngineInput.h>

//�׽�Ʈ
#include "testarea.h"
ShopLevel::ShopLevel()
{
}

ShopLevel::~ShopLevel()
{

}

void ShopLevel::Start()
{

	GetMainCamera()->SetProjectionType(CameraType::Orthogonal);
	GetMainCamera()->GetTransform()->SetLocalPosition({ 0, 0, -1000.0f });
	GetMainCamera()->GetCamTarget()->DepthSettingOff();

	FEffect = GetLastTarget()->CreateEffect<FadeEffect>();
	std::shared_ptr<ShopBG> ShopBGPtr = CreateActor<ShopBG>();
	ShopBGPtr->GetTransform()->SetLocalPosition({ 0.0f,0.0f,0.0f });
	//std::shared_ptr<testarea> testareaPtr = CreateActor<testarea>();
	//testareaPtr->GetTransform()->SetLocalPosition({ 0.0f,0.0f,0.0f });

}

void ShopLevel::Update(float _DeltaTime)
{
	if (GameEngineInput::IsDown("UIDEBUGMODE"))
	{
		IsDebugSwitch();
	}
	BaseLevel::Update(_DeltaTime);
}

void ShopLevel::LevelChangeStart()
{
	if (Char_ChoiceUI::GetPlayerType() == ChoicePlayerType::NERO)
	{
		std::shared_ptr<Nero_ShopUI> Nero_ShopUIPtr = CreateActor<Nero_ShopUI>();
	}
	else if (Char_ChoiceUI::GetPlayerType() == ChoicePlayerType::VERGIL)
	{
		std::shared_ptr<Vergil_ShopUI> Vergil_ShopUIPtr = CreateActor<Vergil_ShopUI>();
	}
}

void ShopLevel::LevelChangeEnd()
{
}
