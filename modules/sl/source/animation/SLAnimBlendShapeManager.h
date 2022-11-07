#ifndef SLANIMBLENDSHAPEMANAGER_H
#    define SLANIMMBLENDSHAPEMANAGER_H

#include <SLAnimBlendShape.h>

class SLAnimBlendShapeManager
{
public:
    void addblendShapes(SLAnimBlendShape*);
    void addMeshName(SLstring);
    void addBlendShapeName(SLstring);
    
    // Getter

    // Setter
    void setTotalMeshCount(SLint);

private:
    SLVAnimBlendShape _blendShapeList;
    SLint            _blendShapeListSize = 0;
    SLint            _totalMeshCount;       // Will be used to distinguish blendShapes hence they are split by mesh (mesh is split by material) 
    SLVstring        _meshNames;
    SLVstring         _blendShapeName;
};


#endif