################################################################################
VERSION = 1

################################################################################
deployroot = ../../Deploy
deploybin = $$deployroot/bin
deployplugin = $$deployroot/plugins
CONFIG (debug, debug|release){
    inputbin = bin/debug
    destination = ./bin/debug
}
else{
    inputbin = bin/release
    destination = ./bin/release
}

################################################################################
OBJECTS_DIR = $$destination/obj
UI_DIR = $$destination/ui
MOC_DIR = $$destination/moc
RCC_DIR = $$destination/rcc
DESTDIR = $$destination

################################################################################
INCLUDEPATH += \
    ./include \
    ./src/private \
    $$UI_DIR

################################################################################
win32-msvc2013:{
    QMAKE_CXXFLAGS_RELEASE += -Zi
    QMAKE_CXXFLAGS += /WX
    QMAKE_LFLAGS_RELEASE += /DEBUG
    DEFINES += _CRT_SECURE_NO_WARNINGS NOMINMAX /MACHINE:X86
    DLLDESTDIR = $$deploybin
}

################################################################################
macx-clang:{
    QMAKE_CXXFLAGS += -std=c++11 -mmacosx-version-min=10.9
    QMAKE_LFLAGS += -mmacosx-version-min=10.9 -stdlib=libc++
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

################################################################################
