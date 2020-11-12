/*  STLover - Simple STL Viewer
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

#include <Window.h>
#include <Message.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <String.h>
#include <Screen.h>
#include <FilePanel.h>
#include <FindDirectory.h>

#include <private/interface/AboutWindow.h>

#include <admesh/stl.h>

class STLView;
class STLStatWindow;

class STLWindow : public BWindow {
	public:
		STLWindow(BRect frame);
		~STLWindow();
		virtual void MessageReceived(BMessage *message);
		virtual bool QuitRequested();

		void OpenFile(const char *file);
		void CloseFile(void);
		void UpdateStats(void);
		void TransformPosition(void);

		int GetErrorTimer() { return errorTimeCounter; }
		int GetBigExtent() { return maxExtent; }
		int GetZDepth() { return zDepth; }
		bool IsLoaded(void) { return (stlObject != NULL && stlValid); }

		void UpdateUI(void);

		static int32 RenderFunction(void *data);		

	private:
		void EnableMenuItems(bool show);
		void LoadSettings(void);
		void SaveSettings(void);
	
		thread_id 	rendererThread;

		BMenuBar *fMenuBar;
		BMenu *fMenuFile;
		BMenu *fMenuFileSaveAs;
		BMenu *fMenuView;
		BMenu *fMenuTools;
		BMenu *fMenuToolsMirror;
		BMenu *fMenuToolsScale;
		BMenu *fMenuToolsMove;
		BMenu *fMenuHelp;
		BMenuItem *fMenuItemAppend;
		BMenuItem *fMenuItemReload;
		BMenuItem *fMenuItemClose;
		BMenuItem *fMenuItemSave;
		BMenuItem *fMenuItemSolid;
		BMenuItem *fMenuItemWireframe;
		BMenuItem *fMenuItemShowBox;
		BMenuItem *fMenuItemShowAxes;
		BMenuItem *fMenuItemShowOXY;
		BMenuItem *fMenuItemStatWin;
		BMenuItem *fMenuItemReset;
		BMenuItem *fMenuItemEditTitle;
		BMenuItem *fMenuItemRotate;
		BMenuItem *fMenuItemRepair;
		BFilePanel	*fOpenFilePanel;
		BFilePanel	*fSaveFilePanel;
		BString fOpenedFileName;

		STLView *stlView;
		STLStatWindow *statWindow;

		bool stlModified;
		bool stlValid;
		bool showBoundingBox;
		bool showWireframe;
		bool showAxes;
		bool showOXY;

		int32 exactFlag;
		int32 nearbyFlag;
		int32 removeUnconnectedFlag;
		int32 fillHolesFlag;
		int32 normalDirectionsFlag;
		int32 normalValuesFlag;
		int32 reverseAllFlag;
		int32 iterationsValue;

		int errorTimeCounter;
		
		float zDepth;
		float maxExtent;

		stl_file* stlObject;
		stl_file* stlObjectView;
		stl_file* stlObjectAppend;
};

#endif
