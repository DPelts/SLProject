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
        eyeBlinkLeft = 0,
        eyeLookDownLeft = 1,
        eyeLookInLeft = 2,
        eyeLookOutLeft = 3,
        eyeLookUpLeft = 4,
        eyeSquintLeft = 5,
        eyeWideLeft = 6,
        eyeBlinkRight = 7,
        eyeLookDownRight = 8,
        eyeLookInRight = 9,
        eyeLookOutRight = 10,
        eyeLookUpRight = 11,
        eyeSquintRight = 12,
        eyeWideRight = 13,
        jawForward = 14,
        jawLeft = 15,
        jawRight = 16,
        jawOpen = 17,
        mouthClose = 18,
        mouthFunnel = 19,
        mouthPucker = 20,
        mouthRight = 21,
        mouthLeft = 22,
        mouthSmileLeft = 23,
        mouthSmileRight = 24,
        mouthFrownRight = 25,
        mouthFrownLeft = 26,
        mouthDimpleLeft = 27,
        mouthDimpleRight = 28,
        mouthStretchLeft = 29,
        mouthStretchRight = 30,
        mouthRollLower = 31,
        mouthRollUpper = 32,
        mouthShrugLower = 33,
        mouthShrugUpper = 34,
        mouthPressLeft = 35,
        mouthPressRight = 36,
        mouthLowerDownLeft = 37,
        mouthLowerDownRight = 38,
        mouthUpperUpLeft = 39,
        mouthUpperUpRight = 40,
        browDownLeft = 41,
        browDownRight = 42,
        browInnerUp = 43,
        browOuterUpLeft = 44,
        browOuterUpRight = 45,
        cheekPuff = 46,
        cheekSquintLeft = 47,
        cheekSquintRight = 48,
        noseSneerLeft = 49,
        noseSneerRight = 50,
        tongueOut = 51,
    };

    enum FaceLandmarkIndex
    {
        eyeTopLeft          = 385,
        eyeTopRight         = 158,
        jaw                 = 151,
        // eyeBottomLeft       = 2,
        // eyeBottomRight      = 3,
        // eyeLookUpLeft       = 4,
        // eyeSquintLeft       = 5,
        // eyeWideLeft         = 6,
        // eyeBlinkRight       = 7,
        // eyeLookDownRight    = 8,
        // eyeLookInRight      = 9,
        // eyeLookOutRight     = 10,
        // eyeLookUpRight      = 11,
        // eyeSquintRight      = 12,
        // eyeWideRight        = 13,
        // jawForward          = 14,
        // jawLeft             = 15,
        // jawRight            = 16,
        // jawOpen             = 17,
        // mouthClose          = 18,
        // mouthFunnel         = 19,
        // mouthPucker         = 20,
        // mouthRight          = 21,
        // mouthLeft           = 22,
        // mouthSmileLeft      = 23,
        // mouthSmileRight     = 24,
        // mouthFrownRight     = 25,
        // mouthFrownLeft      = 26,
        // mouthDimpleLeft     = 27,
        // mouthDimpleRight    = 28,
        // mouthStretchLeft    = 29,
        // mouthStretchRight   = 30,
        // mouthRollLower      = 31,
        // mouthRollUpper      = 32,
        // mouthShrugLower     = 33,
        // mouthShrugUpper     = 34,
        // mouthPressLeft      = 35,
        // mouthPressRight     = 36,
        // mouthLowerDownLeft  = 37,
        // mouthLowerDownRight = 38,
        // mouthUpperUpLeft    = 39,
        // mouthUpperUpRight   = 40,
        // browDownLeft        = 41,
        // browDownRight       = 42,
        // browInnerUp         = 43,
        // browOuterUpLeft     = 44,
        // browOuterUpRight    = 45,
        // cheekPuff           = 46,
        // cheekSquintLeft     = 47,
        // cheekSquintRight    = 48,
        // noseSneerLeft       = 49,
        // noseSneerRight      = 50,
        // tongueOut           = 51,
    };

    SLbool    isCaptureStarted = false;
    SLbool    hasFirstFrameBeenRecorded = false;
    const int originIndex = 0;
    float* _pFacialLandmarks;
    int    mediapipeStatus = 0;
    SLVVec3f idleFaceLandmarks;
    SLVVec3f maxFaceStretch;
    SLVVec3f minFaceStretch;
    // SLVMesh meshList;
};


#endif