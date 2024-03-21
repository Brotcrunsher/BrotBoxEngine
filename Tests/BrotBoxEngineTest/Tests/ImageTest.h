#pragma once

#include "BBE/Image.h"
#include "BBE/UtilTest.h"
#include "BBE/UtilDebug.h"

namespace bbe
{
	namespace test
	{
		void testImage()
		{
			{
				Image image;
				image.load(BBE_APPLICATION_ASSET_PATH "/TestImage.png");

				Colori c = image.getPixel(0, 0);
				assertEquals(c.r, 0);
				assertEquals(c.g, 0);
				assertEquals(c.b, 0);
				assertEquals(c.a, 1);

				c = image.getPixel(1, 0);
				assertEquals(c.r, 1);
				assertEquals(c.g, 0);
				assertEquals(c.b, 0);
				assertEquals(c.a, 1);

				c = image.getPixel(0, 1);
				assertEquals(c.r, 0);
				assertEquals(c.g, 1);
				assertEquals(c.b, 0);
				assertEquals(c.a, 1);

				c = image.getPixel(1, 1);
				assertEquals(c.r, 0);
				assertEquals(c.g, 0);
				assertEquals(c.b, 1);
				assertEquals(c.a, 1);





				c = image.getPixel(image.getWidth() - 1, 0);
				assertEquals(c.r, 0);
				assertEquals(c.g, 0);
				assertEquals(c.b, 0);
				assertEquals(c.a, 1);

				c = image.getPixel(image.getWidth() - 2, 0);
				assertEquals(c.r, 1);
				assertEquals(c.g, 0);
				assertEquals(c.b, 0);
				assertEquals(c.a, 1);

				c = image.getPixel(image.getWidth() - 1, 1);
				assertEquals(c.r, 0);
				assertEquals(c.g, 1);
				assertEquals(c.b, 0);
				assertEquals(c.a, 1);

				c = image.getPixel(image.getWidth() - 2, 1);
				assertEquals(c.r, 0);
				assertEquals(c.g, 0);
				assertEquals(c.b, 1);
				assertEquals(c.a, 1);





				c = image.getPixel(image.getWidth() - 1, image.getWidth() - 1);
				assertEquals(c.r, 0);
				assertEquals(c.g, 0);
				assertEquals(c.b, 0);
				assertEquals(c.a, 1);

				c = image.getPixel(image.getWidth() - 2, image.getWidth() - 1);
				assertEquals(c.r, 1);
				assertEquals(c.g, 0);
				assertEquals(c.b, 0);
				assertEquals(c.a, 1);

				c = image.getPixel(image.getWidth() - 1, image.getWidth() - 2);
				assertEquals(c.r, 0);
				assertEquals(c.g, 1);
				assertEquals(c.b, 0);
				assertEquals(c.a, 1);

				c = image.getPixel(image.getWidth() - 2, image.getWidth() - 2);
				assertEquals(c.r, 0);
				assertEquals(c.g, 0);
				assertEquals(c.b, 1);
				assertEquals(c.a, 1);





				c = image.getPixel(0, image.getWidth() - 1);
				assertEquals(c.r, 0);
				assertEquals(c.g, 0);
				assertEquals(c.b, 0);
				assertEquals(c.a, 1);

				c = image.getPixel(1, image.getWidth() - 1);
				assertEquals(c.r, 1);
				assertEquals(c.g, 0);
				assertEquals(c.b, 0);
				assertEquals(c.a, 1);

				c = image.getPixel(0, image.getWidth() - 2);
				assertEquals(c.r, 0);
				assertEquals(c.g, 1);
				assertEquals(c.b, 0);
				assertEquals(c.a, 1);

				c = image.getPixel(1, image.getWidth() - 2);
				assertEquals(c.r, 0);
				assertEquals(c.g, 0);
				assertEquals(c.b, 1);
				assertEquals(c.a, 1);
			}
		}
	}
}