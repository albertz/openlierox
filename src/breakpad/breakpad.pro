TEMPLATE = lib
TARGET = breakpad
CONFIG -= gui

include( ../../definitions.pro.inc )

INCLUDEPATH += . external/src
SOURCES = BreakPad.cpp
HEADERS = BreakPad.h
LIBS -= libLastFmTools


!win32 {
	SOURCES += $$SYSTEM( ls external/src/client/*.cc )
	SOURCES += external/src/common/convert_UTF.c external/src/common/string_conversion.cc
}

mac* {
    SOURCES += $$SYSTEM( find external/src/client/mac -name \*.cc )
    SOURCES += $$SYSTEM( find external/src/common/mac -name \*.cc -o -name \*.mm )
    
    SOURCES -= external/src/client/mac/handler/minidump_generator_test.cc
    SOURCES -= external/src/client/mac/handler/exception_handler_test.cc
    
    LIBS += -lcrypto
}

win32 {
    SOURCES += external/src/client/windows/handler/exception_handler.cc
    SOURCES += external/src/common/windows/guid_string.cc

    LIBS += ole32.lib
    
    INCLUDEPATH += $$(VSDIR)/vc/atlmfc/include
    
    DEFINES += BREAKPAD_DLLEXPORT_PRO
}

linux* {
    SOURCES += $$SYSTEM( find external/src/client/linux -name \*test\* -prune -o -name \*.cc -print )
    SOURCES += $$SYSTEM( find external/src/common/linux -name \*.cc -o -name \*.c )

    SOURCES += external/src/common/md5.c
}