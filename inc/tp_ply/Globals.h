#ifndef tp_ply_Globals_h
#define tp_ply_Globals_h

#include "tp_utils/Globals.h"

#if defined(TP_PLY_LIBRARY)
#  define TP_PLY_EXPORT TP_EXPORT
#else
#  define TP_PLY_EXPORT TP_IMPORT
#endif

//##################################################################################################
//! Load 3D models from .ply files.
namespace tp_ply
{

}

#endif
