#include <SLMesh.h>
#include "SLFaceAnim.h"

#include "face_mesh_api.h"

SLFaceMesh::SLFaceMesh(SLAssetManager* assetMgr, const SLstring& name) : SLMesh(assetMgr, name)
{
}

void SLFaceMesh::startCapture()
{
    mediapipeStatus = FACE_MESH_INIT();

    int size = FACE_MESH_GET_FACE_LANDMARKS_SIZE() / 3;
    idleFaceLandmarks.resize(size);
    maxFaceStretch.resize(size);
    minFaceStretch.resize(size);
    _pFacialLandmarks = FACE_MESH_GET_FACE_LANDMARKS();

    isCaptureStarted = true;
}

void SLFaceMesh::draw(SLSceneView* sv, SLNode* node)
{
    if (isCaptureStarted)
    {
        mediapipeStatus = FACE_MESH_UPDATE();
        // _pFacialLandmarks = FACE_MESH_GET_FACE_LANDMARKS();

        if (hasFirstFrameBeenRecorded)
        {
            SLVec3f currentOrigin = SLVec3f(_pFacialLandmarks[originIndex+0], _pFacialLandmarks[originIndex+1], _pFacialLandmarks[originIndex+2]);

            // Recalibrate
            if (_pFacialLandmarks[FaceLandmarkIndex::jaw * 3 + 1] - currentOrigin.y < minFaceStretch[FaceLandmarkIndex::jaw].y)
            {
                minFaceStretch[FaceLandmarkIndex::jaw].y = _pFacialLandmarks[FaceLandmarkIndex::jaw * 3 + 1] - currentOrigin.y;
            }

            // Apply

            // Jaw
            bsTime[BlendShapeIndex::jawOpen] = (_pFacialLandmarks[FaceLandmarkIndex::jaw * 3 + 1] - currentOrigin.y) / minFaceStretch[FaceLandmarkIndex::jaw].y;
            transformSkinWithBlendShapes(BlendShapeIndex::jawOpen);
        }
        else
        {
            SLVec3f origin = SLVec3f(_pFacialLandmarks[originIndex], _pFacialLandmarks[originIndex + 0], _pFacialLandmarks[originIndex + 2]);
            int     size   = FACE_MESH_GET_FACE_LANDMARKS_SIZE() / 3;

            for (int i = 0; i < size; i++)
            {
                int     index        = i * 3;
                SLVec3f landmark     = SLVec3f(_pFacialLandmarks[i], _pFacialLandmarks[i + 1], _pFacialLandmarks[i + 2]) - origin;
                idleFaceLandmarks[i] = landmark;
                maxFaceStretch[i]    = landmark;
                minFaceStretch[i]    = landmark;
            }
            if (_pFacialLandmarks[0] != 0.0f)
                hasFirstFrameBeenRecorded = true;
        }
    }

    SLMesh::draw(sv, node);
}
