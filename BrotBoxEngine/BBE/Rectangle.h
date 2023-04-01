#pragma once
#include "../BBE/Vector2.h"
#include "../BBE/Shape2.h"
#include "../BBE/Circle.h"
#include "../BBE/List.h"

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

	template <typename Vec>
	class Rectangle_t : public bbe::Shape2<Vec>
	{
		friend class PrimitiveBrush2D;
		friend class ::bbe::INTERNAL::vulkan::VulkanManager;
	public:
		using SubType = typename Vec::SubType;
		SubType x;
		SubType y;
		SubType width;
		SubType height;
		
		Rectangle_t()
			: x(0), y(0), width(0), height(0)
		{
		}
		Rectangle_t(SubType x, SubType y, SubType width, SubType height)
			: x(x), y(y), width(width), height(height)
		{
		}
		Rectangle_t(const Vec &vec, SubType width, SubType height)
			: x(vec.x), y(vec.y), width(width), height(height)
		{
		}
		Rectangle_t(SubType x, SubType y, const Vec &dim)
			: x(x), y(y), width(dim.x), height(dim.y)
		{
		}
		Rectangle_t(const Vec &vec, const Vec &dim)
			: x(vec.x), y(vec.y), width(dim.x), height(dim.y)
		{
		}

		Vec getPos() const
		{
			return Vec(x, y);
		}
		Vec getDim() const
		{
			return Vec(width, height);
		}

		Rectangle_t<Vec> combine(const Rectangle_t<Vec>& other) const
		{
			const float left = bbe::Math::min(this->getLeft(), other.getLeft());
			const float right = bbe::Math::max(this->getRight(), other.getRight());
			const float top = bbe::Math::min(this->getTop(), other.getTop());
			const float bottom = bbe::Math::max(this->getBottom(), other.getBottom());
			return Rectangle_t<Vec>(
				left,
				top,
				right - left,
				bottom - top
			);
		}
		Rectangle_t<Vec> offset(const Vec& off) const
		{
			return Rectangle_t<Vec>(x + off.x, y + off.y, width, height);
		}

		SubType getLeft() const
		{
			return bbe::Math::min(x, x + width);
		}
		SubType getRight() const
		{
			return bbe::Math::max(x, x + width);
		}
		SubType getTop() const
		{
			return bbe::Math::min(y, y + height);
		}
		SubType getBottom() const
		{
			return bbe::Math::max(y, y + height);
		}

		SubType getArea() const
		{
			return width * height;
		}
		virtual Vec getCenter() const override
		{
			return Vec(
				x + width / 2,
				y + height / 2
			);
		}
		virtual void getVertices(bbe::List<Vec>& outVertices) const override
		{
			outVertices.clear();

			outVertices.add({ x,         y });
			outVertices.add({ x,         y + height });
			outVertices.add({ x + width, y + height });
			outVertices.add({ x + width, y });
		}

		void setPos(SubType x, SubType y)
		{
			this->x = x;
			this->y = y;
		}
		void setPos(const Vec &vec)
		{
			x = vec.x;
			y = vec.y;
		}
		void setDim(SubType width, SubType height)
		{
			this->width = width;
			this->height = height;
		}
		void setDim(const Vec &vec)
		{
			width = vec.x;
			height = vec.y;
		}
		void set(SubType x, SubType y, SubType width, SubType height)
		{
			this->x = x;
			this->y = y;
			this->width = width;
			this->height = height;
		}
		void shrinkInPlace(SubType val)
		{
			x += val;
			y += val;
			width -= val * 2;
			height -= val * 2;
		}
		Rectangle_t<Vec> shrinked(SubType val) const
		{
			return Rectangle_t<Vec>(
				x + val,
				y + val,
				width - val * 2,
				height - val * 2
			);
		}
		Rectangle_t<Vec> stretchedSpace(SubType x, SubType y) const
		{
			return Rectangle_t<Vec>(
				this->x * x,
				this->y * y,
				this->width * x,
				this->height * y
			);
		}


		void translate(SubType x, SubType y)
		{
			this->x += x;
			this->y += y;
		}
		virtual void translate(const Vec &vec) override
		{
			translate(vec.x, vec.y);
		}

		SubType getDistanceTo(const Vec &vec)
		{
			//UNTESTED
			if (vec.x < x)
			{
				if (vec.y < y)
				{
					return vec.getDistanceTo(Vec(x, y));
				}
				else if (vec.y > y + height)
				{
					return vec.getDistanceTo(Vec(x, y + height));
				}
				else
				{
					return x - vec.x;
				}
			}
			else if (vec.x > x + width)
			{
				if (vec.y < y)
				{
					return vec.getDistanceTo(Vec(x + width, y));
				}
				else if (vec.y > y + height)
				{
					return vec.getDistanceTo(Vec(x + width, y + height));
				}
				else
				{
					return vec.x - (x + width);
				}
			}
			else if (vec.y < y)
			{
				return y - vec.y;
			}
			else if (vec.y > y + height)
			{
				return vec.y - (y + height);
			}
			else
			{
				return 0;
			}
		}

		bool isPointInRectangle(const Vec point, bool includeBoundary = false) const
		{
			if (includeBoundary)
			{
				return point.x >= x && point.x < x + width
					&& point.y >= y && point.y < y + height;
			}
			else
			{
				// TODO: This is wrong - right?
				return point.x > x && point.x < x + width
					&& point.y > y && point.y < y + height;
			}
		}
		using Shape2<Vec>::intersects;
		bool intersects(const Rectangle_t<Vec>& rectangle) const
		{
			const Rectangle_t<Vec> hitZone(
				rectangle.x - this->width,
				rectangle.y - this->height,
				this->width + rectangle.width,
				this->height + rectangle.height
			);
			return hitZone.isPointInRectangle(this->getPos());
		}
		bool intersects(const Circle& circle) const
		{
			if (circle.getWidth() != circle.getHeight()) throw NotImplementedException();

			const Vec circleMidPoint = circle.getPos() - circle.getDim() / 2;

			if (circleMidPoint.x >= x - circle.getWidth() / 2
				&& circleMidPoint.x <= x + width + circle.getWidth() / 2
				&& circleMidPoint.y >= y
				&& circleMidPoint.y <= y + height)
				return true;

			if (circleMidPoint.x >= x
				&& circleMidPoint.x <= x + width
				&& circleMidPoint.y >= y - circle.getHeight() / 2
				&& circleMidPoint.y <= y + height + circle.getHeight() / 2)
				return true;

			if (circleMidPoint.getDistanceTo(x, y) < circle.getWidth() / 2) return true;
			if (circleMidPoint.getDistanceTo(x + width, y) < circle.getWidth() / 2) return true;
			if (circleMidPoint.getDistanceTo(x, y + height) < circle.getWidth() / 2) return true;
			if (circleMidPoint.getDistanceTo(x + width, y + height) < circle.getWidth() / 2) return true;

			return false;
		}

		static Vec pack(bbe::List<Rectangle_t<Vec>>& list)
		{
			// TODO pretty naive packing that only puts all the rectangles into one line, while stacking them if possible. Improve!
			if (list.getLength() == 0) return Vec(0, 0);
			bbe::List<Rectangle_t<Vec>*> ptrs;
			ptrs.resizeCapacityAndLengthUninit(list.getLength());
			for (size_t i = 0; i < list.getLength(); i++)
			{
				ptrs[i] = &(list[i]);
			}
			ptrs.sort([](Rectangle_t<Vec>* const& a, const Rectangle_t<Vec>* const& b) {
				return a->height > b->height;
				});
			const SubType height = ptrs[0]->height;
			SubType currentX = 0;
			SubType currentY = 0;
			SubType biggestWidthInCurrentColumn = 0;
			for (size_t i = 0; i < ptrs.getLength(); i++)
			{
				if (currentY + ptrs[i]->height > height)
				{
					currentX += biggestWidthInCurrentColumn;
					currentY = 0;
					biggestWidthInCurrentColumn = 0;
				}
				ptrs[i]->x = currentX;
				ptrs[i]->y = currentY;
				currentY += ptrs[i]->height;
				if (biggestWidthInCurrentColumn < ptrs[i]->width) biggestWidthInCurrentColumn = ptrs[i]->width;
			}
			return Vec(currentX + biggestWidthInCurrentColumn, height);
		}
	};

	using Rectangle  = Rectangle_t<bbe::Vector2>;
	using Rectanglei = Rectangle_t<bbe::Vector2i>;
}
