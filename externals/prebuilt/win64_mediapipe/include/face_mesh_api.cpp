#if 1
#include "face_mesh_api.h"
#include "face_mesh_detect.h"

mediapipe_face_mesh::face_mesh_detect _faceMesh;

int FACE_MESH_INIT()
{
	return _faceMesh.init_graph();
}

int FACE_MESH_UPDATE()
{
	return _faceMesh.update();

}

int FACE_MESH_END()
{
	return _faceMesh.release();

}

float* FACE_MESH_GET_FACE_LANDMARKS()
{
	return _faceMesh.faceLandmarkPointer();

}

int FACE_MESH_GET_FACE_LANDMARKS_SIZE()
{
	return _faceMesh.faceLandmarkSize();

}

#include "../calculator_graph_util.h"
DEFINE_LOAD_GRAPH("graphs\\face_mesh_desktop_live.pbtxt")

#include <windows.h>

namespace mediapipe
{

	std::string getCurrentDir()
	{
		char buff[260];
		GetModuleFileName(NULL, buff, 260);
		std::string::size_type position = std::string(buff).find_last_of("\\/");

		return std::string(buff).substr(0, position + 1);
	}


	DEFINE_SUBGRAPH(FaceLandmarkFrontCpu, "modules\\face_landmark_front_cpu.pbtxt");
	DEFINE_SUBGRAPH(FaceDetectionShortRangeCpu, "modules\\face_detection_short_range_cpu.pbtxt"); // \\ ..\\..\\mediapipe\\modules\\face_detection
	DEFINE_SUBGRAPH(FaceDetectionFrontDetectionToRoi, "modules\\face_detection_front_detection_to_roi.pbtxt");
	DEFINE_SUBGRAPH(FaceLandmarkCpu, "modules\\face_landmark_cpu.pbtxt");
	DEFINE_SUBGRAPH(FaceLandmarkLandmarksToRoi, "modules\\face_landmark_landmarks_to_roi.pbtxt");
	DEFINE_SUBGRAPH(FaceDetectionShortRangeCommon, "modules\\face_detection_short_range_common.pbtxt");
	DEFINE_SUBGRAPH(FaceLandmarksModelLoader, "modules\\face_landmarks_model_loader.pbtxt"); // \\ ..\\..\\mediapipe\\modules\\face_landmark
	DEFINE_SUBGRAPH(TensorsToFaceLandmarks, "modules\\tensors_to_face_landmarks.pbtxt");
	DEFINE_SUBGRAPH(TensorsToFaceLandmarksWithAttention, "modules\\tensors_to_face_landmarks_with_attention.pbtxt");
	DEFINE_SUBGRAPH(FaceGeometryFromLandmarks, "modules\\face_geometry_from_landmarks.pbtxt");  // \\ ..\\..\\mediapipe\\modules\\face_geometry
	DEFINE_SUBGRAPH(FaceLandmarksSmoothing, "subgraphs\\face_landmarks_smoothing.pbtxt");

	DEFINE_SUBGRAPH(FaceRendererCpu, "subgraphs\\face_renderer_cpu.pbtxt");

}


// int main(int argc, char** argv)
// {
// 	google::InitGoogleLogging(argv[0]);
// 	absl::ParseCommandLine(argc, argv);
// 	
// 	mediapipe_face_mesh::face_mesh_detect test;
// 	test.init_graph();
//     int i = 1;
//     while(i == 1)
//     {
//         i = test.update();
//     }
// 
// 	test.release();
// 
//     // cv::Mat myImage;//Declaring a matrix to load the frames//
//     // cv::namedWindow("Video Player");//Declaring the video to show the video//
//     // cv::VideoCapture cap(0);//Declaring an object to capture stream of frames from default camera//
//     // if (!cap.isOpened()) { //This section prompt an error message if no video stream is found//
//     //     std::cout << "No video stream detected" << std::endl;
//     //     system("pause");
//     //     return-1;
//     // }
//     // while (true) { //Taking an everlasting loop to show the video//
//     //     cap >> myImage;
//     //     if (myImage.empty()) { //Breaking the loop if no video frame is detected//
//     //         break;
//     //     }
//     //     cv::imshow("Video Player", myImage);//Showing the video//
//     // }
//     // cap.release();//Releasing the buffer memory//
// 
// 	return 0;
// 
// }
#endif