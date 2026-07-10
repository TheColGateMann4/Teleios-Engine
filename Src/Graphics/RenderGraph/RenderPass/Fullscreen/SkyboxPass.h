#pragma once
#include "FullscreenPass.h"
#include "Graphics/Bindables/Texture.h"

class CachedConstantBuffer;

class SkyboxPass : public FullscreenPass
{
public:
	SkyboxPass(Graphics& graphics);
	virtual ~SkyboxPass() = default;

	virtual void Initialize(Graphics& graphics, Scene& scene) override;

	virtual void InitializeFullscreenResources(Graphics& graphics, Pipeline& pipeline, Scene& scene) override;

	virtual void Update(Graphics& graphics, Pipeline& pipeline, Scene& scene) override;

private:
	void UpdateInverseProjectionMatrix(Graphics& graphics, Scene& scene);

private:
	std::shared_ptr<Texture> m_skyboxTexture;
	RootSignatureConstants* m_pSkyboxTextureIndexConstants = nullptr;
	CachedConstantBuffer* m_pInverseMatriesBuffer = nullptr;
};