#pragma once
#include "includes/CppIncludes.h"
#include "includes/DirectXIncludes.h"
#include "macros/ErrorMacros.h"

struct D3D12_INPUT_ELEMENT_DESC;

namespace DynamicVertex
{
	enum class ElementType
	{
		Position,
		Normal,
		Tangent,
		Bitangent,
		TextureCoords,
		Color3,
		Color4
	};

	template<ElementType type>
	struct ElementMap
	{
		bool valid = false;
	};

	template<>
	struct ElementMap<ElementType::Position>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMFLOAT3;
		static constexpr unsigned int size = sizeof(dataType);
		static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "POSITION";
	};
	template<>
	struct ElementMap<ElementType::Normal>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMFLOAT3;
		static constexpr unsigned int size = sizeof(dataType);
		static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "NORMAL";
	};
	template<>
	struct ElementMap<ElementType::Tangent>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMFLOAT3;
		static constexpr unsigned int size = sizeof(dataType);
		static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "TANGENT";
	};
	template<>
	struct ElementMap<ElementType::Bitangent>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMFLOAT3;
		static constexpr unsigned int size = sizeof(dataType);
		static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "BITANGENT";
	};
	template<>
	struct ElementMap<ElementType::TextureCoords>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMFLOAT2;
		static constexpr unsigned int size = sizeof(dataType);
		static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32_FLOAT;
		static constexpr const char* semantic = "TEXCOORDS";
	};
	template<>
	struct ElementMap<ElementType::Color3>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMFLOAT3;
		static constexpr unsigned int size = sizeof(dataType);
		static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32B32_FLOAT;
		static constexpr const char* semantic = "COLOR";
	};
	template<>
	struct ElementMap<ElementType::Color4>
	{
		static constexpr bool valid = true;

		using dataType = DirectX::XMFLOAT4;
		static constexpr unsigned int size = sizeof(dataType);
		static constexpr DXGI_FORMAT format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		static constexpr const char* semantic = "COLOR";
	};


	class DynamicVertexLayout
	{
		struct LayoutElement
		{
			ElementType type;
			unsigned int size;
			unsigned int offset;
		};

	public:
		template<ElementType elementType>
		void AddElement()
		{
			THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", m_finished);

			LayoutElement element = {};
			element.type = elementType;
			element.size = GetNewElementSize<elementType>();
			element.offset = m_size;


			m_size += element.size;
			m_numElements++;

			m_elements.push_back(std::move(element));
		}

		template<ElementType elementType>
		size_t GetElementOffset() const
		{
			THROW_OBJECT_STATE_ERROR_IF("Layout was unfinished", !m_finished);
			THROW_OBJECT_STATE_ERROR_IF("Layout was empty", m_elements.empty());

			for (const auto& element : m_elements)
				if (element.type == elementType)
					return element.offset;

			THROW_INTERNAL_ERROR("Failed to find element for given type");
		}

		size_t GetSize() const;

		size_t GetNumElements() const;

		DynamicVertexLayout Finish();

		std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout() const;

		std::string GetIdentifier() const;

	private:
		template<ElementType elementType>
		consteval static unsigned int GetNewElementSize()
		{
			return ElementMap<elementType>::size;
		}

		static const char* GetElementSemanticName(ElementType elementType);

		static DXGI_FORMAT GetElementFormat(ElementType elementType);

	private:
		std::vector<LayoutElement> m_elements;
		size_t m_numElements = 0;
		size_t m_size = 0;
		bool m_finished = false;
	};

	class DynamicVertex
	{
		class Vertex
		{
		public:
			Vertex(DynamicVertexLayout& layout, void* vertexData);

			Vertex operator=(const Vertex&) = delete;

		public:
			template<ElementType elementType>
			ElementMap<elementType>::dataType& GetPropety()
			{
				return *reinterpret_cast<ElementMap<elementType>::dataType*>(static_cast<char*>(m_vertexData) + m_layout.GetElementOffset<elementType>());
			}

		private:
			DynamicVertexLayout& m_layout;
			void* m_vertexData;
		};

	public:
		DynamicVertex(DynamicVertexLayout layout, size_t sizeToAllocate);

		~DynamicVertex();

	public:
		void EmplaceBack();

		Vertex Back();

	public:
		void* GetData();

		int GetNumVertices();

		const DynamicVertexLayout& GetLayout() const;

	private:
		DynamicVertexLayout m_layout;
		int m_numVertices;
		size_t m_allocatedSize;
		void* m_data;
	};
};