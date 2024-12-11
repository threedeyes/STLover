NAME = STLover
TYPE = APP
APP_MIME_SIG = application/x-vnd.stlover
SRCS = STLApp.cpp STLInputWindow.cpp STLWindow.cpp STLToolBar.cpp STLStatView.cpp STLRepairWindow.cpp STLLogoView.cpp STLView.cpp main.cpp
RDEFS = Resources.rdef
LIBS = be shared tracker localestub GL GLU glut admesh $(STDCPPLIBS)
SYSTEM_INCLUDE_PATHS = /system/develop/headers/private/interface
LOCALES = ca de en en_AU en_GB es es_419 fr fur it nb nl pt ro ru sv tr uk
OPTIMIZE := FULL
#SYMBOLS := TRUE
COMPILER_FLAGS = -mmmx -msse -msse2
#DEBUGGER := TRUE
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
