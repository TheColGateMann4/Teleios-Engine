#pragma once 

// it is important that constantBuffer, cachedConstantBuffer and nonCachedConstantBuffer are three different types
enum class BindableType
{
	bindable_blendState,
	bindable_constantBuffer,
	bindable_cachedConstantBuffer,
	bindable_nonCachedConstantBuffer,
	bindable_depthStencilState,
	bindable_depthStencilView,
	bindable_indexBuffer,
	bindable_inputLayout,
	bindable_primitiveTechnology,
	bindable_rasterizerState,
	bindable_renderTarget,
	bindable_sampler,
	bindable_staticSampler,
	bindable_shader,
	bindable_shaderResourceView,
	bindable_texture,
	bindable_transformConstantBuffer,
	bindable_unorderedAccessView,
	bindable_vertexBuffer,
	bindable_viewPort,
	bindable_rootSignatureWrapper,
	bindable_rootSignatureConstants
};
