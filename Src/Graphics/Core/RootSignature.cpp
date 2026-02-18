#include "RootSignature.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

#include "Includes/BindablesInclude.h"

#include "Scene/Material.h"

#include "Graphics/Bindables/DescriptorHeapBindable.h"

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

RootSignature::RootSignature(RootSignature&& moved) noexcept
	:
	pRootSignature(moved.pRootSignature),
	m_finished(moved.m_finished),

	m_rootSignatureDesc(moved.m_rootSignatureDesc),
	m_rootParameters(moved.m_rootParameters),
	m_staticSamplers(moved.m_staticSamplers),
	m_descriptorTableRanges(moved.m_descriptorTableRanges)
{

}

RootSignature::RootSignature(const RootSignature& copied)
	:
	pRootSignature(copied.pRootSignature),
	m_finished(copied.m_finished),

	m_rootSignatureDesc(copied.m_rootSignatureDesc),
	m_rootParameters(copied.m_rootParameters),
	m_staticSamplers(copied.m_staticSamplers),
	m_descriptorTableRanges(copied.m_descriptorTableRanges)
{
	THROW_INTERNAL_ERROR("Called copy constructor for root signature\n"); // we actually don't want to call this function, its only so we can use objects as std::any
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

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc = {};
	versionedRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	versionedRootSignatureDesc.Desc_1_1 = m_rootSignatureDesc;

	THROW_ERROR_MESSAGES_BLOB_ERROR(D3D12SerializeVersionedRootSignature(
		&versionedRootSignatureDesc,
		&pRootSignatureBlob,
		&pErrorMessages
	));

	THROW_ERROR(graphics.GetDeviceResources().GetDevice()->CreateRootSignature(
		0,
		pRootSignatureBlob->GetBufferPointer(),
		pRootSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&pRootSignature)
	));

	m_finished = true;
}

void RootSignature::AddConstBufferViewParameter(ConstantBuffer* constantBuffer, TargetSlotAndShader& target)
{
	target.rootIndex = m_rootSignatureDesc.NumParameters;

	m_rootSignatureDesc.NumParameters++;

	D3D12_ROOT_PARAMETER1 rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter.Descriptor = {};
	rootParameter.Descriptor.ShaderRegister = target.slot;
	rootParameter.Descriptor.RegisterSpace = 0;
	rootParameter.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(target.target);

	m_rootParameters.push_back(rootParameter);

	m_rootSignatureDesc.pParameters = m_rootParameters.data();
}

void RootSignature::AddDescriptorTableParameter(DescriptorHeapBindable* descriptorHeapBindable)
{
	TargetSlotAndShader& target = descriptorHeapBindable->GetTargets().front();

	target.rootIndex = m_AddDescriptorTableParameter(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 
		target, 
		UINT_MAX, // UINT_MAX for unbounded size (bindless system)
		D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE // GPU won't assume the descriptors are static
	); 
}

void RootSignature::AddDescriptorTableParameter(Texture* texture, TargetSlotAndShader& target)
{
	target.rootIndex = m_AddDescriptorTableParameter( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, target);
}

void RootSignature::AddDescriptorTableParameter(ShaderResourceViewBase* srv, TargetSlotAndShader& target)
{
	target.rootIndex = m_AddDescriptorTableParameter(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, target);
}

void RootSignature::AddComputeDescriptorTableParameter(Texture* texture, TargetSlotAndShader& target)
{
	texture->SetComputeRootIndex(m_AddDescriptorTableParameter(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, target));
}

void RootSignature::AddComputeDescriptorTableParameter(ShaderResourceViewBase* srv, TargetSlotAndShader& target)
{
	srv->SetComputeRootIndex(m_AddDescriptorTableParameter(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, target));
}


void RootSignature::AddUnorderedAccessViewParameter(UnorderedAccessView* uav, TargetSlotAndShader& target)
{
	target.rootIndex = m_AddDescriptorTableParameter(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, target);
}


void RootSignature::SetGraphicsRootConstants(RootSignatureConstants* constants, TargetSlotAndShader& target)
{
	target.rootIndex = m_rootSignatureDesc.NumParameters;

	m_rootSignatureDesc.NumParameters++;

	D3D12_ROOT_PARAMETER1 rootParameter = {};
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameter.Constants = {};
	rootParameter.Constants.ShaderRegister = target.slot;
	rootParameter.Constants.RegisterSpace = 0;
	rootParameter.Constants.Num32BitValues = constants->GetNumValues();
	rootParameter.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(target.target);

	m_rootParameters.push_back(rootParameter);

	m_rootSignatureDesc.pParameters = m_rootParameters.data();
}

void RootSignature::AddStaticSampler(StaticSampler* staticSampler)
{
	auto& targets = staticSampler->GetTargets();

	for (auto& target : targets)
		m_AddStaticSampler(staticSampler, target);
}

void RootSignature::AddComputeStaticSampler(StaticSampler* staticSampler)
{
	auto& targets = staticSampler->GetTargets();

	for (auto& target : targets)
		m_AddStaticSampler(staticSampler, target);
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

unsigned int RootSignature::m_AddDescriptorTableParameter(D3D12_DESCRIPTOR_RANGE_TYPE descriptorType, TargetSlotAndShader& target, unsigned int numDescriptors, D3D12_DESCRIPTOR_RANGE_FLAGS flags)
{
	unsigned int resultRootIndex = m_rootSignatureDesc.NumParameters;

	{
		m_rootSignatureDesc.NumParameters++;

		D3D12_DESCRIPTOR_RANGE1 descriptorRange = {};
		descriptorRange.RangeType = descriptorType;
		descriptorRange.NumDescriptors = numDescriptors;
		descriptorRange.BaseShaderRegister = target.slot;
		descriptorRange.RegisterSpace = 0;
		descriptorRange.Flags = flags;
		descriptorRange.OffsetInDescriptorsFromTableStart = 0;

		m_descriptorTableRanges.push_back(descriptorRange);


		D3D12_ROOT_PARAMETER1 rootParameter = {};
		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter.DescriptorTable = {}; // we are not setting pointer to data here since every push invalidates pointers to data
		rootParameter.DescriptorTable.NumDescriptorRanges = 1;
		rootParameter.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(target.target);

		m_rootParameters.push_back(rootParameter);
	}

	m_rootSignatureDesc.pParameters = m_rootParameters.data();

	return resultRootIndex;
}

void RootSignature::m_AddStaticSampler(StaticSampler* staticSampler, TargetSlotAndShader& target)
{
	m_rootSignatureDesc.NumStaticSamplers++;

	m_staticSamplers.push_back(staticSampler->Get());

	m_staticSamplers.back().ShaderRegister = target.slot;
	m_staticSamplers.back().ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(target.target);

	m_rootSignatureDesc.pStaticSamplers = m_staticSamplers.data();
}