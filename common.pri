exists($$PWD/../XPlaneSDK) {
  XPLANE_SDK_PATH=$$PWD/../XPlaneSDK
}
exists($$(HOME)/SDK/) {
  XPLANE_SDK_PATH=$$(HOME)/SDK/
}

# Build all the dataref classes as they are needed
# everywhere.

SOURCES += \
    $$PWD/stkpconnect-server/datarefprovider.cpp \
    $$PWD/stkpconnect-server/datarefs/dataref.cpp \
    $$PWD/stkpconnect-server/datarefs/floatdataref.cpp \
    $$PWD/stkpconnect-server/datarefs/floatarraydataref.cpp \
    $$PWD/stkpconnect-server/datarefs/intdataref.cpp \
    $$PWD/stkpconnect-server/datarefs/doubledataref.cpp \
    $$PWD/stkpconnect-server/datarefs/intarraydataref.cpp \
    $$PWD/stkpconnect-server/datarefs/datadataref.cpp

HEADERS += \
    $$PWD/stkpconnect-server/datarefprovider.h \
    $$PWD/stkpconnect-server/datarefs/dataref.h \
    $$PWD/stkpconnect-server/datarefs/floatdataref.h \
    $$PWD/stkpconnect-server/datarefs/floatarraydataref.h \
    $$PWD/stkpconnect-server/datarefs/intdataref.h \
    $$PWD/stkpconnect-server/datarefs/doubledataref.h \
    $$PWD/stkpconnect-server/datarefs/intarraydataref.h \
    $$PWD/stkpconnect-server/datarefs/datadataref.h
