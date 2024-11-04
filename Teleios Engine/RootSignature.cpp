#include "RootSignature.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

#include "ConstantBuffer.h"
#include "Texture.h"
#include "Sampler.h"

RootSignature::RootSignature()
	:
	m_finished(),
	m_rootSignatureDesc()
{
	m_rootSignatureDesc.NumParameters = 0;
	m_rootSignatureDesc.pParameters = nullptr;
	m_rootSignatureDesc.NumStaticSamplers = 0;
	m_rootSignatureDesc.pStaticSamplers = nullptr;
	m_rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT; // using this flag so input layout defines vertex buffer structures
}

ID3D12RootSignature* RootSignature::Get() const
{
	THROW_OBJECT_STATE_ERROR_IF("Object was not finished", !m_finished);

	return pRootSignature.Get();
}

void RootSignature::Initialize(Graphics& graphics)
{
	ConnectDescriptorParametersToRanges();


	HRESULT hr;

	Microsoft::WRL::ComPtr<ID3DBlob> pRootSignatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> pErrorMessages;

	THROW_BLOB_ERROR(D3D12SerializeRootSignature(
		&m_rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&pRootSignatureBlob,
		&pErrorMessages
	));

	THROW_ERROR(graphics.GetDevice()->CreateRootSignature(
		0,
		pRootSignatureBlob->GetBufferPointer(),
		pRootSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&pRootSignature)
	));

	m_finished = true;
}

void RootSignature::AddConstBufferViewParameter(ConstantBuffer* constantBuffer)
{
	auto& targets = constantBuffer->GetTargets();

	for(auto& targetShader : targets)
	{
		targetShader.rootIndex = m_rootSignatureDesc.NumParameters;

		m_rootSignatureDesc.NumParameters++;

		D3D12_ROOT_PARAMETER rootParameter = {};
		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameter.Descriptor = {};
		rootParameter.Descriptor.ShaderRegister = targetShader.slot;
		rootParameter.Descriptor.RegisterSpace = 0;
		rootParameter.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(targetShader.target);

		m_rootParameters.push_back(rootParameter);
	}

	m_rootSignatureDesc.pParameters = m_rootParameters.data();
}

void RootSignature::AddDescriptorTableParameter(Texture* texture)
{
	auto& targets = texture->GetTargets();

	for (auto& targetShader : targets)
	{
		targetShader.rootIndex = m_rootSignatureDesc.NumParameters;

		m_rootSignatureDesc.NumParameters++;

		D3D12_DESCRIPTOR_RANGE descriptorRange = {};
		descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRange.NumDescriptors = 1;
		descriptorRange.BaseShaderRegister = targetShader.slot;
		descriptorRange.RegisterSpace = 0;
		descriptorRange.OffsetInDescriptorsFromTableStart = 0;

		m_descriptorTableRanges.push_back(descriptorRange);


		D3D12_ROOT_PARAMETER rootParameter = {};
		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter.DescriptorTable = {};
		rootParameter.DescriptorTable.NumDescriptorRanges = 1;
		rootParameter.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(targetShader.target);

		m_rootParameters.push_back(rootParameter);
	}

	m_rootSignatureDesc.pParameters = m_rootParameters.data();
}

void RootSignature::AddStaticSampler(StaticSampler* staticSampler)
{
	m_rootSignatureDesc.NumStaticSamplers++;

	auto& targets = staticSampler->GetTargets();

	for (auto& targetShader : targets)
	{
		m_staticSamplers.push_back(staticSampler->Get());

		m_staticSamplers.back().ShaderRegister = targetShader.slot;
		m_staticSamplers.back().ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(targetShader.target);
	}

	m_rootSignatureDesc.pStaticSamplers = m_staticSamplers.data();
}

void RootSignature::ConnectDescriptorParametersToRanges()
{
	size_t descriptorIndex = 0;

	for (size_t index = 0; index < m_rootParameters.size(); index++)
		if(m_rootParameters.at(index).ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			m_rootParameters.at(index).DescriptorTable.pDescriptorRanges = &m_descriptorTableRanges.at(descriptorIndex);
			descriptorIndex++;
		}
}