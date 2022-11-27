//#############################################################################
//  File:      SLFaceAnim.h
//  Date:      November 2022
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/SLProject-Coding-Style
//  Authors:   Dmytriy Pelts - bachelor thesis fall 2022
//  License:   This software is provided under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLFACEANIM_H
#define SLFACEANIM_H

#include <SLMesh.h>

class SLFaceAnim : public SLMesh
{
public:
    SLFaceAnim(SLAssetManager*);
    void draw(SLSceneView*, SLNode*);

    private:
    float* _pFacialLandmarks;

};


#endif