#include "Material.h"

Material::Material(MaterialProperties::MaterialProperties properties)
	:
	m_properties(properties)
{

}

const MaterialProperties::MaterialProperties& Material::GetProperties() const
{
	return m_properties;
}