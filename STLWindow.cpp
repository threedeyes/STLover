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

#include "STLApp.h"
#include "STLView.h"
#include "STLWindow.h"
#include "STLLogoView.h"
#include "STLStatView.h"
#include "STLInputWindow.h"
#include "STLRepairWindow.h"
#include "STLToolBar.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#undef  B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT          "STLoverMainWindow"

STLWindow::STLWindow()
	: BWindow(BRect(100, 100, 100 + 800, 100 + 640), MAIN_WIN_TITLE, B_TITLED_WINDOW, 0),
	fOpenFilePanel(NULL),
	fSaveFilePanel(NULL),
	fMeasureWindow(NULL),
	fStlModified(false),
	fStlLoading(false),
	fShowStat(false),
	fShowBoundingBox(false),
	fShowAxes(false),
	fShowAxesPlane(false),
	fShowAxesCompass(true),
	fShowOXY(false),
	fViewOrtho(false),
	fShowMode(MSG_VIEWMODE_SOLID),
	fMeasureMode(false),
	fExactFlag(false),
	fNearbyFlag(false),
	fRemoveUnconnectedFlag(false),
	fFillHolesFlag(false),
	fNormalDirectionsFlag(false),
	fNormalValuesFlag(false),
	fReverseAllFlag(false),
	fIterationsValue(2),
	fStlValid(false),
	fStlObject(NULL),
	fErrorTimeCounter(0),
	fRenderWork(true),
	fZDepth(-5),
	fMaxExtent(10)
{
	fMenuBar = new BMenuBar(BRect(0, 0, Bounds().Width(), 22), "menubar");
	fMenuFile = new BMenu(B_TRANSLATE("File"));
	fMenuFileSaveAs = new BMenu(B_TRANSLATE("Save as" B_UTF8_ELLIPSIS));
	fMenuView = new BMenu(B_TRANSLATE("View"));
	fMenuAxes = new BMenu(B_TRANSLATE("Axes"));
	fMenuTools = new BMenu(B_TRANSLATE("Tools"));
	fMenuToolsMirror = new BMenu(B_TRANSLATE("Mirror"));
	fMenuToolsScale = new BMenu(B_TRANSLATE("Scale"));
	fMenuToolsMove = new BMenu(B_TRANSLATE("Move"));
	fMenuHelp = new BMenu(B_TRANSLATE("Help"));

	fMenuFileSaveAs->AddItem(new BMenuItem(B_TRANSLATE("STL (ASCII)"), new BMessage(MSG_FILE_EXPORT_STLA)));
	fMenuFileSaveAs->AddItem(new BMenuItem(B_TRANSLATE("STL (Binary)"), new BMessage(MSG_FILE_EXPORT_STLB)));
	fMenuFileSaveAs->AddItem(new BMenuItem(B_TRANSLATE("Geomview OFF"), new BMessage(MSG_FILE_EXPORT_OFF)));
	fMenuFileSaveAs->AddItem(new BMenuItem(B_TRANSLATE("Autodesk DXF"), new BMessage(MSG_FILE_EXPORT_DXF)));
	fMenuFileSaveAs->AddItem(new BMenuItem(B_TRANSLATE("Wavefront OBJ"), new BMessage(MSG_FILE_EXPORT_OBJ)));
	fMenuFileSaveAs->AddItem(new BMenuItem(B_TRANSLATE("VRML"), new BMessage(MSG_FILE_EXPORT_VRML)));

	fMenuItemOpen = new BMenuItem(BRecentFilesList::NewFileListMenu( B_TRANSLATE("Open" B_UTF8_ELLIPSIS),
		NULL, NULL, be_app, 9, true, NULL, APP_SIGNATURE), new BMessage(MSG_FILE_OPEN));
	fMenuItemOpen->SetShortcut('O', 0);
	fMenuFile->AddItem(fMenuItemOpen);
	fMenuItemReload = new BMenuItem(B_TRANSLATE("Reload"), new BMessage(MSG_FILE_RELOAD), 'L');
	fMenuFile->AddItem(fMenuItemReload);
	fMenuFile->AddSeparatorItem();
	fMenuItemSave = new BMenuItem(B_TRANSLATE("Save"), new BMessage(MSG_FILE_SAVE), 'S');
	fMenuFile->AddItem(fMenuItemSave);
	fMenuFile->AddItem(fMenuFileSaveAs);
	fMenuFileSaveAs->SetTargetForItems(this);
	fMenuFile->AddSeparatorItem();
	fMenuItemClose = new BMenuItem(B_TRANSLATE("Close"), new BMessage(MSG_FILE_CLOSE));
	fMenuFile->AddItem(fMenuItemClose);
	fMenuFile->AddItem(new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q'));
	fMenuBar->AddItem(fMenuFile);
	fMenuFile->SetTargetForItems(this);

	fMenuItemShowAxesPlane = new BMenuItem(B_TRANSLATE("Plane"), new BMessage(MSG_VIEWMODE_AXES_PLANE));
	fMenuAxes->AddItem(fMenuItemShowAxesPlane);
	fMenuItemShowAxesCompass = new BMenuItem(B_TRANSLATE("Compass"), new BMessage(MSG_VIEWMODE_AXES_COMPASS));
	fMenuAxes->AddItem(fMenuItemShowAxesCompass);
	fMenuAxes->SetTargetForItems(this);

	fMenuItemPoints = new BMenuItem(B_TRANSLATE("Points"), new BMessage(MSG_VIEWMODE_POINTS));
	fMenuView->AddItem(fMenuItemPoints);
	fMenuItemWireframe = new BMenuItem(B_TRANSLATE("Wireframe"), new BMessage(MSG_VIEWMODE_WIREFRAME));
	fMenuView->AddItem(fMenuItemWireframe);
	fMenuItemSolid = new BMenuItem(B_TRANSLATE("Solid"), new BMessage(MSG_VIEWMODE_SOLID));
	fMenuView->AddItem(fMenuItemSolid);
	fMenuView->AddSeparatorItem();
	fMenuItemOrthographicView = new BMenuItem(B_TRANSLATE("Orthographic projection"), new BMessage(MSG_VIEWMODE_ORTHO));
	fMenuView->AddItem(fMenuItemOrthographicView);
	fMenuView->AddSeparatorItem();
	fMenuItemShowAxes = new BMenuItem(fMenuAxes, new BMessage(MSG_VIEWMODE_AXES));
	fMenuView->AddItem(fMenuItemShowAxes);
	fMenuItemShowOXY = new BMenuItem(B_TRANSLATE("Plane OXY"), new BMessage(MSG_VIEWMODE_OXY));
	fMenuView->AddItem(fMenuItemShowOXY);
	fMenuItemShowBox = new BMenuItem(B_TRANSLATE("Bounding box"), new BMessage(MSG_VIEWMODE_BOUNDING_BOX));
	fMenuView->AddItem(fMenuItemShowBox);
	fMenuView->AddSeparatorItem();
	fMenuItemStat = new BMenuItem(B_TRANSLATE("Statistics"), new BMessage(MSG_VIEWMODE_STAT), 'I');
	fMenuView->AddItem(fMenuItemStat);
	fMenuView->AddSeparatorItem();
	fMenuItemReset = new BMenuItem(B_TRANSLATE("Reset"), new BMessage(MSG_VIEWMODE_RESETPOS), 'R');
	fMenuView->AddItem(fMenuItemReset);

	fMenuToolsMirror->AddItem(new BMenuItem(B_TRANSLATE("Mirror XY"), new BMessage(MSG_TOOLS_MIRROR_XY)));
	fMenuToolsMirror->AddItem(new BMenuItem(B_TRANSLATE("Mirror YZ"), new BMessage(MSG_TOOLS_MIRROR_YZ)));
	fMenuToolsMirror->AddItem(new BMenuItem(B_TRANSLATE("Mirror XZ"), new BMessage(MSG_TOOLS_MIRROR_XZ)));
	fMenuToolsMirror->SetTargetForItems(this);

	fMenuToolsScale->AddItem(new BMenuItem(B_TRANSLATE("Scale" B_UTF8_ELLIPSIS), new BMessage(MSG_TOOLS_SCALE)));
	fMenuToolsScale->AddItem(new BMenuItem(B_TRANSLATE("Axis scaling" B_UTF8_ELLIPSIS), new BMessage(MSG_TOOLS_SCALE_3)));
	fMenuToolsScale->SetTargetForItems(this);
	
	fMenuToolsMove->AddItem(new BMenuItem(B_TRANSLATE("To" B_UTF8_ELLIPSIS), new BMessage(MSG_TOOLS_MOVE_TO)));
	fMenuToolsMove->AddItem(new BMenuItem(B_TRANSLATE("By" B_UTF8_ELLIPSIS), new BMessage(MSG_TOOLS_MOVE_BY)));
	fMenuToolsMove->AddItem(new BMenuItem(B_TRANSLATE("To origin"), new BMessage(MSG_TOOLS_MOVE_CENTER)));
	fMenuToolsMove->AddItem(new BMenuItem(B_TRANSLATE("To (0,0,0)"), new BMessage(MSG_TOOLS_MOVE_ZERO)));
	fMenuToolsMove->AddItem(new BMenuItem(B_TRANSLATE("To top of OXY plane"), new BMessage(MSG_TOOLS_MOVE_MIDDLE)));
	fMenuToolsMove->SetTargetForItems(this);

	fMenuItemEditTitle = new BMenuItem(B_TRANSLATE("Edit title" B_UTF8_ELLIPSIS), new BMessage(MSG_TOOLS_EDIT_TITLE));
	fMenuTools->AddItem(fMenuItemEditTitle);
	fMenuTools->AddSeparatorItem();
	fMenuTools->AddItem(fMenuToolsScale);
	fMenuTools->AddItem(fMenuToolsMove);
	fMenuTools->AddItem(fMenuToolsMirror);
	fMenuItemRotate = new BMenuItem(B_TRANSLATE("Rotate" B_UTF8_ELLIPSIS), new BMessage(MSG_TOOLS_ROTATE));
	fMenuTools->AddItem(fMenuItemRotate);
	fMenuTools->AddSeparatorItem();
	fMenuItemRepair = new BMenuItem(B_TRANSLATE("Repair" B_UTF8_ELLIPSIS), new BMessage(MSG_TOOLS_REPAIR));
	fMenuTools->AddItem(fMenuItemRepair);
	fMenuTools->AddSeparatorItem();
	fMenuItemMeasure = new BMenuItem(B_TRANSLATE("Measure" B_UTF8_ELLIPSIS), new BMessage(MSG_TOOLS_MEASURE));
	fMenuTools->AddItem(fMenuItemMeasure);

	fMenuBar->AddItem(fMenuView);
	fMenuView->SetTargetForItems(this);

	fMenuBar->AddItem(fMenuTools);
	fMenuTools->SetTargetForItems(this);

	fMenuHelp->AddItem(new BMenuItem(B_TRANSLATE("Documentation"), new BMessage(MSG_HELP_WIKI)));
	fMenuHelp->AddSeparatorItem();
	fMenuHelp->AddItem(new BMenuItem(B_TRANSLATE("About"), new BMessage(B_ABOUT_REQUESTED)));
	fMenuBar->AddItem(fMenuHelp);
	fMenuHelp->SetTargetForItems(this);

	AddChild(fMenuBar);

	BRect toolBarRect = Bounds();
	toolBarRect.top = fMenuBar->Frame().bottom + 1;
	fToolBar = new STLToolBar(toolBarRect);
	fToolBar->AddAction(MSG_FILE_OPEN, this, STLoverApplication::GetIcon("document-open", TOOLBAR_ICON_SIZE), B_TRANSLATE("Open"));
	fToolBar->AddAction(MSG_FILE_SAVE, this, STLoverApplication::GetIcon("document-save", TOOLBAR_ICON_SIZE), B_TRANSLATE("Save"));
	fToolBar->AddSeparator();
	fToolBar->AddAction(MSG_TOOLS_EDIT_TITLE, this, STLoverApplication::GetIcon("document-edit", TOOLBAR_ICON_SIZE), B_TRANSLATE("Edit title"));
	fToolBar->AddAction(MSG_TOOLS_MIRROR_XY, this, STLoverApplication::GetIcon("mirror-xy", TOOLBAR_ICON_SIZE), B_TRANSLATE("Mirror XY"));
	fToolBar->AddAction(MSG_TOOLS_MIRROR_YZ, this, STLoverApplication::GetIcon("mirror-yz", TOOLBAR_ICON_SIZE), B_TRANSLATE("Mirror YZ"));
	fToolBar->AddAction(MSG_TOOLS_MIRROR_XZ, this, STLoverApplication::GetIcon("mirror-xz", TOOLBAR_ICON_SIZE), B_TRANSLATE("Mirror XZ"));
	fToolBar->AddAction(MSG_TOOLS_MOVE_MIDDLE, this, STLoverApplication::GetIcon("move-middle", TOOLBAR_ICON_SIZE), B_TRANSLATE("Move to top of OXY plane"));
	fToolBar->AddAction(MSG_TOOLS_MOVE_TO, this, STLoverApplication::GetIcon("move-to", TOOLBAR_ICON_SIZE), B_TRANSLATE("Move to"));
	fToolBar->AddAction(MSG_TOOLS_MOVE_BY, this, STLoverApplication::GetIcon("move-by", TOOLBAR_ICON_SIZE), B_TRANSLATE("Move by"));
	fToolBar->AddAction(MSG_TOOLS_SCALE, this, STLoverApplication::GetIcon("scale", TOOLBAR_ICON_SIZE), B_TRANSLATE("Scale"));
	fToolBar->AddAction(MSG_TOOLS_SCALE_3, this, STLoverApplication::GetIcon("scale-axis", TOOLBAR_ICON_SIZE), B_TRANSLATE("Axis scale"));
	fToolBar->AddAction(MSG_TOOLS_ROTATE, this, STLoverApplication::GetIcon("rotate", TOOLBAR_ICON_SIZE), B_TRANSLATE("Rotate"));
	fToolBar->AddSeparator();
	fToolBar->AddAction(MSG_TOOLS_REPAIR, this, STLoverApplication::GetIcon("tools-wizard", TOOLBAR_ICON_SIZE), B_TRANSLATE("Repair"));
	fToolBar->AddSeparator();
	fToolBar->AddAction(MSG_TOOLS_MEASURE, this, STLoverApplication::GetIcon("tool-measure", TOOLBAR_ICON_SIZE), B_TRANSLATE("Measure"));
	fToolBar->AddSeparator();
	fToolBar->AddAction(MSG_VIEWMODE_STAT, this, STLoverApplication::GetIcon("stat", TOOLBAR_ICON_SIZE), B_TRANSLATE("Statistics"));
	fToolBar->AddGlue();
	fToolBar->ResizeTo(toolBarRect.Width(), fToolBar->MinSize().height);
	fToolBar->GroupLayout()->SetInsets(0);
	AddChild(fToolBar);

	BRect viewToolBarRect = Bounds();
	viewToolBarRect.top = fToolBar->Frame().bottom + 1;
	fViewToolBar = new STLToolBar(viewToolBarRect, B_VERTICAL);
	fViewToolBar->AddAction(MSG_VIEWMODE_ZOOMIN, this, STLoverApplication::GetIcon("zoom-in", TOOLBAR_ICON_SIZE), B_TRANSLATE("Zoom in"));
	fViewToolBar->AddAction(MSG_VIEWMODE_ZOOMOUT, this, STLoverApplication::GetIcon("zoom-out", TOOLBAR_ICON_SIZE), B_TRANSLATE("Zoom out"));
	fViewToolBar->AddAction(MSG_VIEWMODE_ZOOMFIT, this, STLoverApplication::GetIcon("zoom-fit-best", TOOLBAR_ICON_SIZE), B_TRANSLATE("Best fit"));
	fViewToolBar->AddSeparator();
	fViewToolBar->AddAction(MSG_VIEWMODE_RESETPOS, this, STLoverApplication::GetIcon("reset", TOOLBAR_ICON_SIZE), B_TRANSLATE("Reset view"));
	fViewToolBar->AddSeparator();
	fViewToolBar->AddAction(MSG_VIEWMODE_POINTS, this, STLoverApplication::GetIcon("points", TOOLBAR_ICON_SIZE), B_TRANSLATE("Points"));
	fViewToolBar->AddAction(MSG_VIEWMODE_WIREFRAME, this, STLoverApplication::GetIcon("wireframe", TOOLBAR_ICON_SIZE), B_TRANSLATE("Wireframe"));
	fViewToolBar->AddAction(MSG_VIEWMODE_SOLID, this, STLoverApplication::GetIcon("solid", TOOLBAR_ICON_SIZE), B_TRANSLATE("Solid"));
	fViewToolBar->AddAction(MSG_VIEWMODE_AXES, this, STLoverApplication::GetIcon("axes", TOOLBAR_ICON_SIZE), B_TRANSLATE("Show axes"));
	fViewToolBar->AddAction(MSG_VIEWMODE_OXY, this, STLoverApplication::GetIcon("plane", TOOLBAR_ICON_SIZE), B_TRANSLATE("Show plane OXY"));
	fViewToolBar->AddAction(MSG_VIEWMODE_BOUNDING_BOX, this, STLoverApplication::GetIcon("bounding-box", TOOLBAR_ICON_SIZE), B_TRANSLATE("Bounding box"));
	fViewToolBar->AddSeparator();
	fViewToolBar->AddAction(MSG_VIEWMODE_FRONT, this, STLoverApplication::GetIcon("view-front", TOOLBAR_ICON_SIZE), B_TRANSLATE("Front view"));
	fViewToolBar->AddAction(MSG_VIEWMODE_RIGHT, this, STLoverApplication::GetIcon("view-right", TOOLBAR_ICON_SIZE), B_TRANSLATE("Right view"));
	fViewToolBar->AddAction(MSG_VIEWMODE_TOP, this, STLoverApplication::GetIcon("view-top", TOOLBAR_ICON_SIZE), B_TRANSLATE("Top view"));
	fViewToolBar->AddSeparator();
	fViewToolBar->AddAction(MSG_VIEWMODE_ORTHO, this, STLoverApplication::GetIcon("orthographic", TOOLBAR_ICON_SIZE), B_TRANSLATE("Orthographic projection"));
	fViewToolBar->AddGlue();
	fViewToolBar->ResizeTo(fViewToolBar->MinSize().width, viewToolBarRect.Height());
	fViewToolBar->GroupLayout()->SetInsets(0);
	AddChild(fViewToolBar);

	BRect stlRect = Bounds();
	stlRect.top = fToolBar->Frame().bottom + 1;
	stlRect.left =fViewToolBar->Frame().right + 1;
	fStlView = new STLView(stlRect, BGL_RGB | BGL_DOUBLE | BGL_DEPTH);
	AddChild(fStlView);
	fStlView->Hide();

	fStlLogoView = new STLLogoView(stlRect);
	fStlLogoView->SetText(B_TRANSLATE("Drop STL files here"));
	fStlLogoView->SetTextColor(255, 255, 255);
	AddChild(fStlLogoView);

	BRect statRect = Bounds();
	statRect.left = stlRect.right + 1;
	statRect.top = stlRect.top;
	fStatView = new STLStatView(statRect);
	AddChild(fStatView);

	AddShortcut('H', B_COMMAND_KEY,	new BMessage(MSG_EASTER_EGG));
	AddShortcut('Q', B_COMMAND_KEY,	new BMessage(MSG_APP_QUIT));

	LoadSettings();
	UpdateUI();

	fRendererThread = spawn_thread(_RenderFunction, "renderThread", B_NORMAL_PRIORITY, (void*)fStlView);
	resume_thread(fRendererThread);

	SetPulseRate(1000000);
}

STLWindow::~STLWindow()
{
	SaveSettings();

	status_t exitValue;
	fRenderWork = false;
	wait_for_thread(fRendererThread, &exitValue);

	CloseFile();

	if (fOpenFilePanel != NULL)
		fOpenFilePanel->Window()->PostMessage(B_QUIT_REQUESTED);
	if (fSaveFilePanel != NULL)
		fSaveFilePanel->Window()->PostMessage(B_QUIT_REQUESTED);

	be_app->PostMessage(MSG_WINDOW_CLOSED);
}

void
STLWindow::UpdateUI(void)
{
	UpdateStats();
	UpdateUIStates(IsLoaded());
	fStlView->RenderUpdate();
}

void
STLWindow::LoadSettings(void)
{
	BPath path;
	if (find_directory (B_USER_SETTINGS_DIRECTORY, &path) == B_OK) {
		path.Append(APP_SETTINGS_FILENAME);
		BFile file(path.Path(), B_READ_ONLY);

		if (file.InitCheck() != B_OK || file.Lock() != B_OK)
			return;

		bool _fShowBoundingBox = false;
		bool _fShowAxes = false;
		bool _fShowAxesPlane = false;
		bool _fShowAxesCompass = true;
		bool _showStat = false;
		bool _fShowOXY = false;
		bool _fOrthoProj = false;
		uint32 _fShowMode = MSG_VIEWMODE_SOLID;
		BRect _windowRect(100, 100, 100 + 800, 100 + 640);

		file.ReadAttr("WindowRect", B_RECT_TYPE, 0, &_windowRect, sizeof(BRect));
		file.ReadAttr("ShowAxes", B_BOOL_TYPE, 0, &_fShowAxes, sizeof(bool));
		file.ReadAttr("ShowAxesPlane", B_BOOL_TYPE, 0, &_fShowAxesPlane, sizeof(bool));
		file.ReadAttr("ShowAxesCompass", B_BOOL_TYPE, 0, &_fShowAxesCompass, sizeof(bool));
		file.ReadAttr("ShowOXY", B_BOOL_TYPE, 0, &_fShowOXY, sizeof(bool));
		file.ReadAttr("ShowBoundingBox", B_BOOL_TYPE, 0, &_fShowBoundingBox, sizeof(bool));
		file.ReadAttr("ShowStat", B_BOOL_TYPE, 0, &_showStat, sizeof(bool));
		file.ReadAttr("ShowMode", B_UINT32_TYPE, 0, &_fShowMode, sizeof(uint32));
		file.ReadAttr("OrthographicProjection", B_BOOL_TYPE, 0, &_fOrthoProj, sizeof(bool));
		file.ReadAttr("Exact", B_INT32_TYPE, 0, &fExactFlag, sizeof(int32));
		file.ReadAttr("Nearby", B_INT32_TYPE, 0, &fNearbyFlag, sizeof(int32));
		file.ReadAttr("RemoveUnconnected", B_INT32_TYPE, 0, &fRemoveUnconnectedFlag, sizeof(int32));
		file.ReadAttr("FillHoles", B_INT32_TYPE, 0, &fFillHolesFlag, sizeof(int32));
		file.ReadAttr("NormalDirections", B_INT32_TYPE, 0, &fNormalDirectionsFlag, sizeof(int32));
		file.ReadAttr("NormalValues", B_INT32_TYPE, 0, &fNormalValuesFlag, sizeof(int32));
		file.ReadAttr("ReverseAll", B_INT32_TYPE, 0, &fReverseAllFlag, sizeof(int32));
		file.ReadAttr("Iterations", B_INT32_TYPE, 0, &fIterationsValue, sizeof(int32));

		MoveTo(_windowRect.left, _windowRect.top);
		ResizeTo(_windowRect.Width(), _windowRect.Height());

		fShowBoundingBox = _fShowBoundingBox;
		fStlView->ShowBoundingBox(fShowBoundingBox);

		fShowAxes = _fShowAxes;
		fShowAxesPlane = _fShowAxesPlane;
		fShowAxesCompass = _fShowAxesCompass;
		fStlView->ShowAxes(fShowAxes, fShowAxesPlane, fShowAxesCompass);

		fShowOXY = _fShowOXY;
		fStlView->ShowOXY(fShowOXY);

		fShowMode = _fShowMode;
		fStlView->SetViewMode(fShowMode);

		fViewOrtho = _fOrthoProj;
		fStlView->SetOrthographic(fViewOrtho);

		fShowStat = _showStat;
		if (fShowStat)
			UpdateStats();

		UpdateUIStates(IsLoaded());

		file.Unlock();
	}
}

void
STLWindow::SaveSettings(void)
{
	BPath path;
	if (find_directory (B_USER_SETTINGS_DIRECTORY, &path) == B_OK) {
		path.Append(APP_SETTINGS_FILENAME);

		BFile file(path.Path(), B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
		if (file.InitCheck() != B_OK || file.Lock() != B_OK)
			return;

		BRect _windowRect = Frame();

		file.WriteAttr("WindowRect", B_RECT_TYPE, 0, &_windowRect, sizeof(BRect));

		file.WriteAttr("ShowAxes", B_BOOL_TYPE, 0, &fShowAxes, sizeof(bool));
		file.WriteAttr("ShowAxesPlane", B_BOOL_TYPE, 0, &fShowAxesPlane, sizeof(bool));
		file.WriteAttr("ShowAxesCompass", B_BOOL_TYPE, 0, &fShowAxesCompass, sizeof(bool));
		file.WriteAttr("ShowOXY", B_BOOL_TYPE, 0, &fShowOXY, sizeof(bool));
		file.WriteAttr("ShowBoundingBox", B_BOOL_TYPE, 0, &fShowBoundingBox, sizeof(bool));
		file.WriteAttr("ShowStat", B_BOOL_TYPE, 0, &fShowStat, sizeof(bool));
		file.WriteAttr("ShowMode", B_UINT32_TYPE, 0, &fShowMode, sizeof(uint32));
		file.WriteAttr("OrthographicProjection", B_BOOL_TYPE, 0, &fViewOrtho, sizeof(bool));
		file.WriteAttr("Exact", B_INT32_TYPE, 0, &fExactFlag, sizeof(int32));
		file.WriteAttr("Nearby", B_INT32_TYPE, 0, &fNearbyFlag, sizeof(int32));
		file.WriteAttr("RemoveUnconnected", B_INT32_TYPE, 0, &fRemoveUnconnectedFlag, sizeof(int32));
		file.WriteAttr("FillHoles", B_INT32_TYPE, 0, &fFillHolesFlag, sizeof(int32));
		file.WriteAttr("NormalDirections", B_INT32_TYPE, 0, &fNormalDirectionsFlag, sizeof(int32));
		file.WriteAttr("NormalValues", B_INT32_TYPE, 0, &fNormalValuesFlag, sizeof(int32));
		file.WriteAttr("ReverseAll", B_INT32_TYPE, 0, &fReverseAllFlag, sizeof(int32));
		file.WriteAttr("Iterations", B_INT32_TYPE, 0, &fIterationsValue, sizeof(int32));

		file.Sync();
		file.Unlock();
	}
}

void
STLWindow::WindowActivated(bool active)
{
	if (active) {
		BMessage *message = new BMessage(MSG_WINDOW_ACTIVATED);
		message->AddPointer("window", this);
		be_app->PostMessage(message);
	}
}

bool
STLWindow::QuitRequested()
{
	if (fStlModified) {
		BPath path(fOpenedFileName);
		BString alertText(B_TRANSLATE("Save changes to document '%filename%' ?"));
		alertText.ReplaceFirst("%filename%", path.Leaf());
		BAlert* alert = new BAlert(B_TRANSLATE("Save"), alertText,
			B_TRANSLATE("Cancel"), B_TRANSLATE("Don't save"), B_TRANSLATE("Save"), B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
		alert->SetShortcut(0, B_ESCAPE);
		int32 choice = alert->Go();
		switch (choice) {
			case 0:
				return false;
			case 1:
				return true;
			case 2:
			default:
			{
				BPath path(fOpenedFileName);
				if (fStlObject->stats.type == binary)
					stl_write_binary(fStlObject, path.Path(), fStlObject->stats.header);
				else
					stl_write_ascii(fStlObject, path.Path(), fStlObject->stats.header);

				fStlModified = false;
				return true;
			}
		}
	}
	return true;
}

void 
STLWindow::MessageReceived(BMessage *message)
{
	if (message->WasDropped())
		message->what = B_REFS_RECEIVED;

	switch (message->what) {
		case B_KEY_DOWN:
		case B_UNMAPPED_KEY_DOWN:
		{
			if (IsLoaded()) {
				uint32 key = message->FindInt32("key");
				uint32 modifiers = message->FindInt32("modifiers");

				float scaleFactor = fStlView->ScaleFactor();
				float scaleDelta = (GetZDepth() + scaleFactor) * 0.053589838958;
				float xRotate = fStlView->XRotate();
				float yRotate = fStlView->YRotate();
				float rotateDelta = 5.0;

				if (modifiers & B_SHIFT_KEY) {
					scaleDelta /= 5.0;
					rotateDelta /= 5.0;
				}
				if (modifiers & B_CONTROL_KEY) {
					scaleDelta *= 2.0;
					rotateDelta = 90.0;
				}

				switch (key) {
					case 0x25: // Zoom [-]
					case 0x1C:
						scaleFactor += scaleDelta;
						break;
					case 0x3A: // Zoom [+]
					case 0x1D:
						scaleFactor -= scaleDelta;
						break;
					case 0x61: // Left
						yRotate -= rotateDelta;
						break;
					case 0x63: // Right
						yRotate += rotateDelta;
						break;
					case 0x57: // Up
						xRotate -= rotateDelta;
						break;
					case 0x62: // Down
						xRotate += rotateDelta;
						break;
					case 0x5E: // Reset position [Space]
						fStlView->Reset(false, true, true);
						return;
					case 0x64: // Reset scale [0]
					case 0x1B:
						fStlView->Reset(true, false, false);
						return;
				}
				fStlView->SetScaleFactor(scaleFactor);
				fStlView->SetXRotate(xRotate);
				fStlView->SetYRotate(yRotate);
			}
			break;
		}
		case MSG_FILE_OPEN:
		{
			if (fOpenFilePanel == NULL) {
				fOpenFilePanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL,
					B_FILE_NODE, true, NULL, NULL, false, true);
				fOpenFilePanel->SetTarget(this);
			}

			BMessage *openMsg = new BMessage(B_REFS_RECEIVED);
			fOpenFilePanel->SetMessage(openMsg);
			delete openMsg;

			fOpenFilePanel->Show();
			break;
		}
		case MSG_FILE_SAVE:
		{
			BPath path(fOpenedFileName);
			if (fStlObject->stats.type == binary)
				stl_write_binary(fStlObject, path.Path(), fStlObject->stats.header);
			else
				stl_write_ascii(fStlObject, path.Path(), fStlObject->stats.header);
			BNode node(path.Path());
			BNodeInfo nodeInfo(&node);
			nodeInfo.SetType("application/sla");
			fStlModified = false;
			UpdateUIStates(true);
			break;
		}
		case MSG_FILE_EXPORT_STLA:
		case MSG_FILE_EXPORT_STLB:
		case MSG_FILE_EXPORT_DXF:
		case MSG_FILE_EXPORT_VRML:
		case MSG_FILE_EXPORT_OFF:
		case MSG_FILE_EXPORT_OBJ:
		{
			BMessage *fileMsg = new BMessage(*message);
			fileMsg->AddInt32("format", message->what);
			fileMsg->what = B_SAVE_REQUESTED;

			if (fSaveFilePanel == NULL) {
				fSaveFilePanel = new BFilePanel(B_SAVE_PANEL, NULL, NULL,
					B_FILE_NODE, true, fileMsg, NULL, false, true);
				fSaveFilePanel->SetTarget(this);
			} else {
				fSaveFilePanel->SetMessage(fileMsg);
				fSaveFilePanel->SetSaveText("");
			}
			BPath openedFile(fOpenedFileName);
			BString title(B_TRANSLATE(
				"Save '%document%' as a '%format%' file as" B_UTF8_ELLIPSIS));
			title.ReplaceFirst("%document%", openedFile.Leaf());
			if (message->what == MSG_FILE_EXPORT_STLA)
				title.ReplaceFirst("%format%", B_TRANSLATE("STL (ASCII)"));
			if (message->what == MSG_FILE_EXPORT_STLB)
				title.ReplaceFirst("%format%", B_TRANSLATE("STL (Binary)"));
			if (message->what == MSG_FILE_EXPORT_DXF)
				title.ReplaceFirst("%format%", B_TRANSLATE("Autodesk DXF"));
			if (message->what == MSG_FILE_EXPORT_VRML)
				title.ReplaceFirst("%format%", B_TRANSLATE("VRML"));
			if (message->what == MSG_FILE_EXPORT_OFF)
				title.ReplaceFirst("%format%", B_TRANSLATE("Geomview OFF"));
			if (message->what == MSG_FILE_EXPORT_OBJ)
				title.ReplaceFirst("%format%", B_TRANSLATE("Wavefront OBJ"));
			fSaveFilePanel->Window()->SetTitle(title.String());
			fSaveFilePanel->Show();
			delete fileMsg;
			break;
		}
		case MSG_FILE_CLOSE:
		{
			CloseFile();
			break;
		}
		case MSG_FILE_OPENED:
		{
			fZDepth = -5;
			fMaxExtent = 10;

			BPath path(fOpenedFileName);
			SetTitle(path.Leaf());

			TransformPosition();
			fStlView->SetSTL(fStlObject);

			fErrorTimeCounter = 0;
			fStlLoading = false;
			fStlModified = false;
			fStlValid = true;
			UpdateUI();

			fStlLogoView->Hide();
			fStlView->Show();

			break;
		}
		case MSG_FILE_OPEN_FAILED:
		{
			CloseFile();
			fErrorTimeCounter = 4;
			fStlLoading = false;
			break;
		}
		case MSG_PULSE:
		{
			if (fErrorTimeCounter > 1) {
				fErrorTimeCounter--;
				fStlLogoView->SetText(B_TRANSLATE("Unknown file format!"));
				fStlLogoView->SetTextColor(255, 25, 25);
			} else if (fErrorTimeCounter == 1) {
				fErrorTimeCounter--;
				fStlLogoView->SetText(B_TRANSLATE("Drop STL files here"));
				fStlLogoView->SetTextColor(255, 255, 255);
			}
			break;
		}
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			for (int32 i = 0; message->FindRef("refs", i, &ref) == B_OK; i++) {
				BEntry entry(&ref, true);
				if (!entry.Exists())
					continue;
				BPath path;
				if (entry.GetPath(&path) != B_OK)
					continue;
				if (i==0 && !IsLoaded() && !IsLoading()) {
					OpenFile(path.Path());
				} else {
					BMessage *msg = new BMessage(B_REFS_RECEIVED);
					msg->AddRef("refs", &ref);
					be_app->PostMessage(msg);
				}
			}
			break;
		}
		case B_SAVE_REQUESTED:
		{
			entry_ref ref;
			if (message->FindRef("directory", 0, &ref) == B_OK) {
				BEntry entry = BEntry(&ref);
				BPath path;
				entry.GetPath(&path);
				BString filename = message->FindString("name");
				path.Append(filename);
				uint32 format = message->FindInt32("format");
				BString mime("application/sla");
				switch (format) {
					case MSG_FILE_EXPORT_STLA:
						stl_write_ascii(fStlObject, path.Path(), fStlObject->stats.header);
						break;
					case MSG_FILE_EXPORT_STLB:
						stl_write_binary(fStlObject, path.Path(), fStlObject->stats.header);
						break;
					case MSG_FILE_EXPORT_DXF:
						stl_write_dxf(fStlObject, (char*)path.Path(), fStlObject->stats.header);
						mime.SetTo("application/dxf");
						break;
					case MSG_FILE_EXPORT_VRML:
						stl_repair(fStlObject, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1);
						stl_generate_shared_vertices(fStlObject);
						stl_write_vrml(fStlObject, (char*)path.Path());
						mime.SetTo("text/plain");
						break;
					case MSG_FILE_EXPORT_OFF:
						stl_repair(fStlObject, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1);
						stl_generate_shared_vertices(fStlObject);
						stl_write_off(fStlObject, (char*)path.Path());
						mime.SetTo("text/plain");
						break;
					case MSG_FILE_EXPORT_OBJ:
						stl_repair(fStlObject, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1);
						stl_generate_shared_vertices(fStlObject);
						stl_write_obj(fStlObject, (char*)path.Path());
						mime.SetTo("text/plain");
						break;
				}
				BNode node(path.Path());
				BNodeInfo nodeInfo(&node);
				nodeInfo.SetType(mime.String());

				fStlModified = false;
				UpdateUI();
			}
			break;
		}
		case B_CANCEL:
			break;
		case B_ABOUT_REQUESTED:
		{
			BAboutWindow* wind = new BAboutWindow(MAIN_WIN_TITLE, APP_SIGNATURE);
			wind->AddCopyright(2021, "Gerasim Troeglazov (3dEyes**)");
			wind->AddCopyright(2021, "Enrique M.G. (Lt-Henry)");
			wind->AddDescription(
				"This program is free software; you can redistribute it and/or modify "
				"it under the terms of the GNU General Public License as published by "
				"the Free Software Foundation; either version 2 of the License, or "
				"(at your option) any later version.\n\n"
				"This program is distributed in the hope that it will be useful, "
 				"but WITHOUT ANY WARRANTY; without even the implied warranty of "
 				"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
 				"GNU General Public License for more details.\n\n"
				"You should have received a copy of the GNU General Public License along"
 				"with this program; if not, write to the Free Software Foundation, Inc., "
 				"51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA." );
			wind->Show();
			break;
		}
		case MSG_VIEWMODE_OXY:
		{
			fShowOXY = !fShowOXY;
			fStlView->ShowOXY(fShowOXY);
			UpdateUI();
			break;
		}
		case MSG_VIEWMODE_AXES:
		{
			fShowAxes = !fShowAxes;
			if (fShowAxes && !fShowAxesPlane && !fShowAxesCompass) {
				fShowAxesPlane = true;
				fShowAxesCompass = true;
			}
			fStlView->ShowAxes(fShowAxes, fShowAxesPlane, fShowAxesCompass);
			UpdateUI();
			break;
		}
		case MSG_VIEWMODE_AXES_PLANE:
		{
			fShowAxesPlane = !fShowAxesPlane;
			if (fShowAxes && !fShowAxesPlane && !fShowAxesCompass) {
				fShowAxes = false;
			}
			fStlView->ShowAxes(fShowAxes, fShowAxesPlane, fShowAxesCompass);
			UpdateUI();
			break;
		}
		case MSG_VIEWMODE_AXES_COMPASS:
		{
			fShowAxesCompass = !fShowAxesCompass;
			if (fShowAxes && !fShowAxesPlane && !fShowAxesCompass) {
				fShowAxes = false;
			}
			fStlView->ShowAxes(fShowAxes, fShowAxesPlane, fShowAxesCompass);
			UpdateUI();
			break;
		}
		case MSG_VIEWMODE_BOUNDING_BOX:
		{
			fShowBoundingBox = !fShowBoundingBox;
			fStlView->ShowBoundingBox(fShowBoundingBox);
			UpdateUI();
			break;
		}
		case MSG_VIEWMODE_RESETPOS:
		{
			fStlView->Reset();
			UpdateUI();
			break;
		}
		case MSG_VIEWMODE_ZOOMIN:
		{
			float scaleFactor = fStlView->ScaleFactor();
			float scaleDelta = (GetZDepth() + scaleFactor) * 0.053589838958;
			fStlView->SetScaleFactor(scaleFactor - scaleDelta);
			UpdateUI();
			break;
		}
		case MSG_VIEWMODE_ZOOMOUT:
		{
			float scaleFactor = fStlView->ScaleFactor();
			float scaleDelta = (GetZDepth() + scaleFactor) * 0.053589838958;
			fStlView->SetScaleFactor(scaleFactor + scaleDelta);
			UpdateUI();
			break;
		}
		case MSG_VIEWMODE_ZOOMFIT:
		{
			fStlView->Reset(true, false, false);
			UpdateUI();
			break;
		}
		case MSG_VIEWMODE_FRONT:
		{
			fStlView->Reset(true, false, true);
			fStlView->SetXRotate(-90);
			fStlView->SetYRotate(0);
			UpdateUI();
			break;
		}
		case MSG_VIEWMODE_TOP:
		{
			fStlView->Reset(true, false, true);
			fStlView->SetXRotate(0);
			fStlView->SetYRotate(0);
			UpdateUI();
			break;
		}
		
		case MSG_VIEWMODE_ORTHO:
		{
			fViewOrtho=!fViewOrtho;
			UpdateUI();
			
			fStlView->SetOrthographic(fViewOrtho);
			break;
		}
		
		case MSG_VIEWMODE_RIGHT:
		{
			fStlView->Reset(true, false, true);
			fStlView->SetXRotate(-90);
			fStlView->SetYRotate(-90);
			UpdateUI();
			break;
		}

		case MSG_FILE_RELOAD:
		{
			OpenFile(fOpenedFileName.String());
			break;
		}
		case MSG_TOOLS_MEASURE:
		{
			fMeasureMode = !fMeasureMode;

			if (fMeasureMode) {
				fMeasureWindow = new STLInputWindow(B_TRANSLATE("Measure"), this, MSG_TOOLS_MEASURE_DROP, BUTTON_RESET | BUTTON_CLOSE);
				fMeasureWindow->AddGroup("from", B_TRANSLATE("From:"), 3);
				fMeasureWindow->AddFloatField("x1", "", 0.0);
				fMeasureWindow->SetFieldEditable("x1", false);
				fMeasureWindow->SetFieldBackgroundColor("x1", {164, 255, 164});
				fMeasureWindow->AddFloatField("y1", "", 0.0);
				fMeasureWindow->SetFieldEditable("y1", false);
				fMeasureWindow->SetFieldBackgroundColor("y1", {164, 255, 164});
				fMeasureWindow->AddFloatField("z1", "", 0.0);
				fMeasureWindow->SetFieldEditable("z1", false);
				fMeasureWindow->SetFieldBackgroundColor("z1", {164, 255, 164});
				fMeasureWindow->AddGroup("to", B_TRANSLATE("To:"), 3);
				fMeasureWindow->AddFloatField("x2", "", 0.0);
				fMeasureWindow->SetFieldEditable("x2", false);
				fMeasureWindow->SetFieldBackgroundColor("x2", {255, 164, 164});
				fMeasureWindow->AddFloatField("y2", "", 0.0);
				fMeasureWindow->SetFieldEditable("y2", false);
				fMeasureWindow->SetFieldBackgroundColor("y2", {255, 164, 164});
				fMeasureWindow->AddFloatField("z2", "", 0.0);
				fMeasureWindow->SetFieldEditable("z2", false);
				fMeasureWindow->SetFieldBackgroundColor("z2", {255, 164, 164});
				fMeasureWindow->AddFloatField("distance", B_TRANSLATE("Distance:"), 0.0);
				fMeasureWindow->SetFieldEditable("distance", false);
				fMeasureWindow->Show();
			} else {
				if (fMeasureWindow) {
					fMeasureWindow->Lock();
					fMeasureWindow->Quit();
					fMeasureWindow = NULL;
				}
			}

			fStlView->SetMeasureMode(fMeasureMode);
			UpdateUI();
			break;
		}
		case MSG_TOOLS_MEASURE_DROP:
		{
			fMeasureMode = false;
			fStlView->SetMeasureMode(fMeasureMode);
			UpdateUI();
			break;
		}
		case MSG_TOOLS_MEASURE_UPDATE:
		{
			if (fMeasureMode) {
				fMeasureWindow->SetFloatFieldValue("x1", message->FindFloat("x1"));
				fMeasureWindow->SetFloatFieldValue("y1", message->FindFloat("y1"));
				fMeasureWindow->SetFloatFieldValue("z1", message->FindFloat("z1"));
				fMeasureWindow->SetFloatFieldValue("x2", message->FindFloat("x2"));
				fMeasureWindow->SetFloatFieldValue("y2", message->FindFloat("y2"));
				fMeasureWindow->SetFloatFieldValue("z2", message->FindFloat("z2"));
				fMeasureWindow->SetFloatFieldValue("distance", message->FindFloat("distance"));
			}
			break;
		}
		case MSG_VIEWMODE_STAT:
		{
			fShowStat = !fShowStat;
			UpdateUI();
			break;
		}
		case MSG_TOOLS_REPAIR:
		{
			BMessage *options = new BMessage();
			options->AddInt32("fExactFlag", fExactFlag);
			options->AddInt32("fNearbyFlag", fNearbyFlag);
			options->AddInt32("fRemoveUnconnectedFlag", fRemoveUnconnectedFlag);
			options->AddInt32("fFillHolesFlag", fFillHolesFlag);
			options->AddInt32("fNormalDirectionsFlag", fNormalDirectionsFlag);
			options->AddInt32("fNormalValuesFlag", fNormalValuesFlag);
			options->AddInt32("fReverseAllFlag", fReverseAllFlag);
			options->AddInt32("fIterationsValue", fIterationsValue);
			options->AddFloat("toleranceValue", fStlObject->stats.shortest_edge);
			options->AddFloat("incrementValue", fStlObject->stats.bounding_diameter / 10000.0);
			STLRepairWindow *repairDialog = new STLRepairWindow(this, MSG_TOOLS_REPAIR_DO, options);
			repairDialog->Show();
			UpdateUIStates(false);
			break;
		}
		case MSG_TOOLS_REPAIR_DO:
		{
			fExactFlag = message->FindInt32("fExactFlag");
			fNearbyFlag = message->FindInt32("fNearbyFlag");
			fRemoveUnconnectedFlag = message->FindInt32("fRemoveUnconnectedFlag");
			fFillHolesFlag = message->FindInt32("fFillHolesFlag");
			fNormalDirectionsFlag = message->FindInt32("fNormalDirectionsFlag");
			fNormalValuesFlag = message->FindInt32("fNormalValuesFlag");
			fReverseAllFlag = message->FindInt32("fReverseAllFlag");
			fIterationsValue = message->FindInt32("fIterationsValue");
			float toleranceValue = message->FindInt32("toleranceValue");
			float incrementValue = message->FindInt32("incrementValue");
			if (IsLoaded()) {
				stl_repair(fStlObject, 0, fExactFlag, 1, toleranceValue, 1, incrementValue, fNearbyFlag,
					fIterationsValue, fRemoveUnconnectedFlag, fFillHolesFlag, fNormalDirectionsFlag,
					fNormalValuesFlag, fReverseAllFlag, 0);
				fStlModified = true;
				fStlView->Reload();
				UpdateUI();
			}
			break;
		}
		case MSG_TOOLS_EDIT_TITLE:
		{
			STLInputWindow *input = new STLInputWindow(B_TRANSLATE("STL Title"), this, MSG_TOOLS_TITLE_SET);
			input->AddTextField("title", B_TRANSLATE("Title:"), (const char*)fStlObject->stats.header);
			input->Show();
			UpdateUIStates(false);
			break;
		}
		case MSG_TOOLS_TITLE_SET:
		{
			const char *value = message->FindString("title");
			if (value != NULL && IsLoaded()) {
				snprintf(fStlObject->stats.header, 80, value);
				fStlModified = true;
				UpdateUI();
			}
			break;
		}
		case MSG_TOOLS_SCALE:
		{
			STLInputWindow *input = new STLInputWindow(B_TRANSLATE("Scale"), this, MSG_TOOLS_SCALE_SET);
			input->AddFloatField("scale", B_TRANSLATE("Scale factor:"), 1.0);
			input->Show();
			UpdateUIStates(false);
			break;
		}
		case MSG_TOOLS_SCALE_SET:
		{
			float value = message->FindFloat("scale");
			if (IsLoaded()) {
				
				stl_scale(fStlObject, value);
				
				fStlModified = true;
				UpdateUI();
				fStlView->HidePreview();
				fStlView->Reload();
			}
			break;
		}
		case MSG_TOOLS_SCALE_3:
		{
			STLInputWindow *input = new STLInputWindow(B_TRANSLATE("Custom axis scale"), this, MSG_TOOLS_SCALE_SET_3);
			input->AddFloatField("x", B_TRANSLATE("Scale X factor:"), 1.0);
			input->SetFieldBackgroundColor("x", {255, 164, 164});
			input->AddFloatField("y", B_TRANSLATE("Scale Y factor:"), 1.0);
			input->SetFieldBackgroundColor("y", {164, 255, 164});
			input->AddFloatField("z", B_TRANSLATE("Scale Z factor:"), 1.0);
			input->SetFieldBackgroundColor("z", {164, 164, 255});
			input->Show();
			UpdateUIStates(false);
			break;
		}
		case MSG_TOOLS_SCALE_SET_3:
		{
			float values[3];
			values[0] = message->FindFloat("x");
			values[1] = message->FindFloat("y");
			values[2] = message->FindFloat("z");
			
			if (IsLoaded()) {
				stl_scale_versor(fStlObject, values);
				
				fStlModified = true;
				UpdateUI();
				fStlView->HidePreview();
				fStlView->Reload();
			}
			break;
		}
		case MSG_TOOLS_ROTATE:
		{
			STLInputWindow *input = new STLInputWindow(B_TRANSLATE("Rotate"), this, MSG_TOOLS_ROTATE_SET);
			input->AddSliderField("x", B_TRANSLATE("X-axis:"), 0, -180, 180);
			input->SetFieldBackgroundColor("x", {255, 164, 164});
			input->AddSliderField("y", B_TRANSLATE("Y-axis:"), 0, -180, 180);
			input->SetFieldBackgroundColor("y", {164, 255, 164});
			input->AddSliderField("z", B_TRANSLATE("Z-axis:"), 0, -180, 180);
			input->SetFieldBackgroundColor("z", {164, 164, 255});
			input->Show();
			UpdateUIStates(false);
			break;
		}
		case MSG_TOOLS_ROTATE_SET:
		{
			float values[3];
			values[0] = message->FindFloat("x");
			values[1] = message->FindFloat("y");
			values[2] = message->FindFloat("z");

			if (IsLoaded()) {
				stl_rotate_x(fStlObject, values[0]);
				stl_rotate_y(fStlObject, values[1]);
				stl_rotate_z(fStlObject, values[2]);
				
				fStlModified = true;
				UpdateUI();
				fStlView->HidePreview();
				fStlView->Reload();
			}
			break;
		}
		case MSG_TOOLS_MOVE_CENTER:
		{
			stl_translate(fStlObject, -fStlObject->stats.size.x / 2, -fStlObject->stats.size.y / 2, -fStlObject->stats.size.z / 2);
			fStlModified = true;
			fStlView->Reload();
			UpdateUI();
			break;
		}
		case MSG_TOOLS_MOVE_MIDDLE:
		{
			stl_translate(fStlObject, -fStlObject->stats.size.x / 2, -fStlObject->stats.size.y / 2, 0);
			fStlModified = true;
			fStlView->Reload();
			UpdateUI();
			break;
		}
		case MSG_TOOLS_MOVE_ZERO:
		{
			stl_translate(fStlObject, 0, 0, 0);
			fStlModified = true;
			fStlView->Reload();
			UpdateUI();
			break;
		}
		case MSG_TOOLS_MOVE_TO:
		{
			STLInputWindow *input = new STLInputWindow(B_TRANSLATE("Move to"), this, MSG_TOOLS_MOVE_TO_SET);
			input->AddFloatField("x", B_TRANSLATE("X:"), fStlObject->stats.min.x);
			input->SetFieldBackgroundColor("x", {255, 164, 164});
			input->AddFloatField("y", B_TRANSLATE("Y:"), fStlObject->stats.min.y);
			input->SetFieldBackgroundColor("y", {164, 255, 164});
			input->AddFloatField("z", B_TRANSLATE("Z:"), fStlObject->stats.min.z);
			input->SetFieldBackgroundColor("z", {164, 164, 255});
			input->Show();
			UpdateUIStates(false);
			break;
		}
		case MSG_TOOLS_MOVE_TO_SET:
		{
			float values[3];
			values[0] = message->FindFloat("x");
			values[1] = message->FindFloat("y");
			values[2] = message->FindFloat("z");
			if (IsLoaded()) {
				stl_translate(fStlObject, values[0], values[1], values[2]);
				fStlModified = true;
				UpdateUI();
				fStlView->HidePreview();
				fStlView->Reload();
			}
			break;
		}
		case MSG_TOOLS_MOVE_BY:
		{
			STLInputWindow *input = new STLInputWindow(B_TRANSLATE("Move by"), this, MSG_TOOLS_MOVE_BY_SET);
			input->AddFloatField("x", B_TRANSLATE("∆X:"), 0.0);
			input->SetFieldBackgroundColor("x", {255, 164, 164});
			input->AddFloatField("y", B_TRANSLATE("∆Y:"), 0.0);
			input->SetFieldBackgroundColor("y", {164, 255, 164});
			input->AddFloatField("z", B_TRANSLATE("∆Z:"), 0.0);
			input->SetFieldBackgroundColor("z", {164, 164, 255});
			input->Show();
			UpdateUIStates(false);
			break;
		}
		case MSG_TOOLS_MOVE_BY_SET:
		{
			float values[3];
			values[0] = message->FindFloat("x");
			values[1] = message->FindFloat("y");
			values[2] = message->FindFloat("z");
			if (IsLoaded()) {
				stl_translate_relative(fStlObject, values[0], values[1], values[2]);
				fStlModified = true;
				UpdateUI();
				fStlView->HidePreview();
				fStlView->Reload();
			}
			break;
		}
		case MSG_TOOLS_MIRROR_XY:
		{
			stl_mirror_xy(fStlObject);
			fStlModified = true;
			fStlView->Reload();
			UpdateUI();
			break;
		}
		case MSG_TOOLS_MIRROR_YZ:
		{
			stl_mirror_yz(fStlObject);
			fStlModified = true;
			fStlView->Reload();
			UpdateUI();
			break;
		}
		case MSG_TOOLS_MIRROR_XZ:
		{
			stl_mirror_xz(fStlObject);
			fStlModified = true;
			fStlView->Reload();
			UpdateUI();
			break;
		}
		case MSG_INPUT_VALUE_UPDATED:
		{
			int32 action = message->FindInt32("action");

			switch(action) {
				case MSG_TOOLS_SCALE_SET:
				{
					float s = message->FindFloat("scale");

					glm::mat4 matrix;
					matrix = glm::scale(glm::mat4(1.0f), glm::vec3(s,s,s));

					fStlView->ShowPreview(glm::value_ptr(matrix));
					break;
				}
				case MSG_TOOLS_SCALE_SET_3:
				{
					float sx = message->FindFloat("x");
					float sy = message->FindFloat("y");
					float sz = message->FindFloat("z");

					glm::mat4 matrix;
					matrix = glm::scale(glm::mat4(1.0f), glm::vec3(sx,sy,sz));

					fStlView->ShowPreview(glm::value_ptr(matrix));
					break;
				}
				case MSG_TOOLS_MOVE_BY_SET:
				{
					float tx = message->FindFloat("x");
					float ty = message->FindFloat("y");
					float tz = message->FindFloat("z");

					glm::mat4 matrix;
					matrix = glm::translate(glm::mat4(1.0f), glm::vec3(tx,ty,tz));

					fStlView->ShowPreview(glm::value_ptr(matrix));
					break;
				}
				case MSG_TOOLS_MOVE_TO_SET:
				{
					float tx = message->FindFloat("x");
					float ty = message->FindFloat("y");
					float tz = message->FindFloat("z");

					tx-=fStlObject->stats.min.x;
					ty-=fStlObject->stats.min.y;
					tz-=fStlObject->stats.min.z;
					glm::mat4 matrix;
					matrix = glm::translate(glm::mat4(1.0f), glm::vec3(tx,ty,tz));
					
					fStlView->ShowPreview(glm::value_ptr(matrix));
					break;
				}		
				case MSG_TOOLS_ROTATE_SET:
				{
					float rx = message->FindFloat("x");
					float ry = message->FindFloat("y");
					float rz = message->FindFloat("z");

					rx = rx * M_PI / 180.0f;
					ry = ry * M_PI / 180.0f;
					rz = rz * M_PI / 180.0f;

					glm::mat4 matrix;
					glm::mat4 mx, my, mz;
					mx = glm::rotate(glm::mat4(1.0f), rx, glm::vec3(1.0,0.0,0.0));
					my = glm::rotate(glm::mat4(1.0f), ry, glm::vec3(0.0,1.0,0.0));
					mz = glm::rotate(glm::mat4(1.0f), rz, glm::vec3(0.0,0.0,1.0));

					matrix = mz * my * mx;
					fStlView->ShowPreview(glm::value_ptr(matrix));
					break;
				}
				case MSG_TOOLS_MEASURE_DROP:
				{
					int32 extended = message->FindInt32("extended");
					if (extended == MSG_INPUT_RESET) {
						fStlView->SetMeasureMode(false);
						fStlView->SetMeasureMode(fMeasureMode);
					}
					break;
				}
				default:
					break;
			}
			break;
		}	
		case MSG_INPUT_CANCEL:
		{
			fStlView->HidePreview();
			UpdateUI();
			break;
		}
		case MSG_VIEWMODE_POINTS:
		case MSG_VIEWMODE_WIREFRAME:
		case MSG_VIEWMODE_SOLID:
		{
			fShowMode = message->what;
			fStlView->SetViewMode(fShowMode);
			UpdateUI();
			break;
		}
		case MSG_EASTER_EGG:
		{
			app_info info;
			be_app->GetAppInfo(&info);
			BFile file(&info.ref, B_READ_ONLY);

			BResources res;
			if (res.SetTo(&file) != B_OK)
				break;

			size_t size;
			const void* data = res.LoadResource('rSTL', "Haiku.stl", &size);
			if (data == NULL)
				break;

			BFile eggFile("/tmp/Haiku", B_WRITE_ONLY | B_CREATE_FILE);
			eggFile.Write(data, size);

			OpenFile("/tmp/Haiku");

			break;
		}
		case MSG_POPUP_MENU:
		{
			BPoint point;
			uint32 buttons;
			fStlView->GetMouse(&point, &buttons);

			BPopUpMenu* menu = new BPopUpMenu("PopUpMenu",false,false);

			BMenuItem *_menuItemPoints = new BMenuItem(B_TRANSLATE("Points"), new BMessage(MSG_VIEWMODE_POINTS));
			_menuItemPoints->SetMarked(fShowMode == MSG_VIEWMODE_POINTS);
			BMenuItem *_menuItemWireframe = new BMenuItem(B_TRANSLATE("Wireframe"), new BMessage(MSG_VIEWMODE_WIREFRAME));
			_menuItemWireframe->SetMarked(fShowMode == MSG_VIEWMODE_WIREFRAME);
			BMenuItem *_menuItemSolid = new BMenuItem(B_TRANSLATE("Solid"), new BMessage(MSG_VIEWMODE_SOLID));
			_menuItemSolid->SetMarked(fShowMode == MSG_VIEWMODE_SOLID);

			menu->AddItem(_menuItemPoints);
			menu->AddItem(_menuItemWireframe);
			menu->AddItem(_menuItemSolid);
			menu->AddSeparatorItem();

			BMenuItem *_menuItemShowAxes = new BMenuItem(B_TRANSLATE("Axes"), new BMessage(MSG_VIEWMODE_AXES));
			_menuItemShowAxes->SetMarked(fShowAxes);
			BMenuItem *_menuItemShowOXY = new BMenuItem(B_TRANSLATE("Plane OXY"), new BMessage(MSG_VIEWMODE_OXY));
			_menuItemShowOXY->SetMarked(fShowOXY);
			BMenuItem *_menuItemShowBox = new BMenuItem(B_TRANSLATE("Bounding box"), new BMessage(MSG_VIEWMODE_BOUNDING_BOX));
			_menuItemShowBox->SetMarked(fShowBoundingBox);

			menu->AddItem(_menuItemShowAxes);
			menu->AddItem(_menuItemShowOXY);
			menu->AddItem(_menuItemShowBox);
			menu->AddSeparatorItem();
			menu->AddItem(new BMenuItem(B_TRANSLATE("Reset"), new BMessage(MSG_VIEWMODE_RESETPOS), 'R'));
			menu->AddSeparatorItem();
			menu->AddItem(new BMenuItem(B_TRANSLATE("Edit title" B_UTF8_ELLIPSIS), new BMessage(MSG_TOOLS_EDIT_TITLE)));
			menu->AddSeparatorItem();
			menu->AddItem(fMenuToolsScale);
			menu->AddItem(fMenuToolsMove);
			menu->AddItem(fMenuToolsMirror);
			menu->AddItem(new BMenuItem(B_TRANSLATE("Rotate" B_UTF8_ELLIPSIS), new BMessage(MSG_TOOLS_ROTATE)));
			menu->AddSeparatorItem();
			menu->AddItem(new BMenuItem(B_TRANSLATE("Repair" B_UTF8_ELLIPSIS), new BMessage(MSG_TOOLS_REPAIR)));
			menu->SetTargetForItems(this);

			menu->Go(fStlView->ConvertToScreen(point), true, false, true);
			break;
		}
		case MSG_HELP_WIKI:
		{
			entry_ref ref;
			if (get_ref_for_path("/bin/open", &ref))
				break;

			const char* args[] = { URL_HOMEPAGE_WIKI, NULL };
			be_roster->Launch(&ref, 1, args);
			break;
		}
		case MSG_APP_QUIT:
		{
			be_app->PostMessage(B_QUIT_REQUESTED);
			break;
		}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}

void
STLWindow::UpdateUIStates(bool show)
{
	bool locked = LockWithTimeout(0) == B_OK;

	fMenuItemClose->SetEnabled(show);
	fMenuView->SetEnabled(show);
	fMenuTools->SetEnabled(show);
	fMenuToolsMirror->SetEnabled(show);
	fMenuToolsScale->SetEnabled(show);
	fMenuToolsMove->SetEnabled(show);
	fMenuFileSaveAs->SetEnabled(show);
	fMenuItemReload->SetEnabled(show);
	fMenuItemSave->SetEnabled(show && fStlModified);
	fMenuItemShowBox->SetMarked(fShowBoundingBox);
	fMenuItemShowAxes->SetMarked(fShowAxes);
	fMenuItemShowAxesPlane->SetMarked(fShowAxesPlane);
	fMenuItemShowAxesCompass->SetMarked(fShowAxesCompass);
	fMenuItemShowAxesPlane->SetEnabled(fShowAxes);
	fMenuItemShowAxesCompass->SetEnabled(fShowAxes);
	fMenuItemShowOXY->SetMarked(fShowOXY);
	fMenuItemPoints->SetMarked(fShowMode == MSG_VIEWMODE_POINTS);
	fMenuItemWireframe->SetMarked(fShowMode == MSG_VIEWMODE_WIREFRAME);
	fMenuItemSolid->SetMarked(fShowMode == MSG_VIEWMODE_SOLID);
	fMenuItemOrthographicView->SetMarked(fViewOrtho);
	fMenuItemStat->SetEnabled(show);
	fMenuItemStat->SetMarked(fShowStat);

	fToolBar->SetActionEnabled(MSG_FILE_SAVE, show && fStlModified);
	fToolBar->SetActionEnabled(MSG_VIEWMODE_STAT, show);
	fToolBar->SetActionPressed(MSG_VIEWMODE_STAT, fShowStat);
	fToolBar->SetActionEnabled(MSG_TOOLS_EDIT_TITLE, show);
	fToolBar->SetActionEnabled(MSG_TOOLS_MIRROR_XY, show);
	fToolBar->SetActionEnabled(MSG_TOOLS_MIRROR_YZ, show);
	fToolBar->SetActionEnabled(MSG_TOOLS_MIRROR_XZ, show);
	fToolBar->SetActionEnabled(MSG_TOOLS_REPAIR, show);
	fToolBar->SetActionEnabled(MSG_TOOLS_SCALE, show);
	fToolBar->SetActionEnabled(MSG_TOOLS_SCALE_3, show);
	fToolBar->SetActionEnabled(MSG_TOOLS_ROTATE, show);
	fToolBar->SetActionEnabled(MSG_TOOLS_MOVE_TO, show);
	fToolBar->SetActionEnabled(MSG_TOOLS_MOVE_BY, show);
	fToolBar->SetActionEnabled(MSG_TOOLS_MOVE_MIDDLE, show);
	fToolBar->SetActionPressed(MSG_TOOLS_MEASURE, fMeasureMode);
	fToolBar->SetActionEnabled(MSG_TOOLS_MEASURE, show);

	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_POINTS, show);
	fViewToolBar->SetActionPressed(MSG_VIEWMODE_POINTS, fShowMode == MSG_VIEWMODE_POINTS);
	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_WIREFRAME, show);
	fViewToolBar->SetActionPressed(MSG_VIEWMODE_WIREFRAME, fShowMode == MSG_VIEWMODE_WIREFRAME);
	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_SOLID, show);
	fViewToolBar->SetActionPressed(MSG_VIEWMODE_SOLID, fShowMode == MSG_VIEWMODE_SOLID);
	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_AXES, show);
	fViewToolBar->SetActionPressed(MSG_VIEWMODE_AXES, fShowAxes);
	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_OXY, show);
	fViewToolBar->SetActionPressed(MSG_VIEWMODE_OXY, fShowOXY);
	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_BOUNDING_BOX, show);
	fViewToolBar->SetActionPressed(MSG_VIEWMODE_BOUNDING_BOX, fShowBoundingBox);
	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_FRONT, show);
	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_TOP, show);
	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_RIGHT, show);
	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_RESETPOS, show);
	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_ZOOMIN, show);
	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_ZOOMOUT, show);
	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_ZOOMFIT, show);
	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_ORTHO, show);
	fViewToolBar->SetActionPressed(MSG_VIEWMODE_ORTHO, fViewOrtho);

	if (locked)
		UnlockLooper();
}

void
STLWindow::SetSTL(stl_file *stl)
{
	fStlObject = stl;
	fStlView->SetSTL(stl);
}

void
STLWindow::OpenFile(const char *filename)
{	
	CloseFile();

	fOpenedFileName.SetTo(filename);
	fStlLogoView->SetText(B_TRANSLATE("Loading" B_UTF8_ELLIPSIS));

	fStlLoading = true;
	fFileLoaderThread = spawn_thread(_FileLoaderFunction, "loaderThread", B_NORMAL_PRIORITY, (void*)this);
	resume_thread(fFileLoaderThread);
}

void
STLWindow::CloseFile(void)
{
	if (IsLoaded()) {
		fStlLogoView->Show();
		fStlView->Hide();

		SetTitle(MAIN_WIN_TITLE);
		fStlValid = false;

		stl_close(fStlObject);
		delete fStlObject;
		fStlObject = NULL;

		fStlLogoView->SetText(B_TRANSLATE("Drop STL files here"));
		fStlLogoView->SetTextColor(255, 255, 255);
		fErrorTimeCounter = 0;
		UpdateUI();
	}
}

void
STLWindow::UpdateStats(void)
{
	bool isLoaded = IsLoaded();
	if (isLoaded)
		stl_calculate_volume(fStlObject);

	BPath path(fOpenedFileName);
	fStatView->SetTextValue("filename", isLoaded ? path.Leaf() : 0);
	fStatView->SetTextValue("type", isLoaded ? (fStlObject->stats.type == binary ? B_TRANSLATE("Binary") : B_TRANSLATE("ASCII")) : "");
	fStatView->SetTextValue("title", isLoaded ? fStlObject->stats.header : "");

	fStatView->SetFloatValue("min-x", isLoaded ? fStlObject->stats.min.x : 0);
	fStatView->SetFloatValue("min-y", isLoaded ? fStlObject->stats.min.y : 0);
	fStatView->SetFloatValue("min-z", isLoaded ? fStlObject->stats.min.z : 0);
	fStatView->SetFloatValue("max-x", isLoaded ? fStlObject->stats.max.x : 0);
	fStatView->SetFloatValue("max-y", isLoaded ? fStlObject->stats.max.y : 0);
	fStatView->SetFloatValue("max-z", isLoaded ? fStlObject->stats.max.z : 0);
	fStatView->SetFloatValue("width", isLoaded ? fStlObject->stats.size.x : 0);
	fStatView->SetFloatValue("length", isLoaded ? fStlObject->stats.size.y : 0);
	fStatView->SetFloatValue("height", isLoaded ? fStlObject->stats.size.z : 0);
	fStatView->SetFloatValue("volume", isLoaded ? fStlObject->stats.volume : 0, false);
	fStatView->SetIntValue("num_facets", isLoaded ? fStlObject->stats.number_of_facets : 0);
	fStatView->SetIntValue("num_disconnected_facets",
		isLoaded ? (fStlObject->stats.facets_w_1_bad_edge + fStlObject->stats.facets_w_2_bad_edge +
		fStlObject->stats.facets_w_3_bad_edge) : 0);
	fStatView->SetIntValue("parts", isLoaded ? fStlObject->stats.number_of_parts : 0);
	fStatView->SetIntValue("degenerate", isLoaded ? fStlObject->stats.degenerate_facets : 0);
	fStatView->SetIntValue("edges", isLoaded ? fStlObject->stats.edges_fixed : 0);
	fStatView->SetIntValue("removed", isLoaded ? fStlObject->stats.facets_removed : 0);
	fStatView->SetIntValue("added", isLoaded ? fStlObject->stats.facets_added : 0);
	fStatView->SetIntValue("reversed", isLoaded ? fStlObject->stats.facets_reversed : 0);
	fStatView->SetIntValue("backward", isLoaded ? fStlObject->stats.backwards_edges : 0);
	fStatView->SetIntValue("normals", isLoaded ? fStlObject->stats.normals_fixed : 0);

	float widthStatView = fShowStat ? fStatView->PreferredSize().Width() : -1;
	fStatView->MoveTo(Bounds().right - widthStatView, fStlView->Frame().top);
	fStatView->ResizeTo(widthStatView, fStatView->Frame().Height());
	fStlView->ResizeTo((fStatView->Frame().left - fStlView->Frame().left) - 1, fStlView->Bounds().Height());
	fStlLogoView->ResizeTo((fStatView->Frame().left - fStlLogoView->Frame().left) - 1, fStlLogoView->Bounds().Height());
	SetSizeLimits(600, 4096, fStatView->Frame().top + fStatView->PreferredSize().Height(), 4049);
}

void
STLWindow::TransformPosition()
{
	stl_translate(fStlObject, 0, 0, 0);

	float xMaxExtent = 0;
	float yMaxExtent = 0;
	float zMaxExtent = 0;

	for (int i = 0 ; i < fStlObject->stats.number_of_facets ; i++) {
		for (int j = 0; j < 3; j++) {
			if (fStlObject->facet_start[i].vertex[j].x > xMaxExtent)
				xMaxExtent = fStlObject->facet_start[i].vertex[0].x;
			if (fStlObject->facet_start[i].vertex[j].y > yMaxExtent)
				yMaxExtent = fStlObject->facet_start[i].vertex[0].y;
			if (fStlObject->facet_start[i].vertex[j].z > zMaxExtent)
				zMaxExtent = fStlObject->facet_start[i].vertex[0].z;
		}
	}

	float longerSide = xMaxExtent > yMaxExtent ? xMaxExtent : yMaxExtent;
	longerSide += (zMaxExtent * (sin(FOV * (M_PI / 180.0)) / sin((90.0 - FOV) * (M_PI / 180.0))));

	fZDepth = -1.2 *((longerSide / 2.0) / tanf((FOV / 2.0) * (M_PI / 180.0)));

	if ((xMaxExtent > yMaxExtent) && (xMaxExtent > zMaxExtent))
    	fMaxExtent = xMaxExtent;
	if ((yMaxExtent > xMaxExtent) && (yMaxExtent > zMaxExtent))
		fMaxExtent = yMaxExtent;
	if ((zMaxExtent > yMaxExtent) && (zMaxExtent > xMaxExtent))
		fMaxExtent = zMaxExtent;

	stl_translate_relative(fStlObject, -xMaxExtent / 2.0, -yMaxExtent / 2.0, -zMaxExtent / 2.0);
}

int32
STLWindow::_RenderFunction(void *data)
{
	STLView *view = (STLView*)data;
	STLWindow *window = (STLWindow*)view->Window();

	while(window->IsRenderWork()) {
		view->Render();
		snooze(1000000 / FPS_LIMIT);
	}

	return 0;
}

int32
STLWindow::_FileLoaderFunction(void *data)
{
	STLWindow *window = (STLWindow*)data;

	stl_file *stl = new stl_file;

	stl_open(stl, (char*)window->Filename().String());

	if (stl_get_error(stl)) {
		window->SetSTL(NULL);
		window->PostMessage(MSG_FILE_OPEN_FAILED);
		delete stl;
	} else {
		stl_fix_normal_values(stl);
		window->SetSTL(stl);
		window->PostMessage(MSG_FILE_OPENED);
	}

	return 0;
}
