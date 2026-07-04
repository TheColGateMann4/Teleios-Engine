#pragma once
#include "BindableTypes.h"

class BlendState;
class ConstantBuffer;
class CachedConstantBuffer;
class NonCachedConstantBuffer;
class Buffer;
class DepthStencilState;
class DepthStencilView;
class IndexBuffer;
class IndexBufferEntry;
class InputLayout;
class PrimitiveTechnology;
class RasterizerState;
class RenderTarget;
class Sampler;
class StaticSampler;
class Shader;
class ShaderResourceView;
class Texture;
class UnorderedAccessView;
class VertexBuffer;
class VertexBufferEntry;
class ViewPort;
class RootSignatureWrapper;
class RootSignatureConstants;
class DescriptorHeapBindable;
class MaterialBindings;

template<>
struct GetBindableType
{
	bool valid = false;
};

template<bindable_blendState>
struct GetBindableType
{
	bool valid = true;
};