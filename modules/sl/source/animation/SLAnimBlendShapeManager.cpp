#include "SLAnimBlendShapeManager.h"

void SLAnimBlendShapeManager::addblendShapes(SLAnimBlendShape* blendShape)
{
    _blendShapeList.push_back(blendShape);
    ++_blendShapeListSize;
}

void SLAnimBlendShapeManager::addMeshName(SLstring name)
{
    _meshNames.push_back(name);
}

void SLAnimBlendShapeManager::addBlendShapeName(SLstring name)
{
    _blendShapeName.push_back(name);
}

void SLAnimBlendShapeManager::setTotalMeshCount(SLint count)
{
    _totalMeshCount = count;
}
