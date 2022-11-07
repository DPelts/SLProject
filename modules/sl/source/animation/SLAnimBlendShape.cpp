#include "SLAnimBlendShape.h"

SLAnimBlendShape::SLAnimBlendShape()
{
}

SLAnimBlendShape::SLAnimBlendShape(SLint size)
{
    _vertices.resize(size);
}

SLAnimBlendShape::SLAnimBlendShape(vector<SLVec3f> vertices) : _vertices(vertices) {}

SLAnimBlendShape::~SLAnimBlendShape()
{
}

void SLAnimBlendShape::createBlendShape(SLint size)
{
    _vertices.resize(size);
}

void SLAnimBlendShape::addVertex(SLVec3f vertex)
{
    _vertices.push_back(vertex);
}

void SLAnimBlendShape::addName(SLstring name)
{
    _name = name;
}
