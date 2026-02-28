#include "RootSignature.h"
#include "Macros/ErrorMacros.h"
#include "Graphics.h"

#include "Includes/BindablesInclude.h"

#include "Scene/Material.h"

#include "Graphics/Bindables/DescriptorHeapBindable.h"

#include "Graphics/Core/ResourceList.h"

namespace
{
	template<class T>
	std::string GetStringFromEnum(T val)
	{
		static_assert(std::is_enum_v<T>);

		return std::to_string(static_cast<std::underlying_type_t<T>>(val));
	}

	template<class T>
	std::string GetStringFromFlags(T val)
	{
		static_assert(std::is_enum_v<T>);

		using UnderlyingType = std::underlying_type_t<T>;
		constexpr size_t bitCount = sizeof(UnderlyingType) * 8;

		auto underlyingValue = static_cast<UnderlyingType>(val);
		return std::bitset<bitCount>(underlyingValue).to_string();
	}

	std::string GetStringFromFloat(FLOAT val)
	{
		if (val == 0.0f)
			return "FLT_ZERO";
		else if (val == D3D12_FLOAT32_MAX)
			return "FLT_MAX";

		THROW_INTERNAL_ERROR("Unhandled floating point value");
	}
};

RootSignatureParams::RootSignatureParams()
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

RootSignatureParams::RootSignatureParams(const RootSignatureParams& copied)
	:
	m_finished(copied.m_finished),

	m_rootSignatureDesc(copied.m_rootSignatureDesc),
	m_rootParameters(copied.m_rootParameters),
	m_staticSamplers(copied.m_staticSamplers),
	m_descriptorTableRanges(copied.m_descriptorTableRanges)
{
	THROW_INTERNAL_ERROR("Called copy constructor for root signature\n"); // we actually don't want to call this function, its only so we can use objects as std::any
}

D3D12_ROOT_SIGNATURE_DESC1 RootSignatureParams::GetDesc() const
{
	return m_rootSignatureDesc;
}

void RootSignatureParams::Finish()
{
	THROW_INTERNAL_ERROR_IF("RootSignatureParams were already finished", m_finished);

	ConnectDescriptorParametersToRanges();

	m_finished = true;

	CreateIdentifier();
}

bool RootSignatureParams::isFinished() const
{
	return m_finished;
}

const std::string& RootSignatureParams::GetIdentifier() const
{
	THROW_INTERNAL_ERROR_IF("RootSignatureParams were not finished", !m_finished);

	return m_cachedIdentifier;
}

void RootSignatureParams::AddConstBufferViewParameter(ConstantBuffer* constantBuffer, TargetSlotAndShader& target)
{
	THROW_INTERNAL_ERROR_IF("RootSignatureParams were already finished", m_finished);

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

void RootSignatureParams::AddDescriptorTableParameter(DescriptorHeapBindable* descriptorHeapBindable)
{
	TargetSlotAndShader& target = descriptorHeapBindable->GetTargets().front();

	target.rootIndex = m_AddDescriptorTableParameter(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 
		target, 
		UINT_MAX, // UINT_MAX for unbounded size (bindless system)
		D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE // GPU won't assume the descriptors are static
	); 
}

void RootSignatureParams::AddDescriptorTableParameter(Texture* texture, TargetSlotAndShader& target)
{
	target.rootIndex = m_AddDescriptorTableParameter( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, target);
}

void RootSignatureParams::AddDescriptorTableParameter(ShaderResourceViewBase* srv, TargetSlotAndShader& target)
{
	target.rootIndex = m_AddDescriptorTableParameter(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, target);
}

void RootSignatureParams::AddComputeDescriptorTableParameter(Texture* texture, TargetSlotAndShader& target)
{
	texture->SetComputeRootIndex(m_AddDescriptorTableParameter(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, target));
}

void RootSignatureParams::AddComputeDescriptorTableParameter(ShaderResourceViewBase* srv, TargetSlotAndShader& target)
{
	srv->SetComputeRootIndex(m_AddDescriptorTableParameter(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, target));
}


void RootSignatureParams::AddUnorderedAccessViewParameter(UnorderedAccessView* uav, TargetSlotAndShader& target)
{
	target.rootIndex = m_AddDescriptorTableParameter(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, target);
}


void RootSignatureParams::SetGraphicsRootConstants(RootSignatureConstants* constants, TargetSlotAndShader& target)
{
	THROW_INTERNAL_ERROR_IF("RootSignatureParams were already finished", m_finished);

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

void RootSignatureParams::AddStaticSampler(StaticSampler* staticSampler)
{
	auto& targets = staticSampler->GetTargets();

	for (auto& target : targets)
		m_AddStaticSampler(staticSampler, target);
}

void RootSignatureParams::AddComputeStaticSampler(StaticSampler* staticSampler)
{
	auto& targets = staticSampler->GetTargets();

	for (auto& target : targets)
		m_AddStaticSampler(staticSampler, target);
}

void RootSignatureParams::ConnectDescriptorParametersToRanges()
{
	size_t descriptorIndex = 0;

	for (size_t index = 0; index < m_rootParameters.size(); index++)
		if(m_rootParameters.at(index).ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			m_rootParameters.at(index).DescriptorTable.pDescriptorRanges = &m_descriptorTableRanges.at(descriptorIndex);
			descriptorIndex++;
		}
}

unsigned int RootSignatureParams::m_AddDescriptorTableParameter(D3D12_DESCRIPTOR_RANGE_TYPE descriptorType, TargetSlotAndShader& target, unsigned int numDescriptors, D3D12_DESCRIPTOR_RANGE_FLAGS flags)
{
	THROW_INTERNAL_ERROR_IF("RootSignatureParams were already finished", m_finished);

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

void RootSignatureParams::m_AddStaticSampler(StaticSampler* staticSampler, TargetSlotAndShader& target)
{
	THROW_INTERNAL_ERROR_IF("RootSignatureParams were already finished", m_finished);

	m_rootSignatureDesc.NumStaticSamplers++;

	m_staticSamplers.push_back(staticSampler->Get());

	m_staticSamplers.back().ShaderRegister = target.slot;
	m_staticSamplers.back().ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(target.target);

	m_rootSignatureDesc.pStaticSamplers = m_staticSamplers.data();
}

std::string GetDescriptorTableIdentifier(const D3D12_ROOT_DESCRIPTOR_TABLE1& descriptorTable)
{
	std::string result = {};

	result += std::to_string(descriptorTable.NumDescriptorRanges);

	for (int i = 0; i < descriptorTable.NumDescriptorRanges; i++)
	{
		const auto& range = descriptorTable.pDescriptorRanges[i];

		result += GetStringFromEnum(range.RangeType);
		result += std::to_string(range.NumDescriptors);
		result += std::to_string(range.BaseShaderRegister);
		result += std::to_string(range.RegisterSpace);
		result += GetStringFromFlags(range.Flags);
		result += std::to_string(range.OffsetInDescriptorsFromTableStart);
	}

	return result;
}

std::string GetConstantsIdentifier(const D3D12_ROOT_CONSTANTS& constants)
{
	std::string result = {};

	result += std::to_string(constants.ShaderRegister);
	result += std::to_string(constants.RegisterSpace);
	result += std::to_string(constants.Num32BitValues);

	return result;
}

std::string GetDescriptorIdentifier(const D3D12_ROOT_DESCRIPTOR1& descriptor)
{
	std::string result = {};

	result += std::to_string(descriptor.ShaderRegister);
	result += std::to_string(descriptor.RegisterSpace);
	result += GetStringFromFlags(descriptor.Flags);

	return result;
}

std::string GetParamIdentifierByType(const D3D12_ROOT_PARAMETER1& param)
{
	switch (param.ParameterType)
	{
	case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
		return GetDescriptorTableIdentifier(param.DescriptorTable);

	case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
		return GetConstantsIdentifier(param.Constants);

	case D3D12_ROOT_PARAMETER_TYPE_CBV:
	case D3D12_ROOT_PARAMETER_TYPE_SRV:
	case D3D12_ROOT_PARAMETER_TYPE_UAV:
		return GetDescriptorIdentifier(param.Descriptor);

	default:
		THROW_INTERNAL_ERROR("Failed to map root parameter type");
	}
}

std::string GetStaticSamplerIdentifier(const D3D12_STATIC_SAMPLER_DESC& staticSampler)
{
	std::string result = {};

	result += GetStringFromEnum(staticSampler.Filter);
	result += GetStringFromEnum(staticSampler.AddressU);
	result += GetStringFromEnum(staticSampler.AddressV);
	result += GetStringFromEnum(staticSampler.AddressW);
	result += GetStringFromFloat(staticSampler.MipLODBias);
	result += std::to_string(staticSampler.MaxAnisotropy);
	result += GetStringFromEnum(staticSampler.ComparisonFunc);
	result += GetStringFromEnum(staticSampler.BorderColor);
	result += GetStringFromFloat(staticSampler.MinLOD);
	result += GetStringFromFloat(staticSampler.MaxLOD);
	result += std::to_string(staticSampler.ShaderRegister);
	result += std::to_string(staticSampler.RegisterSpace);
	result += GetStringFromEnum(staticSampler.ShaderVisibility);

	return result;
}

std::string GetParamIdentifier(const D3D12_ROOT_PARAMETER1& param)
{
	std::string result = {};

	result += GetStringFromEnum(param.ParameterType);

	result += GetParamIdentifierByType(param);

	result += GetStringFromEnum(param.ShaderVisibility);

	return result;
}

void RootSignatureParams::CreateIdentifier()
{
	THROW_INTERNAL_ERROR_IF("RootSignatureParams didn't have any parameters", m_rootParameters.empty());

	std::string result = {};

	result += GetParamsIdentifier();

	result += GetStaticSamplersIdentifier();

	result += GetFlagsIdentifier();

	m_cachedIdentifier = result;
}

std::string RootSignatureParams::GetParamsIdentifier() const
{
	std::string result = {};

	for (int i = 0; i < m_rootParameters.size(); i++)
	{
		const auto& param = m_rootParameters.at(i);

		result += std::to_string(i);

		result += ":";

		result += GetParamIdentifier(param);

		result += "#";
	}

	return result;
}

std::string RootSignatureParams::GetStaticSamplersIdentifier() const
{
	std::string result = {};

	for (int i = 0; i < m_staticSamplers.size(); i++)
	{
		const auto& staticSamplerParam = m_staticSamplers.at(i);

		result += std::to_string(i);

		result += ":";

		result += GetStaticSamplerIdentifier(staticSamplerParam);

		result += "#";
	}

	return result;
}

std::string RootSignatureParams::GetFlagsIdentifier() const
{
	return GetStringFromFlags(m_rootSignatureDesc.Flags);
}


RootSignature::RootSignature(Graphics& graphics, RootSignatureParams&& params)
	:
	m_params(std::move(params))
{
	if (!m_params.isFinished())
		m_params.Finish();

	HRESULT hr;

	Microsoft::WRL::ComPtr<ID3DBlob> pRootSignatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> pErrorMessages;

	D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedRootSignatureDesc = {};
	versionedRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
	versionedRootSignatureDesc.Desc_1_1 = m_params.GetDesc();

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
}

std::shared_ptr<RootSignature> RootSignature::GetResource(Graphics& graphics, RootSignatureParams&& params)
{
	if (!params.isFinished())
		params.Finish();

	return ResourceList::GetResource<RootSignature>(graphics, std::move(params));
}

ID3D12RootSignature* RootSignature::Get() const
{
	return pRootSignature.Get();
}

std::string RootSignature::GetIdentifier(const RootSignatureParams& params)
{
	return params.GetIdentifier();
}

std::string RootSignature::GetIdentifier()
{
	return m_params.GetIdentifier();
}

unsigned int RootSignature::GetNumParams() const
{
	return m_params.GetDesc().NumParameters;
}