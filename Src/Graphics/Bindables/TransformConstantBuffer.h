#pragma once
#include "Binding.h"
#include "ConstantBuffer.h"

class Graphics;
class Camera;
class CommandList;
class ObjectTransform;

class TransformConstantBuffer : public Bindable, public RootParameterBinding
{
public:
	TransformConstantBuffer(Graphics& graphics, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::VertexShader, 0} });

public:
	void SetParentPtr(ObjectTransform* pObjectTransform);

	void Update(Graphics& graphics, Camera& camera);

	NonCachedConstantBuffer* GetBuffer() const;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList, TargetSlotAndShader& target) override;

	virtual void BindToRootSignature(RootSignature* rootSignature, TargetSlotAndShader& target) override;

	virtual BindableType GetBindableType() const override;

private:
	std::shared_ptr<NonCachedConstantBuffer> m_buffer;
	ObjectTransform* m_pObjectTransform = nullptr;
};
