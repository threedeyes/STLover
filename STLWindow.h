/*  STLover - A powerful tool for viewing and manipulating 3D STL models
 *  Copyright (C) 2020 Gerasim Troeglazov <3dEyes@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef STLOVER_WINDOW
#define STLOVER_WINDOW

#include <Alert.h>
#include <File.h>
#include <Window.h>
#include <Message.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <String.h>
#include <Screen.h>
#include <Roster.h>
#include <FilePanel.h>
#include <FindDirectory.h>

#include <private/interface/AboutWindow.h>

#include <admesh/stl.h>

class STLView;
class STLLogoView;
class STLStatView;
class STLStatWindow;
class STLToolBar;

class STLWindow : public BWindow {
	public:
		STLWindow();
		~STLWindow();
		virtual void WindowActivated(bool active);
		virtual void MessageReceived(BMessage *message);
		virtual bool QuitRequested();

		void SetSTL(stl_file *stl, stl_file *stlView);
		void OpenFile(const char *file);
		void CloseFile(void);
		void UpdateStats(void);
		void TransformPosition(void);

		int GetErrorTimer(void) { return fErrorTimeCounter; }
		int GetBigExtent(void) { return fMaxExtent; }
		int GetZDepth(void) { return fZDepth; }
		bool IsLoaded(void) { return (fStlObject != NULL && fStlValid); }
		bool IsRenderWork(void) {return fRenderWork; }
		BString Filename(void) {return fOpenedFileName; }

		void UpdateUI(void);

		static int32 _RenderFunction(void *data);
		static int32 _FileLoaderFunction(void *data);

	private:
		void UpdateUIStates(bool show);
		void LoadSettings(void);
		void SaveSettings(void);
	
		thread_id fRendererThread;
		thread_id fFileLoaderThread;

		BMenuBar *fMenuBar;
		BMenu *fMenuFile;
		BMenu *fMenuFileSaveAs;
		BMenu *fMenuView;
		BMenu *fMenuTools;
		BMenu *fMenuToolsMirror;
		BMenu *fMenuToolsScale;
		BMenu *fMenuToolsMove;
		BMenu *fMenuHelp;
		BMenuItem *fMenuItemReload;
		BMenuItem *fMenuItemClose;
		BMenuItem *fMenuItemSave;
		BMenuItem *fMenuItemSolid;
		BMenuItem *fMenuItemWireframe;
		BMenuItem *fMenuItemShowBox;
		BMenuItem *fMenuItemShowAxes;
		BMenuItem *fMenuItemShowOXY;
		BMenuItem *fMenuItemStat;
		BMenuItem *fMenuItemReset;
		BMenuItem *fMenuItemEditTitle;
		BMenuItem *fMenuItemRotate;
		BMenuItem *fMenuItemRepair;
		BFilePanel *fOpenFilePanel;
		BFilePanel *fSaveFilePanel;

		BString fOpenedFileName;

		STLView *fStlView;
		STLLogoView *fStlLogoView;
		STLToolBar *fToolBar;
		STLToolBar *fViewToolBar;
		STLStatView *fStatView;

		bool fRenderWork;

		bool fStlModified;
		bool fStlValid;
		bool fShowStat;
		bool fShowBoundingBox;
		bool fShowWireframe;
		bool fShowAxes;
		bool fShowOXY;

		int32 fExactFlag;
		int32 fNearbyFlag;
		int32 fRemoveUnconnectedFlag;
		int32 fFillHolesFlag;
		int32 fNormalDirectionsFlag;
		int32 fNormalValuesFlag;
		int32 fReverseAllFlag;
		int32 fIterationsValue;

		int fErrorTimeCounter;

		float fZDepth;
		float fMaxExtent;

		stl_file *fStlObject;
		stl_file *fStlObjectView;
};

#endif
