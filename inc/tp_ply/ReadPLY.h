#pragma once

#include "tp_ply/Globals.h"

#include <iosfwd>

namespace tp_utils
{
class Progress;
}

namespace tp_math_utils
{
struct Geometry3D;
}

namespace tp_ply
{

//##################################################################################################
bool TP_PLY_EXPORT readPLYFile(const std::string & filePath,
                               int triangleFan,
                               int triangleStrip,
                               int triangles,
                               bool reverse,
                               tp_math_utils::Geometry3D& outputGeometry,
                               tp_utils::Progress* progress);

//##################################################################################################
bool TP_PLY_EXPORT readPLYStream(std::istream& inputStream,
                                 int triangleFan,
                                 int triangleStrip,
                                 int triangles,
                                 bool reverse,
                                 tp_math_utils::Geometry3D& outputGeometry,
                                 tp_utils::Progress* progress);

}
