#include "ErlebARApp.h"
#include <SLGLProgram.h>
#include <SLGLTexture.h>
#include <SLAssimpImporter.h>
#include <views/SelectionView.h>
#include <views/TestView.h>
#include <views/StartUpView.h>
#include <views/WelcomeView.h>
#include <views/SettingsView.h>
#include <views/AboutView.h>
#include <views/TutorialView.h>
#include <views/AreaTrackingView.h>
#include <views/LocationMapView.h>
#include <views/AreaInfoView.h>

#include <SLGLProgramManager.h>

#define LOG_ERLEBAR_WARN(...) Utils::log("ErlebARApp", __VA_ARGS__);
#define LOG_ERLEBAR_INFO(...) Utils::log("ErlebARApp", __VA_ARGS__);
#define LOG_ERLEBAR_DEBUG(...) Utils::log("ErlebARApp", __VA_ARGS__);

ErlebARApp::ErlebARApp()
  : sm::StateMachine((unsigned int)StateId::IDLE),
    SLInputEventInterface(_inputManager)
{
    registerState<ErlebARApp, sm::NoEventData, &ErlebARApp::IDLE>((unsigned int)StateId::IDLE);
    registerState<ErlebARApp, InitEventData, &ErlebARApp::INIT>((unsigned int)StateId::INIT);
    registerState<ErlebARApp, sm::NoEventData, &ErlebARApp::WELCOME>((unsigned int)StateId::WELCOME);
    registerState<ErlebARApp, sm::NoEventData, &ErlebARApp::DESTROY>((unsigned int)StateId::DESTROY);
    registerState<ErlebARApp, sm::NoEventData, &ErlebARApp::SELECTION>((unsigned int)StateId::SELECTION);

    registerState<ErlebARApp, sm::NoEventData, &ErlebARApp::START_TEST>((unsigned int)StateId::START_TEST);
    registerState<ErlebARApp, sm::NoEventData, &ErlebARApp::TEST>((unsigned int)StateId::TEST);
    registerState<ErlebARApp, sm::NoEventData, &ErlebARApp::HOLD_TEST>((unsigned int)StateId::HOLD_TEST);
    registerState<ErlebARApp, sm::NoEventData, &ErlebARApp::RESUME_TEST>((unsigned int)StateId::RESUME_TEST);

    registerState<ErlebARApp, ErlebarEventData, &ErlebARApp::LOCATION_MAP>((unsigned int)StateId::LOCATION_MAP);
    registerState<ErlebARApp, AreaEventData, &ErlebARApp::AREA_INFO>((unsigned int)StateId::AREA_INFO);
    registerState<ErlebARApp, AreaEventData, &ErlebARApp::AREA_TRACKING>((unsigned int)StateId::AREA_TRACKING);
    registerState<ErlebARApp, sm::NoEventData, &ErlebARApp::HOLD_TRACKING>((unsigned int)StateId::HOLD_TRACKING);

    registerState<ErlebARApp, sm::NoEventData, &ErlebARApp::TUTORIAL>((unsigned int)StateId::TUTORIAL);
    registerState<ErlebARApp, sm::NoEventData, &ErlebARApp::SETTINGS>((unsigned int)StateId::SETTINGS);
    registerState<ErlebARApp, sm::NoEventData, &ErlebARApp::ABOUT>((unsigned int)StateId::ABOUT);
    registerState<ErlebARApp, sm::NoEventData, &ErlebARApp::CAMERA_TEST>((unsigned int)StateId::CAMERA_TEST);
}

void ErlebARApp::init(int            scrWidth,
                      int            scrHeight,
                      float          scr2fbX,
                      float          scr2fbY,
                      int            dpi,
                      AppDirectories dirs,
                      SENSCamera*    camera)
{
    //store camera so we can stop on terminate
    _camera = camera;
    addEvent(new InitEvent(scrWidth, scrHeight, scr2fbX, scr2fbY, dpi, dirs));
}

void ErlebARApp::goBack()
{
    addEvent(new GoBackEvent());
}

void ErlebARApp::destroy()
{
    addEvent(new DestroyEvent());
}

void ErlebARApp::hold()
{
    addEvent(new HoldEvent());
}

void ErlebARApp::resume()
{
    addEvent(new ResumeEvent());
}

std::string ErlebARApp::getPrintableState(unsigned int state)
{
    StateId stateId = (StateId)state;
    switch (stateId)
    {
        case StateId::IDLE:
            return "IDLE";
        case StateId::INIT:
            return "INIT";
        case StateId::WELCOME:
            return "WELCOME";
        case StateId::DESTROY:
            return "DESTROY";
        case StateId::SELECTION:
            return "SELECTION";

        case StateId::START_TEST:
            return "START_TEST";
        case StateId::TEST:
            return "TEST";
        case StateId::HOLD_TEST:
            return "HOLD_TEST";
        case StateId::RESUME_TEST:
            return "RESUME_TEST";

        case StateId::LOCATION_MAP:
            return "LOCATION_MAP";
        case StateId::AREA_INFO:
            return "AREA_INFO";
        case StateId::AREA_TRACKING:
            return "AREA_TRACKING";
        case StateId::HOLD_TRACKING:
            return "HOLD_TRACKING";

        case StateId::TUTORIAL:
            return "TUTORIAL";
        case StateId::ABOUT:
            return "ABOUT";
        case StateId::SETTINGS:
            return "SETTINGS";
        default: {
            std::stringstream ss;
            ss << "Undefined state or missing string in ErlebARApp::getPrintableState for id: " << state << "!";
            return ss.str();
        }
    }
}

void ErlebARApp::IDLE(const sm::NoEventData* data, const bool stateEntry)
{
    if (stateEntry)
        LOG_ERLEBAR_DEBUG("IDLE");
}

void ErlebARApp::INIT(const InitEventData* data, const bool stateEntry)
{
    if (stateEntry)
        LOG_ERLEBAR_DEBUG("INIT");

    assert(data != nullptr);

    const DeviceData&  dd         = data->deviceData;
    const std::string& slDataRoot = data->deviceData.dirs().slDataRoot;
    // setup magic paths
    SLGLProgram::defaultPath      = slDataRoot + "/shaders/";
    SLGLTexture::defaultPath      = slDataRoot + "/images/textures/";
    SLGLTexture::defaultPathFonts = slDataRoot + "/images/fonts/";
    SLAssimpImporter::defaultPath = slDataRoot + "/models/";

    _resources = new ErlebAR::Resources(dd.dirs().writableDir + "ErlebARResources.json",
                                        dd.textureDir());

    _welcomeView = new WelcomeView(_inputManager,
                                   *_resources,
                                   dd.scrWidth(),
                                   dd.scrHeight(),
                                   dd.dpi(),
                                   dd.fontDir(),
                                   dd.textureDir(),
                                   dd.dirs().writableDir,
                                   "0.12");

    //instantiation of views
    _selectionView = new SelectionView(*this,
                                       _inputManager,
                                       *_resources,
                                       dd.scrWidth(),
                                       dd.scrHeight(),
                                       dd.dpi(),
                                       dd.fontDir(),
                                       dd.textureDir(),
                                       dd.dirs().writableDir);

    _testView = new TestView(*this,
                             _inputManager,
                             _camera,
                             dd.scrWidth(),
                             dd.scrHeight(),
                             dd.dpi(),
                             dd.fontDir(),
                             dd.dirs().writableDir,
                             dd.dirs().vocabularyDir,
                             dd.calibDir(),
                             dd.videoDir());

    _startUpView = new StartUpView(_inputManager,
                                   dd.scrWidth(),
                                   dd.scrHeight(),
                                   dd.dpi(),
                                   dd.dirs().writableDir);

    _aboutView = new AboutView(*this,
                               _inputManager,
                               *_resources,
                               dd.scrWidth(),
                               dd.scrHeight(),
                               dd.dpi(),
                               dd.fontDir(),
                               dd.dirs().writableDir);

    _settingsView = new SettingsView(*this,
                                     _inputManager,
                                     *_resources,
                                     dd.scrWidth(),
                                     dd.scrHeight(),
                                     dd.dpi(),
                                     dd.fontDir(),
                                     dd.dirs().writableDir);

    _tutorialView = new TutorialView(*this,
                                     _inputManager,
                                     *_resources,
                                     dd.scrWidth(),
                                     dd.scrHeight(),
                                     dd.dpi(),
                                     dd.fontDir(),
                                     dd.dirs().writableDir,
                                     dd.textureDir());

    _locationMapView = new LocationMapView(*this,
                                           _inputManager,
                                           *_resources,
                                           dd.scrWidth(),
                                           dd.scrHeight(),
                                           dd.dpi(),
                                           dd.fontDir(),
                                           dd.dirs().writableDir,
                                           dd.erlebARDir());

    _areaInfoView = new AreaInfoView(*this,
                                     _inputManager,
                                     *_resources,
                                     dd.scrWidth(),
                                     dd.scrHeight(),
                                     dd.dpi(),
                                     dd.fontDir(),
                                     dd.dirs().writableDir);

    _areaTrackingView = new AreaTrackingView(*this,
                                             _inputManager,
                                             *_resources,
                                             _camera,
                                             dd.scrWidth(),
                                             dd.scrHeight(),
                                             dd.dpi(),
                                             dd.fontDir(),
                                             dd.dirs().writableDir);
    addEvent(new DoneEvent());
}

void ErlebARApp::WELCOME(const sm::NoEventData* data, const bool stateEntry)
{
    static HighResTimer timer;
    if (stateEntry)
    {
        timer.start();
    }

    _welcomeView->update();

    if (timer.elapsedTimeInSec() > 2.f)
        addEvent(new DoneEvent());
}

void ErlebARApp::DESTROY(const sm::NoEventData* data, const bool stateEntry)
{
    if (_selectionView)
    {
        delete _selectionView;
        _selectionView = nullptr;
    }
    if (_testView)
    {
        delete _testView;
        _testView = nullptr;
    }
    if (_startUpView)
    {
        delete _startUpView;
        _startUpView = nullptr;
    }
    if (_welcomeView)
    {
        delete _welcomeView;
        _welcomeView = nullptr;
    }
    if (_aboutView)
    {
        delete _aboutView;
        _aboutView = nullptr;
    }
    if (_settingsView)
    {
        delete _settingsView;
        _settingsView = nullptr;
    }
    if (_tutorialView)
    {
        delete _tutorialView;
        _tutorialView = nullptr;
    }
    if (_locationMapView)
    {
        delete _locationMapView;
        _locationMapView = nullptr;
    }
    if (_areaInfoView)
    {
        delete _areaInfoView;
        _areaInfoView = nullptr;
    }
    if (_areaTrackingView)
    {
        delete _areaTrackingView;
        _areaTrackingView = nullptr;
    }

    if (_camera)
    {
        if (_camera->started())
        {
            _camera->stop();
        }
    }
    if (_resources)
    {
        delete _resources;
        _resources = nullptr;
    }

    //ATTENTION: if we dont do this we get problems when opening the app the second time
    //(e.g. "The position attribute has no variable location." from SLGLVertexArray)
    //We still cannot get rid of this stupid singleton instance..
    //Todo: if we have a Renderer once, we can use this clean up opengl specific stuff
    SLGLProgramManager::deletePrograms();
    SLMaterialDefaultGray::deleteInstance();
    SLMaterialDiffuseAttribute::deleteInstance();

    if (_closeCB)
    {
        LOG_ERLEBAR_DEBUG("Close Callback!");
        _closeCB();
    }

    addEvent(new DoneEvent());
}

void ErlebARApp::SELECTION(const sm::NoEventData* data, const bool stateEntry)
{
    _selectionView->update();
}

void ErlebARApp::START_TEST(const sm::NoEventData* data, const bool stateEntry)
{
    if (stateEntry)
    {
        //start camera
        SENSCamera::Config config;
        config.targetWidth   = 640;
        config.targetHeight  = 360;
        config.convertToGray = true;

        _camera->init(SENSCamera::Facing::BACK);
        _camera->start(config);
    }

    if (_camera->permissionGranted() && _camera->started())
    {
        _testView->start();
        addEvent(new DoneEvent());
    }

    assert(_startUpView != nullptr);
    _startUpView->update();
}

void ErlebARApp::TEST(const sm::NoEventData* data, const bool stateEntry)
{
    if (stateEntry)
    {
        _testView->show();
    }
    _testView->update();
}

void ErlebARApp::HOLD_TEST(const sm::NoEventData* data, const bool stateEntry)
{
    if (stateEntry)
    {
        _camera->stop();
    }
}

void ErlebARApp::RESUME_TEST(const sm::NoEventData* data, const bool stateEntry)
{
    //start camera
    SENSCamera::Config config;
    config.targetWidth   = 640;
    config.targetHeight  = 360;
    config.convertToGray = true;

    _camera->init(SENSCamera::Facing::BACK);
    _camera->start(config);

    addEvent(new DoneEvent());
}

void ErlebARApp::LOCATION_MAP(const ErlebarEventData* data, const bool stateEntry)
{
    if (stateEntry)
    {
        if (data)
            _locationMapView->initLocation(data->location);
    }

    _locationMapView->update();
}

void ErlebARApp::AREA_INFO(const AreaEventData* data, const bool stateEntry)
{
    if (stateEntry)
    {
        _areaInfoView->show();
        //We use the area info view to initialize the area tracking. It is a convention
        //for this state, that if there is no data sent, we assume that the previous state was HOLD_TRACKING.
        if (data)
        {
            _areaInfoView->initArea(data->locId, data->areaId);
            _areaTrackingView->initArea(data->locId, data->areaId);
        }
        else
        {
            _areaTrackingView->resume();
        }
    }

    _areaInfoView->update();
}

void ErlebARApp::AREA_TRACKING(const AreaEventData* data, const bool stateEntry)
{
    if (stateEntry)
    {
        //It is a convention for this state, that if there is no data sent,
        //we assume that the previous state was HOLD_TRACKING.
        if (data)
        {
            //_areaTrackingView->initArea(data->locId, data->areaId);
        }
        else
        {
            _areaTrackingView->resume();
        }
    }

    _areaTrackingView->update();
}

void ErlebARApp::HOLD_TRACKING(const sm::NoEventData* data, const bool stateEntry)
{
}

void ErlebARApp::TUTORIAL(const sm::NoEventData* data, const bool stateEntry)
{
    _tutorialView->update();
}

void ErlebARApp::ABOUT(const sm::NoEventData* data, const bool stateEntry)
{
    if (stateEntry)
    {
        _aboutView->show();
    }

    _aboutView->update();
}

void ErlebARApp::SETTINGS(const sm::NoEventData* data, const bool stateEntry)
{
    if (stateEntry)
    {
        _settingsView->show();
    }

    _settingsView->update();
}

void ErlebARApp::CAMERA_TEST(const sm::NoEventData* data, const bool stateEntry)
{
}
