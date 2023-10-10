#pragma once
#include <GameEngineCore/GameEngineGUI.h>
#include "StageDatas.h"

class FieldMapObjEditGUI : public GameEngineGUIWindow
{
public:
	// construtor destructor
	FieldMapObjEditGUI();
	~FieldMapObjEditGUI();

	// delete Function
	FieldMapObjEditGUI(const FieldMapObjEditGUI& _Other) = delete;
	FieldMapObjEditGUI(FieldMapObjEditGUI&& _Other) noexcept = delete;
	FieldMapObjEditGUI& operator=(const FieldMapObjEditGUI& _Other) = delete;
	FieldMapObjEditGUI& operator=(FieldMapObjEditGUI&& _Other) noexcept = delete;

protected:
	void OnGUI(std::shared_ptr<GameEngineLevel> _Level, float _DeltaTime) override;

private:
	void ShowTransformInfo(std::shared_ptr<GameEngineObject> _Obj = nullptr); // 오브젝트 트랜스폼 툴

	class StageEditLevel* Parent = nullptr;
	void CreateStage(StageData _Data);
	
	std::vector<StageData>* AllData_ptr = nullptr;
	size_t* Stage_current_ptr = nullptr;
	size_t* FieldMap_current_ptr = nullptr;

	void FieldMapCombo(std::vector<StageData>& _AllData, size_t& _Stage_current, size_t& _FieldMap_current);
	
	int ObjType_current = 0;
	void ObjTypeSelectCombo();
	const char* ObjType_items[3] = 
	{ "WallLight_On", "WallLight_Off", "ReflectionSetter" };

	void InputObject(std::vector<StageData>& _AllData, size_t& _Stage_current, size_t& _FieldMap_current);

	int Obj_current = 0;
	void ObjCombo(std::vector<StageData>& _AllData, size_t& _Stage_current, size_t& _FieldMap_current);
};