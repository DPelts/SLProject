//#############################################################################
//  File:      SLFaceMesh.h
//  Date:      November 2022
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/SLProject-Coding-Style
//  Authors:   Dmytriy Pelts - bachelor thesis fall 2022
//  License:   This software is provided under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLFACEANIM_H
#define SLFACEANIM_H


class SLFaceMesh : public SLMesh
{
public:
    SLFaceMesh(SLAssetManager*, const SLstring& name);

    void startCapture();
    void draw(SLSceneView*, SLNode*);
    // void addMesh(SLMesh*);
    // void draw(SLSceneView*, SLNode*);

private:
    enum BlendShapeIndex
    {
        eyeBlinkLeft        = 0,
        eyeLookDownLeft     = 1,
        eyeLookInLeft       = 2,
        eyeLookOutLeft      = 3,
        eyeLookUpLeft       = 4,
        eyeSquintLeft       = 5,
        eyeWideLeft         = 6,
        eyeBlinkRight       = 7,
        eyeLookDownRight    = 8,
        eyeLookInRight      = 9,
        eyeLookOutRight     = 10,
        eyeLookUpRight      = 11,
        eyeSquintRight      = 12,
        eyeWideRight        = 13,
        jawForward          = 14,
        jawLeft             = 15,
        jawRight            = 16,
        jawOpen             = 17,
        mouthClose          = 18,
        mouthFunnel         = 19,
        mouthPucker         = 20,
        mouthRight          = 21,
        mouthLeft           = 22,
        mouthSmileLeft      = 23,
        mouthSmileRight     = 24,
        mouthFrownRight     = 25,
        mouthFrownLeft      = 26,
        mouthDimpleLeft     = 27,
        mouthDimpleRight    = 28,
        mouthStretchLeft    = 29,
        mouthStretchRight   = 30,
        mouthRollLower      = 31,
        mouthRollUpper      = 32,
        mouthShrugLower     = 33,
        mouthShrugUpper     = 34,
        mouthPressLeft      = 35,
        mouthPressRight     = 36,
        mouthLowerDownLeft  = 37,
        mouthLowerDownRight = 38,
        mouthUpperUpLeft    = 39,
        mouthUpperUpRight   = 40,
        browDownLeft        = 41,
        browDownRight       = 42,
        browInnerUp         = 43,
        browOuterUpLeft     = 44,
        browOuterUpRight    = 45,
        cheekPuff           = 46,
        cheekSquintLeft     = 47,
        cheekSquintRight    = 48,
        noseSneerLeft       = 49,
        noseSneerRight      = 50,
        tongueOut           = 51,
    };

    enum FLI // FaceLandmarkIndex
    {
        eye_Lid_L_Upper   = 386,
        eye_Lid_L_Lower   = 374,
        eye_Lid_R_Upper   = 159,
        eye_Lid_R_Lower   = 145,
        eye_Outer_L       = 33,
        eye_Outer_R       = 263,
        face_Side_R_Upper = 447,
        face_Side_R_Lower = 366,
        face_Side_L_Upper = 137,
        face_Side_L_Lower = 227,
        nose              = 5,
        jaw               = 152,
        mouth_UpperLip_Outer = 0,
        mouth_LowerLip_Outer = 17,
        mouth_Corner_L       = 291,
        mouth_Corner_R       = 61,
    };

    SLbool    isCaptureStarted          = false;
    SLbool    hasFirstFrameBeenRecorded = false;
    const int originIndex               = 0;
    float*    _pFacialLandmarks;
    int       mediapipeStatus = 0;
    SLVVec3f  idleFaceLandmarks;
    SLVVec3f  maxFaceStretch;
    SLVVec3f  minFaceStretch;

private:
    #define FLISIZE 14
    SLint _usedLandmarkIndicesList[FLISIZE]
    {
      FLI::eye_Lid_L_Upper,
      FLI::eye_Lid_L_Lower,
      FLI::eye_Lid_R_Upper,
      FLI::eye_Lid_R_Lower,
      FLI::face_Side_R_Upper,
      FLI::face_Side_R_Lower,
      FLI::face_Side_L_Upper,
      FLI::face_Side_L_Lower,
      FLI::nose,
      FLI::jaw,
      FLI::mouth_UpperLip_Outer,
      FLI::mouth_LowerLip_Outer,
      FLI::mouth_Corner_L,
      FLI::mouth_Corner_R,
    };

    std::unordered_map<SLint, SLVec3f> _localFLMap;
    std::unordered_map<SLint, SLVec3f> _rotatedFLMap;
    SLVec3f                            _pivot = SLVec3f::ZERO;
    SLVec3f                            _headRot = SLVec3f::ZERO;
    SLfloat                            _jawHeight = 0.0f;

    void SLFaceMesh::initMaps();
    void SLFaceMesh::updateLocalFLMap(const SLVec3f, const SLfloat);
    void SLFaceMesh::updateRotatedLocalFLMap();

    void eyeBlinkTracking();
    void jawTracking();
    void mouthTracking();
    void irisTracking();
    void browTracking();
    void cheekSquintTracking();
};
#endif