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
#include "STLRepairWindow.h"

STLRepairWindow::STLRepairWindow(BWindow* target, uint32 messageId, BMessage *options)
	: BWindow(BRect(100, 100, 400, 400), "Repair", B_MODAL_WINDOW_LOOK, B_MODAL_ALL_WINDOW_FEEL,
	B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS),
	fTarget(target),
	fMessageId(messageId),
	fOptions(options)
{	
	fExactCheckBox = new BCheckBox("Only check for perfectly matched edges");
	fExactCheckBox->SetValue(options->FindInt32("exactFlag"));
	fNearbyCheckBox = new BCheckBox("Find and connect nearby facets. Correct bad facets");
	fNearbyCheckBox->SetValue(options->FindInt32("nearbyFlag"));
	fRemoveUnconnectedCheckBox = new BCheckBox("Remove facets that have 0 neighbors");
	fRemoveUnconnectedCheckBox->SetValue(options->FindInt32("removeUnconnectedFlag"));
	fAddFacetsCheckBox = new BCheckBox("Add facets to fill holes");
	fAddFacetsCheckBox->SetValue(options->FindInt32("fillHolesFlag"));
	fNormalDirectionsCheckBox = new BCheckBox("Check and fix direction of normals(ie cw, ccw)");
	fNormalDirectionsCheckBox->SetValue(options->FindInt32("normalDirectionsFlag"));
	fNormalValuesCheckBox = new BCheckBox("Check and fix normal values");
	fNormalValuesCheckBox->SetValue(options->FindInt32("normalValuesFlag"));
	fReverseCheckBox = new BCheckBox("Reverse the directions of all facets and normals");
	fReverseCheckBox->SetValue(options->FindInt32("reverseAllFlag"));

	int32 iterations = options->FindInt32("iterationsValue");
	float tolerance = options->FindFloat("toleranceValue");
	float increment = options->FindFloat("incrementValue");

	BString iterationsText;
	iterationsText.SetToFormat("%d", iterations);
	BString toleranceText;
	toleranceText.SetToFormat("%g", tolerance);
	BString incrementText;
	incrementText.SetToFormat("%g", increment);

	fIterationsTextControl = new BTextControl("Number of iterations for nearby check", iterationsText.String(), NULL);
	fIterationsTextControl->SetAlignment(B_ALIGN_LEFT, B_ALIGN_LEFT);

	fIncrementTextControl = new BTextControl("Amount to increment tolerance after iteration", incrementText.String(), NULL);
	fIncrementTextControl->SetAlignment(B_ALIGN_LEFT, B_ALIGN_LEFT);

	fToleranceTextControl = new BTextControl("Initial tolerance to use for nearby check", toleranceText.String(), NULL);
	fToleranceTextControl->SetAlignment(B_ALIGN_LEFT, B_ALIGN_LEFT);

	fOkButton = new BButton("Ok", new BMessage(MSG_INPUT_OK));
	fOkButton->SetEnabled(true);

	BButton* cancelButton = new BButton("Cancel", new BMessage(B_QUIT_REQUESTED));

	float padding = be_control_look->DefaultItemSpacing();
	BLayoutBuilder::Grid<>(this, padding, padding)
		.SetInsets(padding, padding, padding, padding)
		.Add(fExactCheckBox, 0, 0, 3, 1)
		.Add(fNearbyCheckBox, 0, 1, 3, 1)
		.AddTextControl(fToleranceTextControl, 1, 2, B_ALIGN_HORIZONTAL_UNSET, 2, 1)
		.AddTextControl(fIterationsTextControl, 1, 3, B_ALIGN_HORIZONTAL_UNSET, 2, 1)
		.AddTextControl(fIncrementTextControl, 1, 4, B_ALIGN_HORIZONTAL_UNSET, 2, 1)
		.Add(fRemoveUnconnectedCheckBox, 0, 5, 3, 1)
		.Add(fAddFacetsCheckBox, 0, 6, 3, 1)
		.Add(fNormalDirectionsCheckBox, 0, 7, 3, 1)
		.Add(fNormalValuesCheckBox, 0, 8, 3, 1)
		.Add(fReverseCheckBox, 0, 9, 3, 1)
		.Add(BSpaceLayoutItem::CreateGlue(), 0, 10)
		.Add(cancelButton, 2, 10)
		.Add(fOkButton, 3, 10);

	fOkButton->MakeDefault(true);
	CenterOnScreen();	
}

void
STLRepairWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_INPUT_OK:
		{
			BMessage *msg = new BMessage(fMessageId);
			msg->AddInt32("exactFlag", fExactCheckBox->Value());
			msg->AddInt32("nearbyFlag", fNearbyCheckBox->Value());
			msg->AddInt32("removeUnconnectedFlag", fRemoveUnconnectedCheckBox->Value());
			msg->AddInt32("fillHolesFlag", fAddFacetsCheckBox->Value());
			msg->AddInt32("normalDirectionsFlag", fNormalDirectionsCheckBox->Value());
			msg->AddInt32("normalValuesFlag", fNormalValuesCheckBox->Value());
			msg->AddInt32("reverseAllFlag", fReverseCheckBox->Value());
			msg->AddInt32("iterationsValue", atoi(fIterationsTextControl->Text()));
			msg->AddFloat("toleranceValue", atof(fToleranceTextControl->Text()));
			msg->AddFloat("incrementValue", atof(fIncrementTextControl->Text()));
			fTarget.SendMessage(msg);

			delete fOptions;
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
