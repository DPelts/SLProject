//#############################################################################
//  File:      AppDemoGuiMapStorage.cpp
//  Author:    Michael Goettlicher
//  Date:      April 2018
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <imgui.h>
#include <imgui_internal.h>

#include <AppWAI.h>
#include <Utils.h>
#include <AppDemoGuiMapStorage.h>

//-----------------------------------------------------------------------------
AppDemoGuiMapStorage::AppDemoGuiMapStorage(const string& name,
                                           SLNode*       mapNode,
                                           std::string   mapDir,
                                           bool*         activator)
  : AppDemoGuiInfosDialog(name, activator),
    _mapNode(mapNode),
    _mapPrefix("slam-map-"),
    _nextId(0)
{
    _mapDir = Utils::unifySlashes(mapDir);

    _existingMapNames.clear();
    vector<pair<int, string>> existingMapNamesSorted;

    //check if visual odometry maps directory exists
    if (!Utils::dirExists(_mapDir))
    {
        Utils::makeDir(_mapDir);
    }
    else
    {
        //parse content: we search for directories in mapsDir
        std::vector<std::string> content = Utils::getFileNamesInDir(_mapDir);
        for (auto path : content)
        {
            std::string name = Utils::getFileName(path);
            //find json files that contain mapPrefix and estimate highest used id
            if (Utils::containsString(name, _mapPrefix))
            {
                //estimate highest used id
                std::vector<std::string> splitted;
                Utils::splitString(name, '-', splitted);
                if (splitted.size())
                {
                    int id = atoi(splitted.back().c_str());
                    existingMapNamesSorted.push_back(make_pair(id, name));
                    if (id >= _nextId)
                    {
                        _nextId = id + 1;
                    }
                }
            }
        }
    }

    //sort existingMapNames
    std::sort(existingMapNamesSorted.begin(), existingMapNamesSorted.end(), [](const pair<int, string>& left, const pair<int, string>& right) { return left.first < right.first; });
    for (auto it = existingMapNamesSorted.begin(); it != existingMapNamesSorted.end(); ++it)
        _existingMapNames.push_back(it->second);
}

//-----------------------------------------------------------------------------
void AppDemoGuiMapStorage::buildInfos(SLScene* s, SLSceneView* sv)
{
    ImGui::Begin("Map storage", _activator, ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::Button("Save map", ImVec2(ImGui::GetContentRegionAvailWidth(), 0.0f)))
    {
        std::string filename;
        if (_currentItem.empty())
            filename = _mapPrefix + std::to_string(_nextId) + ".json";
        else
            filename = _currentItem;

        if (!Utils::dirExists(_mapDir))
            Utils::makeDir(_mapDir);

        if (WAIMapStorage::saveMap(WAIApp::mode->getMap(),
                                   _mapNode,
                                   _mapDir + filename))
        {
            ImGui::Text("Info: Map saved successfully");
        }
        else
        {
            ImGui::Text("Info: Failed to save map");
        }
    }

    ImGui::End();
}
