#pragma once

// TODO: Make independent of RenderMode
#ifdef BBE_RENDERER_VULKAN
#include "GLFW/glfw3.h"
#include "../BBE/Vulkan/VulkanDescriptorSet.h"
#include "../BBE/Vulkan/VulkanHelper.h"
#include "../BBE/ColorByte.h"
#include "../BBE/Color.h"
#include "../BBE/String.h"
#include "../BBE/Vector2.h"

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

	class PrimitiveBrush2D;
	class PrimitiveBrush3D;
	class Terrain;

	enum class ImageFormat
	{
		R8G8B8A8    =  37, //VK_FORMAT_R8G8B8A8_UNORM
		R8          =   9, //VK_FORMAT_R8_UNORM
		R32FLOAT    = 100, //VK_FORMAT_R32_SFLOAT
		R32FLOATBC6 = 143, //VK_FORMAT_BC6H_UFLOAT_BLOCK
	};

	enum class ImageRepeatMode
	{
		REPEAT               = 0, //VK_SAMPLER_ADDRESS_MODE_REPEAT
		MIRRORED_REPEAT      = 1, //VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT
		CLAMP_TO_EDGE        = 2, //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
		CLAMP_TO_BORDER      = 3, //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
		MIRROR_CLAMP_TO_EDGE = 4, //VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
	};

	enum class ImageFilterMode
	{
		LINEAR  = 1, //VK_FILTER_LINEAR
		NEAREST = 0, //VK_FILTER_NEAREST
	};

	class Image
	{
		friend class INTERNAL::vulkan::VulkanManager;
		friend class INTERNAL::vulkan::VulkanDescriptorSet;
		friend class PrimitiveBrush2D;
		friend class PrimitiveBrush3D;
		friend class Terrain;
	private:
		byte           *m_pdata  = nullptr;
		int             m_width  = 0;
		int             m_height = 0;
		ImageFormat     m_format = ImageFormat::R8G8B8A8;
		ImageRepeatMode m_repeatMode = ImageRepeatMode::REPEAT;
		ImageFilterMode m_filterMode = ImageFilterMode::LINEAR;

		struct VulkanData
		{
			int32_t        m_refCount    = 0;
			VkImage        m_image       = VK_NULL_HANDLE;
			VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
			VkImageView    m_imageView   = VK_NULL_HANDLE;
			std::unique_ptr<VkImageLayout[]> m_imageLayout = nullptr;
			VkDevice       m_device      = VK_NULL_HANDLE;
			VkSampler      m_sampler     = VK_NULL_HANDLE;
			INTERNAL::vulkan::VulkanDescriptorSet m_descriptorrSet;

			VulkanData();
			~VulkanData();

			VulkanData(const VulkanData&) = delete;
			VulkanData(VulkanData&&) = delete;
			VulkanData& operator =(const VulkanData&) = delete;
			VulkanData&& operator ==(const VulkanData&&) = delete;

			void incRef();
			void decRef();
		};
		mutable VulkanData* m_pVulkanData = nullptr;

		
		mutable const Image*   m_parentImage = nullptr;

		void createAndUpload(const INTERNAL::vulkan::VulkanDevice &device, const INTERNAL::vulkan::VulkanCommandPool &commandPool, const INTERNAL::vulkan::VulkanDescriptorPool &descriptorPool, const INTERNAL::vulkan::VulkanDescriptorSetLayout &setLayout, const Image* parentImage = nullptr) const;
		void changeLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImageLayout layout, uint32_t baseMipLevel = 0, uint32_t levelCount = 1) const;
		void writeBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer) const;

		VkSampler getSampler() const;
		VkImageView getImageView() const;
		VkImageLayout getImageLayout() const;

		INTERNAL::vulkan::VulkanDescriptorSet& getDescriptorSet() const;

	public:
		Image();
		explicit Image(const char* path);
		explicit Image(const bbe::String& path);
		Image(int width, int height);
		Image(int width, int height, const Color &c);
		Image(int width, int height, const byte* data, ImageFormat format);
		
		Image(const Image& other); //Copy Constructor
		Image(Image&& other); //Move Constructor
		Image& operator=(const Image& other) = delete; //Copy Assignment
		Image& operator=(Image&& other); //Move Assignment

		~Image();
		
		void load(const char* path);
		void load(const bbe::String& path);
		void load(int width, int height);
		void load(int width, int height, const Color &c);
		void load(int width, int height, const byte* data, ImageFormat format);

		void destroy();

		int getWidth() const;
		int getHeight() const;
		Vector2 getDimensions() const;
		int getSizeInBytes() const;
		size_t getAmountOfChannels() const;
		int getBytesPerChannel() const;
		Color getPixel(size_t x, size_t y) const;
		size_t getIndexForRawAccess(size_t x, size_t y) const;

		ImageRepeatMode getRepeatMode() const;
		void setRepeatMode(ImageRepeatMode irm);

		ImageFilterMode getFilterMode() const;
		void setFilterMode(ImageFilterMode ifm);

		bool isLoaded() const;
	};
}
#endif
