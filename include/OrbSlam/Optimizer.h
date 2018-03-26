/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <SLCVMap.h>
#include <SLCVMapPoint.h>
#include <SLCVKeyFrame.h>
#include <SLCVFrame.h>

#include <g2o/types/sim3/types_seven_dof_expmap.h>
#include <g2o/types/sba/types_six_dof_expmap.h>

namespace ORB_SLAM2
{

class Optimizer
{
public:
    void static BundleAdjustment(const std::vector<SLCVKeyFrame*> &vpKF, const std::vector<SLCVMapPoint*> &vpMP,
                                 int nIterations = 5, bool *pbStopFlag=NULL, const unsigned long nLoopKF=0,
                                 const bool bRobust = true);
    void static GlobalBundleAdjustemnt(SLCVMap* pMap, int nIterations=5, bool *pbStopFlag=NULL,
                                       const unsigned long nLoopKF=0, const bool bRobust = true);
    //void static LocalBundleAdjustment(SLCVKeyFrame* pKF, bool *pbStopFlag, SLCVMap *pMap);
    int static PoseOptimization(SLCVFrame* pFrame);

    //// if bFixScale is true, optimize SE3 (stereo,rgbd), Sim3 otherwise (mono)
    //static int OptimizeSim3(SLCVKeyFrame* pKF1, SLCVKeyFrame* pKF2, std::vector<SLCVMapPoint *> &vpMatches1,
    //                        g2o::Sim3 &g2oS12, const float th2, const bool bFixScale);
};

} //namespace ORB_SLAM

#endif // OPTIMIZER_H
