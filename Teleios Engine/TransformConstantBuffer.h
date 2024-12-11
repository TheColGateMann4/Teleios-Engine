#pragma once
#include "ConstantBuffer.h"
#include "Bindable.h"

class Graphics;
class Camera;
class Mesh;
class CommandList;

class TransformConstantBuffer : public Bindable, public CommandListBindable, public RootSignatureBindable
{
public:
	TransformConstantBuffer(Graphics& graphics, std::vector<TargetSlotAndShader> targets = { {ShaderVisibilityGraphic::VertexShader, 0} });

public:
	void SetParentPtr(Mesh* pParent);

	void Update(Graphics& graphics, Camera& camera);

	NonCachedConstantBuffer* GetBuffer() const;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

private:
	std::shared_ptr<NonCachedConstantBuffer> m_buffer;
	Mesh* m_pParent = nullptr;
	bool m_updated;
};