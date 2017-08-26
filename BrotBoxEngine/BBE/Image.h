#pragma once

#include "../BBE/VulkanHelper.h"
#include "../BBE/ColorByte.h"
#include "../BBE/Color.h"
#include "../BBE/String.h"

namespace bbe
{
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanDevice;
			class VulkanCommandPool;
			class VulkanManager;
			class VulkanDescriptorSet;
		}
	}

	class Image
	{
		friend class INTERNAL::vulkan::VulkanManager;
		friend class INTERNAL::vulkan::VulkanDescriptorSet;
	private:
		ColorByte *m_pdata  = nullptr;
		int    m_width  = 0;
		int    m_height = 0;

		VkImage        m_image       = VK_NULL_HANDLE;
		VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
		VkImageView    m_imageView   = VK_NULL_HANDLE;
		VkImageLayout  m_imageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		VkDevice       m_device      = VK_NULL_HANDLE;
		VkSampler      m_sampler     = VK_NULL_HANDLE;

		bool wasUploadedToVulkan = false;
		void createAndUpload(const INTERNAL::vulkan::VulkanDevice &device, const INTERNAL::vulkan::VulkanCommandPool &commandPool);
		void changeLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImageLayout layout);
		void writeBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer);

		VkSampler getSampler() const;
		VkImageView getImageView() const;
		VkImageLayout getImageLayout() const;

	public:
		Image();
		Image(const char* path);
		Image(int width, int height);
		Image(int width, int height, const Color &c);

		~Image();

		void load(const char* path);
		void load(int width, int height);
		void load(int width, int height, const Color &c);

		void destroy();

		int getWidth() const;
		int getHeight() const;
		int getSizeInBytes() const;
		Color getPixel(int x, int y) const;
	};
}