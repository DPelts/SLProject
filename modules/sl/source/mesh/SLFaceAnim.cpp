#include <SLMesh.h>
#include "SLSphere.h"
#include "SLFaceAnim.h"

#include <prebuilt/win64_mediapipe/include/face_mesh_api.h>
#include "opencv2/calib3d.hpp"

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
        mediapipeStatus   = FACE_MESH_UPDATE();
        _pFacialLandmarks = FACE_MESH_GET_FACE_LANDMARKS();

        if (_pFacialLandmarks[0] != 0.0f)
        {
            SLint imageWidth  = 640;
            SLint imageHeight = 480;
            SLint focalLength = 1 * imageWidth;

            int i1 = 1 * 3;
            int i2 = 33 * 3;
            int i3 = 263 * 3;
            int i4 = 61 * 3;
            int i5 = 291 * 3;
            int i6 = 199 * 3;

            SLVec2f nose2D = SLVec2f(_pFacialLandmarks[i1 + 0] * imageWidth, _pFacialLandmarks[i1 + 1] * imageHeight);
            SLVec3f nose3D = SLVec3f(_pFacialLandmarks[i1 + 0] * imageWidth, _pFacialLandmarks[i1 + 1] * imageHeight, _pFacialLandmarks[i1 + 2] * 3000);


            SLVVec2f face_2D;
            face_2D.resize(6);
            face_2D[0] = SLVec2f(_pFacialLandmarks[i1 + 0] * imageWidth, _pFacialLandmarks[i1 + 1] * imageHeight);
            face_2D[1] = SLVec2f(_pFacialLandmarks[i2 + 0] * imageWidth, _pFacialLandmarks[i2 + 1] * imageHeight);
            face_2D[2] = SLVec2f(_pFacialLandmarks[i3 + 0] * imageWidth, _pFacialLandmarks[i3 + 1] * imageHeight);
            face_2D[3] = SLVec2f(_pFacialLandmarks[i4 + 0] * imageWidth, _pFacialLandmarks[i4 + 1] * imageHeight);
            face_2D[4] = SLVec2f(_pFacialLandmarks[i5 + 0] * imageWidth, _pFacialLandmarks[i5 + 1] * imageHeight);
            face_2D[5] = SLVec2f(_pFacialLandmarks[i6 + 0] * imageWidth, _pFacialLandmarks[i6 + 1] * imageHeight);

            SLVVec3f face_3D;
            face_3D.resize(6);
            face_3D[0] = SLVec3f(_pFacialLandmarks[i1 + 0] * imageWidth, _pFacialLandmarks[i1 + 1] * imageHeight,  _pFacialLandmarks[i1 + 2]);
            face_3D[1] = SLVec3f(_pFacialLandmarks[i2 + 0] * imageWidth, _pFacialLandmarks[i2 + 1] * imageHeight,  _pFacialLandmarks[i2 + 2]);
            face_3D[2] = SLVec3f(_pFacialLandmarks[i3 + 0] * imageWidth, _pFacialLandmarks[i3 + 1] * imageHeight,  _pFacialLandmarks[i3 + 2]);
            face_3D[3] = SLVec3f(_pFacialLandmarks[i4 + 0] * imageWidth, _pFacialLandmarks[i4 + 1] * imageHeight,  _pFacialLandmarks[i4 + 2]);
            face_3D[4] = SLVec3f(_pFacialLandmarks[i5 + 0] * imageWidth, _pFacialLandmarks[i5 + 1] * imageHeight,  _pFacialLandmarks[i5 + 2]);
            face_3D[5] = SLVec3f(_pFacialLandmarks[i6 + 0] * imageWidth, _pFacialLandmarks[i6 + 1] * imageHeight,  _pFacialLandmarks[i6 + 2]);

            // cv::Mat face2DMat = (cv::Mat_<double>(2, 6) << face_2D[0].x, face_2D[0].y, face_2D[1].x, face_2D[1].y, face_2D[2].x, face_2D[2].y, face_2D[3].x, face_2D[3].y, face_2D[4].x, face_2D[4].y, face_2D[5].x, face_2D[5].y);
            // cv::Mat face3DMat = (cv::Mat_<double>(3, 6) << face_3D[0].x, face_3D[0].y, face_3D[0].z, face_3D[1].x, face_3D[1].y, face_3D[1].z, face_3D[2].x, face_3D[2].y, face_3D[2].z, face_3D[3].x, face_3D[3].y, face_3D[3].z, face_3D[4].x, face_3D[4].y, face_3D[4].z, face_3D[5].x, face_3D[5].y, face_3D[5].z);
            

            std::vector<cv::Point2f> face2D;
            face2D.push_back(cv::Point2f(face_2D[0].x, face_2D[0].y));
            face2D.push_back(cv::Point2f(face_2D[1].x, face_2D[1].y));
            face2D.push_back(cv::Point2f(face_2D[2].x, face_2D[2].y));
            face2D.push_back(cv::Point2f(face_2D[3].x, face_2D[3].y));
            face2D.push_back(cv::Point2f(face_2D[4].x, face_2D[4].y));
            face2D.push_back(cv::Point2f(face_2D[5].x, face_2D[5].y));

            std::vector<cv::Point3f> face3D;
            face3D.push_back(cv::Point3f(face_3D[0].x, face_3D[0].y, face_3D[0].z));
            face3D.push_back(cv::Point3f(face_3D[1].x, face_3D[1].y, face_3D[1].z));
            face3D.push_back(cv::Point3f(face_3D[2].x, face_3D[2].y, face_3D[2].z));
            face3D.push_back(cv::Point3f(face_3D[3].x, face_3D[3].y, face_3D[3].z));
            face3D.push_back(cv::Point3f(face_3D[4].x, face_3D[4].y, face_3D[4].z));
            face3D.push_back(cv::Point3f(face_3D[5].x, face_3D[5].y, face_3D[5].z));


            cv::Mat distMat = (cv::Mat_<double>(4, 1) << 0.0f, 0.0f, 0.0f, 0.0f);
            cv::Mat camMat = (cv::Mat_<double>(3, 3) << focalLength, 0, imageHeight / 2, 0, focalLength, imageWidth / 2, 0, 0, 1);

            cv::Mat rotVec(3, 1, cv::DataType<double>::type);
            cv::Mat transVec(3, 1, cv::DataType<double>::type);
            if (cv::solvePnP(face3D, face2D, camMat, distMat, rotVec, transVec))
            {
                cv::Mat rMat;
                cv::Mat jacMat;
                cv::Rodrigues(rotVec, rMat, jacMat);
                
                cv::Mat mtxRMat;
                cv::Mat mtxQMat;
                cv::Vec3d angles = cv::RQDecomp3x3(rMat, mtxRMat, mtxQMat);

                SLfloat angleX = (float)angles[0] * 360;
                SLfloat angleY = (float)angles[1] * 360;
                SLfloat angleZ = (float)angles[2] * 360;

                std::cout << "( " << angleX << ", " << angleY << " ," << angleZ << " )" << std::endl;
            } 
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
