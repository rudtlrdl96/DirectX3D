#pragma once
#include "BaseLevel.h"

class ThreadLoadingLevel : public BaseLevel
{
public:
	//이 함수를 통해 로딩 레벨을 활성화 시킵니다
	static void ChangeLevel(const std::string_view& _NextLevelName)
	{
		NextLevelName = _NextLevelName;
		GameEngineCore::ChangeLevel("ThreadLoadingLevel");
	}

	//이 함수를 통해 로딩 레벨을 활성화 시킵니다
	template <typename LevelType>
	static void ChangeLevel()
	{
		const type_info& Info = typeid(LevelType);
		std::string LevelName = Info.name();
		LevelName.replace(0, 6, "");

		ChangeLevel(LevelName);
	}



	ThreadLoadingLevel();
	~ThreadLoadingLevel();

	ThreadLoadingLevel(const ThreadLoadingLevel& _Other) = delete;
	ThreadLoadingLevel(ThreadLoadingLevel&& _Other) noexcept = delete;
	ThreadLoadingLevel& operator=(const ThreadLoadingLevel& _Other) = delete;
	ThreadLoadingLevel& operator=(const ThreadLoadingLevel&& _Other) noexcept = delete;

protected:
	void Start() override;
	void Update(float _DeltaTime) override;
	void LevelChangeStart() override;

private:
	static std::string NextLevelName;

	//<Level이름, 콜백>
	std::map<std::string, std::vector<std::function<void()>>> AllLoadCallBack;
	size_t LoadWorkCount = 0;
	std::atomic<size_t> ExcuteWorkCount = 0;
	float LoadingPercent = 0.f;
	std::shared_ptr<class GameEngineActor> ThreadTestActor = nullptr;

	
	/// <summary>
	/// 특정 레벨로 전환될 때 리소스 1개를 스레드 로딩시키는 함수입니다.
	/// </summary>
	/// <typeparam name="LevelType">어떤 레벨로 넘어갈 때 스레드 로딩을 호출할 지 넣어주세요</typeparam>
	/// <typeparam name="ResourceType">어떤 타입의 리소스를 로드하는 지 넣어주세요</typeparam>
	/// <param name="_Path">ContentResources 이후의 경로를 넣어주세요</param>
	template <typename LevelType, typename ResourceType>
	void PushLoadCallBack(const std::string_view& _Path)
	{
		//LevelType 템플릿 인자의 클래스 명 구하기
		const type_info& Info = typeid(LevelType);
		std::string LevelName = Info.name();
		LevelName.replace(0, 6, "");

		//콜백 저장
		const std::string& Path = _Path.data();
		AllLoadCallBack[LevelName].push_back([Path]()
		{
			GameEnginePath FilePath(Path);
			std::string FileName = FilePath.GetFileName();
			if (nullptr != ResourceType::Find(FileName))
				return;

			GameEngineDirectory StartDir;
			StartDir.MoveParentToDirectory("ContentResources");
			StartDir.Move("ContentResources");
			FileName = StartDir.GetFullPath() + "\\" + FilePath.GetFullPath();

			GameEngineDirectory CheckDir(FileName);
			if (false == CheckDir.IsExists())
			{
				MsgAssert("ThreadLoadingLevel에서 유효하지 않은 경로로 이동하였습니다.\n" + FileName);
				return;	
			}
			
			ResourceType::Load(CheckDir.GetFullPath());
		});
	}

	/// <summary>
	/// 특정 레벨로 전환될 때 해당 경로의 모든 리소스를 찾아 스레드 로딩 시키는 함수입니다
	/// </summary>
	/// <typeparam name="LevelType">어떤 레벨로 넘어갈 때 스레드 로딩을 호출할 지 넣어주세요</typeparam>
	/// <typeparam name="ResourceType">어떤 타입의 리소스를 로드하는 지 넣어주세요</typeparam>
	/// <param name="_Path">ContentResources 이후의 경로를 넣어주세요</param>
	/// <param name="_Extents">로드할 리소스들의 확장자를 넣어주세요</param>
	template <typename LevelType, typename ResourceType>
	void PushAllLoadCallBack(const std::string_view& _Path, const std::vector<std::string_view>& _Extents)
	{
		if (true == _Extents.empty())
		{
			MsgAssert("Thread로딩을 할 리소스의 확장자 타입을 안 넣어주었습니다");
			return;
		}

		GameEngineDirectory StartDir;
		StartDir.MoveParentToDirectory("ContentResources");
		StartDir.Move("ContentResources");

		std::string FilePath = StartDir.GetFullPath() + "\\" + GameEnginePath(_Path).GetFullPath();
		GameEngineDirectory CheckDir = GameEngineDirectory(FilePath);
		if (false == CheckDir.IsExists())
		{
			MsgAssert("ThreadLoadingLevel에서 유효하지 않은 경로로 이동하였습니다.\n" + FilePath);
			return;
		}

		const std::vector<GameEngineFile>& Files = CheckDir.GetAllFile(_Extents);
		for (const GameEngineFile& File : Files)
		{
			std::string RelativePath = File.GetFullPath();
			size_t CutIdx = RelativePath.find("ContentResources");
			RelativePath.replace(0, CutIdx + 17, "");
			PushLoadCallBack<LevelType, ResourceType>(RelativePath);
		}
	}

	void ThreadLoadStart();
};

