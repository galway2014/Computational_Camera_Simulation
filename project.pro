#-------------------------------------------------
#
# Project created by QtCreator 2013-09-08T10:44:46
#
#-------------------------------------------------

QT       += core
QT       -= gui

QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig
PKGCONFIG += opencv

TARGET = lens
TEMPLATE = app

CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

HEADERS		=     ./*.h
SOURCES		=     ./*.cpp
QMAKE_CXXFLAGS += -std=c++0x            
QMAKE_CXXFLAGS += -g -Wall -O0 

#LIBS  += /usr/lib/nvidia-343/libGL.so
LIBS  += -lglut -lGL -lGLU -lGLEW -lXmu -lXi  -lm 
#LIBS += /usr/lib/x86_64-linux-gnu/libX11.so
OBJECTS_DIR = ./obj
