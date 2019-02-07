#include "tp_ply/WritePLY.h"

#include "tp_math_utils/Geometry3D.h"

#include "tinyply.h"

#include <fstream>

namespace tp_ply
{

//##################################################################################################
void writePLYFile(const std::string & filePath,
                  std::string& error,
                  const tp_math_utils::Geometry3D& geometry)
{
  try
  {
    std::ofstream ss(filePath, std::ios::binary);
    if(ss.fail())
    {
      error = "failed to open: " + filePath;
      return;
    }

    writePLYStream(ss, error, geometry);
  }
  catch (const std::exception & e)
  {
    error = std::string("Caught tinyply exception: ") + e.what();
  }
}


//##################################################################################################
void writePLYStream(std::ostream& outputStream,
                    std::string& error,
                    const tp_math_utils::Geometry3D& geometry)
{
  TP_UNUSED(error);

  std::vector<glm::vec3> verts   ;
  std::vector<glm::vec3> normals ;
  std::vector<glm::vec4> colors  ;
  std::vector<glm::vec2> textures;

  verts   .reserve(geometry.verts.size());
  normals .reserve(geometry.verts.size());
  colors  .reserve(geometry.verts.size());
  textures.reserve(geometry.verts.size());

  for(const auto& vert : geometry.verts)
  {
    verts   .emplace_back(vert.vert   );
    normals .emplace_back(vert.normal );
    colors  .emplace_back(vert.color  );
    textures.emplace_back(vert.texture);
  }

  tinyply::PlyFile file;

  file.add_properties_to_element(
        "vertex", { "x", "y", "z" },
        tinyply::Type::FLOAT32, verts.size(), reinterpret_cast<uint8_t*>   (verts.data()), tinyply::Type::INVALID, 0);

  file.add_properties_to_element(
        "vertex", { "nx", "ny", "nz" },
        tinyply::Type::FLOAT32, verts.size(), reinterpret_cast<uint8_t*> (normals.data()), tinyply::Type::INVALID, 0);

  file.add_properties_to_element(
        "vertex", { "red", "green", "blue", "alpha" },
        tinyply::Type::FLOAT32, verts.size(), reinterpret_cast<uint8_t*>  (colors.data()), tinyply::Type::INVALID, 0);

  file.add_properties_to_element(
        "vertex", { "s", "t" },
        tinyply::Type::FLOAT32, verts.size(), reinterpret_cast<uint8_t*>(textures.data()), tinyply::Type::INVALID, 0);

  file.add_properties_to_element(
        "vertex", { "u", "v" },
        tinyply::Type::FLOAT32, verts.size(), reinterpret_cast<uint8_t*>(textures.data()), tinyply::Type::INVALID, 0);

  for(const auto& indexes : geometry.indexes)
  {
    auto listType = tinyply::Type::INVALID;
    size_t stride=0;
    size_t indexCount = indexes.indexes.size();

    std::string type = "face";
    if(indexes.type == geometry.triangleStrip)
      type = "tristrips";
    else if(indexes.type == geometry.triangleFan)
      type = "trifans";
    else if(indexes.type == geometry.triangles)
    {
      listType = tinyply::Type::UINT8;
      stride = 3;
      indexCount = indexCount/3;
      type = "face";
    }

    file.add_properties_to_element(
          type, { "vertex_indices" },
          tinyply::Type::INT32,
          indexCount,
          const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(indexes.indexes.data())),
          listType,
          stride);
  }

  file.write(outputStream, true);
}

}
