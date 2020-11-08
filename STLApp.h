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

#ifndef STLOVER_APPLICATION
#define STLOVER_APPLICATION

#include <Application.h>
#include <Message.h>
#include <GLView.h>
#include <Path.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <admesh/stl.h>

#define APP_SIGNATURE "application/x-vnd.stlviewer"

#define MAIN_WIN_TITLE "STLover"

#define MSG_FILE_OPEN					'OPEN'
#define MSG_FILE_APPEND					'APND'
#define MSG_FILE_SAVE					'SVFL'
#define MSG_FILE_SAVE_AS				'SVAS'
#define MSG_FILE_CLOSE					'CLOS'
#define MSG_FILE_EXPORT_STLA			'ESTA'
#define MSG_FILE_EXPORT_STLB			'ESTB'
#define MSG_FILE_EXPORT_DXF				'EDXF'
#define MSG_FILE_EXPORT_VRML			'EVRM'
#define MSG_FILE_EXPORT_OFF				'EOFF'
#define MSG_FILE_EXPORT_OBJ				'EOBJ'
#define MSG_VIEWMODE_SOLID				'MOD1'
#define MSG_VIEWMODE_WIREFRAME			'MOD2'
#define MSG_VIEWMODE_RESETPOS			'RSPS'
#define MSG_VIEWMODE_BOUNDING_BOX		'BBOX'
#define MSG_VIEWMODE_STAT_WINDOW		'SWIN'
#define MSG_TOOLS_MIRROR_XY				'M_XY'
#define MSG_TOOLS_MIRROR_YZ				'M_YZ'
#define MSG_TOOLS_MIRROR_XZ				'M_XZ'
#define MSG_TOOLS_FILL_HOLES			'HOLE'
#define MSG_TOOLS_REMOVE_UNCONNECTED	'RUNC'
#define MSG_TOOLS_CHECK_DIRECT			'FDIR'
#define MSG_TOOLS_CHECK_NORMALS			'FNRM'
#define MSG_TOOLS_CHECK_NEARBY			'FNRB'
#define MSG_TOOLS_REVERSE				'RVRS'
#define MSG_PULSE						'PULS'
#define MSG_APPEND_REFS_RECIEVED		'APRR'

#define FOV	30
#define FPS_LIMIT 100

class STLWindow;

class STLoverApplication : public BApplication {
	public:
		STLoverApplication();
		virtual void RefsReceived(BMessage* msg);
	private:
		STLWindow *stlWindow;
};

#endif
