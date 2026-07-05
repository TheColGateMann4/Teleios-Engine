#pragma once
#include "Shaders/TargetShaders.h"
#include "Graphics/Bindables/Bindable.h"

class RootSignatureLayout
{
	struct RootSignatureLayoutBinding
	{
		RootSignatureBindable* bindable;
		RootBinding binding;
	};

public:
	void AddParam(RootSignatureBindable* bindable, RootBinding binding);

	void BindToCommandList(Graphics& graphics, CommandList* commandList) const;

private:
	std::vector<RootSignatureLayoutBinding> m_bindings = {};
};