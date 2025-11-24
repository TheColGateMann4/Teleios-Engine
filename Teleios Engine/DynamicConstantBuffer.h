#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "macros/ErrorMacros.h"

namespace DynamicConstantBuffer
{
	enum class ElementType
	{
		Int,
		Uint3,
		Uint,
		Bool,
		Float,
		Float2,
		Float3,
		Float4,
		Matrix
	};


	// imgui data
	struct ImguiData
	{
		virtual ~ImguiData() = default;

		ImguiData(bool show_ = true);

		bool ShouldShow();

		bool show;
	};

	struct ImguiIntData : public ImguiData
	{
		virtual ~ImguiIntData() = default;

		ImguiIntData(bool show_ = true, int min_ = 0, int max_ = 10, const char* format_ = "%d", int flags_ = 0);

	public:
		int min;
		int max;

		const char* format;
		int flags;
	};

	struct ImguiUintData : public ImguiData
	{
		virtual ~ImguiUintData() = default;

		ImguiUintData(bool show_ = true, unsigned int min_ = 0, unsigned int max_ = 10, const char* format_ = "%d", int flags_ = 0);

	public:
		unsigned int min;
		unsigned int max;

		const char* format;
		int flags;
	};

	struct ImguiFloatData : public ImguiData
	{
		virtual ~ImguiFloatData() = default;

		ImguiFloatData(bool show_ = true, float min_ = 0.0f, float max_ = 1.0f, const char* format_ = "%.2f", int flags_ = 0);

	public:
		float min;
		float max;

		const char* format;
		int flags;
	};

	struct ImguiColorData : public ImguiData
	{
		virtual ~ImguiColorData() = default;

		ImguiColorData(bool show_ = true, int flags_ = 0);

	public:
		int flags = 0;
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
		using imguiDataType = ImguiIntData;
	};
	template<>
	struct ElementMap<ElementType::Uint>
	{
		static constexpr bool valid = true;

		using dataType = UINT;
		static constexpr unsigned int size = sizeof(dataType);
		using imguiDataType = ImguiUintData;
	};
	template<>
	struct ElementMap<ElementType::Uint3>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMUINT3;
		static constexpr unsigned int size = sizeof(dataType);
		using imguiDataType = ImguiUintData;
	};
	template<>
	struct ElementMap<ElementType::Bool>
	{
		static constexpr bool valid = true;

		using dataType = UINT;
		static constexpr unsigned int size = sizeof(dataType);
		using imguiDataType = ImguiData;
	};
	template<>
	struct ElementMap<ElementType::Float>
	{
		static constexpr bool valid = true;

		using dataType = float;
		static constexpr unsigned int size = sizeof(dataType);
		using imguiDataType = ImguiFloatData;
	};
	template<>
	struct ElementMap<ElementType::Float2>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMFLOAT2;
		static constexpr unsigned int size = sizeof(dataType);
		using imguiDataType = ImguiFloatData;
	};
	template<>
	struct ElementMap<ElementType::Float3>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMFLOAT3;
		static constexpr unsigned int size = sizeof(dataType);
		using imguiDataType = ImguiColorData;
	};
	template<>
	struct ElementMap<ElementType::Float4>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMFLOAT4;
		static constexpr unsigned int size = sizeof(dataType);
		using imguiDataType = ImguiColorData;
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

			std::shared_ptr<ImguiData> imguiData;
		};

	public:
		ConstantBufferLayout& GetFinished();

		// Adding layout elements that can be displayed in imgui
		template<ElementType elementType, typename imguiDataType = ElementMap<elementType>::imguiDataType>
		void AddElement(const char* name, imguiDataType imguiData = {})
		{
			THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", m_finished);

			LayoutElement element = {};
			element.type = elementType;
			element.size = GetNewElementSize<elementType>();
			element.offset = GetNewElementOffset<elementType>();
			element.name = name;
			element.imguiData = std::make_shared<imguiDataType>(imguiData);

			m_elements.push_back(std::move(element));

			m_size = element.offset + element.size;
		}

		template<ElementType elementType, std::enable_if_t<(elementType == DynamicConstantBuffer::ElementType::Matrix), int> = 0>
		void AddElement(const char* name)
		{
			THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", m_finished);

			LayoutElement element = {};
			element.type = elementType;
			element.size = GetNewElementSize<elementType>();
			element.offset = GetNewElementOffset<elementType>();
			element.name = name;
			element.imguiData = {};

			m_elements.push_back(std::move(element));

			m_size = element.offset + element.size;
		}

	public:
		unsigned int GetSize() const;

		unsigned int GetNumElements() const;


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

		bool DrawImguiProperties(); // returns if buffer was updated

	private:
		std::shared_ptr<char[]> m_data;
		ConstantBufferLayout m_layout;
	};
};