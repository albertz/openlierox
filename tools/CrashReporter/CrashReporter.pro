TEMPLATE = app
QT += gui network xml
CONFIG -= app_bundle

include( ../../definitions.pro.inc )

win32:  TARGET = CrashReporter
linux*: TARGET = last.reporter
mac*:   TARGET = Last.reporter

win32 {
    LIBS += -lshfolder -luser32
    INCLUDEPATH += ../zlib-1.2.3

	RC_FILE = CrashReporter.rc
}

win32 {
    DEFINE = $${LITERAL_HASH}define 
    BINARY = L\"$${TARGET}.exe\"
}
else {
    DEFINE = \\$${LITERAL_HASH}define 
    BINARY = \\\"$$TARGET\\\"
}
system( echo '$$DEFINE CRASH_REPORTER_BINARY $$BINARY' > BinaryFilename.h )


FORMS = CrashReporter.ui
HEADERS = CrashReporter.h
SOURCES = main.cpp CrashReporter.cpp
