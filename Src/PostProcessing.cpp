#include "PostProcessing.h"
#include "Graphics.h"
#include "Pipeline.h"
#include "CommandList.h"
#include "TempCommandList.h"


#include <imgui.h>

#include "TempCommandList.h"

#include "FullscreenPasses/FogFullscreenPass.h"
#include "FullscreenPasses/DepthOfFieldFullscreenPass.h"


PostProcessing::PostProcessing(Graphics& graphics)
{
	// requesting size for RT and DS SRV's. Each one has special SRV for one frame
	graphics.GetDescriptorHeap().RequestMoreSpace(graphics.GetBufferCount() * 2);

	m_effects.push_back(std::make_shared<FullscreenPass>(graphics)); // base class is equivelent to no effect, it just passes pixels through
	m_effects.push_back(std::make_shared<FogFullscreenPass>(graphics));
	m_effects.push_back(std::make_shared<DepthOfFieldFullscreenPass>(graphics));

	THROW_INTERNAL_ERROR_IF("FullscreenPass vector didn't match enum", m_effects.size() != Effect::numEffects);
}

void PostProcessing::Initialize(Graphics& graphics, Pipeline& pipeline)
{
	for (auto& effect : m_effects)
		effect->Initialize(graphics, pipeline);
}

void PostProcessing::Update(Graphics& graphics, Pipeline& pipeline)
{
	for (auto& effect : m_effects)
		effect->Update(graphics, pipeline);

	DrawImguiPropeties(graphics, pipeline);
}

void PostProcessing::ApplyEffect(Graphics& graphics, Pipeline& pipeline)
{
	m_effects.at(static_cast<int>(m_currentEffect))->Draw(graphics, pipeline);
}

void PostProcessing::DrawImguiPropeties(Graphics& graphics, Pipeline& pipeline)
{
	if (ImGui::Begin("Post Processing"))
	{
		PostProcessing::DrawEffectPicker("Current Effect", m_currentEffect);
		ImGui::Separator();
		ImGui::Text("Settings:");

		m_effects.at(static_cast<int>(m_currentEffect))->DrawImguiPropeties(graphics, pipeline);
	}

	ImGui::End();
}

const char* PostProcessing::EffectToString(Effect effect)
{
	switch (effect)
	{
		case Effect::None:           return "None";
		case Effect::Fog:            return "Fog";
		case Effect::DepthOfField:   return "Depth Of Field";
		default:                     return "Unknown";
	}
}

bool PostProcessing::DrawEffectPicker(const char* label, Effect& currentEffect)
{
	bool changed = false;

	if (ImGui::BeginCombo(label, PostProcessing::EffectToString(currentEffect)))
	{
		for (int i = 0; i < static_cast<int>(Effect::numEffects); i++)
		{
			Effect effect = static_cast<Effect>(i);
			bool isSelected = (currentEffect == effect);

			if (ImGui::Selectable(PostProcessing::EffectToString(effect), isSelected))
			{
				currentEffect = effect;
				changed = true;
			}

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	return changed;
}