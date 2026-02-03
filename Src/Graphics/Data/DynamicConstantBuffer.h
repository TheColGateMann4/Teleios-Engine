#pragma once
#include "Includes/CppIncludes.h"
#include "Includes/DirectXIncludes.h"
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
		Matrix,
		List
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

	struct DataInfo;
	struct ArrayDataInfo;

	class Layout
	{
		static constexpr unsigned int alignment = 16;
		static constexpr unsigned int bufferSizeAlignment = 256;

	public:
		struct Element
		{
			Element() = default;
			Element(Element&&) noexcept = default;
			Element(const Element&) = delete;
			Element& operator=(Element&&) noexcept = default;
			Element& operator=(const Element&) = delete;

			ElementType type;
			unsigned int size;
			unsigned int offset;
			std::string name;

			std::unique_ptr<ImguiData> imguiData;
			std::unique_ptr<DataInfo> additionalData;
		};

	public:
		Layout() = default;
		Layout(Layout&&) noexcept = default;
		Layout(const Layout&) = delete;
		Layout& operator=(Layout&&) noexcept = default;
		Layout& operator=(const Layout&) = delete;

	public:
		Layout& GetFinished(bool isPartialBuffer = false);

		void AddArray(std::string name, ArrayDataInfo& arrayData);

		// Adding layout elements that can be displayed in imgui
		template<ElementType elementType, typename imguiDataType = ElementMap<elementType>::imguiDataType, ENABLE_IF(elementType != DynamicConstantBuffer::ElementType::List)>
		void Add(std::string name, imguiDataType imguiData = {})
		{
			THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", m_finished);

			Element element = {};
			element.type = elementType;
			element.size = GetNewElementSize<elementType>();
			element.offset = GetNewElementOffset<elementType>();
			element.name = name;
			element.imguiData = std::make_unique<imguiDataType>(imguiData);

			m_elements.push_back(std::move(element));

			m_size = element.offset + element.size;
		}

		template<ElementType elementType, ENABLE_IF(elementType == DynamicConstantBuffer::ElementType::Matrix && elementType != DynamicConstantBuffer::ElementType::List)>
		void Add(const char* name)
		{
			THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", m_finished);

			Element element = {};
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


		const Element& GetElement(unsigned int index) const;

		const Element& GetElement(const char* name) const;

	private:
		template<ElementType elementType, ENABLE_IF(elementType != DynamicConstantBuffer::ElementType::List)>
		consteval static unsigned int GetNewElementSize()
		{
			return ElementMap<elementType>::size;
		}

		template<ElementType elementType, ENABLE_IF(elementType != DynamicConstantBuffer::ElementType::List)>
		unsigned int GetNewElementOffset() const
		{
			float numPacks = float(m_size) / float(alignment); // number of packs so far we went through
			unsigned int sizeOfLastPack = (numPacks - std::floor(numPacks)) * alignment;
			unsigned int lastPackSizeWithNewElement = sizeOfLastPack + GetNewElementSize<elementType>();

			return lastPackSizeWithNewElement > alignment ? std::ceil(numPacks) * alignment : m_size;
		}

		unsigned int GetAlignedSize() const;
		unsigned int GetPackedSize() const;

	private:
		std::vector<Element> m_elements = {};
		unsigned int m_size = 0;
		unsigned int m_alignedSize = 0;
		bool m_finished = false;
	};

	// additional element data
	struct DataInfo
	{
		virtual ~DataInfo() = default;
	};

	struct ArrayDataInfo : public DataInfo
	{
		ArrayDataInfo() = default;
		ArrayDataInfo(ArrayDataInfo&&) noexcept = default;
		ArrayDataInfo(const ArrayDataInfo&) = delete;
		ArrayDataInfo& operator=(ArrayDataInfo&&) noexcept = default;
		ArrayDataInfo& operator=(const ArrayDataInfo&) = delete;

		virtual ~ArrayDataInfo() = default;

		Layout layout;
		int numElements = -1;
	};

	class ArrayData
	{
	public:
		ArrayData(const Layout& layout, char* data, unsigned int numElements);

		template<ElementType elementType, ENABLE_IF(ElementMap<elementType>::valid)>
		ElementMap<elementType>::dataType* Get(unsigned int i, const char* name)
		{
			THROW_INTERNAL_ERROR_IF("Tried to access index out of bounds", i >= m_numElements);
			
			auto& layoutElement = m_layout.GetElement(name);

			THROW_INTERNAL_ERROR_IF("Tried to get value with different type than given layout element type", layoutElement.type != elementType);

			unsigned int offsetInArray = i * m_layout.GetSize();
			unsigned int offsetOfElementInLayout = layoutElement.offset;
			return reinterpret_cast<ElementMap<elementType>::dataType*>(m_data + offsetInArray + offsetOfElementInLayout);
		}

		bool DrawImguiProperties(bool asArray = true);
		bool DrawImguiProperties(unsigned int i, bool asArray = true);

	private:
		const Layout& m_layout;
		char* m_data;
		int m_numElements;
	};

	class Data
	{
	public:
		Data(Data&& data) noexcept;

		Data(Layout& layout);

	public:
		template<ElementType elementType, ENABLE_IF(ElementMap<elementType>::valid)>
		ElementMap<elementType>::dataType* Get(unsigned int index)
		{
			auto& layoutElement = m_layout.GetElement(index);

			THROW_INTERNAL_ERROR_IF("Tried to get value with different type than given layout element type", layoutElement.type != elementType);

			return reinterpret_cast<ElementMap<elementType>::dataType*>(&m_data.at(layoutElement.offset));
		}

		template<ElementType elementType, ENABLE_IF(ElementMap<elementType>::valid)>
		ElementMap<elementType>::dataType* Get(const char* name)
		{
			auto& layoutElement = m_layout.GetElement(name);

			THROW_INTERNAL_ERROR_IF("Tried to get value with different type than given layout element type", layoutElement.type != elementType);

			return reinterpret_cast<ElementMap<elementType>::dataType*>(&m_data.at(layoutElement.offset));
		}

		ArrayData GetArrayData(const char* name);

	public:
		void* GetPtr();

		const Layout& GetLayout() const;

		bool DrawImguiProperties(); // returns true if buffer was updated

	private:
		std::vector<char> m_data;
		Layout m_layout;
	};

	bool DrawImguiPropety(const DynamicConstantBuffer::Layout::Element& element, void* elementData, const char* elementName);
};