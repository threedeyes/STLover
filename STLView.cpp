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

#define GL_VERSION_4_6 1
#define GL_GLEXT_PROTOTYPES 1

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glext.h>
#include <GLView.h>

#include "STLApp.h"
#include "STLView.h"
#include "STLWindow.h"

#include <cstring>
#include <iostream>
#include <vector>

using namespace std;

#undef  B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT          "STLoverGLView"

STLView::STLView(BRect frame, uint32 type)
	: BGLView(frame, "STLView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW, type),
	needUpdate(true),
	showAxes(false),
	showBox(false),
	showOXY(false),
	fShowPreview(false),
	viewOrtho(false),
	m_buffersInitialized(false)
{
	appIcon = STLoverApplication::GetIcon(NULL, 164);
}

STLView::~STLView()
{
	CleanupBuffers();
	delete appIcon;
}

void
STLView::CleanupBuffers()
{
	if (stlVAO) {
		glDeleteVertexArrays(1, &stlVAO);
		stlVAO = 0;
	}
	if (stlVertexVBO) {
		glDeleteBuffers(1, &stlVertexVBO);
		stlVertexVBO = 0;
	}
	if (stlNormalVBO) {
		glDeleteBuffers(1, &stlNormalVBO);
		stlNormalVBO = 0;
	}
	if (boxVAO) {
		glDeleteVertexArrays(1, &boxVAO);
		glDeleteBuffers(1, &boxVBO);
		boxVAO = 0;
		boxVBO = 0;
	}
	if (axesVAO) {
		glDeleteVertexArrays(1, &axesVAO);
		glDeleteBuffers(1, &axesVBO);
		axesVAO = 0;
		axesVBO = 0;
	}
	if (oxyVAO) {
		glDeleteVertexArrays(1, &oxyVAO);
		glDeleteBuffers(1, &oxyVBO);
		oxyVAO = 0;
		oxyVBO = 0;
	}

	m_buffersInitialized = false;
}

void
STLView::InitializeBuffers()
{
	if (m_buffersInitialized || !stlObject)
		return;

	// STL
	glGenVertexArrays(1, &stlVAO);
	glGenBuffers(1, &stlVertexVBO);
	glGenBuffers(1, &stlNormalVBO);

	glBindVertexArray(stlVAO);

	std::vector<float> vertices;
	std::vector<float> normals;
	stlVertexCount = stlObject->stats.number_of_facets * 3;

	for (size_t i = 0; i < stlObject->stats.number_of_facets; i++) {
		for (int j = 0; j < 3; j++) {
			vertices.push_back(stlObject->facet_start[i].vertex[j].x);
			vertices.push_back(stlObject->facet_start[i].vertex[j].y);
			vertices.push_back(stlObject->facet_start[i].vertex[j].z);
		}

		for (int j = 0; j < 3; j++) {
			normals.push_back(stlObject->facet_start[i].normal.x);
			normals.push_back(stlObject->facet_start[i].normal.y);
			normals.push_back(stlObject->facet_start[i].normal.z);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, stlVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
				vertices.data(), GL_STATIC_DRAW);
	glVertexPointer(3, GL_FLOAT, 0, nullptr);
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, stlNormalVBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float),
				normals.data(), GL_STATIC_DRAW);
	glNormalPointer(GL_FLOAT, 0, nullptr);
	glEnableClientState(GL_NORMAL_ARRAY);

	glBindVertexArray(0);

	// Box
	stl_vertex min = stlObject->stats.min;
	stl_vertex size = stlObject->stats.size;
	boxVertices = {
		min.x, min.y, min.z, min.x + size.x, min.y, min.z,
		min.x + size.x, min.y, min.z, min.x + size.x, min.y + size.y, min.z,
		min.x + size.x, min.y + size.y, min.z, min.x, min.y + size.y, min.z,
		min.x, min.y + size.y, min.z, min.x, min.y, min.z,

		min.x, min.y, min.z + size.z, min.x + size.x, min.y, min.z + size.z,
		min.x + size.x, min.y, min.z + size.z, min.x + size.x, min.y + size.y, min.z + size.z,
		min.x + size.x, min.y + size.y, min.z + size.z, min.x, min.y + size.y, min.z + size.z,
		min.x, min.y + size.y, min.z + size.z, min.x, min.y, min.z + size.z,

		min.x, min.y, min.z, min.x, min.y, min.z + size.z,
		min.x + size.x, min.y, min.z, min.x + size.x, min.y, min.z + size.z,
		min.x + size.x, min.y + size.y, min.z, min.x + size.x, min.y + size.y, min.z + size.z,
		min.x, min.y + size.y, min.z, min.x, min.y + size.y, min.z + size.z
	};

	glGenVertexArrays(1, &boxVAO);
	glGenBuffers(1, &boxVBO);
	glBindVertexArray(boxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
	glBufferData(GL_ARRAY_BUFFER, boxVertices.size() * sizeof(float),
			boxVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// Axes
	axisVertices = {
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // X
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,  // Y
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f   // Z
	};

	glGenVertexArrays(1, &axesVAO);
	glGenBuffers(1, &axesVBO);
	glBindVertexArray(axesVAO);
	glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
	glBufferData(GL_ARRAY_BUFFER, axisVertices.size() * sizeof(float),
			axisVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// OXY
	oxyVertices.clear();
	float margin = 30.0f;
	float xMin = round((stlObject->stats.min.x - margin) / 10.0) * 10.0;
	float xMax = round((stlObject->stats.max.x + margin) / 10.0) * 10.0;
	float yMin = round((stlObject->stats.min.y - margin) / 10.0) * 10.0;
	float yMax = round((stlObject->stats.max.y + margin) / 10.0) * 10.0;

	for (float x = xMin; x <= xMax; x += 10) {
		oxyVertices.push_back(x); oxyVertices.push_back(yMin); oxyVertices.push_back(0.0f);
		oxyVertices.push_back(x); oxyVertices.push_back(yMax); oxyVertices.push_back(0.0f);
	}
	for (float y = yMin; y <= yMax; y += 10) {
		oxyVertices.push_back(xMin); oxyVertices.push_back(y); oxyVertices.push_back(0.0f);
		oxyVertices.push_back(xMax); oxyVertices.push_back(y); oxyVertices.push_back(0.0f);
	}

	glGenVertexArrays(1, &oxyVAO);
	glGenBuffers(1, &oxyVBO);
	glBindVertexArray(oxyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, oxyVBO);
	glBufferData(GL_ARRAY_BUFFER, oxyVertices.size() * sizeof(float),
			oxyVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	m_buffersInitialized = true;
}

void 
STLView::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case B_MOUSE_WHEEL_CHANGED:
		{
			if (stlWindow->IsLoaded()) {
				float dy = message->FindFloat("be:wheel_delta_y");
				scaleFactor += ((dy * (tanf(0.26179939) * (stlWindow->GetZDepth() + scaleFactor)))) * 0.3;
				needUpdate = true;
			}
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}

void
STLView::AttachedToWindow(void)
{
	stlWindow = (STLWindow*)Window();

	LockGL();
	BGLView::AttachedToWindow();
	boundRect = Bounds();
	SetupProjection();

	float Light_Ambient[]=  { 1.0f, 0.9f, 1.0f, 1.0f };
	float Light_Diffuse[]=  { 1.0f, 0.9f, 1.0f, 1.0f };
	float Light_Position[]= { 2.0f, 2.0f, 0.0f, 1.0f };

	glLightfv(GL_LIGHT1, GL_POSITION, Light_Position);
	glLightfv(GL_LIGHT1, GL_AMBIENT,  Light_Ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,  Light_Diffuse);
	glEnable (GL_LIGHT1);

	UnlockGL();
}

void
STLView::SetupProjection(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, boundRect.Width(), boundRect.Height());
	
	if (viewOrtho) {
		float w = (stlWindow->GetBigExtent() / 2.0f);
		float tmp = (stlWindow->GetZDepth() + scaleFactor)/stlWindow->GetZDepth();
		w = w * tmp;
		float ratio = (GLfloat)boundRect.Width() / (GLfloat)boundRect.Height();
		glOrtho(-w * ratio, w * ratio, -w, w, 0.1f, (-stlWindow->GetZDepth() + stlWindow->GetBigExtent()) * 2.0f);
	}
	else {
		gluPerspective(FOV, (GLfloat)boundRect.Width() / (GLfloat)boundRect.Height(),
			0.1f, stlWindow->GetZDepth() + stlWindow->GetBigExtent());
	}
}

void
STLView::FrameResized(float Width, float Height)
{
	LockGL();
	BGLView::FrameResized(Width, Height);
	boundRect = Bounds();
	SetupProjection();
	needUpdate = true;
	UnlockGL();
	Render();
}

void 
STLView::MouseMoved(BPoint p, uint32 transit,const BMessage *message)
{
	uint32 buttons = 0;
	GetMouse(&p, &buttons, false);

	if (buttons & B_PRIMARY_MOUSE_BUTTON && lastMouseButtons != 0) {
		yRotate -= (lastMousePos.x - p.x) * 0.4;
		xRotate -= (lastMousePos.y - p.y) * 0.4;
		lastMousePos = p;
		needUpdate = true;
	}
	if (buttons & B_SECONDARY_MOUSE_BUTTON && lastMouseButtons != 0) {
		xPan += ((lastMousePos.x - p.x) * (tanf(0.26179939) * (stlWindow->GetZDepth() + scaleFactor))) * 0.005;
		yPan -= ((lastMousePos.y - p.y) * (tanf(0.26179939) * (stlWindow->GetZDepth() + scaleFactor))) * 0.005;
		lastMousePos = p;
		needUpdate = true;
	}
}

void
STLView::MouseDown(BPoint p)
{
	lastMousePos = p;
	lastMouseClickTime = system_time();
	lastMouseButtons = Window()->CurrentMessage()->FindInt32("buttons");
	SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY);
}

void
STLView::MouseUp(BPoint p)
{
	if ((lastMouseButtons & B_SECONDARY_MOUSE_BUTTON) &&
		(system_time() - lastMouseClickTime) < 250000)
		Window()->PostMessage(MSG_POPUP_MENU);

	lastMouseButtons = 0;
}

void
STLView::Reset(bool scale, bool rotate, bool pan)
{
	if(scale)
		scaleFactor = 0.0;

	if(rotate) {
		xRotate = -45.0;
		yRotate = 45.0;
	}

	if(pan) {
		xPan = 0.0;
		yPan = 0.0;
	}

	lastMousePos = BPoint(0, 0);
	lastMouseButtons = 0;
	needUpdate = true;
}

void
STLView::SetSTL(stl_file *stl)
{
	LockGL();
	CleanupBuffers();
	stlObject = stl;
	SetupProjection();
	InitializeBuffers();
	Reset();
	UnlockGL();
}

void
STLView::Reload(void)
{
	LockGL();
	CleanupBuffers();
	InitializeBuffers();
	UnlockGL();
}

void
STLView::DrawBox()
{
	glBindVertexArray(boxVAO);
	glColor4f(0.9f, 0.25f, 0.6f, 1.0f);
	glDrawArrays(GL_LINES, 0, boxVertices.size() / 3);
	glBindVertexArray(0);
}

void
STLView::DrawOXY()
{
	glBindVertexArray(oxyVAO);
	glColor4f(0.1f, 0.1f, 1.0f, 1.0f);
	glDrawArrays(GL_LINES, 0, oxyVertices.size() / 3);
	glBindVertexArray(0);
}


void
STLView::Billboard()
{
	float matrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX,matrix);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			matrix[i * 4 + j] = (i == j) ? 1.0f : 0.0f;
		}
	}

	glLoadMatrixf(matrix);
}

void
STLView::DrawAxis()
{
	glBindVertexArray(axesVAO);
	glColor3f(1.0f, 0.0f, 0.0f);
	glDrawArrays(GL_LINES, 0, 2);
	glColor3f(0.0f, 1.0f, 0.0f);
	glDrawArrays(GL_LINES, 2, 2);
	glColor3f(0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);

	glPushMatrix();
	glDisable(GL_DEPTH_TEST);
	DrawAxisLabel(1.1f, 0.0f, 0.0f, "X", 1.0f, 0.0f, 0.0f);
	DrawAxisLabel(0.0f, 1.1f, 0.0f, "Y", 0.0f, 1.0f, 0.0f);
	DrawAxisLabel(0.0f, 0.0f, 1.1f, "Z", 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
}

void
STLView::DrawAxisLabel(float x, float y, float z, const char* label, float r, float g, float b) {
	glPushMatrix();
	glTranslatef(x, y, z);
	Billboard();
	glColor3f(r, g, b);
	float textWidth = 0.0f;
	for (const char* c = label; *c != '\0'; c++) {
		textWidth += glutStrokeWidth(GLUT_STROKE_ROMAN, *c);
	}
	textWidth *= 0.0003f;
	glTranslatef(-textWidth / 2.0f, 0.0f, 0.0f);
	glScalef(0.0003f, 0.0003f, 0.0003f);
	for (const char* c = label; *c != '\0'; c++) {
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
	}
	glPopMatrix();
}

void
STLView::DrawSTL(rgb_color color)
{
	if (!m_buffersInitialized)
		return;

	glBindVertexArray(stlVAO);
	glColor3ub(color.red, color.green, color.blue);
	glDrawArrays(GL_TRIANGLES, 0, stlVertexCount);
	glBindVertexArray(0);
}

void
STLView::Render(void)
{
	if (!needUpdate)
		return;

	if (stlWindow->IsLoaded()) {
		LockGL();
		SetupProjection();
		needUpdate = false;

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glShadeModel(GL_FLAT);

		glClearColor(0.12f, 0.12f, 0.2f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		if (viewOrtho)
			glTranslatef(xPan, yPan, stlWindow->GetZDepth());
		else
			glTranslatef(xPan, yPan, stlWindow->GetZDepth() + scaleFactor);

		glRotatef(xRotate, 1.0f, 0.0f, 0.0f);
		glRotatef(yRotate, 0.0f, 0.0f, 1.0f);

		glPolygonMode(GL_FRONT_AND_BACK,
			viewMode == MSG_VIEWMODE_WIREFRAME ? GL_LINE : GL_FILL);

		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);

		if (fShowPreview) {
			glPushMatrix();
				glMultMatrixf(fPreviewMatrix);
				DrawSTL({128, 101, 0});
			glPopMatrix();
		}
		else {
			DrawSTL();
		}

		glDisable(GL_LIGHTING);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);

		if (showOXY)
			DrawOXY();

		if (showBox)
			DrawBox();

		if (showAxes) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			float aspect = (float)boundRect.Width()/(float)boundRect.Height();
			float fx = aspect * 0.8f;;
			float fy = -0.8f;

			glOrtho(-aspect, aspect, -1, 1, 0.01, 1000);

			glMatrixMode(GL_MODELVIEW);

			glLoadIdentity();

			glTranslatef(fx, fy, -1);
			glScalef(0.1f, 0.1f, 0.1f);
			glRotatef(xRotate, 1.0f, 0.0f, 0.0f);
			glRotatef(yRotate, 0.0f, 0.0f, 1.0f);
			DrawAxis();
		}

		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);

		SwapBuffers();
		UnlockGL();
	}
}

void
STLView::ShowPreview(float *matrix)
{
	std::memcpy(fPreviewMatrix,matrix,sizeof(float) * 16);
	fShowPreview = true;
	RenderUpdate();
}
