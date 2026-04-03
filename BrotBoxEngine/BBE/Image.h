#pragma once

#include "../BBE/ColorByte.h"
#include "../BBE/Color.h"
#include "../BBE/Rectangle.h"
#include "../BBE/String.h"
#include "../BBE/Vector2.h"
#include "../BBE/List.h"
#include "../BBE/AutoRefCountable.h"

#ifdef _WIN32
#ifndef _WINDEF_
// Forward declare it to avoid including the whole windows header everywhere.
class HICON__;
typedef HICON__ *HICON;
class HBITMAP__;
typedef HBITMAP__ *HBITMAP;
#endif
#endif

typedef unsigned char stbi_uc;

namespace bbe
{
	class Font;
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
		R8G8B8A8 = 37,			 //VK_FORMAT_R8G8B8A8_UNORM
		R8 = 9,					 //VK_FORMAT_R8_UNORM
		R32FLOAT = 100,			 //VK_FORMAT_R32_SFLOAT
		R32G32B32A32FLOAT = 109, //VK_FORMAT_R32G32B32A32_SFLOAT
	};

	enum class ImageRepeatMode
	{
		REPEAT = 0,				  //VK_SAMPLER_ADDRESS_MODE_REPEAT
		MIRRORED_REPEAT = 1,	  //VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT
		CLAMP_TO_EDGE = 2,		  //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
		CLAMP_TO_BORDER = 3,	  //VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
		MIRROR_CLAMP_TO_EDGE = 4, //VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
	};

	enum class ImageFilterMode
	{
		LINEAR = 1,	 //VK_FILTER_LINEAR
		NEAREST = 0, //VK_FILTER_NEAREST
	};

	enum class ImageBrushShape
	{
		Circle = 0,
		Square = 1,
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
		int m_width = 0;
		int m_height = 0;
		ImageFormat m_format = ImageFormat::R8G8B8A8;
		ImageRepeatMode m_repeatMode = ImageRepeatMode::REPEAT;
		ImageFilterMode m_filterMode = ImageFilterMode::LINEAR;
		bool keep = false;

		mutable bbe::AutoRef m_prendererData;

		bool finishLoad(stbi_uc *pixels);

	public:
		static const Image &white();
		static const Image &black();

		Image();
		explicit Image(const char *path);
		explicit Image(const bbe::String &path);
		Image(int width, int height);
		Image(int width, int height, const Color &c);
		Image(int width, int height, const void *data, ImageFormat format);

		Image(const Image &other) = default;				//Copy Constructor
		Image(Image &&other) noexcept = default;			//Move Constructor
		Image &operator=(const Image &other) = default;		//Copy Assignment
		Image &operator=(Image &&other) noexcept = default; //Move Assignment

		bool loadRaw(const bbe::ByteBuffer &buffer);
		bool loadRaw(const bbe::List<unsigned char> &rawData);
		bool loadRaw(const unsigned char *rawData, size_t dataLength);
		void load(const char *path);
		void load(const bbe::String &path);
		void load(int width, int height);
		void load(int width, int height, const Color &c);
		void load(int width, int height, const void *data, ImageFormat format);

		int getWidth() const;
		int getHeight() const;
		Vector2i getDimensions() const;
		size_t getSizeInBytes() const;
		size_t getAmountOfChannels() const;
		size_t getBytesPerChannel() const;
		size_t getBytesPerPixel() const;
		Colori getPixel(const bbe::Vector2i &pos) const;
		Colori getPixel(size_t x, size_t y) const;
		// Bilinear sample in pixel coordinates using premultiplied-alpha interpolation.
		// Out-of-bounds samples are treated as fully transparent.
		Colori sampleBilinearPremultiplied(float x, float y) const;
		void setPixel(const bbe::Vector2i &pos, const bbe::Colori &c);
		void setPixel(size_t x, size_t y, const Colori &c);
		size_t getIndexForRawAccess(size_t x, size_t y) const;

		int64_t distance(const Image &other) const;

		ImageRepeatMode getRepeatMode() const;
		void setRepeatMode(ImageRepeatMode irm);

		ImageFilterMode getFilterMode() const;
		void setFilterMode(ImageFilterMode ifm);

		bool isLoadedCpu() const;
		bool isLoadedGpu() const;
		void keepAfterUpload();

		void floodFill(const bbe::Vector2i &pos, const bbe::Colori &to, bool fillDiagonal = false, bool tiled = false);

		// CPU-side alpha-over blit of src onto this image at dstPos (top-left).
		// If tiled is true, the destination wraps modulo this image's dimensions.
		void blendOver(const bbe::Image &src, const bbe::Vector2i &dstPos, bool tiled = false);

		// CPU-side blend of src over this image (same dimensions) using BlendMode and opacity.
		// This is useful for generic image compositing; caller decides what "layers" mean.
		void blend(const bbe::Image &src, float opacity = 1.0f, bbe::BlendMode mode = bbe::BlendMode::Normal);

		// CPU-side alpha-over blit of src rotated around the center of dstRect.
		// Rotation is in radians. Bilinear sampling when antiAlias is true; nearest-neighbor when false (crisp pixels).
		void blendOverRotated(const bbe::Image &src, const bbe::Rectanglei &dstRect, float rotation, bool tiled = false, bool antiAlias = true);

		// Returns a new image rotated around its center (CPU).
		// The returned image is sized to the rotated bounding box.
		// Rotation is in radians. Bilinear when antiAlias is true, nearest-neighbor when false.
		bbe::Image rotatedToFit(float rotation, bool antiAlias = true) const;

		// Returns a new image scaled using nearest-neighbor sampling (CPU).
		// If width/height match, returns a copy.
		bbe::Image scaledNearest(int32_t width, int32_t height) const;

		// Returns a new image with a different canvas size (CPU) while preserving existing pixels.
		// dstPosOfOldOrigin is the destination position of the old pixel (0,0) inside the new image.
		// Pixels outside the old bounds are filled with fillColor.
		bbe::Image resizedCanvas(int32_t newWidth, int32_t newHeight, const bbe::Vector2i &dstPosOfOldOrigin, const bbe::Color &fillColor = bbe::Color(0.f, 0.f, 0.f, 0.f)) const;

		// Draws a brush stamp into this image (CPU) at pos in pixel coordinates.
		// brushRadius is the radius in pixels (>= 0). For Square shape, the distance metric is Chebyshev.
		// For antiAlias=false, the stamp snaps to pixel centers to avoid boundary bleed.
		bool drawBrushStamp(const bbe::Vector2 &pos, const bbe::Colori &color, int32_t brushRadius, ImageBrushShape shape = ImageBrushShape::Circle, bool tiled = false, bool antiAlias = true);

		// Draws a thick line (capsule SDF AA) in this image (CPU).
		// Falls back to stamp-based drawing for tiled, square, or antiAlias=false to preserve crisp pixel results.
		bool drawLineCapsule(const bbe::Vector2 &from, const bbe::Vector2 &to, const bbe::Colori &color, int32_t brushRadius, ImageBrushShape shape = ImageBrushShape::Circle, bool tiled = false, bool antiAlias = true);

		// Fills a triangle in this image (CPU), optionally anti-aliased.
		void fillTriangle(const bbe::Vector2 &v0, const bbe::Vector2 &v1, const bbe::Vector2 &v2, const bbe::Colori &color, bool tiled = false, bool antiAlias = true);

		// Returns an RGBA image of a stroked rounded rectangle (CPU).
		// strokeWidth is in pixels; values <= 0 produce a filled rounded rectangle.
		static Image strokedRoundedRect(int32_t width, int32_t height, const bbe::Colori &color, int32_t strokeWidth, int32_t cornerRadius, float rotation = 0.f, bool antiAlias = true);

		// Returns an RGBA image of a stroked ellipse (ring). If strokeWidth <= 0, returns a filled ellipse.
		static Image strokedEllipse(int32_t width, int32_t height, const bbe::Colori &color, int32_t strokeWidth, float rotation = 0.f, bool antiAlias = true);

		// Renders text into a standalone RGBA image (straight-alpha coverage for bilinear-safe rotation).
		// antiAlias=false thresholds stb_truetype grayscale to binary edges (matches other CPU AA-off paths).
		static Image renderTextToImage(const Font &font, const bbe::String &text, const bbe::Vector2i &topLeft, const bbe::Colori &color, bool antiAlias = true);

		// Draws an arrow into this image (CPU). strokeRadius matches drawLineCapsule brushRadius.
		void drawArrow(const bbe::Vector2 &from,
		               const bbe::Vector2 &to,
		               const bbe::Colori &color,
		               int32_t strokeRadius,
		               int32_t headSize,
		               int32_t headWidth,
		               bool doubleHeaded = false,
		               bool filledHead = true,
		               bool tiled = false,
		               bool antiAlias = true);

		// Draws a Bezier curve by sampling Math::interpolateBezier and drawing thick segments.
		// points contains the full control polygon including start and end points.
		void drawBezier(const bbe::List<bbe::Vector2> &points,
		                const bbe::Colori &color,
		                int32_t strokeRadius,
		                bool tiled = false,
		                bool antiAlias = true,
		                int32_t minSamples = 200);

		// CPU text: blends font glyphs onto this image (R8 coverage → alpha-over). Optionally tiled.
		void blendText(const Font &font, const bbe::String &text, const bbe::Vector2i &topLeft, const bbe::Colori &color, bool tiled = false, bool antiAlias = true);

		void writeToFile(const bbe::String &path) const;
		void writeToFile(const char *path) const;

		void mirrorHorizontally();
		void mirrorVertically();

		// Exact 90° rotations (CPU). Dimensions become height×width; y-down, x-right screen space.
		[[nodiscard]] Image rotated90Clockwise() const;
		[[nodiscard]] Image rotated90CounterClockwise() const;

		static bool supportsClipboardImages();
		static bool isImageInClipbaord();
		static bbe::Image getClipboardImage();
		void copyToClipboard() const;

#ifdef _WIN32
		HBITMAP toBitmap() const;
		HICON toIcon() const;
		static Image screenshot(int x, int y, int width, int height);
#endif
	};
}
