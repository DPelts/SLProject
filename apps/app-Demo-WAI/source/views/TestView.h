#ifndef TEST_VIEW_H
#define TEST_VIEW_H

#include <scenes/AppWAIScene.h>
#include <SLSceneView.h>
#include <SLTransformNode.h>
#include <AppDemoWaiGui.h>
#include <SlamParams.h>
#include <AppDemoGuiSlamLoad.h>
#include <SENSVideoStream.h>
#include <CVCalibration.h>
#include <WAIOrbVocabulary.h>
#include <queue>
#include <ImageBuffer.h>
#include <WAIAutoCalibration.h>
#include <MapPointEdition.h>
#include <SENSCalibration.h>

class WAISlam;
struct WAIEvent;
class SENSCamera;

class TestView : protected SLSceneView
{
public:
    TestView(sm::EventHandler&   eventHandler,
             SLInputManager&     inputManager,
             const ImGuiEngine&  imGuiEngine,
             ErlebAR::Resources& resources,
             SENSCamera*         camera,
             const DeviceData&   deviceData);
    ~TestView();

    bool update();
    //try to load slam params and start slam
    void start();
    //void postStart();
    //call when view becomes visible
    void show() { _gui.onShow(); }

protected:
    void tryLoadLastSlam();
    void handleEvents();
    void loadWAISceneView(std::string location, std::string area);
    void saveMap(std::string location, std::string area, std::string marker);
    void saveVideo(std::string filename);
    void startOrbSlam(SlamParams slamParams);
    void transformMapNode(SLTransformSpace tSpace,
                          SLVec3f          rotation,
                          SLVec3f          translation,
                          float            scale);
    void downloadCalibrationFilesTo(std::string dir);

    void updateVideoTracking();
    void updateTrackingVisualization(const bool iKnowWhereIAm);
    void updateTrackingVisualization(const bool iKnowWhereIAm, SENSFrame& frame);
    void setupDefaultErlebARDirTo(std::string dir);
    void updateSceneCameraFov();

    //video
    //CVCalibration                    _calibration       = {CVCameraType::FRONTFACING, ""};
    std::unique_ptr<SENSCalibration> _calibrationLoaded;
    SENSCamera*                      _camera      = nullptr;
    cv::VideoWriter*                 _videoWriter = nullptr;
    std::unique_ptr<SENSVideoStream> _videoFileStream;
    bool                             _pauseVideo           = false;
    int                              _videoCursorMoveIndex = 0;
    bool                             _showUndistorted      = true;
    cv::Size2i                       _videoFrameSize;

    //slam
    WAISlam*   _mode = nullptr;
    SlamParams _currentSlamParams;

    FeatureExtractorFactory      _featureExtractorFactory;
    std::unique_ptr<KPextractor> _trackingExtractor;
    std::unique_ptr<KPextractor> _initializationExtractor;
    std::unique_ptr<KPextractor> _markerExtractor;
    ImageBuffer                  _imgBuffer;

    std::queue<WAIEvent*> _eventQueue;

    //scene
    AppWAIScene       _scene;
    WAIOrbVocabulary* _voc;

    SLAssetManager _assets;

    std::string _configDir;
    std::string _vocabularyDir;
    std::string _calibDir;
    std::string _videoDir;
    std::string _dataDir;

    bool             _fillAutoCalibration;
    AutoCalibration* _autoCal = nullptr;

    SLTransformNode* _transformationNode = nullptr;
    MapEdition*      _mapEdition         = nullptr;

    //gui (declaration down here because it depends on a lot of members in initializer list of constructor)
    AppDemoWaiGui _gui;
};

#endif //TEST_VIEW_H
