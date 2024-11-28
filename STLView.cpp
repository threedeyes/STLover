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
	InitShaders();
}

STLView::~STLView()
{
	CleanupBuffers();
	delete appIcon;
	glDeleteProgram(shaderProgram);
}

void
STLView::InitShaders()
{
	const char* vertexShaderSource = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;
		layout (location = 1) in vec3 aNormal;
		out vec3 FragPos;
		out vec3 Normal;
		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;
		void main()
		{
			FragPos = vec3(model * vec4(aPos, 1.0));
			Normal = mat3(transpose(inverse(model))) * aNormal;
			gl_Position = projection * view * vec4(FragPos, 1.0);
		}
	)";

	const char* fragmentShaderSource = R"(
		#version 330 core
		in vec3 FragPos;
		in vec3 Normal;
		out vec4 FragColor;

		uniform vec3 objectColor;
		uniform vec3 lightPos;
		uniform vec3 viewPos;

		void main()
		{
			vec3 lightColor = vec3(1.0, 1.0, 1.0);

			float ambientStrength = 0.75;
			vec3 ambient = ambientStrength * lightColor;

			vec3 norm = normalize(Normal);
			vec3 lightDir = normalize(lightPos - FragPos);
			float diff = max(dot(norm, lightDir), 0.0);
			vec3 diffuse = diff * lightColor;

			float specularStrength = 0.9;
			vec3 viewDir = normalize(viewPos - FragPos);
			vec3 reflectDir = reflect(-lightDir, norm);
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
			vec3 specular = specularStrength * spec * lightColor;

			vec3 result = (ambient + diffuse + specular) * objectColor;
			FragColor = vec4(result, 1.0);
		}
	)";

	const char* lineVertexShaderSource = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;
		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;
		void main()
		{
			gl_Position = projection * view * model * vec4(aPos, 1.0);
		}
	)";

	const char* lineFragmentShaderSource = R"(
		#version 330 core
		out vec4 FragColor;
		uniform vec3 lineColor;
		void main()
		{
			FragColor = vec4(lineColor, 1.0);
		}
	)";

	shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
	lineShaderProgram = CreateShaderProgram(lineVertexShaderSource, lineFragmentShaderSource);

	if (shaderProgram == 0 || lineShaderProgram == 0) {
		std::cerr << "Failed to create shader program" << std::endl;
		return;
	}

	modelLoc = glGetUniformLocation(shaderProgram, "model");
	viewLoc = glGetUniformLocation(shaderProgram, "view");
	projLoc = glGetUniformLocation(shaderProgram, "projection");
	colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
}

GLuint
STLView::CreateShaderProgram(const char* vertexSource, const char* fragmentSource)
{
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

	if (!vertexShader || !fragmentShader) {
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
		std::cerr << "Shader program linking error: " << infoLog << std::endl;
		glDeleteProgram(program);
		program = 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return program;
}

GLuint
STLView::CompileShader(GLenum type, const char* source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
		std::cerr << "Shader compilation error: " << infoLog << std::endl;
		glDeleteShader(shader);
		return 0;
	}

	return shader;
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
	vertices.reserve(stlVertexCount * 3);
	normals.reserve(stlVertexCount * 3);

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, stlNormalVBO);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float),
				normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	SetupProjection();
	UnlockGL();
}

void STLView::SetupProjection(void)
{
	glViewport(0, 0, boundRect.Width(), boundRect.Height());

	float aspectRatio = boundRect.Width() / boundRect.Height();
	
	if (viewOrtho) {
		float w = stlWindow->GetBigExtent() / 2.0f;
		float tmp = (stlWindow->GetZDepth() + scaleFactor) / stlWindow->GetZDepth();
		w = w * tmp;
		projectionMatrix = glm::ortho(-w * aspectRatio, w * aspectRatio, -w, w, 0.1f,
			(-stlWindow->GetZDepth() + stlWindow->GetBigExtent()) * 2.0f);
	} else {
		float fov = glm::radians((float)FOV);
		float nearPlane = 0.1f;
		float farPlane = stlWindow->GetZDepth() + stlWindow->GetBigExtent();
		projectionMatrix = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
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
STLView::MouseMoved(BPoint p, uint32 transit, const BMessage *message)
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
	glUseProgram(lineShaderProgram);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniform3f(colorLoc, 0.9f, 0.25f, 0.6f);

	glBindVertexArray(boxVAO);
	glDrawArrays(GL_LINES, 0, boxVertices.size() / 3);
	glBindVertexArray(0);
	glUseProgram(0);
}

void
STLView::DrawOXY()
{
	glUseProgram(lineShaderProgram);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniform3f(colorLoc, 0.1f, 0.1f, 1.0f);

	glBindVertexArray(oxyVAO);
	glDrawArrays(GL_LINES, 0, oxyVertices.size() / 3);
	glBindVertexArray(0);
	glUseProgram(0);
}

void
STLView::DrawAxis()
{
	glUseProgram(0);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float aspect = boundRect.Width() / boundRect.Height();
	float fx = aspect * 0.8f;;
	float fy = -0.8f;
	glOrtho(-aspect, aspect, -1, 1, 0.01, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(fx, fy, -1);
	glScalef(0.1f, 0.1f, 0.1f);
	glRotatef(xRotate, 1.0f, 0.0f, 0.0f);
	glRotatef(yRotate, 0.0f, 0.0f, 1.0f);

	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(axesVAO);
	glColor3f(1.0f, 0.0f, 0.0f);
	glDrawArrays(GL_LINES, 0, 2);
	glColor3f(0.0f, 1.0f, 0.0f);
	glDrawArrays(GL_LINES, 2, 2);
	glColor3f(0.0f, 0.0f, 1.0f);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);

	double alpha = std::abs(cos(xRotate * M_PI / 180.0));
	double beta = std::abs(cos(yRotate * M_PI / 180.0));
	double eps = 0.035;

	if (beta > eps * 2.0f|| alpha > eps * 2.0f)
		DrawAxisLabel(1.2f, 0.0f, 0.0f, "X", 1.0f, 0.0f, 0.0f);

	if (std::abs(1.0 - beta) > eps || alpha > eps)
		DrawAxisLabel(0.0f, 1.2f, 0.0f, "Y", 0.0f, 1.0f, 0.0f);

	if (std::abs(1.0 - alpha) > eps)
		DrawAxisLabel(0.0f, 0.0f, 1.2f, "Z", 0.0f, 0.0f, 1.0f);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}

void
STLView::DrawAxisLabel(float x, float y, float z, const char* label, float r, float g, float b) {
	   glPushMatrix();
	   glTranslatef(x, y, z);
	   Billboard();
	   glColor3f(r, g, b);
	   float scaleText = 0.00025f;
	   float textWidth = 0.0f;
	   for (const char* c = label; *c != '\0'; c++) {
			textWidth += glutStrokeWidth(GLUT_STROKE_ROMAN, *c);
	   }
	   textWidth *= scaleText;
	   glTranslatef(-textWidth / 2.0f, 0.0f, 0.0f);
	   glScalef(scaleText, scaleText, scaleText);
	   for (const char* c = label; *c != '\0'; c++) {
			glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
	   }
	   glPopMatrix();
}

void
STLView::Billboard()
{
	float modelview[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

	for(int i = 0; i < 3; i++) {
		for(int j = 0; j < 3; j++) {
			modelview[i*4+j] = (i == j) ? 1.0f : 0.0f;
		}
	}

	glLoadMatrixf(modelview);
}

void
STLView::DrawSTL(rgb_color color)
{
	if (!m_buffersInitialized)
		return;

	glUseProgram(shaderProgram);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniform3f(colorLoc, color.red / 255.0f, color.green / 255.0f, color.blue / 255.0f);

	GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
	GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
	glm::vec3 lightPos(-1000.0f, -1000.0f, 1000.0f);
	glm::vec3 viewPos(0.0f, 0.0f, stlWindow->GetZDepth() + scaleFactor);
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
	glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));

	glBindVertexArray(stlVAO);
	glDrawArrays(GL_TRIANGLES, 0, stlVertexCount);
	glBindVertexArray(0);

	glUseProgram(0);
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

		glClearColor(0.12f, 0.12f, 0.2f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		viewMatrix = glm::mat4(1.0f);
		if (viewOrtho)
			viewMatrix = glm::translate(viewMatrix, glm::vec3(xPan, yPan, stlWindow->GetZDepth()));
		else
			viewMatrix = glm::translate(viewMatrix, glm::vec3(xPan, yPan, stlWindow->GetZDepth() + scaleFactor));

		viewMatrix = glm::rotate(viewMatrix, glm::radians(xRotate), glm::vec3(1.0f, 0.0f, 0.0f));
		viewMatrix = glm::rotate(viewMatrix, glm::radians(yRotate), glm::vec3(0.0f, 0.0f, 1.0f));

		modelMatrix = glm::mat4(1.0f);

		if (viewMode == MSG_VIEWMODE_WIREFRAME)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (fShowPreview) {
			glm::mat4 previewMatrix = glm::make_mat4(fPreviewMatrix);
			modelMatrix = previewMatrix * modelMatrix;
			DrawSTL({128, 101, 0});
		} else {
			DrawSTL();
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_LINE_SMOOTH);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

		if (showOXY)
			DrawOXY();

		if (showBox)
			DrawBox();

		if (showAxes)
			DrawAxis();

		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);

		SwapBuffers();
		UnlockGL();
	}
}

void STLView::ShowPreview(float *matrix)
{
	std::memcpy(fPreviewMatrix, matrix, sizeof(float) * 16);
	fShowPreview = true;
	RenderUpdate();
}
