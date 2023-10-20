#include "PrecompileHeader.h"
#include "Shop_VergilMirgeUI.h"
#include "Shop_TitleButton.h"
#include "Shop_ItemButton.h"
#include "Shop_ExplaneUI.h"
#include "Vergil_ShopUI.h"
#include <GameEngineCore/GameEngineFontRenderer.h>
#include <GameEngineCore/GameEngineLevel.h>
Shop_VergilMirgeUI::Shop_VergilMirgeUI()
{
}

Shop_VergilMirgeUI::~Shop_VergilMirgeUI()
{
}

void Shop_VergilMirgeUI::Start()
{

	Shop_ItemButton::CreateItemUI(GetLevel(), Pos_0, { ._Name = "SPIRAL BLADE",._Price = "1200",._png = "StreakSkill.png",._BaseNone = "Shop_SkillMovieNone.png", ._BaseSelect = "Shop_SkillMovie.png",.IsValue = true ,.Value = 1 }, GetTransform(), float4(157.0f, 100.0f, 0.0f), float4(-160.0f, 0.0f, 0.0f));
	Shop_ItemButton::CreateItemUI(GetLevel(), Pos_1, { ._Name = "STORM BLADE",._Price = "4000",._png = "SplitSkill.png",._BaseNone = "Shop_SkillMovieNone.png", ._BaseSelect = "Shop_SkillMovie.png" ,.IsValue = true,.Value = 1 }, GetTransform(), float4(157.0f, 100.0f, 0.0f), float4(-160.0f, 0.0f, 0.0f));
	Shop_ItemButton::CreateItemUI(GetLevel(), Pos_2, { ._Name = "BLESTERING BLADE",._Price = "7500",._png = "RedQuennCSkill.png",._BaseNone = "Shop_SkillMovieNone.png", ._BaseSelect = "Shop_SkillMovie.png",.IsValue = true ,.Value = 1 }, GetTransform(), float4(157.0f, 100.0f, 0.0f), float4(-160.0f, 0.0f, 0.0f));
	Shop_ItemButton::CreateItemUI(GetLevel(), Pos_3, { ._Name = "HEAVY RAIN BLAND",._Price = "9000",._png = "RouletteSpinSkill.png",._BaseNone = "Shop_SkillMovieNone.png", ._BaseSelect = "Shop_SkillMovie.png" ,.IsValue = true ,.Value = 1 }, GetTransform(), float4(157.0f, 100.0f, 0.0f), float4(-160.0f, 0.0f, 0.0f));

	ExplanePtr = GetLevel()->CreateActor<Shop_ExplaneUI>();
	ExplanePtr->GetTransform()->SetParent(GetTransform());
	ExplanePtr->GetTransform()->SetWorldPosition({ 0.0f,0.0f,0.0f });
	SetValue();
}

void Shop_VergilMirgeUI::Update(float _Delta)
{
	if (Vergil_ShopUI::Vergil_ShopBar->GetUIIndex() == 1)
	{
		SetIndex();
		Shop_ItemButton::SecoundSkills[PrevIndex]->SetSelectValue(false);
		Shop_ItemButton::SecoundSkills[Index]->SetSelectValue(true);
		SetExText();
	}
	else
	{
		Shop_ItemButton::SecoundSkills[PrevIndex]->SetSelectValue(false);
		Shop_ItemButton::SecoundSkills[Index]->SetSelectValue(false);
		PrevIndex = 3;
		Index = 0;
	}
}

void Shop_VergilMirgeUI::SetIndex()
{
	if (true == GameEngineInput::IsUp("UI_UP"))
	{
		PrevIndex = Index;
		if (Index == 0)
		{
			Index = 3;
		}
		else
		{
			Index--;
		}


	}
	if (true == GameEngineInput::IsUp("UI_DOWN"))
	{
		PrevIndex = Index;
		if (Index == 3)
		{
			Index = 0;
		}
		else
		{
			Index++;
		}



	}
}

void Shop_VergilMirgeUI::SetExText()
{
	switch (Index)
	{
	case 0:
		ExplanePtr->SetExplaneText({ ._EName = "SPIRAL BLADES" ,._KName = "스파이럴 블레이드" ,._Explane1 = "미라지 블레이드로 자신의 주변을 에워",._Explane2 = "싸 방패로 활용합니다.",._Explane3 = "발동하면 DT게이지를 소모합니다." });
		ExplanePtr->SetSecoundText({ ._EName = "COMMAND" ,._Explane1 = "SpiralbladeExplane.png" });
		ExplanePtr->GetExplane()->GetTransform()->SetLocalPosition({ 457.0f,-139.0f,0.0f });
		break;
	case 1:
		ExplanePtr->SetExplaneText({ ._EName = "STORM BLADES" ,._KName = "스톰 블레이드" ,._Explane1 = "여러 개의 검으로 적을 에워싼 다음,",._Explane2 = "동시에 공격하여 공중으로 날려버립니다." ,._Explane3 = "발동하면 DT게이지를 소모합니다." });
		ExplanePtr->SetSecoundText({ ._EName = "COMMAND" ,._Explane1 = "StormBladeExplane.png" });
		ExplanePtr->GetExplane()->GetTransform()->SetLocalPosition({ 506.0f,-154.0f,0.0f });
		break;
	case 2:
		ExplanePtr->SetExplaneText({ ._EName = "BLESTERING BLADES" ,._KName = "블리스터링 블레이드" ,._Explane1 = "자신의 주변에 미라지 블레이드를",._Explane2 = "만들어 적을 향해 연속으로 발사합니다.",._Explane3 = "발동하면 DT 게이지를 소모합니다." });
		ExplanePtr->SetSecoundText({ ._EName = "COMMAND" ,._Explane1 = "BlesteringBladeExplane.png" });
		ExplanePtr->GetExplane()->GetTransform()->SetLocalPosition({ 512.0f,-154.0f,0.0f });
		break;
	case 3:
		ExplanePtr->SetExplaneText({ ._EName = "HEAVY RAIN BLAND" ,._KName = "헤비레인 블레이드" ,._Explane1 = "적의 머리 위로 무수히 많은 미라지",._Explane2 = "블레이드를 만들어 내어 검의 비를",._Explane3 = "내리고, 일시적으로 제자리에 적을"
			,._Explane4 = "정시켜 놓습니다." ,._Explane5 = "발동하면 DT 게이지를 소모합니다."});
		ExplanePtr->SetSecoundText({ ._EName = "COMMAND" ,._Explane1 = "HeavyRainBladeExplane.png" });
		ExplanePtr->GetExplane()->GetTransform()->SetLocalPosition({ 540.0f,-154.0f,0.0f });
		break;
	default:
		break;
	}
}

void Shop_VergilMirgeUI::SetValue()
{
	for (int i = 0; i < Shop_ItemButton::SecoundSkills.size(); i++)
	{
		Shop_ItemButton::SecoundSkills[i]->SetSizeValue(true);
	}
}

