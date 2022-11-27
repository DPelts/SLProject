#include "SLFaceAnim.h"

#include "face_mesh_api.h"

SLFaceAnim::SLFaceAnim(SLAssetManager* assetMgr) : SLMesh(assetMgr)
{
    int status = FACE_MESH_INIT();
}

void SLFaceAnim::draw(SLSceneView* sv, SLNode* node)
{

}
