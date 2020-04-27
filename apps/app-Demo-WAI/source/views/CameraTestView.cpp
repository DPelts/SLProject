#include <views/CameraTestView.h>

CameraTestView::CameraTestView(sm::EventHandler&   eventHandler,
                               SLInputManager&     inputManager,
                               ErlebAR::Resources& resources,
                               SENSCamera*         sensCamera,
                               int                 screenWidth,
                               int                 screenHeight,
                               int                 dotsPerInch,
                               std::string         fontPath,
                               std::string         imguiIniPath)
  : SLSceneView(nullptr, dotsPerInch, inputManager),
    _gui(eventHandler,
         resources,
         dotsPerInch,
         screenWidth,
         screenHeight,
         fontPath,
         std::bind(&CameraTestView::startCamera, this),
         std::bind(&CameraTestView::stopCamera, this)),
    _scene("CameraTestScene"),
    _camera(sensCamera)
{
    scene(&_scene);
    init("CameraTestView", screenWidth, screenHeight, nullptr, nullptr, &_gui, imguiIniPath);
    onInitialize();
    //set the cameraNode as active camera so that we see the video image
    camera(_scene.cameraNode);
}

bool CameraTestView::update()
{
    if (_camera->started() && _camera->permissionGranted())
    {
        SENSFramePtr frame = _camera->getLatestFrame();
        if (frame)
            _scene.updateVideoImage(frame->imgRGB);
    }

    return onPaint();
}

void CameraTestView::startCamera()
{
    if (!_camera)
        return;

    SENSCamera::Config config;
    config.targetWidth   = 640;
    config.targetHeight  = 360;
    config.convertToGray = true;

    _camera->init(SENSCamera::Facing::BACK);
    _camera->start(config);
}

void CameraTestView::stopCamera()
{
    if (!_camera)
        return;

    _camera->stop();
}
