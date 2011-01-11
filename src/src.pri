# -----------------------------------
# This file is part of the QexRemint.
# -----------------------------------

#Defines
DEFINES += QEX_REMINT_LIB

#Header files
HEADERS += $$PWD/QexDeviceHelper.h \
           $$PWD/QexMetaTranslator.h \
           $$PWD/QexMethodInvoker.h \
           $$PWD/QexMethodTranslator.h \
           $$PWD/QexPropertyRequester.h \
           $$PWD/QexPropertyTranslator.h \
           $$PWD/QexRemintConfig.h \
           $$PWD/QexRemintReader.h \
           $$PWD/QexSignalPacker.h \
           $$PWD/QexSignalSlotTranslator.h \
           $$PWD/QexSlotActivator.h

#Source files
SOURCES += $$PWD/QexDeviceHelper.cpp \
           $$PWD/QexMetaTranslator.cpp \
           $$PWD/QexMethodInvoker.cpp \
           $$PWD/QexMethodTranslator.cpp \
           $$PWD/QexPropertyRequester.cpp \
           $$PWD/QexPropertyTranslator.cpp \
           $$PWD/QexRemintReader.cpp \
           $$PWD/QexSignalPacker.cpp \
           $$PWD/QexSignalSlotTranslator.cpp \
           $$PWD/QexSlotActivator.cpp
