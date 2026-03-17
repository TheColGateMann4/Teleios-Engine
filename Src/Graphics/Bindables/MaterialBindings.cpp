#include "MaterialBindings.h"

#include "Texture.h"

#define ADD_TEXTURE_INDEX_TO_LAYOUT(type, str) \
if(texturesByTypes.at(static_cast<int>(type)) != nullptr) \
	layout.Add<DynamicConstantBuffer::ElementType::Uint>(str);

#define SET_TEXTURE_INDEX_DATA(type, str) \
{ \
	Texture* tex = texturesByTypes.at(static_cast<int>(type)); \
	if(tex != nullptr) \
		*bufferData.Get<DynamicConstantBuffer::ElementType::Uint>(str) = tex->GetOffsetInDescriptor(); \
}

MaterialBindings::MaterialBindings(const std::vector<Texture*>&textures)
{
	m_descriptorHeapBindable = DescriptorHeapBindable::GetResource();

	std::vector<Texture*> texturesByTypes(static_cast<int>(TextureType::texture_types_num), nullptr);

	for (auto* texture : textures)
	{
		TextureType textureType = texture->GetTextureType();

		texturesByTypes.at(static_cast<int>(textureType)) = texture;
	}

	DynamicConstantBuffer::Layout layout;
	ADD_TEXTURE_INDEX_TO_LAYOUT(TextureType::texture_albedo, "b_diffuseTextureID");
	ADD_TEXTURE_INDEX_TO_LAYOUT(TextureType::texture_normal, "b_normalTextureID");
	ADD_TEXTURE_INDEX_TO_LAYOUT(TextureType::texture_metalness_roughness, "b_metalnessRoughnessTextureID");
	ADD_TEXTURE_INDEX_TO_LAYOUT(TextureType::texture_metalness, "b_metalnessTextureID");
	ADD_TEXTURE_INDEX_TO_LAYOUT(TextureType::texture_roughness, "b_roughnessTextureID");
	ADD_TEXTURE_INDEX_TO_LAYOUT(TextureType::texture_specular, "b_specularTextureID");
	ADD_TEXTURE_INDEX_TO_LAYOUT(TextureType::texture_glosiness, "b_glosinessTextureID");
	ADD_TEXTURE_INDEX_TO_LAYOUT(TextureType::texture_reflectivity, "b_reflectivityTextureID");
	ADD_TEXTURE_INDEX_TO_LAYOUT(TextureType::texture_ambient, "b_ambientTextureID");
	ADD_TEXTURE_INDEX_TO_LAYOUT(TextureType::texture_opacity, "b_opacityTextureID");

	layout.GetFinished(DynamicConstantBuffer::Layout::LayoutType::data);

	DynamicConstantBuffer::Data bufferData(layout);
	SET_TEXTURE_INDEX_DATA(TextureType::texture_albedo, "b_diffuseTextureID");
	SET_TEXTURE_INDEX_DATA(TextureType::texture_normal, "b_normalTextureID");
	SET_TEXTURE_INDEX_DATA(TextureType::texture_metalness_roughness, "b_metalnessRoughnessTextureID");
	SET_TEXTURE_INDEX_DATA(TextureType::texture_metalness, "b_metalnessTextureID");
	SET_TEXTURE_INDEX_DATA(TextureType::texture_roughness, "b_roughnessTextureID");
	SET_TEXTURE_INDEX_DATA(TextureType::texture_specular, "b_specularTextureID");
	SET_TEXTURE_INDEX_DATA(TextureType::texture_specular, "b_specularTextureID");
	SET_TEXTURE_INDEX_DATA(TextureType::texture_reflectivity, "b_reflectivityTextureID");
	SET_TEXTURE_INDEX_DATA(TextureType::texture_ambient, "b_ambientTextureID");
	SET_TEXTURE_INDEX_DATA(TextureType::texture_opacity, "b_opacityTextureID");

	m_textureIndexesConstants = std::make_shared<RootSignatureConstants>(bufferData, std::vector<TargetSlotAndShader>{{ShaderVisibilityGraphic::PixelShader, 2}});
}

#undef ADD_TEXTURE_INDEX_TO_LAYOUT
#undef SET_TEXTURE_INDEX_DATA

RootSignatureConstants* MaterialBindings::GetTextureIndexesConstants()
{
	return m_textureIndexesConstants.get();
}

DescriptorHeapBindable* MaterialBindings::GetDescriptorHeapBindable()
{
	return m_descriptorHeapBindable.get();
}