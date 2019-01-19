#ifndef tp_ply_WritePLY_h
#define tp_ply_WritePLY_h

#include "tp_ply/Globals.h"

#include <iosfwd>

namespace tp_math_utils
{
struct Geometry3D;
}

namespace tp_ply
{

//##################################################################################################
void writePLYFile(const std::string & filePath,
                  std::string& error,
                  const tp_math_utils::Geometry3D& geometry);

//##################################################################################################
void writePLYStream(std::ostream& outputStream,
                    std::string& error,
                    const tp_math_utils::Geometry3D& geometry);

}

#endif
