TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += server.cpp \
	   main.cpp \
    util.cpp
	
HEADERS += server.h \
    util.h
