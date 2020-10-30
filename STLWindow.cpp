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

STLWindow::STLWindow(BRect frame, uint32 type)
	: BWindow(frame, "STLover - Simple STL Viewer", B_TITLED_WINDOW, 0),
	fOpenFilePanel(NULL),
	stlModified(false),
	stlValid(false),
	stlObject(NULL),
	errorTimeCounter(0),
	zDepth(-5),
	maxExtent(10)
{
	fMenuBar = new BMenuBar(BRect(0, 0, Bounds().Width(), 22), "menubar");
	fMenuFile = new BMenu("File");
	fMenuView = new BMenu("View");
	fMenuTools = new BMenu("Tools");
	fMenuToolsMirror = new BMenu("Mirror");
	fMenuHelp = new BMenu("Help");

	fMenuFile->AddItem(new BMenuItem("Open...", new BMessage(MSG_FILE_OPEN), 'O'));
	fMenuItemClose = new BMenuItem("Close", new BMessage(MSG_FILE_CLOSE));
	fMenuFile->AddItem(fMenuItemClose);
	fMenuFile->AddSeparatorItem();
	fMenuItemSave = new BMenuItem("Save", new BMessage(MSG_FILE_SAVE), 'S');
	fMenuFile->AddItem(fMenuItemSave);
	fMenuFile->AddSeparatorItem();
	fMenuFile->AddItem(new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q'));
	fMenuBar->AddItem(fMenuFile);
	fMenuFile->SetTargetForItems(this);

	fMenuItemSolid = new BMenuItem("Solid", new BMessage(MSG_VIEWMODE_SOLID));
	fMenuItemSolid->SetMarked(true);
	fMenuView->AddItem(fMenuItemSolid);
	fMenuItemWireframe = new BMenuItem("Wireframe", new BMessage(MSG_VIEWMODE_WIREFRAME));	
	fMenuView->AddItem(fMenuItemWireframe);
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

	stlView = new STLView(Bounds(), type);
	AddChild(stlView);

	stlView->AddChild(fMenuBar);

	SetSizeLimits(320, 4096, 256, 4049);
	
	Show();
	stlView->RenderUpdate();

	SetPulseRate(1000000);

	rendererThread = spawn_thread(RenderFunction, "renderThread", B_DISPLAY_PRIORITY, (void*)stlView);
	resume_thread(rendererThread);
}

STLWindow::~STLWindow()
{
	kill_thread(rendererThread);
	CloseFile();
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
		case MSG_FILE_OPEN:
		{
			if (!fOpenFilePanel) {
				fOpenFilePanel = new BFilePanel(B_OPEN_PANEL, NULL, NULL,
					B_FILE_NODE, true, NULL, NULL, false, true);
				fOpenFilePanel->SetTarget(this);
			}
			fOpenFilePanel->Show();
			break;
		}
		case MSG_FILE_SAVE:
		{
			BPath path(fOpenedFileName);
			stl_write_binary(stlObject, path.Path(), path.Leaf());
			BNode node(path.Path());
			BNodeInfo nodeInfo(&node);
			nodeInfo.SetType("application/sla");
			stlModified = false;
			EnableMenuItems(true);
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
		case B_REFS_RECEIVED:
			DetachCurrentMessage();
			message->what = B_REFS_RECEIVED;
			BMessenger(be_app).SendMessage(message);
		case B_CANCEL:
			break;
		case B_ABOUT_REQUESTED:
		{
			BAboutWindow* wind = new BAboutWindow("STLover", APP_SIGNATURE);
			wind->AddCopyright(2020, "Gerasim Troeglazov (3dEyes**)");
			wind->AddDescription("Simple STL Viewer");
			wind->Show();
			break;
		}
		case MSG_VIEWMODE_RESETPOS:
		{
			OpenFile(fOpenedFileName.String());
			break;
		}
		case MSG_TOOLS_MIRROR_XY:
		{
			stl_mirror_xy(stlObject);
			stlModified = true;
			EnableMenuItems(true);
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_MIRROR_YZ:
		{
			stl_mirror_yz(stlObject);
			stlModified = true;
			EnableMenuItems(true);
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_MIRROR_XZ:
		{
			stl_mirror_xz(stlObject);
			stlModified = true;
			EnableMenuItems(true);
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_FILL_HOLES:
		{
			stl_fill_holes(stlObject);
			stlModified = true;
			EnableMenuItems(true);
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_REMOVE_UNCONNECTED:
		{
			stl_remove_unconnected_facets(stlObject);
			stlModified = true;
			EnableMenuItems(true);
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_CHECK_DIRECT:
		{
			stl_fix_normal_directions(stlObject);
			stlModified = true;
			EnableMenuItems(true);
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_CHECK_NORMALS:
		{
			stl_fix_normal_values(stlObject);
			stlModified = true;
			EnableMenuItems(true);
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_CHECK_NEARBY:
		{
			stl_check_facets_nearby(stlObject, 0);
			stlModified = true;
			EnableMenuItems(true);
			stlView->RenderUpdate();
			break;
		}
		case MSG_TOOLS_REVERSE:
		{
			stl_reverse_all_facets(stlObject);
			stlModified = true;
			EnableMenuItems(true);
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
	fMenuView->SetEnabled(show);
	fMenuTools->SetEnabled(show);
	fMenuItemSave->SetEnabled(show && stlModified);
}

void
STLWindow::OpenFile(const char *filename)
{	
	CloseFile();

	stlObject = (stl_file*)malloc(sizeof(stl_file));
	memset(stlObject,0,sizeof(stl_file));

	zDepth = -5;
	maxExtent = 10;

	stl_open(stlObject, (char*)filename);
	stlView->SetSTL(stlObject);
	
	if (stl_get_error(stlObject)) {
		CloseFile();
		errorTimeCounter = 3;
		return;
	}
	
	stl_fix_normal_values(stlObject);

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
}

void
STLWindow::CloseFile(void)
{
	if (IsLoaded()) {
		SetTitle("STLover - Simple STL Viewer");
		stl_file* stl = stlObject;
		stlObject = NULL;
		stlValid = false;
		stl_close(stl);
		free (stl);
		stlView->RenderUpdate();
		errorTimeCounter = 0;
		EnableMenuItems(false);
	}
}

void
STLWindow::TransformPosition()
{
	stl_translate(stlObject, 0, 0, 0);

	float xMaxExtent = 0;
	float yMaxExtent = 0;
	float zMaxExtent = 0;

	for (int i = 0 ; i < stlObject->stats.number_of_facets ; i++) {
		for (int j = 0; j < 3; j++) {
			if (stlObject->facet_start[i].vertex[j].x > xMaxExtent)
				xMaxExtent = stlObject->facet_start[i].vertex[0].x;
			if (stlObject->facet_start[i].vertex[j].y > yMaxExtent)
				yMaxExtent = stlObject->facet_start[i].vertex[0].y;
			if (stlObject->facet_start[i].vertex[j].z > zMaxExtent)
				zMaxExtent = stlObject->facet_start[i].vertex[0].z;
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

	stl_translate_relative(stlObject, -xMaxExtent / 2.0, -yMaxExtent / 2.0, -zMaxExtent / 2.0);
}

int32
STLWindow::RenderFunction(void *data)
{
	STLView *view = (STLView*)data;
	for(;;) {
		view->Render();
		snooze(1000000/100);
	}
}
