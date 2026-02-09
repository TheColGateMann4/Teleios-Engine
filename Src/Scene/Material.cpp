#include "Material.h"
#include "Includes/BindablesInclude.h"

Material::Material(Graphics& graphics, std::string filePath, MaterialProperties::MaterialProperties properties)
	:
	m_properties(properties)
{
	{
		m_shaderMacros.push_back({ L"METALNESS_PIPELINE" });

		if (m_properties.roughnessMetalnessInOneTexture)
			m_shaderMacros.push_back({ L"METALNESS_ROUGHNESS_ONE_TEXTURE" });

		// textures
		if (m_properties.hasAnyMap)
		{
			m_bindables.push_back(StaticSampler::GetBindableResource(graphics, D3D12_FILTER_MIN_MAG_MIP_POINT));

			if (m_properties.ignoreDiffseAlpha)
				m_shaderMacros.push_back({ L"IGNORE_DIFFUSE_ALPHA" });

			m_shaderMacros.push_back({ L"TEXTURE_ANY" });
			m_shaderMacros.push_back({ L"INPUT_TEXCCORDS" }); // since we are handling textures, we will need texcoords argument provided to our shaders


			if (m_properties.hasAlbedoMap)
			{
				m_bindables.push_back(Texture::GetBindableResource(graphics, (filePath + m_properties.albedoMapPath).c_str(), true, true, true, { {ShaderVisibilityGraphic::PixelShader, 0} }));
				m_shaderMacros.push_back({ L"TEXTURE_DIFFUSE" });
			}

			if (m_properties.hasNormalMap)
			{
				m_bindables.push_back(Texture::GetBindableResource(graphics, (filePath + m_properties.normalMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 1} }));
				m_shaderMacros.push_back({ L"TEXTURE_NORMAL" });

				m_shaderMacros.push_back({ L"INPUT_TANGENT" });
				m_shaderMacros.push_back({ L"INPUT_BITANGENT" });
			}

			if (m_properties.metalRoughnessSystem)
			{
				if (m_properties.hasMetalnessMap)
				{
					std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + m_properties.specularMetalnessMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 3} });

					m_bindables.push_back(std::move(specularTexture));
					m_shaderMacros.push_back({ L"TEXTURE_METALNESS" });
				}

				if (m_properties.hasRoughnessMap)
				{
					std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + m_properties.specularMetalnessMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 4} });

					m_bindables.push_back(std::move(specularTexture));
					m_shaderMacros.push_back({ L"TEXTURE_ROUGHNESS" });
				}

				// reflectivity 5
			}
			else
			{
				if (m_properties.hasSpecularMap)
				{
					std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + m_properties.specularMetalnessMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 2} });

					m_properties.specularOneChannelOnly = specularTexture->GetTexture()->GetFormat() == DXGI_FORMAT_R8_UNORM;

					m_bindables.push_back(std::move(specularTexture));
					m_shaderMacros.push_back({ L"TEXTURE_SPECULAR" });
				}
			}

			if (m_properties.hasAmbientMap)
			{
				std::shared_ptr<Texture> specularTexture = Texture::GetBindableResource(graphics, (filePath + m_properties.ambientMapPath).c_str(), false, true, true, { {ShaderVisibilityGraphic::PixelShader, 6} });

				m_bindables.push_back(std::move(specularTexture));
				m_shaderMacros.push_back({ L"TEXTURE_AMBIENT" });
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

			m_bindables.push_back(std::make_shared<CachedConstantBuffer>(graphics, bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 1}}));
		}

		m_bindables.push_back(RasterizerState::GetBindableResource(graphics, m_properties.twoSided));
	}
}

const MaterialProperties::MaterialProperties& Material::GetProperties() const
{
	return m_properties;
}

std::vector<ShaderMacro> Material::GetShaderMacros() const
{
	return m_shaderMacros;
}

const std::vector<std::shared_ptr<Bindable>>& Material::GetBindables() const
{
	return m_bindables;
}