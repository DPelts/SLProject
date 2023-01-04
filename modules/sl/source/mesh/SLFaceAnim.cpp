#include <SLMesh.h>
// #include "SLSphere.h"
#include <unordered_map>
#include "SLFaceAnim.h"

#include <prebuilt/win64_mediapipe/include/face_mesh_api.h>

#define LMK(Index, XYZ) _pFacialLandmarks[Index * 3 + XYZ]
#define LMK3D(index) LMK(index, 0), LMK(index, 1), LMK(index, 2)
#define CLAMP_TOP(value) \
    if (value > 1.0f) { value = 1.0f; }
#define CLAMP_BOTTOM(value) \
    if (value < 0.0f) { value = 0.0f; }
#define CLAMP(value) \
    if (value < 0.0f) { value = 0.0f; } \
    if (value > 1.0f) {value = 1.0f; }

SLFaceMesh::SLFaceMesh(SLAssetManager* assetMgr, const SLstring& name) : SLMesh(assetMgr, name)
{
}

SLVec3f getCenterPoint(SLVec3f p1, SLVec3f p2)
{
    return (p1 + p2) / 2;
}

SLfloat getLengthAtScale(SLVec3f p1, SLVec3f p2, SLfloat scale)
{
    return ((p2 - p1).length()) / scale;
}

void SLFaceMesh::initMaps()
{
    _localFLMap.reserve(FLISIZE);
    _rotatedFLMap.reserve(FLISIZE);

    for (SLint i = 0; i < FLISIZE; i++)
    {
        const SLint index = _usedLandmarkIndicesList[i];
        _localFLMap[index]   = SLVec3f::ZERO;
        _rotatedFLMap[index] = SLVec3f::ZERO;
    }
}

void SLFaceMesh::updateLocalFLMap(const SLVec3f pivot, const SLfloat pivotLength)
{
    for (SLint i = 0; i < FLISIZE; i++)
    {
        const SLint index = _usedLandmarkIndicesList[i];
        _localFLMap[index] = (SLVec3f(LMK3D(index)) - pivot) / pivotLength;
    }
}

void SLFaceMesh::updateRotatedLocalFLMap()
{
    SLVec3f forward = _localFLMap[FLI::nose];  // Nose
    SLVec3f right   = getCenterPoint(_localFLMap[FLI::face_Side_R_Upper], _localFLMap[FLI::face_Side_R_Lower]);
    SLVec3f normal  = forward.normalized();
    SLVec3f tangent  = right.normalized();
    SLVec3f down;
    down.cross(normal, tangent);
    down            = -down;
    SLVec3f binormal = down.normalized();
    // std::cout << normal << "\t" << tangent << "\t" << binormal << std::endl;
    // SLMat4f matrix = SLMat4f(tangent.x, tangent.y, tangent.z, 0.0f, normal.x, normal.y, normal.z, 0.0f, binormal.x, binormal.y, binormal.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    SLfloat r00               = tangent.x;
    SLfloat r01               = tangent.y;
    SLfloat r02               = tangent.z;
    SLfloat r12               = normal.z;
    SLfloat r22               = binormal.z; 

    SLfloat roll            = atan(-r12 / r22);
    SLfloat pitch           = atan(r02 * cos(roll) / r22);
    SLfloat yaw            = atan(-r01 / r00);
    
    roll *= 180.0f / PI;
    pitch *= 180.0f / PI;
    yaw *= 180.0f / PI;

    if (roll > 0)
    {
        roll = -(90.0f - roll);
    }
    else if (roll < 0)
    {
        roll = roll + 90.0f;
        pitch = -pitch;
    }

    _headRot                   = SLVec3f(roll, pitch, yaw);

    for (SLint i = 0; i < FLISIZE; i++)
    {
        SLint index = _usedLandmarkIndicesList[i];
        SLVec3f point = _localFLMap[index];
        SLfloat theta = roll * 0.0174533f;
        SLMat3f r     = SLMat3f(1.0f, 0.0f, 0.0f, 0.0f, cos(theta), -sin(theta), 0.0f, sin(theta), cos(theta));
        // SLMat3f r = SLMat3f(1.0f, 0.0f, 0.0f, 0.0f, cos(theta), sin(theta), 0.0f, -sin(theta), cos(theta));
        SLVec3f temp = r * point;
        // std::cout << r.m() << std::endl << point << std::endl << temp << std::endl << std::endl;
        point = r * point;
        theta = pitch * 0.0174533f;
        r                    = SLMat3f(cos(theta), 0.0f, sin(theta), 0.0f, 1.0f, 0.0f, -sin(theta), 0.0f, cos(theta));
        // r                    = SLMat3f(cos(theta),  0.0f, -sin(theta), 0.0f, 1.0f, 0.0f, sin(theta), 0.0f, cos(theta));
        point = r * point;
        theta = yaw * 0.0174533 / 2.5f;
        r                    = SLMat3f(cos(theta), -sin(theta), 0.0f, sin(theta), cos(theta), 0.0f, 0.0f, 0.0f, 1.0f);
        // r                    = SLMat3f(cos(theta), sin(theta), 0.0f, -sin(theta),  cos(theta),  0.0f, 0.0f, 0.0f, 1.0f);
        point = r * point;
        _rotatedFLMap[index] = point;
    }
}

void SLFaceMesh::startCapture()
{
    mediapipeStatus = FACE_MESH_INIT();

    int size = FACE_MESH_GET_FACE_LANDMARKS_SIZE() / 3;
    idleFaceLandmarks.resize(size);
    maxFaceStretch.resize(size);
    minFaceStretch.resize(size);
    _pFacialLandmarks = FACE_MESH_GET_FACE_LANDMARKS();

    initMaps();

    isCaptureStarted = true;
}

void SLFaceMesh::eyeBlinkTracking()
{
    SLVec3f face_Eye_L_Upper = _localFLMap[FLI::eye_Lid_L_Upper]; // (SLVec3f(LMK3D(386)) - pivot) / pivotLength;
    SLVec3f face_Eye_L_Lower = _localFLMap[FLI::eye_Lid_L_Lower]; // (SLVec3f(LMK3D(374)) - pivot) / pivotLength;
    SLVec3f face_Eye_R_Upper = _localFLMap[FLI::eye_Lid_R_Upper]; // (SLVec3f(LMK3D(159)) - pivot) / pivotLength;
    SLVec3f face_Eye_R_Lower = _localFLMap[FLI::eye_Lid_R_Lower]; // (SLVec3f(LMK3D(145)) - pivot) / pivotLength;

    SLfloat eye_L_scale = (face_Eye_L_Upper - face_Eye_L_Lower).length();
    SLfloat eye_L_minus = eye_L_scale - 0.016f;
    CLAMP_BOTTOM(eye_L_minus)
    SLfloat ajusted_eye_L_scale = 1.0f - (eye_L_minus) / 0.025f;
    CLAMP(ajusted_eye_L_scale)

    SLfloat eye_R_scale = (face_Eye_R_Upper - face_Eye_R_Lower).length();
    SLfloat eye_R_minus = eye_R_scale - 0.016f;
    CLAMP_BOTTOM(eye_R_minus)
    SLfloat ajusted_eye_R_scale = 1.0f - (eye_R_minus) / 0.025f;
    CLAMP(ajusted_eye_R_scale)

    bsTime[BlendShapeIndex::eyeBlinkLeft] = ajusted_eye_L_scale;
    transformSkinWithBlendShapes(BlendShapeIndex::eyeBlinkLeft);
    bsTime[BlendShapeIndex::eyeBlinkRight] = ajusted_eye_R_scale;
    transformSkinWithBlendShapes(BlendShapeIndex::eyeBlinkRight);
}

void SLFaceMesh::jawTracking()
{
    // SLVec3f eyePivot = getCenterPoint(_rotatedFLMap[FLI::eye_Outer_L], _rotatedFLMap[FLI::eye_Outer_L]);
    // SLVec3f jaw       = _rotatedFLMap[FLI::jaw];
    SLVec3f eyePivot = getCenterPoint(_localFLMap[FLI::eye_Outer_L], _localFLMap[FLI::eye_Outer_R]);
    SLVec3f jaw       = _localFLMap[FLI::jaw];
    // SLVec3f diff            = jaw - eyePivot;
    SLfloat diff            = (jaw - eyePivot).length();
    _jawHeight           = (diff - 0.36f) / 0.08f; // 0.42f;
    CLAMP_TOP(_jawHeight)
    CLAMP_BOTTOM(_jawHeight)
    // std::cout << diff << "\t" << _jawHeight << std::endl;
    bsTime[BlendShapeIndex::jawOpen] = _jawHeight;
    transformSkinWithBlendShapes(BlendShapeIndex::jawOpen);
}

void SLFaceMesh::mouthTracking()
{
    SLfloat lipDist = (_localFLMap[FLI::mouth_UpperLip_Outer] - _localFLMap[FLI::mouth_LowerLip_Outer]).length();
    // Mouth open/close
    SLfloat       jawPercent         = (_jawHeight - 0.05f) / 0.7f;
    SLfloat       mouthHeightPercent = (lipDist) / 0.176f;
    CLAMP_BOTTOM(mouthHeightPercent)
    if (jawPercent < 0.1f)
        jawPercent = 0.1f;
    SLfloat       ratio              = mouthHeightPercent / jawPercent;
    if (_jawHeight > 0.7f)
        ratio = 1.0f;
    SLfloat regulator = 1.0f - ratio;
    // std::cout << "Jaw: " << _jawHeight << "\tRegu: " << regulator << "\tLipDist: " << lipDist << "\t jawP:" << jawPercent << "\t mouthP:" << mouthHeightPercent << "\t Ratio:" << ratio << std::endl;
    regulator                           = (regulator > _jawHeight) ? _jawHeight : regulator;
    CLAMP_TOP(regulator)
    CLAMP_BOTTOM(regulator)
    bsTime[BlendShapeIndex::mouthClose] = regulator;
    transformSkinWithBlendShapes(BlendShapeIndex::mouthClose);

    // Mouth Pucker
    SLfloat mouthCornerDist = (_localFLMap[FLI::mouth_Corner_L] - _localFLMap[FLI::mouth_Corner_R]).length();
    SLfloat puckerRatio = 1.0 - (mouthCornerDist - 0.06f) / 0.05f;
    CLAMP_TOP(puckerRatio)
    CLAMP_BOTTOM(puckerRatio)
    bsTime[BlendShapeIndex::mouthPucker] = puckerRatio;
    transformSkinWithBlendShapes(BlendShapeIndex::mouthPucker);

    // Mouth Corner Movement
    SLVec3f mouthCornerCenter = getCenterPoint(_localFLMap[FLI::mouth_Corner_L], _localFLMap[FLI::mouth_Corner_R]);
    SLVec3f mouthLipCenter    = getCenterPoint(_localFLMap[FLI::mouth_UpperLip_Outer], _localFLMap[FLI::mouth_LowerLip_Outer]);
    SLfloat centerDist        = (mouthCornerCenter - mouthLipCenter).length();
    // SLfloat cornerDist        = (_localFLMap[FLI::mouth_Corner_L] - _localFLMap[FLI::mouth_Corner_R]).length();
    // std::cout << mouthCornerDist << "\t" << centerDist << std::endl;
    SLfloat topRatio = (centerDist - 0.039f) / 0.01f;
    CLAMP_TOP(topRatio)
    CLAMP_BOTTOM(topRatio)
    SLfloat cornerRatio = (mouthCornerDist - 0.115) / 0.015f;
    CLAMP_TOP(cornerRatio)
    CLAMP_BOTTOM(cornerRatio)
    SLfloat smilePercent = cornerRatio * topRatio;
    SLfloat dimpelPercent = cornerRatio - smilePercent;
    bsTime[BlendShapeIndex::mouthSmileLeft] = smilePercent;
    transformSkinWithBlendShapes(BlendShapeIndex::mouthSmileLeft);
    bsTime[BlendShapeIndex::mouthSmileRight] = smilePercent;
    transformSkinWithBlendShapes(BlendShapeIndex::mouthSmileRight);
    bsTime[BlendShapeIndex::mouthDimpleLeft] = dimpelPercent;
    transformSkinWithBlendShapes(BlendShapeIndex::mouthDimpleLeft);
    bsTime[BlendShapeIndex::mouthDimpleRight] = dimpelPercent;
    transformSkinWithBlendShapes(BlendShapeIndex::mouthDimpleRight);
    // std::cout << "topRatio: " << topRatio << "\t cornerRatio: " << cornerRatio << "\t smile: " << smilePercent << "\t dimpel: " << dimpelPercent << std::endl;
    
    // SLVec3f mouthCornerCenterRotated = getCenterPoint(_rotatedFLMap[FLI::mouth_Corner_L], _rotatedFLMap[FLI::mouth_Corner_R]);
    // SLVec3f mouthLipCenterRotated    = getCenterPoint(_rotatedFLMap[FLI::mouth_UpperLip_Outer], _rotatedFLMap[FLI::mouth_LowerLip_Outer]);
    // SLfloat centerDistRotated        = (mouthCornerCenter.y - mouthLipCenter.y);
    // std::cout << "centerDist: " << centerDistRotated << std::endl;
    // SLVec3f eyePivotRotated = getCenterPoint(_rotatedFLMap[FLI::eye_Outer_L], _rotatedFLMap[FLI::eye_Outer_R]);
    // eyePivotRotated.y += mouthLipCenter.y - eyePivotRotated.y;
    // SLfloat mouthCenterOffset = eyePivotRotated.x - mouthLipCenterRotated.x;
    // std::cout << "" << mouthCenterOffset << std::endl;

}

void SLFaceMesh::irisTracking()
{
    SLVec3f eye_L_center = getCenterPoint(_rotatedFLMap[FLI::eye_Inner_L], _rotatedFLMap[FLI::eye_Outer_L]);
    SLVec3f eye_R_center = getCenterPoint(_rotatedFLMap[FLI::eye_Inner_R], _rotatedFLMap[FLI::eye_Outer_R]);
    SLVec3f iris_L       = _rotatedFLMap[FLI::iris_L];
    SLVec3f iris_R       = _rotatedFLMap[FLI::iris_R];
    SLVec3f diff_L       = iris_L - eye_L_center;
    SLVec3f diff_R       = iris_R - eye_R_center;

    diff_L.y += 0.058f;
    if (diff_L.y < 0.0f)
    {
        SLfloat ratio = diff_L.y / -0.007f;
        CLAMP_TOP(ratio)
        CLAMP_BOTTOM(ratio)
        bsTime[BlendShapeIndex::eyeLookUpLeft] = ratio;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookUpLeft);
        bsTime[BlendShapeIndex::eyeLookDownLeft] = 0.0f;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookDownLeft);

        bsTime[BlendShapeIndex::eyeLookUpRight] = ratio;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookUpRight);
        bsTime[BlendShapeIndex::eyeLookDownRight] = 0.0f;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookDownRight);
        // std::cout << "Y: " << diff_L.y << "\n Ratio: " << ratio << std::endl << std::endl;
    }
    else
    {
        SLfloat ratio = diff_L.y / 0.015f;
        CLAMP_TOP(ratio)
        CLAMP_BOTTOM(ratio)
        bsTime[BlendShapeIndex::eyeLookUpLeft] = 0.0f;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookUpLeft);
        bsTime[BlendShapeIndex::eyeLookDownLeft] = ratio;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookDownLeft);

        bsTime[BlendShapeIndex::eyeLookUpRight] = 0.0f;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookUpRight);
        bsTime[BlendShapeIndex::eyeLookDownRight] = ratio;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookDownRight);
        // std::cout << "Y: " << diff_L.y << "\n Ratio: " << ratio << std::endl << std::endl;
    }
    
    diff_L.x += 0.064f;
    if (diff_L.x < 0.0f)
    {
        SLfloat ratio = diff_L.x / -0.009f;
        CLAMP_TOP(ratio)
        CLAMP_BOTTOM(ratio)
        bsTime[BlendShapeIndex::eyeLookInLeft] = ratio;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookInLeft);
        bsTime[BlendShapeIndex::eyeLookOutLeft] = 0.0f;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookOutLeft);

        bsTime[BlendShapeIndex::eyeLookInRight] = 0.0f;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookInRight);
        bsTime[BlendShapeIndex::eyeLookOutRight] = ratio;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookOutRight);
        // std::cout << "In x: " << diff_L.x << "\n Ratio: " << ratio << std::endl << std::endl;
    }
    else
    {
        SLfloat ratio = diff_L.x / 0.015f;
        CLAMP_TOP(ratio)
        CLAMP_BOTTOM(ratio)
        bsTime[BlendShapeIndex::eyeLookInLeft] = 0.0f;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookInLeft);
        bsTime[BlendShapeIndex::eyeLookOutLeft] = ratio;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookOutLeft);

        bsTime[BlendShapeIndex::eyeLookInRight] = ratio;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookInRight);
        bsTime[BlendShapeIndex::eyeLookOutRight] = 0.0f;
        transformSkinWithBlendShapes(BlendShapeIndex::eyeLookOutRight);
        // std::cout << "Out x: " << diff_L.x << "\n Ratio: " << ratio << std::endl << std::endl;
    }
    
    // std::cout << "[ " << diff_R.x << ", " << diff_R.y << " ]" << std::endl;
    // diff_R.y += 0.058f;
    // if (diff_R.y < 0.0f)
    // {
    //     SLfloat ratio = diff_R.y / -0.007f;
    //     CLAMP_TOP(ratio)
    //     CLAMP_BOTTOM(ratio)
    //     bsTime[BlendShapeIndex::eyeLookUpRight] = ratio;
    //     transformSkinWithBlendShapes(BlendShapeIndex::eyeLookUpRight);
    //     bsTime[BlendShapeIndex::eyeLookDownRight] = 0.0f;
    //     transformSkinWithBlendShapes(BlendShapeIndex::eyeLookDownRight);
    // }
    // else
    // {
    //     SLfloat ratio = diff_R.y / 0.015f;
    //     CLAMP_TOP(ratio)
    //     CLAMP_BOTTOM(ratio)
    //     bsTime[BlendShapeIndex::eyeLookUpRight] = 0.0f;
    //     transformSkinWithBlendShapes(BlendShapeIndex::eyeLookUpRight);
    //     bsTime[BlendShapeIndex::eyeLookDownRight] = ratio;
    //     transformSkinWithBlendShapes(BlendShapeIndex::eyeLookDownRight);
    // }
    // 
    // diff_R.x = 0.062f;
    // if (diff_R.x < 0.0f)
    // {
    //     SLfloat ratio = diff_R.x / 0.01f;
    //     std::cout << "Ratio: " << ratio << std::endl;
    //     CLAMP_TOP(ratio)
    //     CLAMP_BOTTOM(ratio)
    //     bsTime[BlendShapeIndex::eyeLookInRight] = 0.0f;
    //     transformSkinWithBlendShapes(BlendShapeIndex::eyeLookInRight);
    //     bsTime[BlendShapeIndex::eyeLookOutRight] = ratio;
    //     transformSkinWithBlendShapes(BlendShapeIndex::eyeLookOutRight);
    // }
    // else
    // {
    //     SLfloat ratio = diff_R.x / -0.02f;
    //     std::cout << "Ratio: " << ratio << std::endl;
    //     CLAMP_TOP(ratio)
    //     CLAMP_BOTTOM(ratio)
    //     bsTime[BlendShapeIndex::eyeLookInRight] = ratio;
    //     transformSkinWithBlendShapes(BlendShapeIndex::eyeLookInRight);
    //     bsTime[BlendShapeIndex::eyeLookOutRight] = 0.0f;
    //     transformSkinWithBlendShapes(BlendShapeIndex::eyeLookOutRight);
    // }
}

void SLFaceMesh::cheekSquintTracking()

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

            updateLocalFLMap(pivot, pivotLength);
            updateRotatedLocalFLMap();

            // Testing pivot and scale
            // SLVec3f rightRot       = getCenterPoint(_localFLMap[FLI::face_Side_R_Upper], _localFLMap[FLI::face_Side_R_Lower]);
            // SLVec3f leftRot        = getCenterPoint(_localFLMap[FLI::face_Side_L_Upper], _localFLMap[FLI::face_Side_L_Lower]);
            // SLVec3f pivotRot       = getCenterPoint(rightRot, leftRot);
            // SLfloat pivotLengthRot = (rightRot - leftRot).length();
            // std::cout << "Pivot: " << pivot << "\t PivotRot: " << pivotRot << "\t PivotLength: " << pivotLength << std::endl
            //           << "\t PivotLengthRot: " << pivotLengthRot << "RightRot: " << rightRot << std::endl;

            eyeBlinkTracking();
            jawTracking();
            mouthTracking();
            irisTracking();
            cheekSquintTracking();


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
