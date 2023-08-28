#include "PrecompileHeader.h"
#include "StageEditLevel.h"

#include <GameEngineCore/GameEngineGUI.h>

#include "NetworkManager.h"
#include "StageEditGUI.h"
#include "EditorUI.h"
#include "FreeCameraActor.h"

StageEditLevel::StageEditLevel()
{
}

StageEditLevel::~StageEditLevel()
{
}

void StageEditLevel::Start()
{
	if (GetMainCamera() != nullptr)
	{
		GetMainCamera()->SetProjectionType(CameraType::Perspective);
	}
	GetMainCamera()->GetTransform()->SetLocalPosition({0,0,-500});

	GameEngineDirectory NewDir;
	NewDir.MoveParentToDirectory("ContentResources");
	NewDir.Move("ContentResources");
	NewDir.Move("Texture");
	NewDir.Move("StageEditorUI");

	std::vector<GameEngineFile> File = NewDir.GetAllFile({ ".Png", });

	for (size_t i = 0; i < File.size(); i++)
	{
		GameEngineTexture::Load(File[i].GetFullPath());
	}

	AcEditorUI = CreateActor<EditorUI>();
	FreeCam = CreateActor<FreeCameraActor>();
}

void StageEditLevel::Update(float _DeltaTime)
{
	//Test
	if (true == GameEngineInput::IsDown("SelectLevel_01"))
	{
		GameEngineCore::ChangeLevel("MainLevel");
		return;
	}
}

void StageEditLevel::LevelChangeStart()
{
	if (Editor == nullptr)
	{
		std::shared_ptr<GameEngineGUIWindow> EditGUI = GameEngineGUI::GUIWindowCreate<StageEditGUI>("StageEditGUI");
		Editor = EditGUI;
	}
	Editor->On();
}

void StageEditLevel::LevelChangeEnd()
{
	Editor->Off();
}

