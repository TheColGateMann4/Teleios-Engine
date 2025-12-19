#pragma once
#include "Graphics/FullscreenPasses/FullscreenPass.h"

class Graphics;
class Pipeline;

class PostProcessing
{
private:
	enum Effect
	{
		None,
		Fog,
		DepthOfField,

		numEffects
	};

public:
	PostProcessing(Graphics& graphics);

	void Initialize(Graphics& graphics, Pipeline& pipeline);

	void Update(Graphics& graphics, Pipeline& pipeline);

public:
	void ApplyEffect(Graphics& graphics, Pipeline& pipeline);

private:
	void DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline);
	static const char* EffectToString(Effect effect);
	static bool DrawEffectPicker(const char* label, Effect& currentEffect);

private:
	Effect m_currentEffect = Effect::None;
	std::vector<std::shared_ptr<FullscreenPass>> m_effects;
};