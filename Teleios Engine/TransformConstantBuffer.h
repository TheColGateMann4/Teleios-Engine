#pragma once
#include "ConstantBuffer.h"
#include "Bindable.h"

class Graphics;
class Camera;
class Drawable;
class CommandList;

class TransformConstantBuffer : public Bindable, public CommandListBindable, public RootSignatureBindable
{
public:
	TransformConstantBuffer(Graphics& graphics, Drawable* pObject, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::VertexShader, 0} });

public:
	void Update(Graphics& graphics, Camera& camera);

	NonCachedConstantBuffer* GetBuffer() const;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

private:
	std::shared_ptr<NonCachedConstantBuffer> m_buffer;
	Drawable* m_pObject;
	bool m_updated;
};