#pragma once
#include "ConstantBuffer.h"
#include "Bindable.h"

class Graphics;
class Camera;
class SceneObject;
class CommandList;

class TransformConstantBuffer : public Bindable, public CommandListBindable, public RootSignatureBindable
{
public:
	TransformConstantBuffer(Graphics& graphics, SceneObject* pObject);

public:
	void Update(Graphics& graphics, Camera& camera);

	NonCachedConstantBuffer* GetBuffer() const;

	virtual void BindToCommandList(Graphics& graphics, CommandList* commandList) override;

	virtual void BindToRootSignature(Graphics& graphics, RootSignature* rootSignature) override;

	virtual std::vector<TargetSlotAndShader>& GetTargets() override;

private:
	std::shared_ptr<NonCachedConstantBuffer> m_buffer;
	SceneObject* m_pObject;
	bool m_updated;
};