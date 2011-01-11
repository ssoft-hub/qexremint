#Includes
INCLUDEPATH += $$PWD/include

CONFIG += qt warn_on debug_and_release exceptions stl embed_manifest_exe thread

CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/bin/debug
    LIBS   += -L$$PWD/bin/debug
    QMAKE_LIBDIR += $$PWD/bin/debug
} else {
    DESTDIR = $$PWD/bin/release
    LIBS   += -L$$PWD/bin/release
    QMAKE_LIBDIR += $$PWD/bin/release
}
