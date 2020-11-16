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

#include "STLApp.h"
#include "STLView.h"
#include "STLWindow.h"
#include "STLStatView.h"
#include "STLInputWindow.h"
#include "STLRepairWindow.h"
#include "STLToolBar.h"

STLWindow::STLWindow()
	: BWindow(BRect(100, 100, 100 + 720, 100 + 512), MAIN_WIN_TITLE, B_TITLED_WINDOW, 0),
	fOpenFilePanel(NULL),
	fSaveFilePanel(NULL),
	fStlModified(false),
	fShowStat(false),
	fShowWireframe(false),
	fShowBoundingBox(false),
	fShowAxes(false),
	fShowOXY(false),
	fExactFlag(false),
	fNearbyFlag(false),
	fRemoveUnconnectedFlag(false),
	fFillHolesFlag(false),
	fNormalDirectionsFlag(false),
	fNormalValuesFlag(false),
	fReverseAllFlag(false),
	fIterationsValue(2),
	fStlValid(false),
	fStlLoading(false),
	fStlObject(NULL),
	fStlObjectView(NULL),
	fStlObjectAppend(NULL),
	fErrorTimeCounter(0),
	fRenderWork(true),
	fZDepth(-5),
	fMaxExtent(10)
{
	fMenuBar = new BMenuBar(BRect(0, 0, Bounds().Width(), 22), "menubar");
	fMenuFile = new BMenu("File");
	fMenuFileSaveAs = new BMenu("Save as" B_UTF8_ELLIPSIS);
	fMenuView = new BMenu("View");
	fMenuTools = new BMenu("Tools");
	fMenuToolsMirror = new BMenu("Mirror");
	fMenuToolsScale = new BMenu("Scale");
	fMenuToolsMove = new BMenu("Move");
	fMenuHelp = new BMenu("Help");

	fMenuFileSaveAs->AddItem(new BMenuItem("STL (ASCII)", new BMessage(MSG_FILE_EXPORT_STLA)));
	fMenuFileSaveAs->AddItem(new BMenuItem("STL (Binary)", new BMessage(MSG_FILE_EXPORT_STLB)));
	fMenuFileSaveAs->AddItem(new BMenuItem("Geomview OFF", new BMessage(MSG_FILE_EXPORT_OFF)));
	fMenuFileSaveAs->AddItem(new BMenuItem("Autodesk DXF", new BMessage(MSG_FILE_EXPORT_DXF)));
	fMenuFileSaveAs->AddItem(new BMenuItem("Wavefront OBJ", new BMessage(MSG_FILE_EXPORT_OBJ)));
	fMenuFileSaveAs->AddItem(new BMenuItem("VRML", new BMessage(MSG_FILE_EXPORT_VRML)));

	fMenuFile->AddItem(new BMenuItem("Open" B_UTF8_ELLIPSIS, new BMessage(MSG_FILE_OPEN), 'O'));
	fMenuItemReload = new BMenuItem("Reload", new BMessage(MSG_FILE_RELOAD));
	fMenuFile->AddItem(fMenuItemReload);
//	fMenuItemAppend = new BMenuItem("Append...", new BMessage(MSG_FILE_APPEND));
//	fMenuFile->AddItem(fMenuItemAppend);
	fMenuFile->AddSeparatorItem();
	fMenuItemSave = new BMenuItem("Save", new BMessage(MSG_FILE_SAVE), 'S');
	fMenuFile->AddItem(fMenuItemSave);
	fMenuFile->AddItem(fMenuFileSaveAs);
	fMenuFileSaveAs->SetTargetForItems(this);
	fMenuFile->AddSeparatorItem();
	fMenuItemClose = new BMenuItem("Close", new BMessage(MSG_FILE_CLOSE));
	fMenuFile->AddItem(fMenuItemClose);
	fMenuFile->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));
	fMenuBar->AddItem(fMenuFile);
	fMenuFile->SetTargetForItems(this);

	fMenuItemSolid = new BMenuItem("Solid", new BMessage(MSG_VIEWMODE_SOLID));
	fMenuView->AddItem(fMenuItemSolid);
	fMenuItemWireframe = new BMenuItem("Wireframe", new BMessage(MSG_VIEWMODE_WIREFRAME));
	fMenuView->AddItem(fMenuItemWireframe);
	fMenuView->AddSeparatorItem();
	fMenuItemShowAxes = new BMenuItem("Axes", new BMessage(MSG_VIEWMODE_AXES));
	fMenuView->AddItem(fMenuItemShowAxes);
	fMenuItemShowOXY = new BMenuItem("Plane OXY", new BMessage(MSG_VIEWMODE_OXY));
	fMenuView->AddItem(fMenuItemShowOXY);
	fMenuItemShowBox = new BMenuItem("Bounding box", new BMessage(MSG_VIEWMODE_BOUNDING_BOX));
	fMenuView->AddItem(fMenuItemShowBox);
	fMenuView->AddSeparatorItem();
	fMenuItemStat = new BMenuItem("Show statistics", new BMessage(MSG_VIEWMODE_STAT), 'I');
	fMenuView->AddItem(fMenuItemStat);
	fMenuView->AddSeparatorItem();
	fMenuItemReset = new BMenuItem("Reset", new BMessage(MSG_VIEWMODE_RESETPOS), 'R');
	fMenuView->AddItem(fMenuItemReset);

	fMenuToolsMirror->AddItem(new BMenuItem("Mirror XY", new BMessage(MSG_TOOLS_MIRROR_XY)));
	fMenuToolsMirror->AddItem(new BMenuItem("Mirror YZ", new BMessage(MSG_TOOLS_MIRROR_YZ)));
	fMenuToolsMirror->AddItem(new BMenuItem("Mirror XZ", new BMessage(MSG_TOOLS_MIRROR_XZ)));
	fMenuToolsMirror->SetTargetForItems(this);

	fMenuToolsScale->AddItem(new BMenuItem("Scale" B_UTF8_ELLIPSIS, new BMessage(MSG_TOOLS_SCALE)));
	fMenuToolsScale->AddItem(new BMenuItem("Axis scaling" B_UTF8_ELLIPSIS, new BMessage(MSG_TOOLS_SCALE_3)));
	fMenuToolsScale->SetTargetForItems(this);
	
	fMenuToolsMove->AddItem(new BMenuItem("To" B_UTF8_ELLIPSIS, new BMessage(MSG_TOOLS_MOVE_TO)));
	fMenuToolsMove->AddItem(new BMenuItem("By" B_UTF8_ELLIPSIS, new BMessage(MSG_TOOLS_MOVE_BY)));
	fMenuToolsMove->AddItem(new BMenuItem("To Center", new BMessage(MSG_TOOLS_MOVE_CENTER)));
	fMenuToolsMove->AddItem(new BMenuItem("To (0,0,0)", new BMessage(MSG_TOOLS_MOVE_ZERO)));
	fMenuToolsMove->AddItem(new BMenuItem("On the Middle", new BMessage(MSG_TOOLS_MOVE_MIDDLE)));
	fMenuToolsMove->SetTargetForItems(this);

	fMenuItemEditTitle = new BMenuItem("Edit title" B_UTF8_ELLIPSIS, new BMessage(MSG_TOOLS_EDIT_TITLE));
	fMenuTools->AddItem(fMenuItemEditTitle);
	fMenuTools->AddSeparatorItem();
	fMenuTools->AddItem(fMenuToolsScale);
	fMenuTools->AddItem(fMenuToolsMove);
	fMenuTools->AddItem(fMenuToolsMirror);
	fMenuItemRotate = new BMenuItem("Rotate" B_UTF8_ELLIPSIS, new BMessage(MSG_TOOLS_ROTATE));
	fMenuTools->AddItem(fMenuItemRotate);
	fMenuTools->AddSeparatorItem();
	fMenuItemRepair = new BMenuItem("Repair" B_UTF8_ELLIPSIS, new BMessage(MSG_TOOLS_REPAIR));
	fMenuTools->AddItem(fMenuItemRepair);

	fMenuBar->AddItem(fMenuView);
	fMenuView->SetTargetForItems(this);

	fMenuBar->AddItem(fMenuTools);
	fMenuTools->SetTargetForItems(this);

	fMenuHelp->AddItem(new BMenuItem("About", new BMessage(B_ABOUT_REQUESTED)));
	fMenuBar->AddItem(fMenuHelp);		
	fMenuHelp->SetTargetForItems(this);

	AddChild(fMenuBar);

	BRect toolBarRect = Bounds();
	toolBarRect.top = fMenuBar->Frame().bottom + 1;
	fToolBar = new STLToolBar(toolBarRect);
	fToolBar->AddAction(MSG_FILE_OPEN, this, STLoverApplication::GetIcon("document-open", TOOLBAR_ICON_SIZE), "Open");
	fToolBar->AddAction(MSG_FILE_SAVE, this, STLoverApplication::GetIcon("document-save", TOOLBAR_ICON_SIZE), "Save");
	fToolBar->AddSeparator();
	fToolBar->AddAction(MSG_TOOLS_EDIT_TITLE, this, STLoverApplication::GetIcon("document-edit", TOOLBAR_ICON_SIZE), "Edit title");
	fToolBar->AddAction(MSG_TOOLS_MIRROR_XY, this, STLoverApplication::GetIcon("mirror-xy", TOOLBAR_ICON_SIZE), "Mirror XY");
	fToolBar->AddAction(MSG_TOOLS_MIRROR_YZ, this, STLoverApplication::GetIcon("mirror-yz", TOOLBAR_ICON_SIZE), "Mirror YZ");
	fToolBar->AddAction(MSG_TOOLS_MIRROR_XZ, this, STLoverApplication::GetIcon("mirror-xz", TOOLBAR_ICON_SIZE), "Mirror XZ");
	fToolBar->AddAction(MSG_TOOLS_MOVE_MIDDLE, this, STLoverApplication::GetIcon("move-middle", TOOLBAR_ICON_SIZE), "Put on the Middle");
	fToolBar->AddAction(MSG_TOOLS_MOVE_TO, this, STLoverApplication::GetIcon("move-to", TOOLBAR_ICON_SIZE), "Move to");
	fToolBar->AddAction(MSG_TOOLS_MOVE_BY, this, STLoverApplication::GetIcon("move-by", TOOLBAR_ICON_SIZE), "Move by");
	fToolBar->AddAction(MSG_TOOLS_SCALE, this, STLoverApplication::GetIcon("scale", TOOLBAR_ICON_SIZE), "Scale");
	fToolBar->AddAction(MSG_TOOLS_SCALE_3, this, STLoverApplication::GetIcon("scale-axis", TOOLBAR_ICON_SIZE), "Axis scale");
	fToolBar->AddAction(MSG_TOOLS_ROTATE, this, STLoverApplication::GetIcon("rotate", TOOLBAR_ICON_SIZE), "Rotate");
	fToolBar->AddSeparator();
	fToolBar->AddAction(MSG_TOOLS_REPAIR, this, STLoverApplication::GetIcon("tools-wizard", TOOLBAR_ICON_SIZE), "Repair");
	fToolBar->AddSeparator();
	fToolBar->AddAction(MSG_VIEWMODE_STAT, this, STLoverApplication::GetIcon("stat", TOOLBAR_ICON_SIZE), "Statistics");
	fToolBar->AddGlue();
	fToolBar->ResizeTo(toolBarRect.Width(), fToolBar->MinSize().height);
	fToolBar->GroupLayout()->SetInsets(0);
	AddChild(fToolBar);

	BRect viewToolBarRect = Bounds();
	viewToolBarRect.top = fToolBar->Frame().bottom + 1;
	fViewToolBar = new STLToolBar(viewToolBarRect, B_VERTICAL);
	fViewToolBar->AddAction(MSG_VIEWMODE_ZOOMIN, this, STLoverApplication::GetIcon("zoom-in", TOOLBAR_ICON_SIZE), "Zoom in");
	fViewToolBar->AddAction(MSG_VIEWMODE_ZOOMOUT, this, STLoverApplication::GetIcon("zoom-out", TOOLBAR_ICON_SIZE), "Zoom out");
	fViewToolBar->AddAction(MSG_VIEWMODE_ZOOMFIT, this, STLoverApplication::GetIcon("zoom-fit-best", TOOLBAR_ICON_SIZE), "Best fit");
	fViewToolBar->AddSeparator();
	fViewToolBar->AddAction(MSG_VIEWMODE_RESETPOS, this, STLoverApplication::GetIcon("reset", TOOLBAR_ICON_SIZE), "Reset view");
	fViewToolBar->AddSeparator();
	fViewToolBar->AddAction(MSG_VIEWMODE_WIREFRAME_TOGGLE, this, STLoverApplication::GetIcon("wireframe", TOOLBAR_ICON_SIZE), "Wireframe");
	fViewToolBar->AddAction(MSG_VIEWMODE_AXES, this, STLoverApplication::GetIcon("axes", TOOLBAR_ICON_SIZE), "Show axes");
	fViewToolBar->AddAction(MSG_VIEWMODE_OXY, this, STLoverApplication::GetIcon("plane", TOOLBAR_ICON_SIZE), "Show plane OXY");
	fViewToolBar->AddAction(MSG_VIEWMODE_BOUNDING_BOX, this, STLoverApplication::GetIcon("bounding-box", TOOLBAR_ICON_SIZE), "Bounding box");
	fViewToolBar->AddSeparator();
	fViewToolBar->AddAction(MSG_VIEWMODE_FRONT, this, STLoverApplication::GetIcon("view-front", TOOLBAR_ICON_SIZE), "Front view");
	fViewToolBar->AddAction(MSG_VIEWMODE_RIGHT, this, STLoverApplication::GetIcon("view-right", TOOLBAR_ICON_SIZE), "Right view");
	fViewToolBar->AddAction(MSG_VIEWMODE_TOP, this, STLoverApplication::GetIcon("view-top", TOOLBAR_ICON_SIZE), "Top view");
	fViewToolBar->AddGlue();
	fViewToolBar->ResizeTo(fViewToolBar->MinSize().width, viewToolBarRect.Height());
	fViewToolBar->GroupLayout()->SetInsets(0);
	AddChild(fViewToolBar);

	BRect stlRect = Bounds();
	stlRect.top = fToolBar->Frame().bottom + 1;
	stlRect.left =fViewToolBar->Frame().right + 1;
	fStlView = new STLView(stlRect, BGL_RGB | BGL_DOUBLE | BGL_DEPTH);
	AddChild(fStlView);

	BRect statRect = Bounds();
	statRect.left = stlRect.right + 1;
	statRect.top = stlRect.top;
	fStatView = new STLStatView(statRect);
	AddChild(fStatView);

	AddShortcut('H', B_COMMAND_KEY,	new BMessage(MSG_EASTER_EGG));
	AddShortcut('Q', B_COMMAND_KEY,	new BMessage(MSG_APP_QUIT));

	LoadSettings();
	UpdateUI();

	rendererThread = spawn_thread(RenderFunction, "renderThread", B_NORMAL_PRIORITY, (void*)fStlView);
	resume_thread(rendererThread);

	SetPulseRate(1000000);
}

STLWindow::~STLWindow()
{
	SaveSettings();

	status_t exitValue;
	fRenderWork = false;
	wait_for_thread(rendererThread, &exitValue);

	CloseFile();

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
		bool _showStat = false;
		bool _fShowWireframe = false;
		bool _fShowOXY = false;
		BRect _windowRect(100, 100, 100 + 720, 100 + 512);

		file.ReadAttr("WindowRect", B_RECT_TYPE, 0, &_windowRect, sizeof(BRect));
		file.ReadAttr("ShowAxes", B_BOOL_TYPE, 0, &_fShowAxes, sizeof(bool));
		file.ReadAttr("ShowOXY", B_BOOL_TYPE, 0, &_fShowOXY, sizeof(bool));
		file.ReadAttr("ShowBoundingBox", B_BOOL_TYPE, 0, &_fShowBoundingBox, sizeof(bool));
		file.ReadAttr("ShowStat", B_BOOL_TYPE, 0, &_showStat, sizeof(bool));
		file.ReadAttr("ShowWireframe", B_BOOL_TYPE, 0, &_fShowWireframe, sizeof(bool));
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
		fStlView->ShowAxes(fShowAxes);

		fShowOXY = _fShowOXY;
		fStlView->ShowOXY(fShowOXY);

		fShowWireframe = _fShowWireframe;
		fStlView->SetViewMode(_fShowWireframe ? MSG_VIEWMODE_WIREFRAME : MSG_VIEWMODE_SOLID);

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
		file.WriteAttr("ShowOXY", B_BOOL_TYPE, 0, &fShowOXY, sizeof(bool));
		file.WriteAttr("ShowBoundingBox", B_BOOL_TYPE, 0, &fShowBoundingBox, sizeof(bool));
		file.WriteAttr("ShowStat", B_BOOL_TYPE, 0, &fShowStat, sizeof(bool));
		file.WriteAttr("ShowWireframe", B_BOOL_TYPE, 0, &fShowWireframe, sizeof(bool));

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
		BAlert* alert = new BAlert("Save", "Save changes to current icon before closing?",
			"Cancel", "Don't save", "Save", B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
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

	if (be_app->CountWindows() == 1)
		be_app->PostMessage(B_QUIT_REQUESTED);

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
		case MSG_FILE_APPEND:
		case MSG_FILE_OPEN:
		{
			if (!fOpenFilePanel) {
				fOpenFilePanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL,
					B_FILE_NODE, true, NULL, NULL, false, true);
				fOpenFilePanel->SetTarget(this);
			}

			BMessage *openMsg = new BMessage(message->what == MSG_FILE_APPEND ? MSG_APPEND_REFS_RECIEVED : B_REFS_RECEIVED);
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

			if (!fSaveFilePanel) {
				fSaveFilePanel = new BFilePanel(B_SAVE_PANEL, NULL, NULL,
					B_FILE_NODE, true, fileMsg, NULL, false, true);
				fSaveFilePanel->SetTarget(this);
			} else {
				fSaveFilePanel->SetMessage(fileMsg);
				fSaveFilePanel->SetSaveText("");
			}
			BPath openedFile(fOpenedFileName);
			BString title("Save ");
			title << openedFile.Leaf() << " to ";
			if (message->what == MSG_FILE_EXPORT_STLA)
				title << "STL (ASCII)";
			if (message->what == MSG_FILE_EXPORT_STLB)
				title << "STL (Binary)";
			if (message->what == MSG_FILE_EXPORT_DXF)
				title << "Autodesk DXF";
			if (message->what == MSG_FILE_EXPORT_VRML)
				title << "VRML";
			if (message->what == MSG_FILE_EXPORT_OFF)
				title << "Geomview OFF";
			if (message->what == MSG_FILE_EXPORT_OBJ)
				title << "Wavefront OBJ";
			title << " file as...";
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
		case MSG_PULSE:
		{
			if (fErrorTimeCounter > 0) {
				fErrorTimeCounter--;
				fStlView->RenderUpdate();
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
				if (i==0 && !IsLoaded()) {
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
		case MSG_APPEND_REFS_RECIEVED:
		{
			entry_ref ref;

			if (message->FindRef("refs", 0, &ref) == B_OK) {
				BEntry entry = BEntry(&ref);
				BPath path;
				entry.GetPath(&path);
				stl_open_merge(fStlObject, (char*)path.Path());
				stl_repair(fStlObject, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1);
				fStlModified = true;
				UpdateUI();
			}
			break;
		}
		case B_CANCEL:
			break;
		case B_ABOUT_REQUESTED:
		{
			BAboutWindow* wind = new BAboutWindow(MAIN_WIN_TITLE, APP_SIGNATURE);
			wind->AddCopyright(2020, "Gerasim Troeglazov (3dEyes**)");
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
			fStlView->ShowAxes(fShowAxes);
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
				stl_repair(fStlObjectView, 0, fExactFlag, 1, toleranceValue, 1, incrementValue, fNearbyFlag,
					fIterationsValue, fRemoveUnconnectedFlag, fFillHolesFlag, fNormalDirectionsFlag,
					fNormalValuesFlag, fReverseAllFlag, 0);
				fStlModified = true;
				UpdateUI();
			}
			break;
		}
		case MSG_TOOLS_EDIT_TITLE:
		{
			STLInputWindow *input = new STLInputWindow("STL Title", 1, this, MSG_TOOLS_TITLE_SET);
			input->SetTextValue(0, "Title:", (const char*)fStlObject->stats.header);
			input->Show();
			break;
		}
		case MSG_TOOLS_TITLE_SET:
		{
			const char *value = message->FindString("value");
			if (value != NULL && IsLoaded()) {
				snprintf(fStlObject->stats.header, 80, value);
				fStlModified = true;
				UpdateUI();
			}
			break;
		}
		case MSG_TOOLS_SCALE:
		{
			STLInputWindow *input = new STLInputWindow("Scale", 1, this, MSG_TOOLS_SCALE_SET);
			input->SetTextValue(0, "Scale factor:", "1.0");
			input->Show();
			break;
		}
		case MSG_TOOLS_SCALE_SET:
		{
			const char *value = message->FindString("value");
			if (value != NULL && IsLoaded()) {
				float scaleFactor = atof(value);
				stl_scale(fStlObject, scaleFactor);
				stl_scale(fStlObjectView, scaleFactor);
				fStlModified = true;
				UpdateUI();
			}
			break;
		}
		case MSG_TOOLS_SCALE_3:
		{
			STLInputWindow *input = new STLInputWindow("Custom axis scale", 3, this, MSG_TOOLS_SCALE_SET_3);
			input->SetTextValue(0, "Scale X factor:", "1.0");
			input->SetTextValue(1, "Scale Y factor:", "1.0");
			input->SetTextValue(2, "Scale Z factor:", "1.0");
			input->Show();
			break;
		}
		case MSG_TOOLS_SCALE_SET_3:
		{
			const char *scaleX = message->FindString("value");
			const char *scaleY = message->FindString("value2");
			const char *scaleZ = message->FindString("value3");
			if (scaleX != NULL && scaleY != NULL && scaleZ != NULL && IsLoaded()) {
				float scaleVersor[3];
				scaleVersor[0] = atof(scaleX);
				scaleVersor[1] = atof(scaleY);
				scaleVersor[2] = atof(scaleZ);
				stl_scale_versor(fStlObject, scaleVersor);
				stl_scale_versor(fStlObjectView, scaleVersor);
				fStlModified = true;
				UpdateUI();
			}
			break;
		}
		case MSG_TOOLS_ROTATE:
		{
			STLInputWindow *input = new STLInputWindow("Rotate", 3, this, MSG_TOOLS_ROTATE_SET);
			input->SetTextValue(0, "X-axis:", "0.0");
			input->SetTextValue(1, "Y-axis:", "0.0");
			input->SetTextValue(2, "Z-axis:", "0.0");
			input->Show();
			break;
		}
		case MSG_TOOLS_ROTATE_SET:
		{
			const char *rotateX = message->FindString("value");
			const char *rotateY = message->FindString("value2");
			const char *rotateZ = message->FindString("value3");
			if (rotateX != NULL && rotateY != NULL && rotateZ != NULL && IsLoaded()) {
				float rotateXAngle = atof(rotateX);
				float rotateYAngle = atof(rotateY);
				float rotateZAngle = atof(rotateZ);
				stl_rotate_x(fStlObject, rotateXAngle);
				stl_rotate_x(fStlObjectView, rotateXAngle);
				stl_rotate_y(fStlObject, rotateYAngle);
				stl_rotate_y(fStlObjectView, rotateYAngle);
				stl_rotate_z(fStlObject, rotateZAngle);
				stl_rotate_z(fStlObjectView, rotateZAngle);
				fStlModified = true;
				UpdateUI();
			}
			break;
		}
		case MSG_TOOLS_MOVE_CENTER:
		{
			stl_translate(fStlObject, -fStlObject->stats.size.x / 2, -fStlObject->stats.size.y / 2, -fStlObject->stats.size.z / 2);
			fStlModified = true;
			UpdateUI();
			break;
		}
		case MSG_TOOLS_MOVE_MIDDLE:
		{
			stl_translate(fStlObject, -fStlObject->stats.size.x / 2, -fStlObject->stats.size.y / 2, 0);
			fStlModified = true;
			UpdateUI();
			break;
		}
		case MSG_TOOLS_MOVE_ZERO:
		{
			stl_translate(fStlObject, 0, 0, 0);
			fStlModified = true;
			UpdateUI();
			break;
		}
		case MSG_TOOLS_MOVE_TO:
		{
			STLInputWindow *input = new STLInputWindow("Move to", 3, this, MSG_TOOLS_MOVE_TO_SET);
			input->SetFloatValue(0, "X:", fStlObject->stats.min.x);
			input->SetFloatValue(1, "Y:", fStlObject->stats.min.y);
			input->SetFloatValue(2, "Z:", fStlObject->stats.min.z);
			input->Show();
			break;
		}
		case MSG_TOOLS_MOVE_TO_SET:
		{
			const char *x = message->FindString("value");
			const char *y = message->FindString("value2");
			const char *z = message->FindString("value3");
			if (x != NULL && y != NULL && z != NULL && IsLoaded()) {
				float xValue = atof(x);
				float yValue = atof(y);
				float zValue = atof(z);
				stl_translate(fStlObject, xValue, yValue, zValue);
				fStlModified = true;
				UpdateUI();
			}
			break;
		}
		case MSG_TOOLS_MOVE_BY:
		{
			STLInputWindow *input = new STLInputWindow("Move by", 3, this, MSG_TOOLS_MOVE_BY_SET);
			input->SetFloatValue(0, "∆X:", 0);
			input->SetFloatValue(1, "∆Y:", 0);
			input->SetFloatValue(2, "∆Z:", 0);
			input->Show();
			break;
		}
		case MSG_TOOLS_MOVE_BY_SET:
		{
			const char *dx = message->FindString("value");
			const char *dy = message->FindString("value2");
			const char *dz = message->FindString("value3");
			if (dx != NULL && dy != NULL && dz != NULL && IsLoaded()) {
				float dxValue = atof(dx);
				float dyValue = atof(dy);
				float dzValue = atof(dz);
				stl_translate_relative(fStlObject, dxValue, dyValue, dzValue);
				fStlModified = true;
				UpdateUI();
			}
			break;
		}
		case MSG_TOOLS_MIRROR_XY:
		{
			stl_mirror_xy(fStlObject);
			stl_mirror_xy(fStlObjectView);
			fStlModified = true;
			UpdateUI();
			break;
		}
		case MSG_TOOLS_MIRROR_YZ:
		{
			stl_mirror_yz(fStlObject);
			stl_mirror_yz(fStlObjectView);
			fStlModified = true;
			UpdateUI();
			break;
		}
		case MSG_TOOLS_MIRROR_XZ:
		{
			stl_mirror_xz(fStlObject);
			stl_mirror_xz(fStlObjectView);
			fStlModified = true;
			UpdateUI();
			break;
		}
		case MSG_VIEWMODE_SOLID:
		case MSG_VIEWMODE_WIREFRAME:
		{
			fShowWireframe = (message->what == MSG_VIEWMODE_WIREFRAME);
			fStlView->SetViewMode(fShowWireframe ? MSG_VIEWMODE_WIREFRAME : MSG_VIEWMODE_SOLID);
			UpdateUI();
			break;
		}
		case MSG_VIEWMODE_WIREFRAME_TOGGLE:
		{
			fShowWireframe = !fShowWireframe;
			fStlView->SetViewMode(fShowWireframe ? MSG_VIEWMODE_WIREFRAME : MSG_VIEWMODE_SOLID);
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

			BMenuItem *_menuItemSolid = new BMenuItem("Solid", new BMessage(MSG_VIEWMODE_SOLID));
			_menuItemSolid->SetMarked(!fShowWireframe);
			BMenuItem *_menuItemWireframe = new BMenuItem("Wireframe", new BMessage(MSG_VIEWMODE_WIREFRAME));
			_menuItemWireframe->SetMarked(fShowWireframe);

			menu->AddItem(_menuItemSolid);
			menu->AddItem(_menuItemWireframe);
			menu->AddSeparatorItem();

			BMenuItem *_menuItemfShowAxes = new BMenuItem("Axes", new BMessage(MSG_VIEWMODE_AXES));
			_menuItemfShowAxes->SetMarked(fShowAxes);
			BMenuItem *_menuItemfShowOXY = new BMenuItem("Plane OXY", new BMessage(MSG_VIEWMODE_OXY));
			_menuItemfShowOXY->SetMarked(fShowOXY);
			BMenuItem *_menuItemShowBox = new BMenuItem("Bounding box", new BMessage(MSG_VIEWMODE_BOUNDING_BOX));
			_menuItemShowBox->SetMarked(fShowBoundingBox);

			menu->AddItem(_menuItemfShowAxes);
			menu->AddItem(_menuItemfShowOXY);
			menu->AddItem(_menuItemShowBox);
			menu->AddSeparatorItem();
			menu->AddItem(new BMenuItem("Reset", new BMessage(MSG_VIEWMODE_RESETPOS), 'R'));
			menu->AddSeparatorItem();
			menu->AddItem(new BMenuItem("Edit title...", new BMessage(MSG_TOOLS_EDIT_TITLE)));
			menu->AddSeparatorItem();
			menu->AddItem(fMenuToolsScale);
			menu->AddItem(fMenuToolsMove);
			menu->AddItem(fMenuToolsMirror);
			menu->AddItem(new BMenuItem("Rotate...", new BMessage(MSG_TOOLS_ROTATE)));
			menu->AddSeparatorItem();
			menu->AddItem(new BMenuItem("Repair", new BMessage(MSG_TOOLS_REPAIR)));
			menu->SetTargetForItems(this);

			menu->Go(fStlView->ConvertToScreen(point), true, false, true);
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
//	fMenuItemAppend->SetEnabled(show);
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
	fMenuItemShowOXY->SetMarked(fShowOXY);
	fMenuItemSolid->SetMarked(!fShowWireframe);
	fMenuItemWireframe->SetMarked(fShowWireframe);
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

	fViewToolBar->SetActionEnabled(MSG_VIEWMODE_WIREFRAME_TOGGLE, show);
	fViewToolBar->SetActionPressed(MSG_VIEWMODE_WIREFRAME_TOGGLE, fShowWireframe);
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

	if (locked)
		UnlockLooper();
}

void
STLWindow::OpenFile(const char *filename)
{	
	CloseFile();
	fStlLoading = true;
	fStlView->RenderUpdate();
	fStlView->Render();

	fStlObject = (stl_file*)malloc(sizeof(stl_file));
	memset(fStlObject, 0, sizeof(stl_file));

	fStlObjectView = (stl_file*)malloc(sizeof(stl_file));
	memset(fStlObjectView, 0, sizeof(stl_file));

	fZDepth = -5;
	fMaxExtent = 10;

	stl_open(fStlObject, (char*)filename);
	stl_open(fStlObjectView, (char*)filename);
	fStlView->SetSTL(fStlObject, fStlObjectView);
	
	if (stl_get_error(fStlObject) || stl_get_error(fStlObjectView)) {
		CloseFile();
		fErrorTimeCounter = 3;
		return;
	}
	
	stl_fix_normal_values(fStlObject);
	stl_fix_normal_values(fStlObjectView);

	TransformPosition();

	BPath path(filename);
	SetTitle(path.Leaf());
	fOpenedFileName.SetTo(filename);

	fStlView->LockGL();
	fStlView->LockLooper();

	GLfloat Width = fStlView->Bounds().Width() + 1;
	GLfloat Height =  fStlView->Bounds().Height() + 1;
	glViewport(0, 0, Width, Height);
  	glMatrixMode(GL_PROJECTION);
  	glLoadIdentity();

  	gluPerspective(FOV, (GLfloat)Width/(GLfloat)Height, 0.1f, (fZDepth + fMaxExtent));

  	glMatrixMode(GL_MODELVIEW);

	fErrorTimeCounter = 0;
	fStlView->RenderUpdate();

  	fStlView->UnlockLooper();
	fStlView->UnlockGL();

	fStlLoading = false;
	fStlModified = false;
	fStlValid = true;
	UpdateUI();
}

void
STLWindow::CloseFile(void)
{
	if (IsLoaded()) {
		SetTitle(MAIN_WIN_TITLE);
		fStlValid = false;

		stl_file* stl = fStlObject;
		fStlObject = NULL;
		stl_close(stl);
		free (stl);

		stl = fStlObjectView;
		fStlObjectView = NULL;
		stl_close(stl);
		free (stl);

		fStlLoading = false;
		fErrorTimeCounter = 0;
		UpdateUI();
	}
}

void
STLWindow::UpdateStats(void)
{
	bool isLoaded = IsLoaded();
	if (isLoaded) {
		stl_calculate_volume(fStlObject);
//			stl_calculate_surface_area(fStlObject);
	}
	BPath path(fOpenedFileName);
	fStatView->SetTextValue("filename", isLoaded ? path.Leaf() : 0);
	fStatView->SetTextValue("type", isLoaded ? (fStlObject->stats.type == binary ? "Binary" : "ASCII") : "");
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
//		fStatView->SetFloatValue("surface", isLoaded ? fStlObject->stats.surface_area : 0);
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
	SetSizeLimits(600, 4096, fStatView->Frame().top + fStatView->PreferredSize().Height(), 4049);
}

void
STLWindow::TransformPosition()
{
	stl_translate(fStlObjectView, 0, 0, 0);

	float xMaxExtent = 0;
	float yMaxExtent = 0;
	float zMaxExtent = 0;

	for (int i = 0 ; i < fStlObjectView->stats.number_of_facets ; i++) {
		for (int j = 0; j < 3; j++) {
			if (fStlObjectView->facet_start[i].vertex[j].x > xMaxExtent)
				xMaxExtent = fStlObjectView->facet_start[i].vertex[0].x;
			if (fStlObjectView->facet_start[i].vertex[j].y > yMaxExtent)
				yMaxExtent = fStlObjectView->facet_start[i].vertex[0].y;
			if (fStlObjectView->facet_start[i].vertex[j].z > zMaxExtent)
				zMaxExtent = fStlObjectView->facet_start[i].vertex[0].z;
		}
	}

	float longerSide = xMaxExtent > yMaxExtent ? xMaxExtent : yMaxExtent;
	longerSide += (zMaxExtent * (sin(FOV * (M_PI / 180.0)) / sin((90.0 - FOV) * (M_PI / 180.0))));

	fZDepth = -((longerSide / 2.0) / tanf((FOV / 2.0) * (M_PI / 180.0)));

	if ((xMaxExtent > yMaxExtent) && (xMaxExtent > zMaxExtent))
    	fMaxExtent = xMaxExtent;
	if ((yMaxExtent > xMaxExtent) && (yMaxExtent > zMaxExtent))
		fMaxExtent = yMaxExtent;
	if ((zMaxExtent > yMaxExtent) && (zMaxExtent > xMaxExtent))
		fMaxExtent = zMaxExtent;

	stl_translate_relative(fStlObjectView, -xMaxExtent / 2.0, -yMaxExtent / 2.0, -zMaxExtent / 2.0);
}

int32
STLWindow::RenderFunction(void *data)
{
	STLView *view = (STLView*)data;
	STLWindow *window = (STLWindow*)view->Window();

	while(window->IsRenderWork()) {
		view->Render();
		snooze(1000000 / FPS_LIMIT);
	}

	return 0;
}
