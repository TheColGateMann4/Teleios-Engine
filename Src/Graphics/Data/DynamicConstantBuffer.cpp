#include "DynamicConstantBuffer.h"

#include <imgui.h>


#undef min 
#undef max

DynamicConstantBuffer::ImguiData::ImguiData(bool show_)
	:
	show(show_)
{

}

bool DynamicConstantBuffer::ImguiData::ShouldShow()
{
	return show;
}

DynamicConstantBuffer::ImguiIntData::ImguiIntData(bool show_, int min_, int max_, const char* format_, int flags_)
	:
	ImguiData(show_),
	min(min_),
	max(max_),
	format(format_),
	flags(flags_)
{

}


DynamicConstantBuffer::ImguiFloatData::ImguiFloatData(bool show_, float min_, float max_, const char* format_, int flags_)
	:
	ImguiData(show_),
	min(min_),
	max(max_),
	format(format_),
	flags(flags_)
{

}

DynamicConstantBuffer::ImguiColorData::ImguiColorData(bool show_, int flags_)
	:
	ImguiData(show_),
	flags(flags_)
{

}

DynamicConstantBuffer::Layout& DynamicConstantBuffer::Layout::GetFinished(bool isPartialBuffer)
{
	THROW_OBJECT_STATE_ERROR_IF("Layout was empty", m_elements.empty());

	m_finished = true;

	m_alignedSize = isPartialBuffer ? GetPackedSize() : GetAlignedSize();

	return *this;
}

void DynamicConstantBuffer::Layout::AddArray(std::string name, ArrayDataInfo& arrayData)
{
	THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", m_finished);
	THROW_INTERNAL_ERROR_IF("Array size was not set", arrayData.numElements == -1);

	arrayData.layout.GetFinished(true);

	Element element = {};
	element.type = ElementType::List;
	element.size = arrayData.layout.GetSize() * arrayData.numElements;
	element.offset = GetPackedSize();
	element.name = name;
	element.imguiData = {};
	element.additionalData = std::make_unique<ArrayDataInfo>(std::move(arrayData));

	m_elements.push_back(std::move(element));

	m_size = element.offset + element.size;
}

unsigned int DynamicConstantBuffer::Layout::GetSize() const
{
	THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", !m_finished);

	return m_alignedSize;
}

unsigned int DynamicConstantBuffer::Layout::GetNumElements() const
{
	THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", !m_finished);

	return m_elements.size();
}

const DynamicConstantBuffer::Layout::Element& DynamicConstantBuffer::Layout::GetElement(unsigned int index) const
{
	THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", !m_finished);
	THROW_OBJECT_STATE_ERROR_IF("Layout was empty", m_elements.empty());
	THROW_OBJECT_STATE_ERROR_IF("Index exceeded layout range", index > m_size - 1);

	return m_elements.at(index);
}

const DynamicConstantBuffer::Layout::Element& DynamicConstantBuffer::Layout::GetElement(const char* name) const
{
	THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", !m_finished);
	THROW_OBJECT_STATE_ERROR_IF("Layout was empty", m_elements.empty());
	THROW_OBJECT_STATE_ERROR_IF("Empty name was passed", name == nullptr);

	for (const auto& element : m_elements)
		if (strcmp(name, element.name.c_str()) == 0)
			return element;

	std::string errorStr = "Failed to find element in layout with name: \"";
	errorStr += name;
	errorStr += "\".";

	THROW_INTERNAL_ERROR(errorStr.c_str());
}

unsigned int DynamicConstantBuffer::Layout::GetAlignedSize() const
{
	float numPacks = float(m_size) / float(bufferSizeAlignment);

	return (numPacks != std::floor(numPacks)) ? std::ceil(numPacks) * bufferSizeAlignment : m_size;
}

unsigned int DynamicConstantBuffer::Layout::GetPackedSize() const
{
	float numPacks = float(m_size) / float(alignment);

	return (numPacks != std::floor(numPacks)) ? std::ceil(numPacks) * alignment : m_size;
}


DynamicConstantBuffer::ArrayData::ArrayData(const Layout& layout, char* data, unsigned int numElements)
	:
	m_layout(layout),
	m_data(data),
	m_numElements(numElements)
{

}

bool DynamicConstantBuffer::ArrayData::DrawImguiProperties(bool asArray)
{
	bool changed = false;

	auto checkChanged = [&changed](bool expressionReturn) mutable
		{
			changed = changed || expressionReturn;
		};

	for (int i = 0; i < m_numElements; i++)
		DrawImguiProperties(i, asArray);

	return changed;
}

bool DynamicConstantBuffer::ArrayData::DrawImguiProperties(unsigned int i, bool asArray)
{
	bool changed = false;

	auto checkChanged = [&changed](bool expressionReturn) mutable
		{
			changed = changed || expressionReturn;
		};

	unsigned int offsetInArray = i * m_layout.GetSize();

	for (unsigned int itemIndex = 0; itemIndex < m_layout.GetNumElements(); itemIndex++)
	{
		auto& element = m_layout.GetElement(itemIndex);
		void* elementData = m_data + offsetInArray + element.offset;

		if (asArray)
		{
			std::string elementName = element.name + '[' + std::to_string(i) + ']';
			checkChanged(DrawImguiPropety(element, elementData, elementName.c_str()));
		}
		else
		{
			checkChanged(DrawImguiPropety(element, elementData, element.name.c_str()));
		}
	}

	return changed;
}

DynamicConstantBuffer::Data::Data(Data&& data) noexcept
	:
	m_data(std::move(data.m_data)),
	m_layout(std::move(data.m_layout))
{

}

DynamicConstantBuffer::Data::Data(Layout& layout)
	:
	m_layout(std::move(layout.GetFinished()))
{
	m_data.resize(layout.GetSize());
}

DynamicConstantBuffer::ArrayData DynamicConstantBuffer::Data::GetArrayData(const char* name)
{
	const auto& element = m_layout.GetElement(name);

	THROW_INTERNAL_ERROR_IF("Element was not Array type", element.type != ElementType::List);

	const ArrayDataInfo* arrayDataInfo = static_cast<const ArrayDataInfo*>(element.additionalData.get());

	return ArrayData(arrayDataInfo->layout, &m_data.at(element.offset), arrayDataInfo->numElements);
}

void* DynamicConstantBuffer::Data::GetPtr()
{
	return m_data.data();
}

const DynamicConstantBuffer::Layout& DynamicConstantBuffer::Data::GetLayout() const
{
	return m_layout;
}

bool DynamicConstantBuffer::Data::DrawImguiProperties()
{
	bool changed = false;

	auto checkChanged = [&changed](bool expressionReturn) mutable
		{
			changed = changed || expressionReturn;
		};

	for(unsigned int itemIndex = 0; itemIndex < m_layout.GetNumElements(); itemIndex++)
	{
		auto& element = m_layout.GetElement(itemIndex);
		void* elementData = &m_data.at(element.offset);

		if(static_cast<ImguiData*>(element.imguiData.get()));

		checkChanged(DrawImguiPropety(element, elementData, element.name.c_str()));
	}

	return changed;
}

bool DynamicConstantBuffer::DrawImguiPropety(const DynamicConstantBuffer::Layout::Element& element, void* elementData, const char* elementName)
{
	if (element.type == DynamicConstantBuffer::ElementType::List)
	{
		const DynamicConstantBuffer::ArrayDataInfo* arrayDataInfo = static_cast<const DynamicConstantBuffer::ArrayDataInfo*>(element.additionalData.get());
		DynamicConstantBuffer::ArrayData array(arrayDataInfo->layout, static_cast<char*>(elementData) + element.offset, arrayDataInfo->numElements);

		return array.DrawImguiProperties();
	}

	if (!element.imguiData) // if object doesn't have propeties then we don't handle it for imgui layer
		return false;

	auto* imguiData = static_cast<DynamicConstantBuffer::ImguiData*>(element.imguiData.get());
	if (!imguiData->ShouldShow())
		return false;

	switch (element.type)
	{
		case DynamicConstantBuffer::ElementType::Int:
		{
			auto* imguiIntData = static_cast<DynamicConstantBuffer::ImguiIntData*>(imguiData);
		
			return ImGui::SliderInt(elementName, reinterpret_cast<int*>(elementData), imguiIntData->min, imguiIntData->max, imguiIntData->format, imguiIntData->flags);
		}
		case DynamicConstantBuffer::ElementType::Bool:
		{
			return ImGui::Checkbox(elementName, reinterpret_cast<bool*>(elementData));
		}
		case DynamicConstantBuffer::ElementType::Float:
		{
			auto* imguiFloatData = static_cast<DynamicConstantBuffer::ImguiFloatData*>(imguiData);
		
			return ImGui::SliderFloat(elementName, reinterpret_cast<float*>(elementData), imguiFloatData->min, imguiFloatData->max, imguiFloatData->format, imguiFloatData->flags);
		}
		case DynamicConstantBuffer::ElementType::Float2:
		{
			auto* imguiFloatData = static_cast<DynamicConstantBuffer::ImguiFloatData*>(imguiData);
		
			return ImGui::SliderFloat2(elementName, reinterpret_cast<float*>(elementData), imguiFloatData->min, imguiFloatData->max, imguiFloatData->format, imguiFloatData->flags);
		}
		case DynamicConstantBuffer::ElementType::Float3:
		{
			auto* imguiColorData = static_cast<DynamicConstantBuffer::ImguiColorData*>(imguiData);
		
			return ImGui::ColorEdit3(elementName, reinterpret_cast<float*>(elementData), imguiColorData->flags);
		}
		case DynamicConstantBuffer::ElementType::Float4:
		{
			auto* imguiColorData = static_cast<DynamicConstantBuffer::ImguiColorData*>(imguiData);
		
			return ImGui::ColorEdit4(elementName, reinterpret_cast<float*>(elementData), imguiColorData->flags);
		}
		case DynamicConstantBuffer::ElementType::Matrix:
		{
			THROW_INTERNAL_ERROR("Tried to draw imgui layout for buffer with matrix element");
		}
	}
}