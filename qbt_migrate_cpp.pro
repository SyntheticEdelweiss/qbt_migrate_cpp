TARGET = qbt_migrate_cpp
TEMPLATE = app
CONFIG += console warn_on c++17

CONFIG(debug, debug|release) {
DESTDIR      = $$PWD/debug/bin/
MOC_DIR      = $$PWD/debug/moc/
OBJECTS_DIR  = $$PWD/debug/obj/
}
CONFIG(release, debug|release) {
DESTDIR      = $$PWD/release/bin/
MOC_DIR      = $$PWD/release/moc/
OBJECTS_DIR  = $$PWD/release/obj/

QMAKE_CXXFLAGS += -O2
}

SOURCES += main.cpp
