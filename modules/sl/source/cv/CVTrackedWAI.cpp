//#############################################################################
//  File:      CVTrackedWAI.cpp
//  Date:      Spring 2020
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/SLProject-Coding-Style
//  Authors:   Michael Goettlicher, Jan Dellsperger
//  License:   This software is provided under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <cv/CVTrackedWAI.h>
#include <SL.h>
#include <Profiler.h>

//-----------------------------------------------------------------------------
CVTrackedWAI::CVTrackedWAI(const string& vocabularyFile)
{
    _voc          = new WAIOrbVocabulary();
    float startMS = _timer.elapsedTimeInMilliSec();

    try
    {
        _voc->loadFromFile(vocabularyFile);
    }
    catch (std::exception& e)
    {
        Utils::log("SLProject",
                   "Could not open the ORB vocabulary file: %s",
                   e.what());
        exit(0);
    }
    SL_LOG("Loaded voc file : %f ms", _timer.elapsedTimeInMilliSec() - startMS);
}
//-----------------------------------------------------------------------------
CVTrackedWAI::~CVTrackedWAI()
{
    delete _trackingExtractor;
    delete _waiSlamer;
}
//-----------------------------------------------------------------------------
/*! This function is called every frame in the apps onUpdateVideo function.
 It uses the on-the-fly built 3D point cloud generated by the ORB-SLAM2 library
 that is integrated within the lib-WAI. It only works well if the camera is
 calibrated.
 @param imageGray Image for processing
 @param imageRgb Image for visualizations
 @param calib Pointer to a valid camera calibration
 @return returns true if pose estimation was successful
 */
bool CVTrackedWAI::track(CVMat          imageGray,
                         CVMat          imageRgb,
                         CVCalibration* calib)
{
    PROFILE_FUNCTION();

    bool result = false;

    float startMS = _timer.elapsedTimeInMilliSec();

    if (!_waiSlamer)
    {
        if (_voc == nullptr)
            return false;

        int   nf           = 1000; // NO. of features
        float fScaleFactor = 1.2f; // Scale factor for pyramid construction
        int   nLevels      = 8;    // NO. of pyramid levels
        int   fIniThFAST   = 20;   // Init threshold for FAST corner detector
        int   fMinThFAST   = 7;    // Min. threshold for FAST corner detector
        _trackingExtractor = new ORB_SLAM2::ORBextractor(nf,
                                                         fScaleFactor,
                                                         nLevels,
                                                         fIniThFAST,
                                                         fMinThFAST);

        _initializationExtractor = new ORB_SLAM2::ORBextractor(2 * nf,
                                                               fScaleFactor,
                                                               nLevels,
                                                               fIniThFAST,
                                                               fMinThFAST);

        WAISlam::Params params;
        params.cullRedundantPerc   = 0.95f;
        params.ensureKFIntegration = false;
        params.fixOldKfs           = false;
        params.onlyTracking        = false;
        params.retainImg           = false;
        params.serial              = false;
        params.trackOptFlow        = false;

        _waiSlamer = new WAISlam(calib->cameraMat(),
                                 calib->distortion(),
                                 _voc,
                                 _initializationExtractor,
                                 _trackingExtractor,
                                 _trackingExtractor,
                                 nullptr, // global map
                                 params);
    }

    if (_waiSlamer->update(imageGray))
    {
        cv::Mat pose = _waiSlamer->getPose();

        // ORB-SLAM uses a right handed Y-down coordinate system
        // SLProject uses a right handed Y-Up coordinate system
        cv::Mat rot         = cv::Mat::eye(4, 4, CV_32F);
        rot.at<float>(1, 1) = -1.0f; // mirror y-axis
        rot.at<float>(2, 2) = -1.0f; // mirror z-axis

        pose = rot * pose;

        pose.copyTo(_objectViewMat);

        result = true;
    }

    if (_drawDetection)
    {
        _waiSlamer->drawInfo(imageRgb, 1.0f, true, true, true);
    }

    // TODO(dgj1): at the moment we cant differentiate between these two
    // as they are both done in the same call to WAI
    CVTracked::detectTimesMS.set(_timer.elapsedTimeInMilliSec() - startMS);
    CVTracked::poseTimesMS.set(_timer.elapsedTimeInMilliSec() - startMS);

    return result;
}
//-----------------------------------------------------------------------------
