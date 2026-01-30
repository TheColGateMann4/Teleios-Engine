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

unsigned int DynamicConstantBuffer::ConstantBufferLayout::GetNumElements() const
{
	THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", !m_finished);

	return m_elements.size();
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
	float numPacks = float(m_size) / float(bufferSizeAlignment);

	return (numPacks != std::floor(numPacks)) ? std::ceil(numPacks) * bufferSizeAlignment : m_size;
}



DynamicConstantBuffer::ConstantBufferData::ConstantBufferData(const ConstantBufferData& data)
	:
	m_data(data.m_data),
	m_layout(data.m_layout)
{

}

DynamicConstantBuffer::ConstantBufferData::ConstantBufferData(ConstantBufferData&& data) noexcept
	:
	m_data(std::move(data.m_data)),
	m_layout(std::move(data.m_layout))
{

}

DynamicConstantBuffer::ConstantBufferData::ConstantBufferData(ConstantBufferLayout& layout)
	:
	m_layout(layout.GetFinished())
{
	m_data = std::shared_ptr<char[]>(new char[layout.GetSize()]);
}

void* DynamicConstantBuffer::ConstantBufferData::GetPtr()
{
	return m_data.get();
}

const DynamicConstantBuffer::ConstantBufferLayout& DynamicConstantBuffer::ConstantBufferData::GetLayout() const
{
	return m_layout;
}

bool DynamicConstantBuffer::ConstantBufferData::DrawImguiProperties()
{
	bool changed = false;

	auto checkChanged = [](bool& checkValue, bool expressionReturn) mutable
		{
			checkValue = checkValue || expressionReturn;
		};

	for(unsigned int itemIndex = 0; itemIndex < m_layout.GetNumElements(); itemIndex++)
	{
		auto& layoutElement = m_layout.GetElement(itemIndex);
		void* elementData = m_data.get() + layoutElement.offset;

		if(!layoutElement.imguiData) // if object doesn't have propeties then we don't handle it for imgui layer
			continue;

		switch (layoutElement.type)
		{
			case ElementType::Int:
			{
				auto imguiIntData = static_cast<ImguiIntData*>(layoutElement.imguiData.get());

				if (imguiIntData->ShouldShow())
					checkChanged(changed, ImGui::SliderInt(layoutElement.name, reinterpret_cast<int*>(elementData), imguiIntData->min, imguiIntData->max, imguiIntData->format, imguiIntData->flags));
				break;
			}
			case ElementType::Bool:
			{
				if (layoutElement.imguiData->ShouldShow())
					checkChanged(changed, ImGui::Checkbox(layoutElement.name, reinterpret_cast<bool*>(elementData)));
				break;
			}
			case ElementType::Float:
			{
				auto imguiFloatData = static_cast<ImguiFloatData*>(layoutElement.imguiData.get());

				if (imguiFloatData->ShouldShow())
					checkChanged(changed, ImGui::SliderFloat(layoutElement.name, reinterpret_cast<float*>(elementData), imguiFloatData->min, imguiFloatData->max, imguiFloatData->format, imguiFloatData->flags));
				break;
			}
			case ElementType::Float2:
			{
				auto imguiFloatData = static_cast<ImguiFloatData*>(layoutElement.imguiData.get());

				if (imguiFloatData->ShouldShow())
					checkChanged(changed, ImGui::SliderFloat2(layoutElement.name, reinterpret_cast<float*>(elementData), imguiFloatData->min, imguiFloatData->max, imguiFloatData->format, imguiFloatData->flags));
				break;
			}
			case ElementType::Float3:
			{
				auto imguiColorData = static_cast<ImguiColorData*>(layoutElement.imguiData.get());

				if (imguiColorData->ShouldShow())
					checkChanged(changed, ImGui::ColorEdit3(layoutElement.name, reinterpret_cast<float*>(elementData), imguiColorData->flags));
				break;
			}
			case ElementType::Float4:
			{
				auto imguiColorData = static_cast<ImguiColorData*>(layoutElement.imguiData.get());

				if (imguiColorData->ShouldShow())
					checkChanged(changed, ImGui::ColorEdit4(layoutElement.name, reinterpret_cast<float*>(elementData), imguiColorData->flags));
				break;
			}
			case ElementType::Matrix:
				THROW_INTERNAL_ERROR("Tried to draw imgui layout for buffer with matrix element");
		}
	}

	return changed;
}