#pragma once

#include "../BBE/Vector2.h"
#include "../BBE/Vector3.h"

namespace bbe
{
	template<typename Vec>
	class Shape
	{
	public:
		struct ProjectionResult
		{
			Vec start;
			Vec stop;
		};
	protected:
		static bool projectionsIntersect(const ProjectionResult& pr1, const ProjectionResult& pr2)
		{
			const float pr1StartLengthSq = pr1.start.getLengthSqSigned();
			const float pr2StartLengthSq = pr2.start.getLengthSqSigned();

			if (pr1StartLengthSq < pr2StartLengthSq)
			{
				const float pr1StopLengthSq = pr1.stop.getLengthSqSigned();
				return pr2StartLengthSq <= pr1StopLengthSq;
			}
			else if (pr2StartLengthSq < pr1StartLengthSq)
			{
				const float pr2StopLengthSq = pr2.stop.getLengthSqSigned();
				return pr1StartLengthSq <= pr2StopLengthSq;
			}
			else
			{
				return true;
			}

			return false;
		}
	public:
		virtual Vec getCenter() const = 0;

		virtual bbe::List<Vec> getVertices() const
		{
			bbe::List<Vec> retVal;
			getVertices(retVal);
			return retVal;
		}

		virtual void getVertices(bbe::List<Vec>& outVertices) const = 0;

		virtual bbe::List<Vec> getNormals() const = 0;

		virtual void translate(const Vec& translation) = 0;

		virtual ProjectionResult project(const Vec& projection) const
		{
			auto vertices = getVertices();
			auto projections = bbe::Math::project(vertices, projection);

			float minLengthSq = projections[0].getLengthSqSigned();
			float maxLengthSq = minLengthSq;
			ProjectionResult retVal{ projections[0], projections[0] };

			for (size_t i = 1; i < projections.getLength(); i++)
			{
				const float lengthSq = projections[i].getLengthSqSigned();
				if (lengthSq > maxLengthSq)
				{
					maxLengthSq = lengthSq;
					retVal.stop = projections[i];
				}
				if (lengthSq < minLengthSq)
				{
					minLengthSq = lengthSq;
					retVal.start = projections[i];
				}
			}

			return retVal;
		}

		virtual bool intersects(const Shape<Vec>& other) const
		{
			auto normalsThis = getNormals();
			auto normalsOther = other.getNormals();

			for (const Vec& normal : normalsThis)
			{
				auto p1 = project(normal);
				auto p2 = other.project(normal);
				if (!projectionsIntersect(p1, p2)) return false;
			}

			for (const Vec& normal : normalsOther)
			{
				auto p1 = project(normal);
				auto p2 = other.project(normal);
				if (!projectionsIntersect(p1, p2)) return false;
			}

			return true;
		}
	};

	class Shape2 : public Shape<bbe::Vector2>
	{
	public:
		virtual bbe::List<Vector2> getNormals() const override
		{
			auto vertices = getVertices();
			bbe::List<Vector2> retVal;
			retVal.resizeCapacityAndLength(vertices.getLength());

			for (size_t i = 0; i < vertices.getLength() - 1; i++)
			{
				retVal[i] = vertices[i + 1] - vertices[i];
			}

			retVal[retVal.getLength() - 1] = vertices[0] - vertices[vertices.getLength() - 1];

			for (size_t i = 0; i < retVal.getLength(); i++)
			{
				retVal[i] = retVal[i].rotate90Clockwise();
				retVal[i] = retVal[i].normalize();
			}

			return retVal;
		}
	};

	class Shape3 : public Shape<bbe::Vector3>
	{
	public:
		virtual bool intersects(const Shape<bbe::Vector3>& other) const override
		{
			if (!Shape<bbe::Vector3>::intersects(other)) return false;

			// See: https://gamedev.stackexchange.com/questions/44500/how-many-and-which-axes-to-use-for-3d-obb-collision-with-sat
			auto normalsThis = getNormals();
			auto normalsOther = other.getNormals();

			for (const bbe::Vector3& normalThis : normalsThis)
			{
				for (const bbe::Vector3& normalOther : normalsOther)
				{
					const bbe::Vector3 cross = normalThis.cross(normalOther);
					auto p1 = project(cross);
					auto p2 = other.project(cross);
					if (!projectionsIntersect(p1, p2)) return false;
				}
			}

			return true;
		}
	};

}