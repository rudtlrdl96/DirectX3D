#pragma once
#include <GameEngineCore/GameEngineRenderTarget.h>

// 설명 :

class JudgementCut : public GameEnginePostProcess
{
	class JudgementBuffer
	{
	public:
		float4 Lines[32] = {};
		int LineCount = 0;
		float LineThickness = 1.5f;
		float LineInter = 0.5f;
	};

public:
	// constrcuter destructer
	JudgementCut();
	~JudgementCut();

	// delete Function
	JudgementCut(const JudgementCut& _Other) = delete;
	JudgementCut(JudgementCut&& _Other) noexcept = delete;
	JudgementCut& operator=(const JudgementCut& _Other) = delete;
	JudgementCut& operator=(JudgementCut&& _Other) noexcept = delete;

	// 효과의 라인굵기를 설정합니다. 기본 = 1.5f
	inline void SetLineThickness(float _Value)
	{
		ShaderBuffer.LineThickness = _Value;
	}

	// 효과의 스피드를 설정합니다. 기본 = 25.0f
	inline void SetSpeed(float _Speed)
	{
		Speed = _Speed;
	}

	// 효과의 최대 값을 설정합니다. 기본 = 5.0f
	inline void SetMaxCut(float _MaxCut)
	{
		MaxCut = _MaxCut;
	}

	void AddLine(float4 _Line);

	void JudgementCutOn();
	void JudgementCutOff();

protected:
	void Start(GameEngineRenderTarget* _Target) override;
	void Effect(GameEngineRenderTarget* _Target, float _DeltaTime) override;

private:
	std::shared_ptr<GameEngineRenderUnit> JudgementUnit;

	RenderBaseValue BaseValue = RenderBaseValue();
	JudgementBuffer ShaderBuffer = JudgementBuffer();

	bool IsEffectOn = false;

	float Speed = 25.0f;
	float MaxCut = 5.0f;
};

