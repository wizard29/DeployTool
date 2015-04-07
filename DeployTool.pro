################################################################################
include(general.pri)

################################################################################
TARGET = DeployTool
QT += widgets concurrent xml

################################################################################
HEADERS += \
    include/DTNamespace.h \
    include/DTOutputModel.h \
    include/DTDocumentWindow.h \
    include/DTDependencyModel.h \
    include/DTDependencyManager.h

################################################################################
SOURCES += \
    src/main.cpp \
    src/DTOutputModel.cpp \
    src/DTDocumentWindow.cpp \
    src/DTDependencyModel.cpp \
    src/DTDependencyManager.cpp

################################################################################
FORMS += \

################################################################################

RESOURCES += \
    resource/resource.qrc
