#pragma once

#include "../BBE/ColorByte.h"
#include "../BBE/Color.h"
#include "../BBE/String.h"
#include "../BBE/Vector2.h"
#include "../BBE/ManuallyRefCountable.h"

typedef unsigned char stbi_uc;

namespace bbe
{
	class PrimitiveBrush2D;
	class PrimitiveBrush3D;
	class Terrain;
	namespace INTERNAL
	{
		namespace vulkan
		{
			class VulkanManager;
			class VulkanImage;
			class VulkanDescriptorSet;
		}
		namespace openGl
		{
			class OpenGLManager;
			class OpenGLImage;
		}
	}

	enum class ImageFormat
	{
		R8G8B8A8    =  37, //VK_FORMAT_R8G8B8A8_UNORM
		R8          =   9, //VK_FORMAT_R8_UNORM
		R32FLOAT    = 100, //VK_FORMAT_R32_SFLOAT
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
		friend class INTERNAL::vulkan::VulkanImage;
		friend class INTERNAL::vulkan::VulkanDescriptorSet;
		friend class INTERNAL::openGl::OpenGLImage;
		friend class INTERNAL::openGl::OpenGLManager;
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

		mutable bbe::ManuallyRefCountable* m_prendererData = nullptr;

		mutable const Image*   m_parentImage = nullptr;

		void finishLoad(stbi_uc* pixels);

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
		
		void loadRaw(const bbe::List<unsigned char>& rawData);
		void loadRaw(const unsigned char* rawData, size_t dataLength);
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
