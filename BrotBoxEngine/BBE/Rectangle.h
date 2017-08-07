#pragma once
#include "../BBE/VulkanBuffer.h"
#include "../BBE/Vector2.h"

namespace bbe
{
	class VulkanDevice;

	class Rectangle
	{
		friend class PrimitiveBrush2D;
	private:
		float m_x;
		float m_y;
		float m_width;
		float m_height;

		bool m_bufferDirty = true;
		bbe::INTERNAL::vulkan::VulkanBuffer m_vertexBuffer;
		bbe::INTERNAL::vulkan::VulkanBuffer m_indexBuffer;
		bbe::INTERNAL::vulkan::VulkanBuffer* getVulkanVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, int screenWidth, int screenHeight);
		bbe::INTERNAL::vulkan::VulkanBuffer* getVulkanIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice);
		void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, int screenWidth, int screenHeight);
		void createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice);

	public:
		Rectangle();
		Rectangle(float x, float y, float width, float height);

		~Rectangle();
		Rectangle(const Rectangle&);
		Rectangle(Rectangle&&);
		Rectangle& operator=(const Rectangle&);
		Rectangle& operator=(Rectangle&&);

		float getX() const;
		float getY() const;
		float getWidth() const;
		float getHeight() const;

		void setX(float x);
		void setY(float y);
		void setWidth(float width);
		void setHeight(float height);
		void set(float x, float y, float width, float height);

		void translate(float x, float y);
		void translate(const Vector2 &vec);
	};
}
