#include "PrecompileHeader.h"
#include "Enemy_RedEmpusa.h"
#include <GameEngineCore/GameEngineFBXRenderer.h>


#include "AnimationEvent.h"

Enemy_RedEmpusa::Enemy_RedEmpusa() 
{
}

Enemy_RedEmpusa::~Enemy_RedEmpusa() 
{
}

void Enemy_RedEmpusa::EnemyMeshLoad()
{
	std::string Path = GameEnginePath::GetFileFullPath("ContentResources",
		{
			"Character", "Enemy", "em0102", "mesh"
		}, "em0102.FBX");

	GameEngineFBXMesh::Load(Path);

	EnemyRenderer->SetFBXMesh("em0102.fbx", "NoneAlphaMesh");
	EnemyRenderer->GetTransform()->SetLocalScale({ 0.1f, 0.1f, 0.1f });
}

void Enemy_RedEmpusa::EnemyTypeLoad()
{
	EnemyCodeValue = EnemyCode::RedEmpusa;
	EnemyTypeValue = EnemyType::Normal;
	EnemySizeValue = EnemySize::Middle;
}

void Enemy_RedEmpusa::EnemyAnimationLoad()
{
	//Animation정보 경로를 찾아서 모든animation파일 로드
	GameEngineDirectory NewDir;
	NewDir.MoveParentToDirectory("ContentResources");
	NewDir.Move("ContentResources");
	NewDir.Move("Character");
	NewDir.Move("Enemy");
	NewDir.Move("em0102");
	NewDir.Move("Animation");
	AnimationEvent::LoadAll(
		{
			.Dir = NewDir.GetFullPath().c_str(), .Renderer = EnemyRenderer,
			.Objects = {(GameEngineObject*)MonsterCollision.get()},
			.CallBacks_void =
			{
				std::bind([=] {CheckBool = true; }),
				//Functional사용할 함수들 적는곳
			},
			.CallBacks_int = {
				std::bind(&GameEngineFSM::ChangeState, &EnemyFSM, std::placeholders::_1)} //ChangeState,인자있음
		}
	);
}

void Enemy_RedEmpusa::EnemyCreateFSM()
{
}
