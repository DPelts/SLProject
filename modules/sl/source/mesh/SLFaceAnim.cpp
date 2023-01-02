#include <SLMesh.h>
#include "SLSphere.h"
#include "SLFaceAnim.h"

#include <prebuilt/win64_mediapipe/include/face_mesh_api.h>

#define LMK(Index, XYZ) _pFacialLandmarks[Index * 3 + XYZ]
#define LMK3D(index) LMK(index, 0), LMK(index, 1), LMK(index, 2)
#define CLAMP_TOP(value) \
    if (value < 0.0f) { value = 0.0f; }
#define CLAMP_BOTTOM(value) \
    if (value > 1.0f) { value = 1.0f; }
#define CLAMP(value) \
    if (value < 0.0f) { value = 0.0f; } \
    if (value > 1.0f) {value = 1.0f; }

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

SLVec3f getCenterPoint(SLVec3f p1, SLVec3f p2)
{
    return (p1 + p2) / 2;
}

SLfloat getLengthAtScale(SLVec3f p1, SLVec3f p2, SLfloat scale)
{
    return ((p2 - p1).length()) / scale;
}

SLfloat minEye = 10.0f;
SLfloat maxEye = 0.0f;

void SLFaceMesh::draw(SLSceneView* sv, SLNode* node)
{
    if (isCaptureStarted)
    {
        mediapipeStatus   = FACE_MESH_UPDATE();
        _pFacialLandmarks = FACE_MESH_GET_FACE_LANDMARKS();

        if (_pFacialLandmarks[0] != 0.0f)
        {
            SLVec3f face_Side_Right_Upper = SLVec3f(LMK3D(447));
            SLVec3f face_Side_Right_Lower = SLVec3f(LMK3D(366));
            SLVec3f face_Side_Left_Upper = SLVec3f(LMK3D(137));
            SLVec3f face_Side_Left_Lower = SLVec3f(LMK3D(227));

            SLVec3f right = getCenterPoint(face_Side_Right_Upper, face_Side_Right_Lower);
            SLVec3f left  = getCenterPoint(face_Side_Left_Upper, face_Side_Left_Lower);
            SLVec3f pivot = getCenterPoint(right, left);
            SLfloat pivotLength = (right - left).length() * 3;
            std::cout << pivotLength << std::endl;

            SLVec3f face_Eye_L_Upper = (SLVec3f(LMK3D(386)) - pivot) / pivotLength;
            SLVec3f face_Eye_L_Lower = (SLVec3f(LMK3D(374)) - pivot) / pivotLength;
            SLVec3f face_Eye_R_Upper = (SLVec3f(LMK3D(159)) - pivot) / pivotLength;
            SLVec3f face_Eye_R_Lower = (SLVec3f(LMK3D(145)) - pivot) / pivotLength;

            SLfloat eye_L_scale = (face_Eye_L_Upper - face_Eye_L_Lower).length();
            SLfloat eye_L_minus         = eye_L_scale - 0.016f;
            CLAMP_BOTTOM(eye_L_minus)
            SLfloat ajusted_eye_L_scale = 1.0f - (eye_L_minus) / 0.025f;
            CLAMP(ajusted_eye_L_scale)

            SLfloat eye_R_scale         = (face_Eye_R_Upper - face_Eye_R_Lower).length();
            SLfloat eye_R_minus = eye_R_scale - 0.016f;
            CLAMP_BOTTOM(eye_R_minus)
            SLfloat ajusted_eye_R_scale = 1.0f - (eye_R_minus) / 0.025f;
            CLAMP(ajusted_eye_R_scale)

            bsTime[BlendShapeIndex::eyeBlinkLeft] = ajusted_eye_L_scale;
            transformSkinWithBlendShapes(BlendShapeIndex::eyeBlinkLeft);
            bsTime[BlendShapeIndex::eyeBlinkRight] = ajusted_eye_R_scale;
            transformSkinWithBlendShapes(BlendShapeIndex::eyeBlinkRight);



        }
    }
    /*
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
    */
    SLMesh::draw(sv, node);
}
