#include "BBE/PosNormalPair.h"

void bbe::PosNormalPair::transform(bbe::List<PosNormalPair>& vertices, const bbe::Matrix4& matrix)
{
	bbe::Matrix4 normalTransform = matrix.toNormalTransform();
	for (PosNormalPair& p : vertices)
	{
		p.pos = matrix * p.pos;
		p.normal = normalTransform * p.normal;
		// TODO: Probably the UV-Coordinates should also be transformed somehow?
	}
}
