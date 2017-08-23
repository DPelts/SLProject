//#############################################################################
//  File:      SLCVFeatureManager.h
//  Author:    Marcus Hudritsch
//  Date:      Autumn 2016
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLCVFEATUREMANAGER_H
#define SLCVFEATUREMANAGER_H

#include <SLCV.h>

//-----------------------------------------------------------------------------
//! Wrapper class around OpenCV feature detector & describer
class SLCVFeatureManager
{
    public:
                    SLCVFeatureManager      ();
                   ~SLCVFeatureManager      ();

        void        deleteAll               ();

        void        detect                  (SLCVInputArray image,
                                             SLCVVKeyPoint &keypoints,
                                             SLCVInputArray mask = cv::noArray());

        void        describe                (SLCVInputArray  image,
                                             SLCVVKeyPoint&  keypoints,
                                             SLCVOutputArray descriptors);

        void        detectAndDescribe       (SLCVInputArray  image,
                                             SLCVVKeyPoint&  keypoints,
                                             SLCVOutputArray descriptors,
                                             SLCVInputArray  mask=cv::noArray());

        void        createDetectorDescriptor(SLCVDetectDescribeType detectDescribeType);

        void        setDetectorDescriptor   (SLCVDetectDescribeType detectDescribeType,
                                             SLCVFeatureDetector* detector,
                                             SLCVDescriptorExtractor* descriptor);
        // Getter
        SLCVDetectDescribeType type         () {return _type;}

    private:
        SLCVDetectDescribeType      _type;          //!< Type of detector-descriptor pair
        SLCVFeatureDetector*        _detector;      //!< Pointer to the OpenCV feature detector
        SLCVDescriptorExtractor*    _descriptor;    //!< Pointer to the OpenCV descriptor extractor
};
//-----------------------------------------------------------------------------
#endif // SLCVDETECTOR_H
