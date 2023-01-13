#pragma once
#include "../BBE/Vector2.h"
#include "../BBE/Shape2.h"

namespace bbe
{
	class VulkanDevice;
	class Circle;

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
	private:
		using SubType = typename Vec::SubType;
		SubType m_x;
		SubType m_y;
		SubType m_width;
		SubType m_height;

	public:
		Rectangle_t()
			: m_x(0), m_y(0), m_width(0), m_height(0)
		{
		}
		Rectangle_t(SubType x, SubType y, SubType width, SubType height)
			: m_x(x), m_y(y), m_width(width), m_height(height)
		{
		}
		Rectangle_t(const Vec &vec, SubType width, SubType height)
			: m_x(vec.x), m_y(vec.y), m_width(width), m_height(height)
		{
		}
		Rectangle_t(SubType x, SubType y, const Vec &dim)
			: m_x(x), m_y(y), m_width(dim.x), m_height(dim.y)
		{
		}
		Rectangle_t(const Vec &vec, const Vec &dim)
			: m_x(vec.x), m_y(vec.y), m_width(dim.x), m_height(dim.y)
		{
		}

		Vec getPos() const
		{
			return Vec(m_x, m_y);
		}
		Vec getDim() const
		{
			return Vec(m_width, m_height);
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
			return Rectangle_t<Vec>(m_x + off.x, m_y + off.y, m_width, m_height);
		}

		SubType getLeft() const
		{
			return bbe::Math::min(m_x, m_x + m_width);
		}
		SubType getRight() const
		{
			return bbe::Math::max(m_x, m_x + m_width);
		}
		SubType getTop() const
		{
			return bbe::Math::min(m_y, m_y + m_height);
		}
		SubType getBottom() const
		{
			return bbe::Math::max(m_y, m_y + m_height);
		}

		SubType getX() const
		{
			return m_x;
		}
		SubType getY() const
		{
			return m_y;
		}
		SubType getWidth() const
		{
			return m_width;
		}
		SubType getHeight() const
		{
			return m_height;
		}
		SubType getArea() const
		{
			return getWidth() * getHeight();
		}
		virtual Vec getCenter() const override
		{
			return Vec(
				getX() + getWidth() / 2,
				getY() + getHeight() / 2
			);
		}
		virtual void getVertices(bbe::List<Vec>& outVertices) const override
		{
			outVertices.clear();

			outVertices.add({ m_x,           m_y });
			outVertices.add({ m_x,           m_y + m_height });
			outVertices.add({ m_x + m_width, m_y + m_height });
			outVertices.add({ m_x + m_width, m_y });
		}

		void setX(SubType x)
		{
			m_x = x;
		}
		void setY(SubType y)
		{
			m_y = y;
		}
		void setPos(SubType x, SubType y)
		{
			m_x = x;
			m_y = y;
		}
		void setPos(const Vec &vec)
		{
			m_x = vec.x;
			m_y = vec.y;
		}
		void setWidth(SubType width)
		{
			m_width = width;
		}
		void setHeight(SubType height)
		{
			m_height = height;
		}
		void setDim(SubType width, SubType height)
		{
			m_width = width;
			m_height = height;
		}
		void setDim(const Vec &vec)
		{
			m_width = vec.x;
			m_height = vec.y;
		}
		void set(SubType x, SubType y, SubType width, SubType height)
		{
			setX(x);
			setY(y);
			setWidth(width);
			setHeight(height);
		}
		void shrinkInPlace(SubType val)
		{
			m_x += val;
			m_y += val;
			m_width -= val * 2;
			m_height -= val * 2;
		}
		Rectangle_t<Vec> shrinked(SubType val) const
		{
			return Rectangle_t<Vec>(
				m_x + val,
				m_y + val,
				m_width - val * 2,
				m_height - val * 2
			);
		}
		Rectangle_t<Vec> stretchedSpace(SubType x, SubType y) const
		{
			return Rectangle_t<Vec>(
				m_x * x,
				m_y * y,
				m_width * x,
				m_height * y
			);
		}


		void translate(SubType x, SubType y)
		{
			m_x += x;
			m_y += y;
		}
		virtual void translate(const Vec &vec) override
		{
			translate(vec.x, vec.y);
		}

		SubType getDistanceTo(const Vec &vec)
		{
			//UNTESTED
			if (vec.x < m_x)
			{
				if (vec.y < m_y)
				{
					return vec.getDistanceTo(Vec(m_x, m_y));
				}
				else if (vec.y > m_y + m_height)
				{
					return vec.getDistanceTo(Vec(m_x, m_y + m_height));
				}
				else
				{
					return m_x - vec.x;
				}
			}
			else if (vec.x > m_x + m_width)
			{
				if (vec.y < m_y)
				{
					return vec.getDistanceTo(Vec(m_x + m_width, m_y));
				}
				else if (vec.y > m_y + m_height)
				{
					return vec.getDistanceTo(Vec(m_x + m_width, m_y + m_height));
				}
				else
				{
					return vec.x - (m_x + m_width);
				}
			}
			else if (vec.y < m_y)
			{
				return m_y - vec.y;
			}
			else if (vec.y > m_y + m_height)
			{
				return vec.y - (m_y + m_height);
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
				return point.x >= this->getX() && point.x < this->getX() + this->getWidth()
					&& point.y >= this->getY() && point.y < this->getY() + this->getHeight();
			}
			else
			{
				// TODO: This is wrong - right?
				return point.x > this->getX() && point.x < this->getX() + this->getWidth()
					&& point.y > this->getY() && point.y < this->getY() + this->getHeight();
			}
		}
		using Shape2<Vec>::intersects;
		bool intersects(const Rectangle_t<Vec>& rectangle) const
		{
			const Rectangle_t<Vec> hitZone(
				rectangle.getX() - this->getWidth(),
				rectangle.getY() - this->getHeight(),
				this->getWidth() + rectangle.getWidth(),
				this->getHeight() + rectangle.getHeight()
			);
			return hitZone.isPointInRectangle(this->getPos());
		}
		bool intersects(const Circle& circle) const
		{
			if (circle.getWidth() != circle.getHeight()) throw NotImplementedException();

			const Vec circleMidPoint = circle.getPos() - circle.getDim() / 2;

			if (circleMidPoint.x >= m_x - circle.getWidth() / 2
				&& circleMidPoint.x <= m_x + m_width + circle.getWidth() / 2
				&& circleMidPoint.y >= m_y
				&& circleMidPoint.y <= m_y + m_height)
				return true;

			if (circleMidPoint.x >= m_x
				&& circleMidPoint.x <= m_x + m_width
				&& circleMidPoint.y >= m_y - circle.getHeight() / 2
				&& circleMidPoint.y <= m_y + m_height + circle.getHeight() / 2)
				return true;

			if (circleMidPoint.getDistanceTo(m_x, m_y) < circle.getWidth() / 2) return true;
			if (circleMidPoint.getDistanceTo(m_x + m_width, m_y) < circle.getWidth() / 2) return true;
			if (circleMidPoint.getDistanceTo(m_x, m_y + m_height) < circle.getWidth() / 2) return true;
			if (circleMidPoint.getDistanceTo(m_x + m_width, m_y + m_height) < circle.getWidth() / 2) return true;

			return false;
		}
	};

	using Rectangle  = Rectangle_t<bbe::Vector2>;
	using Rectanglei = Rectangle_t<bbe::Vector2i>;
}
