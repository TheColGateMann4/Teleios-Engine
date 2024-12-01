#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "macros/ErrorMacros.h"

namespace DynamicConstantBuffer
{
	enum class ElementType
	{
		Int,
		Bool,
		Float,
		Float2,
		Float3,
		Float4,
		Matrix
	};

	template<ElementType type>
	struct ElementMap
	{
		bool valid = false;
	};

	template<>
	struct ElementMap<ElementType::Int>
	{
		static constexpr bool valid = true;

		using dataType = INT;
		static constexpr unsigned int size = sizeof(dataType);
	};
	template<>
	struct ElementMap<ElementType::Bool>
	{
		static constexpr bool valid = true;

		using dataType = UINT;
		static constexpr unsigned int size = sizeof(dataType);
	};
	template<>
	struct ElementMap<ElementType::Float>
	{
		static constexpr bool valid = true;

		using dataType = float;
		static constexpr unsigned int size = sizeof(dataType);
	};
	template<>
	struct ElementMap<ElementType::Float2>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMFLOAT2;
		static constexpr unsigned int size = sizeof(dataType);
	};
	template<>
	struct ElementMap<ElementType::Float3>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMFLOAT3;
		static constexpr unsigned int size = sizeof(dataType);
	};
	template<>
	struct ElementMap<ElementType::Float4>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMFLOAT4;
		static constexpr unsigned int size = sizeof(dataType);
	};
	template<>
	struct ElementMap<ElementType::Matrix>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMMATRIX;
		static constexpr unsigned int size = sizeof(dataType);
	};


	class ConstantBufferLayout
	{
		static constexpr unsigned int alignment = 16;
		static constexpr unsigned int bufferSizeAlignment = 256;

		struct LayoutElement
		{
			ElementType type;
			unsigned int size;
			unsigned int offset;
			const char* name;
		};

	public:
		ConstantBufferLayout& GetFinished();

		template<ElementType elementType>
		void AddElement(const char* name)
		{
			THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", m_finished);

			LayoutElement element = {};
			element.type = elementType;
			element.size = GetNewElementSize<elementType>();
			element.offset = GetNewElementOffset<elementType>();
			element.name = name;

			m_elements.push_back(std::move(element));

			m_size = element.offset + element.size;
		}

	public:
		unsigned int GetSize() const;


		const LayoutElement& GetElement(unsigned int index) const;

		const LayoutElement& GetElement(const char* name) const;

	private:
		template<ElementType elementType>
		consteval static unsigned int GetNewElementSize()
		{
			return ElementMap<elementType>::size;
		}

		template<ElementType elementType>
		unsigned int GetNewElementOffset() const
		{
			float numPacks = float(m_size) / float(alignment); // number of packs so far we went through
			unsigned int sizeOfLastPack = (numPacks - std::floor(numPacks)) * alignment;
			unsigned int lastPackSizeWithNewElement = sizeOfLastPack + GetNewElementSize<elementType>();

			return lastPackSizeWithNewElement > alignment ? std::ceil(numPacks) * alignment : m_size;
		}

		unsigned int GetAlignedSize() const;

	private:
		std::vector<LayoutElement> m_elements = {};
		unsigned int m_size = 0;
		unsigned int m_alignedSize = 0;
		bool m_finished = false;
	};

	class ConstantBufferData
	{
	public:
		ConstantBufferData(const ConstantBufferData& data);

		ConstantBufferData(ConstantBufferData&& data) noexcept;

		ConstantBufferData(ConstantBufferLayout& layout);

	public:
		template<ElementType elementType, std::enable_if_t<ElementMap<elementType>::valid, int> = 0>
		ElementMap<elementType>::dataType* GetValuePointer(unsigned int index)
		{
			auto& layoutElement = m_layout.GetElement(index);

			THROW_INTERNAL_ERROR_IF("Tried to get value with different type than given layout element type", layoutElement.type != elementType);

			return reinterpret_cast<ElementMap<elementType>::dataType*>(m_data.get() + layoutElement.offset);
		}

		template<ElementType elementType, std::enable_if_t<ElementMap<elementType>::valid, int> = 0>
		ElementMap<elementType>::dataType* GetValuePointer(const char* name)
		{
			auto& layoutElement = m_layout.GetElement(name);

			THROW_INTERNAL_ERROR_IF("Tried to get value with different type than given layout element type", layoutElement.type != elementType);

			return reinterpret_cast<ElementMap<elementType>::dataType*>(m_data.get() + layoutElement.offset);
		}

	public:
		void* GetPtr();

		const ConstantBufferLayout& GetLayout() const;

	private:
		std::shared_ptr<char[]> m_data;
		ConstantBufferLayout m_layout;
	};
};

