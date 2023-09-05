#include "PrecompileHeader.h"
#include "Nero_ShopTitleBar.h"
#include "Shop_TitleButton.h"
#include "Shop_NeroSkillUI.h"
#include "Shop_ItemButton.h"
#include <GameEngineCore/GameEngineLevel.h>
Nero_ShopTitleBar::Nero_ShopTitleBar()
{
}

Nero_ShopTitleBar::~Nero_ShopTitleBar()
{
}

void Nero_ShopTitleBar::Start()
{
	SkillButton = GetLevel()->CreateActor<Shop_TitleButton>();
	SkillButton->SetUIText("SKILLS");
	SkillButton->GetTransform()->SetLocalPosition({ -600.0f,370.0f,0.0f });
	SkillButton->SetEvent([this]()
		{
			TitleIndex = 0;
		});
	DBButton = GetLevel()->CreateActor<Shop_TitleButton>();
	DBButton->SetUIText("DEVIL BREAKERS");
	DBButton->GetTransform()->SetLocalPosition({ -300.0f,370.0f,0.0f });
	DBButton->SetEvent([this]()
		{
			TitleIndex = 1;
		});
	//�׽�Ʈ
	RQButton = GetLevel()->CreateActor<Shop_NeroSkillUI>();
	RQButton->GetTransform()->SetLocalPosition({ 0.0f,0.0f,0.0f });
}

void Nero_ShopTitleBar::Update(float _Delta)
{
	if (TitleIndex == 0)
	{
		DBButton->SetSelectValue(false);
		SkillButton->SetSelectValue(true);
		LerpTime = 0.0f;
	}
	else if (TitleIndex == 1)
	{
		LerpTime += _Delta;
		RQButton->GetTransform()->SetLocalPosition(float4::LerpClamp(float4::ZERONULL, { -1000.0f,0.0f,0.0f }, LerpTime * 2));
		if (RQButton->GetTransform()->GetLocalPosition() == float4(-1000.0f, 0.0f, 0.f))
		{
			RQButton->Off();
		}
		SkillButton->SetSelectValue(false);
		DBButton->SetSelectValue(true);
	}
	if (true == GameEngineInput::IsUp("UI_Tab"))
	{
		if (TitleIndex == 1)
		{
			TitleIndex = 0;
		}
		else
		{
			TitleIndex++;
		}
	}
}

