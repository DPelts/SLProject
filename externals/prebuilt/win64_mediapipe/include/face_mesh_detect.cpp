#include "face_mesh_detect.h"

#define IMG_WIDTH 640 * 2
#define IMG_HEIGHT 480 * 2

int mediapipe_face_mesh::face_mesh_detect::init_graph() noexcept
{
    absl::Status run_status = InitGraph();
    if (!run_status.ok())
    {
        return 0;
    }

    _isInit = 1;

    return 1;
}

int mediapipe_face_mesh::face_mesh_detect::update()
{
    if (!_isInit)
        return 0;

    absl::Status run_status = RunGraph();
    if (!run_status.ok())
    {
        return 0;
    }

    return 1;
}

int mediapipe_face_mesh::face_mesh_detect::release()
{
    absl::Status run_status = ReleaseGraph();

    if (!run_status.ok())
        return 0;

    return 1;
}

absl::Status mediapipe_face_mesh::face_mesh_detect::InitGraph()
{
    std::string p = getCurrentDir() + faceMeshPath;
    const char* path = p.c_str();

    std::string calculator_graph_config_contents;
    MP_RETURN_IF_ERROR(mediapipe::file::GetContents(path, &calculator_graph_config_contents));
    mediapipe::CalculatorGraphConfig config = mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(calculator_graph_config_contents);

    MP_RETURN_IF_ERROR(_graph.Initialize(config));
    capture.open(0);

    RET_CHECK(capture.isOpened());

    cv::namedWindow(kWindowName,1); // flags=WINDOW_AUTOSIZE
    capture.set(cv::CAP_PROP_FRAME_WIDTH, IMG_WIDTH);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, IMG_HEIGHT);
    capture.set(cv::CAP_PROP_FPS, 30);

    mediapipe::StatusOrPoller sop = _graph.AddOutputStreamPoller(kOutputStream);
    assert(sop.ok());
    _pPoller = std::make_unique<mediapipe::OutputStreamPoller>(std::move(sop.value()));
    
    mediapipe::StatusOrPoller sopLandmark = _graph.AddOutputStreamPoller(kOutputLandmarks);
    assert(sopLandmark.ok());
    _pPollerLandmarks = std::make_unique<mediapipe::OutputStreamPoller>(std::move(sopLandmark.value()));

    MP_RETURN_IF_ERROR(_graph.StartRun({}));

    return absl::OkStatus();
}

absl::Status mediapipe_face_mesh::face_mesh_detect::RunGraph()
{
    cv::Mat cameraFrameRaw;
    RET_CHECK(capture.isOpened());

    capture >> cameraFrameRaw;

    if (!cameraFrameRaw.empty())
    {
        cv::Mat camera_frame;
        cv::cvtColor(cameraFrameRaw, camera_frame, cv::COLOR_BGR2RGB);
        cv::flip(camera_frame, camera_frame, 1); // flipcode=HORIZONTAL

        auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
            mediapipe::ImageFormat::SRGB, camera_frame.cols, camera_frame.rows,
            mediapipe::ImageFrame::kDefaultAlignmentBoundary);
        cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
        camera_frame.copyTo(input_frame_mat);

        size_t frame_timestamp_us = (size_t)((double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6);
        MP_RETURN_IF_ERROR(_graph.AddPacketToInputStream(
            kInputStream, mediapipe::Adopt(input_frame.release())
            .At(mediapipe::Timestamp(frame_timestamp_us))));

        mediapipe::Packet packet;
        mediapipe::Packet packet_landmarks;
        if (_pPoller->Next(&packet))
        {
            auto& output_frame = packet.Get<mediapipe::ImageFrame>();

            // Convert back to opencv for display or saving.
            cv::Mat output_frame_mat = mediapipe::formats::MatView(&output_frame);
            cv::cvtColor(camera_frame, output_frame_mat, cv::COLOR_RGB2BGR);
            {
                // for (size_t i = 0; i < 478; i++)
                // {
                //     int pointer = i * 3;
                //     cv::Point p1 = cv::Point(faceLandmarkList[pointer] * IMG_WIDTH, faceLandmarkList[pointer + 1] * IMG_HEIGHT);
                //     std::string t = std::to_string(i);
                //     const char* text = t.c_str();
                //     cv::putText(camera_frame, text, p1, cv::FONT_HERSHEY_DUPLEX, 0.2f, CV_RGB(0, 255, 0), 1);
                //     // cv::circle(camera_frame, p1, 1.0f, CV_RGB(0, 0, 255));
                // }
                cv::imshow(kWindowName, output_frame_mat);
                const int pressed_key = cv::waitKey(5);
            }
            if (_pPollerLandmarks->QueueSize() > 0)
            {
                if (_pPollerLandmarks->Next(&packet_landmarks))
                {
                    auto& output_landmarklist = packet_landmarks.Get<std::vector<mediapipe::NormalizedLandmarkList>>();
                    for (auto& landmarks : output_landmarklist)
                    {
                        int const num_landmarks = landmarks.landmark_size();
                        size = num_landmarks * 3;
                        for (int i = 0; i < num_landmarks; ++i)
                        {
                            auto const& m = landmarks.landmark(i);
                            int pointer = i * 3;
                            faceLandmarkList[pointer] = m.x();
                            faceLandmarkList[pointer+1] = m.y();
                            faceLandmarkList[pointer+2] = m.z();

                            // cv::Point p1 = cv::Point(faceLandmarkList[pointer] * IMG_WIDTH, faceLandmarkList[pointer + 1] * IMG_HEIGHT);
                            // std::string t = std::to_string(i);
                            // const char* text = t.c_str();
                            // cv::putText(camera_frame, text, p1, cv::FONT_HERSHEY_DUPLEX, 0.1f, CV_RGB(0, 255, 0), 1);
                        }
                    }
                }
            }
            // cv::imshow(kWindowName, camera_frame);
            // const int pressed_key = cv::waitKey(5);
        }
    }

    return absl::OkStatus();
}

absl::Status mediapipe_face_mesh::face_mesh_detect::ReleaseGraph()
{
    cv::destroyWindow(kWindowName);

    MP_RETURN_IF_ERROR(_graph.CloseInputStream(kInputStream));
    MP_RETURN_IF_ERROR(_graph.CloseInputStream(kOutputStream));

    _isInit = 0;
    return _graph.WaitUntilDone();
}

/*
#include "face_mesh_detect.h"

int mediapipe_face_mesh::face_mesh_detect::init_graph() noexcept
{
    absl::Status run_status = InitGraph();
    if (!run_status.ok())
    {
        return 0;
    }

    _isInit = 1;

    return 1 ;
}

int mediapipe_face_mesh::face_mesh_detect::update()
{
    if (!_isInit)
        return 0;

    absl::Status run_status = RunGraph();
    if (!run_status.ok())
    {
        return 0;
    }

    return 1;
}

int mediapipe_face_mesh::face_mesh_detect::release()
{
    absl::Status run_status = ReleaseGraph();

    if (!run_status.ok())
        return 0;

    return 1;
}

absl::Status mediapipe_face_mesh::face_mesh_detect::InitGraph()
{
    std::string p = getCurrentDir() + faceMeshPath;
    const char* path = p.c_str();

    std::string calculator_graph_config_contents;
    MP_RETURN_IF_ERROR(mediapipe::file::GetContents(path, &calculator_graph_config_contents));
    mediapipe::CalculatorGraphConfig config = mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(calculator_graph_config_contents);

    MP_RETURN_IF_ERROR(_graph.Initialize(config));
    capture.open(0);

    RET_CHECK(capture.isOpened());

    cv::namedWindow(kWindowName, 1);
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    capture.set(cv::CAP_PROP_FPS, 30);

    mediapipe::StatusOrPoller sop = _graph.AddOutputStreamPoller(kOutputStream);
    assert(sop.ok());
    _pPoller = std::make_unique<mediapipe::OutputStreamPoller>(std::move(sop.value()));

    mediapipe::StatusOrPoller sopLandmark = _graph.AddOutputStreamPoller(kOutputLandmarks);
    assert(sopLandmark.ok());
    _pPollerLandmarks = std::make_unique<mediapipe::OutputStreamPoller>(std::move(sopLandmark.value()));

    MP_RETURN_IF_ERROR(_graph.StartRun({}));

    return absl::OkStatus();
}

absl::Status mediapipe_face_mesh::face_mesh_detect::RunGraph()
{
    cv::Mat cameraFrameRaw;
    RET_CHECK(capture.isOpened());

    capture >> cameraFrameRaw;

    if (!cameraFrameRaw.empty())
    {
        cv::Mat camera_frame;
        cv::cvtColor(cameraFrameRaw, camera_frame, cv::COLOR_BGR2RGB);
        cv::flip(camera_frame, camera_frame, 1);

        auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
            mediapipe::ImageFormat::SRGB, camera_frame.cols, camera_frame.rows,
            mediapipe::ImageFrame::kDefaultAlignmentBoundary);
        cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
        camera_frame.copyTo(input_frame_mat);

        size_t frame_timestamp_us = (size_t)((double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6);
        MP_RETURN_IF_ERROR(_graph.AddPacketToInputStream(
            kInputStream, mediapipe::Adopt(input_frame.release())
            .At(mediapipe::Timestamp(frame_timestamp_us))));

        mediapipe::Packet packet;
        mediapipe::Packet packet_landmarks;
        if (_pPoller->Next(&packet))
        {
            auto& output_frame = packet.Get<mediapipe::ImageFrame>();

            // Convert back to opencv for display or saving.
            cv::Mat output_frame_mat = mediapipe::formats::MatView(&output_frame);
            cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);
            {
                cv::imshow(kWindowName, output_frame_mat);
                const int pressed_key = cv::waitKey(5);
                // if (pressed_key >= 0 && pressed_key != 255)
            }
            if (_pPollerLandmarks->QueueSize() > 0)
            {
                if (_pPollerLandmarks->Next(&packet_landmarks))
                {
                    auto& output_landmarklist = packet_landmarks.Get<std::vector<mediapipe::NormalizedLandmarkList>>();
                    printf("\n/////\nNormalizedLandmarkList[%d]:\n", (int)output_landmarklist.size());
                    for (auto& landmarks : output_landmarklist)
                    {
                        int const num_landmarks = landmarks.landmark_size();
                        size = num_landmarks * 3;
                        for (int i = 0; i < num_landmarks; ++i)
                        {
                            auto const& m = landmarks.landmark(i);
                            int pointer = i * 3;
                            // printf(" output: [%d] %f, %f, %f\n", i, m.x(), m.y(), m.z());
                            faceLandmarkList[pointer] = m.x();
                            faceLandmarkList[pointer + 1] = m.y();
                            faceLandmarkList[pointer + 2] = m.z();
                        }
                    }
                    //printf("/////\n");
                }
            }

            // for (int i = 0; i < 478; i++)
            // {
            //     int pointer = i * 3;
            //     printf(" output: [%d] %f, %f, %f\n", i, faceLandmarkList[i], faceLandmarkList[i+1], faceLandmarkList[i+2]);
            // }
            // 
            // printf("/////\n");

            // if (_pPollerLandmarks->QueueSize() > 0)
            // {
            //     if (_pPollerLandmarks->Next(&packet))
            //     {
            //         auto& output_landmarklist = packet.Get<std::vector<mediapipe::NormalizedLandmarkList>>();
            //         printf("\n/////\nNormalizedLandmarkList[%d]:\n", (int)output_landmarklist.size());
            //         for (auto& landmarks : output_landmarklist)
            //         {
            //             int const num_landmarks = landmarks.landmark_size();
            //             for (int i = 0; i < num_landmarks; ++i)
            //             {
            //                 auto const& m = landmarks.landmark(i);
            //                 printf(" output: [%d] %f, %f, %f\n", i, m.x(), m.y(), m.z());
            //             }
            //         }
            //         printf("/////\n");
            //     }
            // 
            // }
        }
    }

    return absl::OkStatus();
}

absl::Status mediapipe_face_mesh::face_mesh_detect::ReleaseGraph()
{
    cv::destroyWindow(kWindowName);

    MP_RETURN_IF_ERROR(_graph.CloseInputStream(kInputStream));
    MP_RETURN_IF_ERROR(_graph.CloseInputStream(kOutputStream));

    _isInit = 0;
    return _graph.WaitUntilDone();
}

*/