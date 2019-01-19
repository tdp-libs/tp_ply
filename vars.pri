TARGET = tp_ply
TEMPLATE = lib

DEFINES += TP_PLY_LIBRARY

HEADERS += inc/tp_ply/Globals.h

SOURCES += src/ReadPLY.cpp
HEADERS += inc/tp_ply/ReadPLY.h

SOURCES += src/WritePLY.cpp
HEADERS += inc/tp_ply/WritePLY.h
