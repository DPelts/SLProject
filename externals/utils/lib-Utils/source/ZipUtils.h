//#############################################################################
//  File:      ZipUtils.h
//  Author:    Luc Girod
//  Date:      2020
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef CPLVRLAB_ZIP_UTILS_H
#define CPLVRLAB_ZIP_UTILS_H

#include <string>
#include <functional>

namespace ZipUtils
{
bool zip(string path, string zipname = "");

bool unzip(string                                       zipfile,
           function<bool(string path, string filename)> processFile,
           function<bool(const char* data, size_t len)> writeChunk,
           function<bool(string path)>                  processDir);

bool unzip(string path, string dest = "", bool override = true);
}
#endif
