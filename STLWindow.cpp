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
#include "STLStatWindow.h"

STLWindow::STLWindow(BRect frame)
	: BWindow(frame, MAIN_WIN_TITLE, B_TITLED_WINDOW, 0),
	fOpenFilePanel(NULL),
	fSaveFilePanel(NULL),
	stlModified(false),
	showBoundingBox(false),
	statWindow(NULL),
	stlValid(false),	
	stlObject(NULL),
	stlObjectView(NULL),
	stlObjectAppend(NULL),
	errorTimeCounter(0),
	zDepth(-5),
	maxExtent(10)
{
	fMenuBar = new BMenuBar(BRect(0, 0, Bounds().Width(), 22), "menubar");
	fMenuFile = new BMenu("File");
	fMenuFileSaveAs = new BMenu("Save as...");
	fMenuView = new BMenu("View");
	fMenuTools = new BMenu("Tools");
	fMenuToolsMirror = new BMenu("Mirror");
	fMenuHelp = new BMenu("Help");

	fMenuFileSaveAs->AddItem(new BMenuItem("STL (ASCII)", new BMessage(MSG_FILE_EXPORT_STLA)));
	fMenuFileSaveAs->AddItem(new BMenuItem("STL (Binary)", new BMessage(MSG_FILE_EXPORT_STLB)));
	fMenuFileSaveAs->AddItem(new BMenuItem("Geomview OFF", new BMessage(MSG_FILE_EXPORT_OFF)));
	fMenuFileSaveAs->AddItem(new BMenuItem("Autodesk DXF", new BMessage(MSG_FILE_EXPORT_DXF)));
	fMenuFileSaveAs->AddItem(new BMenuItem("Wavefront OBJ", new BMessage(MSG_FILE_EXPORT_OBJ)));
	fMenuFileSaveAs->AddItem(new BMenuItem("VRML", new BMessage(MSG_FILE_EXPORT_VRML)));

	fMenuFile->AddItem(new BMenuItem("Open...", new BMessage(MSG_FILE_OPEN), 'O'));
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
	fMenuItemSolid->SetMarked(true);
	fMenuView->AddItem(fMenuItemSolid);
	fMenuItemWireframe = new BMenuItem("Wireframe", new BMessage(MSG_VIEWMODE_WIREFRAME));	
	fMenuView->AddItem(fMenuItemWireframe);
	fMenuView->AddSeparatorItem();
	fMenuItemShowBox = new BMenuItem("Bounding box", new BMessage(MSG_VIEWMODE_BOUNDING_BOX));
	fMenuView->AddItem(fMenuItemShowBox);
	fMenuView->AddSeparatorItem();
	fMenuItemStatWin = new BMenuItem("Show statistics", new BMessage(MSG_VIEWMODE_STAT_WINDOW));
	fMenuView->AddItem(fMenuItemStatWin);
	fMenuView->AddSeparatorItem();
	fMenuView->AddItem(new BMenuItem("Reload", new BMessage(MSG_VIEWMODE_RESETPOS)));

	fMenuToolsMirror->AddItem(new BMenuItem("Mirror XY", new BMessage(MSG_TOOLS_MIRROR_XY)));
	fMenuToolsMirror->AddItem(new BMenuItem("Mirror YZ", new BMessage(MSG_TOOLS_MIRROR_YZ)));
	fMenuToolsMirror->AddItem(new BMenuItem("Mirror XZ", new BMessage(MSG_TOOLS_MIRROR_XZ)));
	
	fMenuTools->AddItem(new BMenuItem("Add facets to fill holes", new BMessage(MSG_TOOLS_FILL_HOLES)));
	fMenuTools->AddItem(new BMenuItem("Remove unconnected facets", new BMessage(MSG_TOOLS_REMOVE_UNCONNECTED)));
	fMenuTools->AddItem(new BMenuItem("Check and fix direction of normals", new BMessage(MSG_TOOLS_CHECK_DIRECT)));
	fMenuTools->AddItem(new BMenuItem("Check and fix normal values", new BMessage(MSG_TOOLS_CHECK_NORMALS)));
	fMenuTools->AddItem(new BMenuItem("Find and connect nearby facets", new BMessage(MSG_TOOLS_CHECK_NEARBY)));
	fMenuTools->AddItem(new BMenuItem("Reverse the directions of all facets and normals", new BMessage(MSG_TOOLS_REVERSE)));
	fMenuTools->AddItem(fMenuToolsMirror);
	fMenuToolsMirror->SetTargetForItems(this);

	fMenuBar->AddItem(fMenuView);
	fMenuView->SetTargetForItems(this);

	fMenuBar->AddItem(fMenuTools);
	fMenuTools->SetTargetForItems(this);

	fMenuHelp->AddItem(new BMenuItem("About", new BMessage(B_ABOUT_REQUESTED)));
	fMenuBar->AddItem(fMenuHelp);		
	fMenuHelp->SetTargetForItems(this);
	
	EnableMenuItems(false);

	stlView = new STLView(Bounds(), BGL_RGB | BGL_DOUBLE | BGL_DEPTH);
	AddChild(stlView);

	stlView->AddChild(fMenuBar);

	SetSizeLimits(320, 4096, 256, 4049);

	LoadSettings();
	
	Show();
	stlView->RenderUpdate();

	rendererThread = spawn_thread(RenderFunction, "renderThread", B_DISPLAY_PRIORITY, (void*)stlView);
	resume_thread(rendererThread);

	SetPulseRate(1000000);
}

STLWindow::~STLWindow()
{
	SaveSettings();

	if (statWindow != NULL) {
		statWindow->Lock();
		statWindow->Quit();
	}

	kill_thread(rendererThread);
	CloseFile();
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

		bool _showBoundingBox = false;
		bool _showStatWindow = false;
		bool _showWireframe = false;

		file.ReadAttr("ShowBoundingBox", B_BOOL_TYPE, 0, &_showBoundingBox, sizeof(bool));
		file.ReadAttr("ShowStatWindow", B_BOOL_TYPE, 0, &_showStatWindow, sizeof(bool));
		file.ReadAttr("ShowWireframe", B_BOOL_TYPE, 0, &_showWireframe, sizeof(bool));

		showBoundingBox = _showBoundingBox;
		stlView->ShowBoundingBox(showBoundingBox);

		if (_showStatWindow)
			this->PostMessage(MSG_VIEWMODE_STAT_WINDOW);

		if (_showWireframe)
			this->PostMessage(MSG_VIEWMODE_WIREFRAME);
		else
			this->PostMessage(MSG_VIEWMODE_SOLID);

		EnableMenuItems(IsLoaded());

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

		bool _showStatWindow = statWindow == NULL ? false : !statWindow->IsHidden();
		bool _showWireframe = fMenuItemWireframe->IsMarked();

		file.WriteAttr("ShowBoundingBox", B_BOOL_TYPE, 0, &showBoundingBox, sizeof(bool));
		file.WriteAttr("ShowStatWindow", B_BOOL_TYPE, 0, &_showStatWindow, sizeof(bool));
		file.WriteAttr("ShowWireframe", B_BOOL_TYPE, 0, &_showWireframe, sizeof(bool));

		file.Sync();
		file.Unlock();
	}
}


bool
STLWindow::QuitRequested() {
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

void 
STLWindow::MessageReceived(BMessage *message)
{
	if (message->WasDropped()) {
		DetachCurrentMessage();
		message->what = B_REFS_RECEIVED;
		BMessenger(be_app).SendMessage(message);
		return;
	}
	switch (message->what) {
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
			if (stlObject->stats.type == binary)
				stl_write_binary(stlObject, path.Path(), path.Leaf());
			else
				stl_write_ascii(stlObject, path.Path(), path.Leaf());
			BNode node(path.Path());
			BNodeInfo nodeInfo(&node);
			nodeInfo.SetType("application/sla");
			stlModified = false;
			EnableMenuItems(true);
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
			if (errorTimeCounter > 0) {
				errorTimeCounter--;
				stlView->RenderUpdate();
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
						stl_write_ascii(stlObject, path.Path(), path.Leaf());
						break;
					case MSG_FILE_EXPORT_STLB:
						stl_write_binary(stlObject, path.Path(), path.Leaf());
						break;
					case MSG_FILE_EXPORT_DXF:
						stl_write_dxf(stlObject, (char*)path.Path(), (char*)path.Leaf());
						mime.SetTo("application/dxf");
						break;
					case MSG_FILE_EXPORT_VRML:
						stl_repair(stlObject, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1);
						stl_generate_shared_vertices(stlObject);
						stl_write_vrml(stlObject, (char*)path.Path());
						mime.SetTo("text/plain");
						break;
					case MSG_FILE_EXPORT_OFF:
						stl_repair(stlObject, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1);
						stl_generate_shared_vertices(stlObject);
						stl_write_off(stlObject, (char*)path.Path());
						mime.SetTo("text/plain");
						break;
					case MSG_FILE_EXPORT_OBJ:
						stl_repair(stlObject, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1);
						stl_generate_shared_vertices(stlObject);
						stl_write_obj(stlObject, (char*)path.Path());
						mime.SetTo("text/plain");
						break;
				}
				BNode node(path.Path());
				BNodeInfo nodeInfo(&node);
				nodeInfo.SetType(mime.String());

				stlModified = false;
				EnableMenuItems(true);
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
				stl_open_merge(stlObject, (char*)path.Path());
				stl_repair(stlObject, 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1);
				stlModified = true;
				EnableMenuItems(true);
				stlView->RenderUpdate();
			}
			break;
		}
		case B_REFS_RECEIVED:
			DetachCurrentMessage();
			BMessenger(be_app).SendMessage(message);
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
		case MSG_VIEWMODE_BOUNDING_BOX:
		{
			showBoundingBox = !showBoundingBox;
			stlView->ShowBoundingBox(showBoundingBox);
			EnableMenuItems(true);
			stlView->RenderUpdate();
			break;
		}
		case MSG_VIEWMODE_RESETPOS:
		{
			OpenFile(fOpenedFileName.String());
			break;
		}
		case MSG_VIEWMODE_STAT_WINDOW:
		{
			if (statWindow == NULL) {
				BScreen screen(B_MAIN_SCREEN_ID);
				BRect rect(Frame().right + 12, Frame().top, Frame().right + 250, Frame().bottom);

				if (rect.right >= screen.Frame().Width())
					rect.OffsetTo(screen.Frame().right - rect.Width(), rect.top);

				statWindow = new STLStatWindow(rect, this);				
				statWindow->Show();
				Activate();
			} else {
				if (statWindow->IsHidden())
					statWindow->Show();
				else
					statWindow->Hide();
			}
			UpdateStats();
			fMenuItemStatWin->SetMarked(!statWindow->IsHidden());
			break;
		}
		case MSG_TOOLS_MIRROR_XY:
		{
			stl_mirror_xy(stlObject);
			stl_mirror_xy(stlObjectView);
			stlModified = true;
			EnableMenuItems(true);
			UpdateStats();
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_MIRROR_YZ:
		{
			stl_mirror_yz(stlObject);
			stl_mirror_yz(stlObjectView);
			stlModified = true;
			EnableMenuItems(true);
			UpdateStats();
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_MIRROR_XZ:
		{
			stl_mirror_xz(stlObject);
			stl_mirror_xz(stlObjectView);
			stlModified = true;
			EnableMenuItems(true);
			UpdateStats();
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_FILL_HOLES:
		{
			stl_fill_holes(stlObject);
			stl_fill_holes(stlObjectView);
			stlModified = true;
			EnableMenuItems(true);
			UpdateStats();
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_REMOVE_UNCONNECTED:
		{
			stl_remove_unconnected_facets(stlObject);
			stl_remove_unconnected_facets(stlObjectView);
			stlModified = true;
			EnableMenuItems(true);
			UpdateStats();
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_CHECK_DIRECT:
		{
			stl_fix_normal_directions(stlObject);
			stl_fix_normal_directions(stlObjectView);
			stlModified = true;
			EnableMenuItems(true);
			UpdateStats();
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_CHECK_NORMALS:
		{
			stl_fix_normal_values(stlObject);
			stl_fix_normal_values(stlObjectView);
			stlModified = true;
			EnableMenuItems(true);
			UpdateStats();
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_CHECK_NEARBY:
		{
			stl_check_facets_nearby(stlObject, 0);
			stl_check_facets_nearby(stlObjectView, 0);
			stlModified = true;
			EnableMenuItems(true);
			UpdateStats();
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_REVERSE:
		{
			stl_reverse_all_facets(stlObject);
			stl_reverse_all_facets(stlObjectView);
			stlModified = true;
			EnableMenuItems(true);
			UpdateStats();
			stlView->RenderUpdate();
			break;
		}
		case MSG_VIEWMODE_SOLID:
		{
			stlView->LockGL();
   			glPolygonMode(GL_FRONT, GL_FILL);
   			glPolygonMode(GL_BACK, GL_FILL);
   			stlView->UnlockGL();
   			fMenuItemSolid->SetMarked(true);
   			fMenuItemWireframe->SetMarked(false);
   			stlView->RenderUpdate();
			break;
		}
		case MSG_VIEWMODE_WIREFRAME:
		{
			stlView->LockGL();
			glPolygonMode(GL_FRONT, GL_LINE);
			glPolygonMode(GL_BACK, GL_LINE);
			stlView->UnlockGL();
   			fMenuItemSolid->SetMarked(false);
  			fMenuItemWireframe->SetMarked(true);
			stlView->RenderUpdate();
			break;
		}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}

void
STLWindow::EnableMenuItems(bool show)
{
	fMenuItemClose->SetEnabled(show);
//	fMenuItemAppend->SetEnabled(show);
	fMenuView->SetEnabled(show);
	fMenuTools->SetEnabled(show);
	fMenuFileSaveAs->SetEnabled(show);
	fMenuItemSave->SetEnabled(show && stlModified);
	fMenuItemShowBox->SetMarked(showBoundingBox);
	if (statWindow != NULL)
		fMenuItemStatWin->SetMarked(!statWindow->IsHidden());
	else
		fMenuItemStatWin->SetMarked(false);
}

void
STLWindow::OpenFile(const char *filename)
{	
	CloseFile();

	stlObject = (stl_file*)malloc(sizeof(stl_file));
	memset(stlObject, 0, sizeof(stl_file));

	stlObjectView = (stl_file*)malloc(sizeof(stl_file));
	memset(stlObjectView, 0, sizeof(stl_file));

	zDepth = -5;
	maxExtent = 10;

	stl_open(stlObject, (char*)filename);
	stl_open(stlObjectView, (char*)filename);
	stlView->SetSTL(stlObjectView);
	
	if (stl_get_error(stlObject) || stl_get_error(stlObjectView)) {
		CloseFile();
		errorTimeCounter = 3;
		return;
	}
	
	stl_fix_normal_values(stlObject);
	stl_fix_normal_values(stlObjectView);

	TransformPosition();

	BPath path(filename);
	SetTitle(path.Leaf());
	fOpenedFileName.SetTo(filename);

	stlView->LockGL();
	stlView->LockLooper();

	GLfloat Width = stlView->Bounds().Width() + 1;
	GLfloat Height =  stlView->Bounds().Height() + 1;
	glViewport(0, 0, Width, Height);
  	glMatrixMode(GL_PROJECTION);
  	glLoadIdentity();

  	gluPerspective(FOV, (GLfloat)Width/(GLfloat)Height, 0.1f, (zDepth + maxExtent));

  	glMatrixMode(GL_MODELVIEW);

	errorTimeCounter = 0;
	stlView->RenderUpdate();

  	stlView->UnlockLooper();
	stlView->UnlockGL();

	stlModified = false;
	stlValid = true;
	EnableMenuItems(true);
	UpdateStats();
}

void
STLWindow::CloseFile(void)
{
	if (IsLoaded()) {
		SetTitle(MAIN_WIN_TITLE);
		stlValid = false;

		stl_file* stl = stlObject;
		stlObject = NULL;
		stl_close(stl);
		free (stl);

		stl = stlObjectView;
		stlObjectView = NULL;
		stl_close(stl);
		free (stl);

		stlView->RenderUpdate();
		errorTimeCounter = 0;
		EnableMenuItems(false);
		UpdateStats();
	}
}

void
STLWindow::UpdateStats(void)
{
	if (statWindow != NULL) {
		bool isLoaded = IsLoaded();
		if (isLoaded) {
			stl_calculate_volume(stlObject);
//			stl_calculate_surface_area(stlObject);
		}
		BPath path(fOpenedFileName);
		statWindow->SetTextValue("filename", isLoaded ? path.Leaf() : 0);
		statWindow->SetTextValue("type", isLoaded ? (stlObject->stats.type == binary ? "Binary" : "ASCII") : "");
		statWindow->SetTextValue("title", isLoaded ? stlObject->stats.header : "");

		statWindow->SetFloatValue("min-x", isLoaded ? stlObject->stats.min.x : 0);
		statWindow->SetFloatValue("min-y", isLoaded ? stlObject->stats.min.y : 0);
		statWindow->SetFloatValue("min-z", isLoaded ? stlObject->stats.min.z : 0);
		statWindow->SetFloatValue("max-x", isLoaded ? stlObject->stats.max.x : 0);
		statWindow->SetFloatValue("max-y", isLoaded ? stlObject->stats.max.y : 0);
		statWindow->SetFloatValue("max-z", isLoaded ? stlObject->stats.max.z : 0);
		statWindow->SetFloatValue("width", isLoaded ? stlObject->stats.size.x : 0);
		statWindow->SetFloatValue("length", isLoaded ? stlObject->stats.size.y : 0);
		statWindow->SetFloatValue("height", isLoaded ? stlObject->stats.size.z : 0);
		statWindow->SetFloatValue("volume", isLoaded ? stlObject->stats.volume : 0);
//		statWindow->SetFloatValue("surface", isLoaded ? stlObject->stats.surface_area : 0);
		statWindow->SetIntValue("num_facets", isLoaded ? stlObject->stats.number_of_facets : 0);
		statWindow->SetIntValue("num_disconnected_facets",
			isLoaded ? (stlObject->stats.facets_w_1_bad_edge + stlObject->stats.facets_w_2_bad_edge + 
			stlObject->stats.facets_w_3_bad_edge) : 0);
		statWindow->SetIntValue("parts", isLoaded ? stlObject->stats.number_of_parts : 0);
		statWindow->SetIntValue("degenerate", isLoaded ? stlObject->stats.degenerate_facets : 0);
		statWindow->SetIntValue("edges", isLoaded ? stlObject->stats.edges_fixed : 0);
		statWindow->SetIntValue("removed", isLoaded ? stlObject->stats.facets_removed : 0);
		statWindow->SetIntValue("added", isLoaded ? stlObject->stats.facets_added : 0);
		statWindow->SetIntValue("reversed", isLoaded ? stlObject->stats.facets_reversed : 0);
		statWindow->SetIntValue("backward", isLoaded ? stlObject->stats.backwards_edges : 0);
		statWindow->SetIntValue("normals", isLoaded ? stlObject->stats.normals_fixed : 0);
	}
}

void
STLWindow::TransformPosition()
{
	stl_translate(stlObjectView, 0, 0, 0);

	float xMaxExtent = 0;
	float yMaxExtent = 0;
	float zMaxExtent = 0;

	for (int i = 0 ; i < stlObjectView->stats.number_of_facets ; i++) {
		for (int j = 0; j < 3; j++) {
			if (stlObjectView->facet_start[i].vertex[j].x > xMaxExtent)
				xMaxExtent = stlObjectView->facet_start[i].vertex[0].x;
			if (stlObjectView->facet_start[i].vertex[j].y > yMaxExtent)
				yMaxExtent = stlObjectView->facet_start[i].vertex[0].y;
			if (stlObjectView->facet_start[i].vertex[j].z > zMaxExtent)
				zMaxExtent = stlObjectView->facet_start[i].vertex[0].z;
		}
	}

	float longerSide = xMaxExtent > yMaxExtent ? xMaxExtent : yMaxExtent;
	longerSide += (zMaxExtent * (sin(FOV * (M_PI / 180.0)) / sin((90.0 - FOV) * (M_PI / 180.0))));

	zDepth = -((longerSide / 2.0) / tanf((FOV / 2.0) * (M_PI / 180.0)));

	if ((xMaxExtent > yMaxExtent) && (xMaxExtent > zMaxExtent))
    	maxExtent = xMaxExtent;
	if ((yMaxExtent > xMaxExtent) && (yMaxExtent > zMaxExtent))
		maxExtent = yMaxExtent;
	if ((zMaxExtent > yMaxExtent) && (zMaxExtent > xMaxExtent))
		maxExtent = zMaxExtent;

	stl_translate_relative(stlObjectView, -xMaxExtent / 2.0, -yMaxExtent / 2.0, -zMaxExtent / 2.0);
}

int32
STLWindow::RenderFunction(void *data)
{
	STLView *view = (STLView*)data;
	for(;;) {
		view->Render();
		snooze(1000000 / FPS_LIMIT);
	}
}
