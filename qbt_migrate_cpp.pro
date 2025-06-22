TARGET = qbt_migrate_cpp
TEMPLATE = app
CONFIG += console warn_on c++17
win32: !contains(CONFIG, build_all): CONFIG -= debug_and_release

CONFIG(debug, debug|release) {
    BUILD_TYPE = debug
    DEFINES *= DEBUG_BUILD
}
CONFIG(release, debug|release) {
    BUILD_TYPE = release
    DEFINES *= RELEASE_BUILD
    QMAKE_CXXFLAGS += -O2
}

DESTDIR     = $$PWD/bin

# if (PWD != build_dir), such as the case with shadow build
!equals(PWD, $${OUT_PWD}) {
    TARGET      = $${TARGET}.$${BUILD_TYPE}
} else {
    TARGET      = $${TARGET}.$${BUILD_TYPE}
    MOC_DIR     = $$PWD/build/$$BUILD_TYPE
    OBJECTS_DIR = $$PWD/build/$$BUILD_TYPE
    RCC_DIR     = $$PWD/build/$$BUILD_TYPE
}


SOURCES += main.cpp
