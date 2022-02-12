#pragma once

#include "../BBE/Vulkan/VulkanBuffer.h"
#include "../BBE/Vulkan/VulkanCommandPool.h"
#include "../BBE/Vector2.h"

namespace bbe
{
	class VulkanDevice;

	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
		}
	}

	class Rectangle;

	class Circle
	{
		friend class PrimitiveBrush2D;
		friend class INTERNAL::vulkan::VulkanManager;
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

		static const uint32_t AMOUNTOFVERTICES;

	public:
		Circle();
		Circle(float x, float y, float width, float height);
		Circle(const Vector2 &vec, float width, float height);
		Circle(float x, float y, const Vector2 &dim);
		Circle(const Vector2 &vec, const Vector2 &dim);

		float getX() const;
		float getY() const;
		Vector2 getPos() const;
		Vector2 getMiddle() const;
		float getWidth() const;
		float getHeight() const;
		Vector2 getDim() const;

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

		bool intersects(const Circle& other) const;
		bool intersects(const Rectangle& other) const;
		bool resolveIntersection(Circle& other, float massThis = 1, float massOther = 1);
	};
}