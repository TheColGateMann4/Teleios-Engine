#include "DynamicConstantBuffer.h"


DynamicConstantBuffer::ConstantBufferLayout& DynamicConstantBuffer::ConstantBufferLayout::GetFinished()
{
	THROW_OBJECT_STATE_ERROR_IF("Layout was empty", m_elements.empty());

	m_finished = true;

	m_alignedSize = GetAlignedSize();

	return *this;
}

unsigned int DynamicConstantBuffer::ConstantBufferLayout::GetSize() const
{
	THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", !m_finished);

	return m_alignedSize;
}

const DynamicConstantBuffer::ConstantBufferLayout::LayoutElement& DynamicConstantBuffer::ConstantBufferLayout::GetElement(unsigned int index) const
{
	THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", !m_finished);
	THROW_OBJECT_STATE_ERROR_IF("Layout was empty", m_elements.empty());
	THROW_OBJECT_STATE_ERROR_IF("Index exceeded layout range", index > m_size - 1);

	return m_elements.at(index);
}

const DynamicConstantBuffer::ConstantBufferLayout::LayoutElement& DynamicConstantBuffer::ConstantBufferLayout::GetElement(const char* name) const
{
	THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", !m_finished);
	THROW_OBJECT_STATE_ERROR_IF("Layout was empty", m_elements.empty());
	THROW_OBJECT_STATE_ERROR_IF("Empty name was passed", name == nullptr);

	for (const auto& element : m_elements)
		if (strcmp(name, element.name) == 0)
			return element;

	std::string errorStr = "Failed to find element in layout with name: \"";
	errorStr += name;
	errorStr += "\".";

	THROW_INTERNAL_ERROR(errorStr.c_str());
}

unsigned int DynamicConstantBuffer::ConstantBufferLayout::GetAlignedSize() const
{
	float numPacks = float(m_size) / float(alignment);

	return (numPacks != std::floor(numPacks)) ? std::ceil(numPacks) * alignment : m_size;
}

DynamicConstantBuffer::ConstantBufferData::ConstantBufferData(ConstantBufferData& data)
	:
	ConstantBufferData(data.m_layout)
{
	memcpy_s(m_data, m_layout.GetSize(), data.m_data, data.m_layout.GetSize());
}

DynamicConstantBuffer::ConstantBufferData::ConstantBufferData(ConstantBufferLayout& layout)
	:
	m_layout(layout.GetFinished())
{
	m_data = new char[m_layout.GetSize()];
}

DynamicConstantBuffer::ConstantBufferData::~ConstantBufferData()
{
	delete[] m_data;
}

void* DynamicConstantBuffer::ConstantBufferData::GetPtr()
{
	return m_data;
}

const DynamicConstantBuffer::ConstantBufferLayout& DynamicConstantBuffer::ConstantBufferData::GetLayout() const
{
	return m_layout;
}