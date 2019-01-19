#ifndef tp_ply_ReadPLY_h
#define tp_ply_ReadPLY_h

#include "tp_ply/Globals.h"

#include <iosfwd>

namespace tp_math_utils
{
struct Geometry3D;
}

namespace tp_ply
{

//##################################################################################################
void readPLYFile(const std::string & filePath,
                 std::string& error,
                 int triangleFan,
                 int triangleStrip,
                 int triangles,
                 bool reverse,
                 tp_math_utils::Geometry3D& outputGeometry);

//##################################################################################################
void readPLYStream(std::istream& inputStream,
                   std::string& error,
                   int triangleFan,
                   int triangleStrip,
                   int triangles,
                   bool reverse,
                   tp_math_utils::Geometry3D& outputGeometry);

}

#endif
