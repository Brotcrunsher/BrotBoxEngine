#pragma once

#include "../BBE/ColorByte.h"
#include "../BBE/Color.h"
#include "../BBE/String.h"
#include "../BBE/Vector2.h"
#include "../BBE/AutoRefCountable.h"

typedef unsigned char stbi_uc;

namespace bbe
{
	class PrimitiveBrush2D;
	class PrimitiveBrush3D;
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
			struct OpenGLImage;
		}
	}

	enum class ImageFormat
	{
		R8G8B8A8          =  37, //VK_FORMAT_R8G8B8A8_UNORM
		R8                =   9, //VK_FORMAT_R8_UNORM
		R32FLOAT          = 100, //VK_FORMAT_R32_SFLOAT
		R32G32B32A32FLOAT = 109, //VK_FORMAT_R32G32B32A32_SFLOAT
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
		friend struct INTERNAL::openGl::OpenGLImage;
		friend class INTERNAL::openGl::OpenGLManager;
		friend class PrimitiveBrush2D;
		friend class PrimitiveBrush3D;
	private:
		mutable bbe::List<byte> m_pdata;
		int             m_width  = 0;
		int             m_height = 0;
		ImageFormat     m_format = ImageFormat::R8G8B8A8;
		ImageRepeatMode m_repeatMode = ImageRepeatMode::REPEAT;
		ImageFilterMode m_filterMode = ImageFilterMode::LINEAR;
		bool keep = false;

		mutable bbe::AutoRef m_prendererData;

		void finishLoad(stbi_uc* pixels);

	public:
		static const Image& white();
		static const Image& black();
		
		Image();
		explicit Image(const char* path);
		explicit Image(const bbe::String& path);
		Image(int width, int height);
		Image(int width, int height, const Color &c);
		Image(int width, int height, const void* data, ImageFormat format);
		
		Image(const Image& other) = default; //Copy Constructor
		Image(Image&& other) noexcept = default; //Move Constructor
		Image& operator=(const Image& other) = default; //Copy Assignment
		Image& operator=(Image&& other) noexcept = default; //Move Assignment
		
		void loadRaw(const bbe::ByteBuffer& buffer);
		void loadRaw(const bbe::List<unsigned char>& rawData);
		void loadRaw(const unsigned char* rawData, size_t dataLength);
		void load(const char* path);
		void load(const bbe::String& path);
		void load(int width, int height);
		void load(int width, int height, const Color &c);
		void load(int width, int height, const void* data, ImageFormat format);

		int getWidth() const;
		int getHeight() const;
		Vector2i getDimensions() const;
		size_t getSizeInBytes() const;
		size_t getAmountOfChannels() const;
		size_t getBytesPerChannel() const;
		Color getPixel(size_t x, size_t y) const;
		size_t getIndexForRawAccess(size_t x, size_t y) const;

		ImageRepeatMode getRepeatMode() const;
		void setRepeatMode(ImageRepeatMode irm);

		ImageFilterMode getFilterMode() const;
		void setFilterMode(ImageFilterMode ifm);

		bool isLoadedCpu() const;
		bool isLoadedGpu() const;
		void keepAfterUpload();
	};
}
