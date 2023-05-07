NAME = STLover
TYPE = APP
APP_MIME_SIG = application/x-vnd.stlover
SRCS = STLApp.cpp STLWindow.cpp STLToolBar.cpp STLStatView.cpp STLInputWindow.cpp STLRepairWindow.cpp STLLogoView.cpp STLView.cpp main.cpp
RDEFS = Resources.rdef
LIBS = be shared tracker localestub GL GLU admesh $(STDCPPLIBS)
LOCALES = ca de en en_AU en_GB es fr fur it pt ro ru sv tr uk
OPTIMIZE := FULL
SYMBOLS := FLASE
DEBUGGER := FALSE
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
