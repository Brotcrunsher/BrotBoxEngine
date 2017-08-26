#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW\glfw3.h"

#include "../BBE/Rectangle.h"
#include "../BBE/Circle.h"
#include "../BBE/Color.h"

namespace bbe
{
	class Image;

	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;
			class VulkanManager;
			class VulkanCommandPool;
			class VulkanDescriptorPool;
			class VulkanDescriptorSetLayout;
		}
	}

	enum class PipelineRecord
	{
		NONE, PRIMITIVE, IMAGE
	};

	class PrimitiveBrush2D
	{
		friend class INTERNAL::vulkan::VulkanManager;
	private:
		INTERNAL::vulkan::VulkanDevice              *m_pdevice              = nullptr;
		INTERNAL::vulkan::VulkanCommandPool         *m_pcommandPool         = nullptr;
		INTERNAL::vulkan::VulkanDescriptorPool      *m_pdescriptorPool      = nullptr;
		INTERNAL::vulkan::VulkanDescriptorSetLayout *m_pdescriptorSetLayout = nullptr;
		VkCommandBuffer    m_currentCommandBuffer = VK_NULL_HANDLE;
		VkPipelineLayout   m_layoutPrimitive      = VK_NULL_HANDLE;
		VkPipeline         m_pipelinePrimitive    = VK_NULL_HANDLE;
		VkPipelineLayout   m_layoutImage          = VK_NULL_HANDLE;
		VkPipeline         m_pipelineImage        = VK_NULL_HANDLE;
		VkDescriptorSet    m_descriptorSet        = VK_NULL_HANDLE;
		int                m_screenWidth;
		int                m_screenHeight;

		PipelineRecord   m_pipelineRecord = PipelineRecord::NONE;

		void INTERNAL_fillRect(const Rectangle &rect);
		void INTERNAL_drawImage(const Rectangle &rect, const Image &image);
		void INTERNAL_fillCircle(const Circle &circle);
		void INTERNAL_setColor(float r, float g, float b, float a);
		void INTERNAL_beginDraw(
			INTERNAL::vulkan::VulkanDevice &device,
			INTERNAL::vulkan::VulkanCommandPool &commandPool,
			INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool,
			INTERNAL::vulkan::VulkanDescriptorSetLayout &descriptorSetLayout,
			VkCommandBuffer commandBuffer,
			VkPipeline pipelinePrimitive, 
			VkPipelineLayout layoutPrimitive, 
			VkPipeline pipelineImage, 
			VkPipelineLayout layoutImage, 
			int screenWidth, int screenHeight);

	public:


		void fillRect(const Rectangle &rect);
		void fillRect(float x, float y, float width, float height);

		void fillCircle(const Circle &circle);
		void fillCircle(float x, float y, float width, float height);

		void drawImage(const Rectangle &rect, const Image &image);
		void drawImage(float x, float y, float width, float height, const Image &image);

		void setColor(float r, float g, float b, float a);
		void setColor(float r, float g, float b);
		void setColor(const Color &c);
	};
}
