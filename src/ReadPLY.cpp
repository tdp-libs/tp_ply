#include "tp_ply/ReadPLY.h"

#include "tp_math_utils/Geometry3D.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/FileUtils.h"
#include "tp_utils/Progress.h"

#include "tinyply.h"

#include <memory>
#include <fstream>
#include <type_traits>
#include <cstring>

namespace tp_ply
{

namespace
{

//##################################################################################################
struct ElementDetails_lt
{
  std::shared_ptr<tinyply::PlyData> vertices;
  std::shared_ptr<tinyply::PlyData> normals;
  std::shared_ptr<tinyply::PlyData> texcoords;
  std::shared_ptr<tinyply::PlyData> colors;
  std::shared_ptr<tinyply::PlyData> alpha;

  std::shared_ptr<tinyply::PlyData> vertexIndices;

  //GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP, GL_TRIANGLES
  int geometryType=0;
};

//##################################################################################################
std::shared_ptr<tinyply::PlyData> requestProperties(tinyply::PlyFile& file,
                                                    std::string& error,
                                                    const std::string & elementKey,
                                                    const std::initializer_list<std::string> propertyKeys,
                                                    const uint32_t list_size_hint = 0)
{
  try
  {
    return file.request_properties_from_element(elementKey, propertyKeys, list_size_hint);
  }
  catch (const std::exception & e)
  {
    error = std::string("tinyply exception: ") + e.what();
    return nullptr;
  }
}

//##################################################################################################
//! Parse an element and request useful properties from it.
void parseElement(tinyply::PlyFile& file,
                  std::string& error,
                  const tinyply::PlyElement& element,
                  ElementDetails_lt& elementDetails)
{
  int gotVertices      = 0;
  int gotNormals       = 0;
  int gotTexture       = 0;
  int gotColor         = 0;
  int gotAlpha         = 0;

  int gotVertexIndices = 0;

  for(const auto& p : element.properties)
  {
    if(p.name == "x")
      gotVertices++;
    else if(p.name == "y")
      gotVertices++;
    else if(p.name == "z")
      gotVertices++;

    else if(p.name == "nx")
      gotNormals++;
    else if(p.name == "ny")
      gotNormals++;
    else if(p.name == "nz")
      gotNormals++;

    else if(p.name == "u")
      gotTexture++;
    else if(p.name == "v")
      gotTexture++;

    else if(p.name == "red")
      gotColor++;
    else if(p.name == "green")
      gotColor++;
    else if(p.name == "blue")
      gotColor++;

    else if(p.name == "alpha")
      gotAlpha++;

    else if(p.name == "vertex_indices")
      gotVertexIndices++;
  }

  if(gotVertices == 3)
    elementDetails.vertices = requestProperties(file, error, element.name, {"x", "y", "z"});

  if(gotNormals == 3)
    elementDetails.normals = requestProperties(file, error, element.name, {"nx", "ny", "nz"});

  if(gotTexture == 2)
    elementDetails.texcoords = requestProperties(file, error, element.name, {"u", "v"});

  if(gotColor == 3)
    elementDetails.colors = requestProperties(file, error, element.name, {"red", "green", "blue"});

  if(gotAlpha == 1)
    elementDetails.alpha = requestProperties(file, error, element.name, {"alpha"});

  if(gotVertexIndices == 1)
    elementDetails.vertexIndices = requestProperties(file, error, element.name, {"vertex_indices"});
}

//##################################################################################################
bool readVertices(const std::shared_ptr<tinyply::PlyData>& vertices, tp_math_utils::Geometry3D& outputGeometry)
{
  if(!vertices)
    return false;

  auto add = [&](auto t)
  {
    using T = decltype(t);
    if(vertices->buffer.size_bytes() != (vertices->count*sizeof(T)*3))
      return false;

    outputGeometry.verts.resize(vertices->count);

    const uint8_t* src = vertices->buffer.get();
    const uint8_t* srcMax = src + vertices->buffer.size_bytes();
    auto dst = outputGeometry.verts.data();
    for(; src<srcMax; src+=(sizeof(T)*3), dst++)
    {
      T tmp[3];
      memcpy(&tmp, src, (sizeof(T)*3));
      dst->vert.x = float(tmp[0]);
      dst->vert.y = float(tmp[1]);
      dst->vert.z = float(tmp[2]);
    }

    return true;
  };

  switch(vertices->t)
  {
    case tinyply::Type::FLOAT32: return add(float());
    case tinyply::Type::FLOAT64: return add(double());
    default: return false;
  }
}

//##################################################################################################
void readNormals(const std::shared_ptr<tinyply::PlyData>& normals, tp_math_utils::Geometry3D& outputGeometry)
{
  if(!normals)
    return;

  auto add = [&](auto t)
  {
    using T = decltype(t);
    if(normals->buffer.size_bytes() != (normals->count*sizeof(T)*3))
      return;

    if(normals->count != outputGeometry.verts.size())
      return;

    const uint8_t* src = normals->buffer.get();
    const uint8_t* srcMax = src + normals->buffer.size_bytes();
    auto dst = outputGeometry.verts.data();
    for(; src<srcMax; src+=(sizeof(T)*3), dst++)
    {
      T tmp[3];
      memcpy(&tmp, src, (sizeof(T)*3));
      dst->normal.x = float(tmp[0]);
      dst->normal.y = float(tmp[1]);
      dst->normal.z = float(tmp[2]);
    }
  };

  switch(normals->t)
  {
    case tinyply::Type::FLOAT32: return add(float());
    case tinyply::Type::FLOAT64: return add(double());
    default: return;
  }
}

//##################################################################################################
void readTextures(const std::shared_ptr<tinyply::PlyData>& textures, tp_math_utils::Geometry3D& outputGeometry)
{
  if(!textures)
    return;

  auto add = [&](auto t)
  {
    using T = decltype(t);
    if(textures->buffer.size_bytes() != (textures->count*sizeof(T)*2))
      return;

    if(textures->count != outputGeometry.verts.size())
      return;

    const uint8_t* src = textures->buffer.get();
    const uint8_t* srcMax = src + textures->buffer.size_bytes();
    auto dst = outputGeometry.verts.data();
    for(; src<srcMax; src+=(sizeof(T)*2), dst++)
    {
      T tmp[3];
      memcpy(&tmp, src, (sizeof(T)*3));
      dst->texture.x = float(tmp[0]);
      dst->texture.y = float(tmp[1]);
    }
  };

  switch(textures->t)
  {
    case tinyply::Type::FLOAT32: return add(float());
    case tinyply::Type::FLOAT64: return add(double());
    default: return;
  }
}

//##################################################################################################
void readFaces(std::string& error,
               const ElementDetails_lt& faces,
               bool reverse,
               tp_math_utils::Geometry3D& outputGeometry)
{
  try
  {
    if(!faces.vertexIndices)
      return;

    if(!faces.vertexIndices->buffer.size_bytes())
    {
      error = "readPLYStream index buffer should not be empty.";
      return;
    }

    auto addIndexes = [&](auto t)
    {
      using T = decltype(t);

      size_t count = faces.vertexIndices->buffer.size_bytes()/sizeof(T);

      if(!count)
        return false;

      tp_math_utils::Indexes3D indexes;
      indexes.type = faces.geometryType;
      indexes.indexes.reserve(count);

      const uint8_t* src = faces.vertexIndices->buffer.get();
      const uint8_t* srcMax = src + faces.vertexIndices->buffer.size_bytes();
      for(; src<srcMax; src+=sizeof(T))
      {
        T tmp;
        memcpy(&tmp, src, sizeof(T));

        if(!std::is_unsigned<T>::value && tmp<0)
        {
          if(indexes.indexes.size()>=3)
            outputGeometry.indexes.push_back(indexes);
          indexes.indexes.clear();
        }
        else
          indexes.indexes.push_back(int(tmp));
      }

      if(indexes.indexes.size()>=3)
        outputGeometry.indexes.push_back(indexes);

      if(reverse)
        for(auto& i : outputGeometry.indexes)
          std::reverse(i.indexes.begin(), i.indexes.end());

      return true;
    };

    switch(faces.vertexIndices->t)
    {
      case tinyply::Type::INT8:   addIndexes(int8_t());   return;
      case tinyply::Type::INT16:  addIndexes(int16_t());  return;
      case tinyply::Type::INT32:  addIndexes(int32_t());  return;
      case tinyply::Type::UINT8:  addIndexes(uint8_t());  return;
      case tinyply::Type::UINT16: addIndexes(uint16_t()); return;
      case tinyply::Type::UINT32: addIndexes(uint32_t()); return;
      default:
      {
        error = "Unsupported index type.";
        return;
      }
    }
  }
  catch (const std::exception & e)
  {
    error = std::string("Failed to read vertex_indices from e.name. Error: ") + e.what();
    return;
  }
}
}


//##################################################################################################
bool readPLYFile(const std::string & filePath,
                 int triangleFan,
                 int triangleStrip,
                 int triangles,
                 bool reverse,
                 tp_math_utils::Geometry3D& outputGeometry,
                 tp_utils::Progress* progress)
{
  try
  {
    std::ifstream ss(tp_utils::u8path(filePath), std::ios::binary);
    if(ss.fail())
    {
      progress->addError("failed to open: " + filePath);
      return false;
    }

    readPLYStream(ss,
                  triangleFan,
                  triangleStrip,
                  triangles,
                  reverse,
                  outputGeometry,
                  progress);
  }
  catch (const std::exception & e)
  {
    progress->addError(std::string("Caught tinyply exception: ") + e.what());
    return false;
  }

  return true;
}


//##################################################################################################
bool readPLYStream(std::istream& inputStream,
                   int triangleFan,
                   int triangleStrip,
                   int triangles,
                   bool reverse,
                   tp_math_utils::Geometry3D& outputGeometry,
                   tp_utils::Progress* progress)
{
  outputGeometry.triangleFan   = triangleFan;
  outputGeometry.triangleStrip = triangleStrip;
  outputGeometry.triangles     = triangles;

  try
  {
    tinyply::PlyFile file;
    progress->addMessage("Read header from input stream.");
    file.parse_header(inputStream);


    outputGeometry.comments = file.get_comments();


    ElementDetails_lt vertexDetails;
    ElementDetails_lt faceDetails;
    ElementDetails_lt tristripsDetails;
    ElementDetails_lt trifanDetails;

    faceDetails     .geometryType = triangles;
    tristripsDetails.geometryType = triangleStrip;
    trifanDetails   .geometryType = triangleFan;

    //-- Parse the header --------------------------------------------------------------------------
    progress->addMessage("Parse header.");

    auto elements = file.get_elements();
    progress->addMessage("Number of elements: " + std::to_string(elements.size()));

    bool printProperties = false;
    for(const auto& e : elements)
    {
      if(printProperties)
      {
        tpWarning() << "element - " << e.name << " (" << e.size << ")" << std::endl;
        // for(const auto& p : e.properties)
        //   tpWarning() << "\tproperty - " << p.name << " (" << tinyply::PropertyTable[p.propertyType].str << ")" << std::endl;
      }

      std::string error;

      if(e.name == "vertex")
        parseElement(file, error, e, vertexDetails);

      else if(e.name == "face")
        parseElement(file, error, e, faceDetails);

      else if(e.name == "tristrips")
        parseElement(file, error, e, tristripsDetails);

      if(!error.empty())
      {
        progress->addError("Error processing elements!");
        progress->addError(error);
        progress->addError("Element name: " + e.name);
        return false;
      }
    }

    //-- Read the requested properties -------------------------------------------------------------
    file.read(inputStream);

    //-- Read in the verts -------------------------------------------------------------------------
    if(!readVertices(vertexDetails.vertices, outputGeometry))
    {
      progress->addError("Error reading vertices.");
      return false;
    }

    //-- Read in the other vertex properties -------------------------------------------------------
    readNormals (vertexDetails.normals  , outputGeometry);
    readTextures(vertexDetails.texcoords, outputGeometry);

    //-- Read in the faces -------------------------------------------------------------------------
    {
      std::string error;
      readFaces(error,      faceDetails, reverse, outputGeometry);
      readFaces(error, tristripsDetails, reverse, outputGeometry);

      if(!error.empty())
      {
        progress->addError("Error reading faces!");
        progress->addError(error);
        return false;
      }
    }
  }
  catch (const std::exception & e)
  {
    progress->addError(std::string("Caught tinyply exception: ") + e.what());
    return false;
  }

  return true;
}

}
