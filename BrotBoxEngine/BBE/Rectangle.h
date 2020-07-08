#pragma once
#include "../BBE/VulkanBuffer.h"
#include "../BBE/VulkanCommandPool.h"
#include "../BBE/Vector2.h"

namespace bbe
{
	class VulkanDevice;
	class Circle;

	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
		}
	}

	class Rectangle
	{
		friend class PrimitiveBrush2D;
		friend class ::bbe::INTERNAL::vulkan::VulkanManager;
	private:
		float m_x;
		float m_y;
		float m_width;
		float m_height;

		static void s_init(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		static void s_initIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		static void s_initVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, INTERNAL::vulkan::VulkanCommandPool &commandPool, VkQueue queue);
		static void s_destroy();
		static bbe::INTERNAL::vulkan::VulkanBuffer s_indexBuffer;
		static bbe::INTERNAL::vulkan::VulkanBuffer s_vertexBuffer;

	public:
		Rectangle();
		Rectangle(float x, float y, float width, float height);
		Rectangle(const Vector2 &vec, float width, float height);
		Rectangle(float x, float y, const Vector2 &dim);
		Rectangle(const Vector2 &vec, const Vector2 &dim);

		Vector2 getPos() const;
		Vector2 getDim() const;

		float getX() const;
		float getY() const;
		float getWidth() const;
		float getHeight() const;

		void setX(float x);
		void setY(float y);
		void setPos(float x, float y);
		void setPos(const Vector2 &vec);
		void setWidth(float width);
		void setHeight(float height);
		void setDim(float width, float height);
		void setDim(const Vector2 &vec);
		void set(float x, float y, float width, float height);

		void translate(float x, float y);
		void translate(const Vector2 &vec);

		float getDistanceTo(const Vector2 &vec);

		bool isPointInRectangle(const Vector2 point) const;
		bool intersects(const Rectangle& rectangle) const;
		bool intersects(const Circle& circle) const;
	};
}
