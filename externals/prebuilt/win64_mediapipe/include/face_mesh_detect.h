#ifndef FACE_MESH_DETECT_H
#define FACE_MESH_DETECT_H

#define MAX_PATH 260

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/modules/face_geometry/protos/face_geometry.pb.h"

#include <windows.h>

namespace mediapipe_face_mesh
{
	class face_mesh_detect
	{
	public:
		int init_graph() noexcept;
		int update();
		int release();
		float* faceLandmarkPointer() { return faceLandmarkList; }
		int faceLandmarkSize() { return size; }

	private:
		absl::Status InitGraph();
		absl::Status RunGraph();
		absl::Status ReleaseGraph();

		std::string getCurrentDir() 
		{
			char buff[MAX_PATH];
			GetModuleFileName(NULL, buff, MAX_PATH);
			std::string::size_type position = std::string(buff).find_last_of("\\/");

			return std::string(buff).substr(0, position + 1);
		}

		char const* kWindowName = "face landmark";
		char const* kInputStream = "input_video";
		char const* kOutputStream = "output_video";
		char const* kOutputLandmarks = "multi_face_landmarks";
		std::string faceMeshPath = "graphs\\face_mesh_desktop_live.pbtxt";

		int _isInit = 0;
		mediapipe::CalculatorGraph _graph;
		std::unique_ptr<mediapipe::OutputStreamPoller> _pPoller;
		std::unique_ptr<mediapipe::OutputStreamPoller> _pPollerLandmarks;
		cv::VideoCapture capture;

		float faceLandmarkList[478 * 3];
		int size = 0;
	};
}

#endif