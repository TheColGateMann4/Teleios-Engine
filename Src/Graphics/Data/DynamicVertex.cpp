#include "DynamicVertex.h"
#include "Includes/DirectXIncludes.h"

/*
*					Dynamic Vertex Layout
*/

size_t DynamicVertex::DynamicVertexLayout::GetSize() const
{
	return m_size;
}

size_t DynamicVertex::DynamicVertexLayout::GetNumElements() const
{
	return m_numElements;
}

DynamicVertex::DynamicVertexLayout DynamicVertex::DynamicVertexLayout::Finish()
{
	m_finished = true;

	return *this;
}

std::vector<D3D12_INPUT_ELEMENT_DESC> DynamicVertex::DynamicVertexLayout::GetInputLayout() const
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> result;

	for(const auto& layoutElement : m_elements)
	{
		D3D12_INPUT_ELEMENT_DESC elementDesc = {};
		elementDesc.SemanticName = GetElementSemanticName(layoutElement.type);
		elementDesc.SemanticIndex = 0;
		elementDesc.Format = GetElementFormat(layoutElement.type);
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = layoutElement.offset;
		elementDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		result.push_back(elementDesc);
	}

	return result;
}

std::string DynamicVertex::DynamicVertexLayout::GetIdentifier() const
{
	std::string resultString;

	for (const auto& layoutElement : m_elements)
	{
		resultString += GetElementSemanticName(layoutElement.type);
		resultString += '#';
	}

	return resultString;
}

const char* DynamicVertex::DynamicVertexLayout::GetElementSemanticName(ElementType elementType)
{
	switch (elementType)
	{
		case ElementType::Position:
			return ElementMap<ElementType::Position>::semantic;

		case ElementType::Normal:
			return ElementMap<ElementType::Normal>::semantic;

		case ElementType::TextureCoords:
			return ElementMap<ElementType::TextureCoords>::semantic;

		case ElementType::Tangent:
			return ElementMap<ElementType::Tangent>::semantic;

		case ElementType::Bitangent:
			return ElementMap<ElementType::Bitangent>::semantic;

		case ElementType::Color3:
			return ElementMap<ElementType::Color3>::semantic;

		case ElementType::Color4:
			return ElementMap<ElementType::Color4>::semantic;

		default:
		{
			THROW_INTERNAL_ERROR("Layout type element was not handled");
		}
	}
}

DXGI_FORMAT DynamicVertex::DynamicVertexLayout::GetElementFormat(ElementType elementType)
{
	switch (elementType)
	{
		case ElementType::Position:
			return ElementMap<ElementType::Position>::format;

		case ElementType::Normal:
			return ElementMap<ElementType::Normal>::format;

		case ElementType::TextureCoords:
			return ElementMap<ElementType::TextureCoords>::format;

		case ElementType::Tangent:
			return ElementMap<ElementType::Tangent>::format;

		case ElementType::Bitangent:
			return ElementMap<ElementType::Bitangent>::format;

		case ElementType::Color3:
			return ElementMap<ElementType::Color3>::format;

		case ElementType::Color4:
			return ElementMap<ElementType::Color4>::format;

		default:
		{
			THROW_INTERNAL_ERROR("Layout type element was not handled");
		}
	}
}

/*
*					Vertex
*/

DynamicVertex::DynamicVertex::Vertex::Vertex(DynamicVertexLayout& layout, void* vertexData)
	:
	m_layout(layout),
	m_vertexData(vertexData)
{

}

/*
*					Dynamic Vertex
*/ 

DynamicVertex::DynamicVertex::DynamicVertex(DynamicVertexLayout layout, size_t sizeToAllocate)
	:
	m_layout(layout.Finish()),
	m_numVertices(-1),
	m_allocatedSize(sizeToAllocate),
	m_data(new char[sizeToAllocate * m_layout.GetSize()])
{

}

DynamicVertex::DynamicVertex::~DynamicVertex()
{
	delete[] m_data;
}

void DynamicVertex::DynamicVertex::EmplaceBack()
{
	m_numVertices++;
}

DynamicVertex::DynamicVertex::Vertex DynamicVertex::DynamicVertex::Back()
{
	return Vertex{ m_layout, static_cast<char*>(m_data) + m_layout.GetSize() * m_numVertices };
}

void* DynamicVertex::DynamicVertex::GetData()
{
	return m_data;
}

int DynamicVertex::DynamicVertex::GetNumVertices()
{
	return m_numVertices;
}

const DynamicVertex::DynamicVertexLayout& DynamicVertex::DynamicVertex::GetLayout() const
{
	return m_layout;
}