#include "BBE/BrotBoxEngine.h" // NOLINT(misc-include-cleaner): examples/tests intentionally use the engine umbrella.
#include "gtest/gtest.h"
#include <limits>

TEST(Image, SetPixelR8DoesNotOverwriteNeighborPixels)
{
	const unsigned char pixels[2] = { 1, 2 };
	bbe::Image image(2, 1, pixels, bbe::ImageFormat::R8);

	image.setPixel(0, 0, bbe::Colori(9, 120, 200, 255));

	const bbe::Colori leftPixel = image.getPixel(0, 0);
	const bbe::Colori rightPixel = image.getPixel(1, 0);
	ASSERT_EQ(leftPixel.r, 9);
	ASSERT_EQ(leftPixel.g, 9);
	ASSERT_EQ(leftPixel.b, 9);
	ASSERT_EQ(leftPixel.a, 255);
	ASSERT_EQ(rightPixel.r, 2);
	ASSERT_EQ(rightPixel.g, 2);
	ASSERT_EQ(rightPixel.b, 2);
	ASSERT_EQ(rightPixel.a, 255);
}

TEST(Image, PixelAccessOutOfBoundsDies)
{
	const unsigned char pixel = 7;
	bbe::Image image(1, 1, &pixel, bbe::ImageFormat::R8);

	ASSERT_DEATH((void)image.getPixel(1, 0), ".*");
}

TEST(Image, OversizedFloatImageDiesBeforeAllocation)
{
	bbe::Image image;
	ASSERT_DEATH(image.load(std::numeric_limits<int>::max(), std::numeric_limits<int>::max(), nullptr, bbe::ImageFormat::R32G32B32A32FLOAT), ".*");
}
