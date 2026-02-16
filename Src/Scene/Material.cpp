#include "Material.h"
#include "Includes/BindablesInclude.h"

#include "Graphics/Core/Graphics.h"

Material::Material(Graphics& graphics, std::string filePath, MaterialProperties::MaterialProperties properties)
	:
	RootSignatureBindable(std::vector<TargetSlotAndShader>{ {ShaderVisibilityGraphic::PixelShader, 0} }),
	m_properties(properties)
{
	std::vector<ShaderMacro> shaderMacros;

	{
		shaderMacros.push_back({ L"METALNESS_PIPELINE" });

		if (m_properties.roughnessMetalnessInOneTexture)
			shaderMacros.push_back({ L"METALNESS_ROUGHNESS_ONE_TEXTURE" });

		// textures
		if (m_properties.hasAnyMap)
		{
			m_bindableContainer.AddBindable(StaticSampler::GetBindableResource(graphics, D3D12_FILTER_MIN_MAG_MIP_POINT));

			if (m_properties.ignoreDiffseAlpha)
				shaderMacros.push_back({ L"IGNORE_DIFFUSE_ALPHA" });

			shaderMacros.push_back({ L"TEXTURE_ANY" });
			shaderMacros.push_back({ L"INPUT_TEXCCORDS" }); // since we are handling textures, we will need texcoords argument provided to our shaders


			if (m_properties.hasAlbedoMap)
			{
				m_bindableContainer.AddBindable(Texture::GetBindableResource(graphics, (filePath + m_properties.albedoMapPath).c_str(), true, true, true, { {ShaderVisibilityGraphic::PixelShader, 0} }));
				shaderMacros.push_back({ L"TEXTURE_DIFFUSE" });
			}

			if (m_properties.hasNormalMap)
			{
				m_bindableContainer.AddBindable(Texture::GetBindableResource(graphics, (filePath + m_properties.normalMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 1} }));
				shaderMacros.push_back({ L"TEXTURE_NORMAL" });

				shaderMacros.push_back({ L"INPUT_TANGENT" });
				shaderMacros.push_back({ L"INPUT_BITANGENT" });
			}

			if (m_properties.metalRoughnessSystem)
			{
				if (m_properties.hasMetalnessMap)
				{
					std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + m_properties.specularMetalnessMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 3} });

					m_bindableContainer.AddBindable(std::move(specularTexture));
					shaderMacros.push_back({ L"TEXTURE_METALNESS" });
				}

				if (m_properties.hasRoughnessMap)
				{
					std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + m_properties.specularMetalnessMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 4} });

					m_bindableContainer.AddBindable(std::move(specularTexture));
					shaderMacros.push_back({ L"TEXTURE_ROUGHNESS" });
				}

				// reflectivity 5
			}
			else
			{
				if (m_properties.hasSpecularMap)
				{
					std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + m_properties.specularMetalnessMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 2} });

					m_properties.specularOneChannelOnly = specularTexture->GetTexture()->GetFormat() == DXGI_FORMAT_R8_UNORM;

					m_bindableContainer.AddBindable(std::move(specularTexture));
					shaderMacros.push_back({ L"TEXTURE_SPECULAR" });
				}
			}

			if (m_properties.hasAmbientMap)
			{
				std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + m_properties.ambientMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 6} });

				m_bindableContainer.AddBindable(std::move(specularTexture));
				shaderMacros.push_back({ L"TEXTURE_AMBIENT" });
			}

			// opacity 7

		}

		//Constant buffer describing model material propeties
		{
			DynamicConstantBuffer::Layout layout;
			{
				layout.Add<DynamicConstantBuffer::ElementType::Float3>("ambient");
				layout.Add<DynamicConstantBuffer::ElementType::Float3>("diffuse");

				if (m_properties.metalRoughnessSystem)
				{
					layout.Add<DynamicConstantBuffer::ElementType::Float3>("reflectivity");
					layout.Add<DynamicConstantBuffer::ElementType::Float>("metalness", DynamicConstantBuffer::ImguiFloatData{ true, 0.0f, 1.0f });
					layout.Add<DynamicConstantBuffer::ElementType::Float>("roughness", DynamicConstantBuffer::ImguiFloatData{ true, 0.0f, 1.0f });
				}
				else
				{
					layout.Add<DynamicConstantBuffer::ElementType::Float3>("defaultSpecularColor");
					layout.Add<DynamicConstantBuffer::ElementType::Bool>("specularOneChannelOnly", DynamicConstantBuffer::ImguiData{ false });
					layout.Add<DynamicConstantBuffer::ElementType::Float>("specular", DynamicConstantBuffer::ImguiFloatData{ true, 0.001f, 150.0f });
					layout.Add<DynamicConstantBuffer::ElementType::Float>("glosiness", DynamicConstantBuffer::ImguiFloatData{ true, 0.001f, 150.0f });
				}
				layout.Add<DynamicConstantBuffer::ElementType::Float>("opacity", DynamicConstantBuffer::ImguiFloatData{ true, 0.0f, 1.0f });
			}

			DynamicConstantBuffer::Data bufferData(layout);
			{
				*bufferData.Get<DynamicConstantBuffer::ElementType::Float3>("ambient") = m_properties.ambient;
				*bufferData.Get<DynamicConstantBuffer::ElementType::Float3>("diffuse") = m_properties.albedo;

				if (m_properties.metalRoughnessSystem)
				{
					*bufferData.Get<DynamicConstantBuffer::ElementType::Float3>("reflectivity") = m_properties.reflective;
					*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("metalness") = m_properties.metalness;
					*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("roughness") = m_properties.roughness;
				}
				else
				{
					*bufferData.Get<DynamicConstantBuffer::ElementType::Float3>("defaultSpecularColor") = m_properties.specularColor;
					*bufferData.Get<DynamicConstantBuffer::ElementType::Bool>("specularOneChannelOnly") = m_properties.specularOneChannelOnly;
					*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("specular") = m_properties.specular;
					*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("glosiness") = m_properties.glosiness;
				}
				*bufferData.Get<DynamicConstantBuffer::ElementType::Float>("opacity") = m_properties.opacity;
			}

			m_bindableContainer.AddBindable(std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}}));
		}

		m_bindableContainer.AddBindable(RasterizerState::GetBindableResource(graphics, m_properties.twoSided));
	}

	shaderMacros.push_back({ L"INPUT_NORMAL" }); // model objects will always have normals since we will generate them with assimp if they do not

	m_bindableContainer.AddBindable(Shader::GetBindableResource(graphics, L"PS_GBuffer", ShaderType::PixelShader, shaderMacros));
	m_bindableContainer.AddBindable(Shader::GetBindableResource(graphics, L"VS", ShaderType::VertexShader, shaderMacros));


	// Requesting size for descriptor table for this material
	{
		const auto& rootSignatureBindables = m_bindableContainer.GetRootSignatureBindables();
		unsigned int rsBindablesNum = rootSignatureBindables.size();

		graphics.GetDescriptorHeap().RequestMoreSpace(rsBindablesNum);
	}
}

const MaterialProperties::MaterialProperties& Material::GetProperties() const
{
	return m_properties;
}

const MeshBindableContainer& Material::GetBindableContainer() const
{
	return m_bindableContainer;
}

void Material::Bind(Graphics& graphics, CommandList* commandList)
{
	if (!m_hasDescriptorBindables)
		return;

	commandList->SetGraphicsDescriptorTable(graphics, this, GetTargets().at(0));
}

void Material::BindToRootSignature(RootSignature* rootSignature)
{
	rootSignature->AddDescriptorTableParameter(this, GetTargets().at(0));
}

void Material::Initialize(Graphics& graphics, DescriptorHeap::DescriptorInfo descriptorInfo, unsigned int descriptorNum)
{
	THROW_INTERNAL_ERROR("Tried to explicitly place Material in descriptor");
}

void Material::Initialize(Graphics& graphics)
{
	const auto& descriptorBindables = m_bindableContainer.GetDescriptorBindables();
	unsigned int rsBindablesNum = descriptorBindables.size();

	if (rsBindablesNum == 0)
		return;

	m_hasDescriptorBindables = true;

	std::vector<DescriptorHeap::DescriptorInfo> descriptorTableInfo = graphics.GetDescriptorHeap().GetNextHandles(rsBindablesNum);

	m_descriptorInfo = descriptorTableInfo.front();

	for (int i = 0; i < rsBindablesNum; i++)
	{
		// For now we won't support resources that need many descriptors. But later we will have to make whole table N times if single resource in our table needs N frames. 
		// For now we just write 0 since we don't use any dynamic resources that are changing in any way
		descriptorBindables.at(i)->Initialize(graphics, descriptorTableInfo.at(i), 0);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE Material::GetDescriptorHeapGPUHandle(Graphics& graphics) const
{
	return m_descriptorInfo.descriptorHeapGpuHandle;
}

BindableType Material::GetBindableType() const
{
	return BindableType::bindable_material;
}

DescriptorType Material::GetDescriptorType() const
{
	return DescriptorType::descriptor_none;
}

RootSignatureBindableType Material::GetRootSignatureBindableType() const
{
	return RootSignatureBindableType::rootSignature_none;
}