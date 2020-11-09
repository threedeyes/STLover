NAME = STLover
TYPE = APP
APP_MIME_SIG = application/x-vnd.stlviewer
SRCS = STLApp.cpp STLWindow.cpp STLStatWindow.cpp STLInputWindow.cpp STLView.cpp main.cpp
RDEFS = Resources.rdef
LIBS = be tracker GL GLU admesh $(STDCPPLIBS)
OPTIMIZE := FULL
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
