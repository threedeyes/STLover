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

#ifndef STLOVER_APPLICATION
#define STLOVER_APPLICATION

#include <Application.h>
#include <Message.h>
#include <GLView.h>
#include <File.h>
#include <Path.h>
#include <MimeType.h>
#include <Resources.h>
#include <AppFileInfo.h>
#include <IconUtils.h>
#include <Catalog.h>

#define GL_VERSION_4_6 1
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GLView.h>

#include <private/interface/WindowStack.h>

#include <admesh/stl.h>

#define APP_SIGNATURE "application/x-vnd.stlover"
#define STL_SIGNATURE "application/sla"

#define MAIN_WIN_TITLE B_TRANSLATE_SYSTEM_NAME("STLover")
#define APP_SETTINGS_FILENAME "STLover_settings"

#define MSG_FILE_OPEN					'OPEN'
#define MSG_FILE_APPEND					'APND'
#define MSG_FILE_SAVE					'SVFL'
#define MSG_FILE_SAVE_AS				'SVAS'
#define MSG_FILE_CLOSE					'CLOS'
#define MSG_FILE_RELOAD					'RELD'
#define MSG_FILE_EXPORT_STLA			'ESTA'
#define MSG_FILE_EXPORT_STLB			'ESTB'
#define MSG_FILE_EXPORT_DXF				'EDXF'
#define MSG_FILE_EXPORT_VRML			'EVRM'
#define MSG_FILE_EXPORT_OFF				'EOFF'
#define MSG_FILE_EXPORT_OBJ				'EOBJ'
#define MSG_VIEWMODE_SOLID				'SOLD'
#define MSG_VIEWMODE_WIREFRAME			'WIRF'
#define MSG_VIEWMODE_WIREFRAME_TOGGLE	'WIRT'
#define MSG_VIEWMODE_RESETPOS			'RSPS'
#define MSG_VIEWMODE_AXES				'AXES'
#define MSG_VIEWMODE_AXES_PLANE			'AXPL'
#define MSG_VIEWMODE_AXES_COMPASS		'AXCS'
#define MSG_VIEWMODE_OXY				'POXY'
#define MSG_VIEWMODE_BOUNDING_BOX		'BBOX'
#define MSG_VIEWMODE_STAT				'STAT'
#define MSG_VIEWMODE_STAT_WINDOW		'SWIN'
#define MSG_VIEWMODE_ZOOMIN				'ZOIN'
#define MSG_VIEWMODE_ZOOMOUT			'ZOUT'
#define MSG_VIEWMODE_ZOOMFIT			'ZFIT'
#define MSG_VIEWMODE_TOP				'VTOP'
#define MSG_VIEWMODE_RIGHT				'VRGT'
#define MSG_VIEWMODE_FRONT				'VFRT'
#define MSG_VIEWMODE_ORTHO				'VORT'
#define MSG_TOOLS_EDIT_TITLE			'EDTI'
#define MSG_TOOLS_TITLE_SET				'TIST'
#define MSG_TOOLS_SCALE					'SCAL'
#define MSG_TOOLS_SCALE_3				'SCL3'
#define MSG_TOOLS_SCALE_SET				'SCST'
#define MSG_TOOLS_SCALE_SET_3			'SST3'
#define MSG_TOOLS_ROTATE				'ROTA'
#define MSG_TOOLS_ROTATE_SET			'SROT'
#define MSG_TOOLS_MIRROR_XY				'M_XY'
#define MSG_TOOLS_MIRROR_YZ				'M_YZ'
#define MSG_TOOLS_MIRROR_XZ				'M_XZ'
#define MSG_TOOLS_MOVE_TO				'MVTO'
#define MSG_TOOLS_MOVE_TO_SET			'MVTS'
#define MSG_TOOLS_MOVE_BY				'MVBY'
#define MSG_TOOLS_MOVE_BY_SET			'MVBS'
#define MSG_TOOLS_MOVE_ZERO				'ZERO'
#define MSG_TOOLS_MOVE_CENTER			'CNTR'
#define MSG_TOOLS_MOVE_MIDDLE			'MDLE'
#define MSG_TOOLS_REPAIR				'REPR'
#define MSG_TOOLS_REPAIR_DO				'REPD'
#define MSG_TOOLS_MEASURE				'RLRO'
#define MSG_TOOLS_MEASURE_CLOSE			'RLRC'
#define MSG_TOOLS_MEASURE_UPDATE		'RLUP'
#define MSG_PULSE						'PULS'
#define MSG_APPEND_REFS_RECIEVED		'APRR'
#define MSG_INPUT_VALUE_UPDATED			'IVUP'
#define MSG_INPUT_OK					'IWOK'
#define MSG_INPUT_CANCEL				'IWKO'
#define MSG_INPUT_RESET					'IRES'
#define MSG_EASTER_EGG					'EEGG'
#define MSG_POPUP_MENU					'POPM'
#define MSG_APP_QUIT					'QAPP'
#define MSG_WINDOW_ACTIVATED			'AWIN'
#define MSG_WINDOW_CLOSED				'CWIN'
#define MSG_FILE_OPENED					'FOOK'
#define MSG_FILE_OPEN_FAILED			'FOER'

#define FOV	30
#define FPS_LIMIT 100

#define TOOLBAR_ICON_SIZE 22
#define INPUT_WINDOW_ALIGN_MARGIN 16

class STLWindow;

class STLoverApplication : public BApplication {
	public:
		STLoverApplication();
		virtual void MessageReceived(BMessage *message);
		virtual void RefsReceived(BMessage* message);
		virtual	void ArgvReceived(int32 argc, char** argv);
		virtual void ReadyToRun();

		static BBitmap *GetIcon(const char *iconName, int iconSize);

	private:
		void InstallMimeType(void);
		STLWindow *CreateWindow(void);
		STLWindow *lastActivatedWindow;
};

#endif
