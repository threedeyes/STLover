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
#include "STLRepairWindow.h"

#undef  B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT          "STLoverRepairWindow"

STLRepairWindow::STLRepairWindow(BWindow* target, uint32 messageId, BMessage *options)
	: BWindow(BRect(100, 100, 400, 400), B_TRANSLATE("Repair"), B_FLOATING_WINDOW_LOOK, B_FLOATING_SUBSET_WINDOW_FEEL,
	B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fTarget(target),
	fMessageId(messageId),
	fOptions(options)
{	
	fExactCheckBox = new BCheckBox(B_TRANSLATE("Only check for perfectly matched edges"));
	fExactCheckBox->SetValue(options->FindInt32("exactFlag"));
	fNearbyCheckBox = new BCheckBox(B_TRANSLATE("Find and connect nearby facets. Correct bad facets"));
	fNearbyCheckBox->SetValue(options->FindInt32("nearbyFlag"));
	fRemoveUnconnectedCheckBox = new BCheckBox(B_TRANSLATE("Remove facets that have 0 neighbors"));
	fRemoveUnconnectedCheckBox->SetValue(options->FindInt32("removeUnconnectedFlag"));
	fAddFacetsCheckBox = new BCheckBox(B_TRANSLATE("Add facets to fill holes"));
	fAddFacetsCheckBox->SetValue(options->FindInt32("fillHolesFlag"));
	fNormalDirectionsCheckBox = new BCheckBox(B_TRANSLATE("Check and fix direction of normals"));
	fNormalDirectionsCheckBox->SetValue(options->FindInt32("normalDirectionsFlag"));
	fNormalValuesCheckBox = new BCheckBox(B_TRANSLATE("Check and fix normal values"));
	fNormalValuesCheckBox->SetValue(options->FindInt32("normalValuesFlag"));
	fReverseCheckBox = new BCheckBox(B_TRANSLATE("Reverse the directions of all facets and normals"));
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

	fIterationsTextControl = new BTextControl(B_TRANSLATE("Number of iterations for nearby check"), iterationsText.String(), NULL);
	fIterationsTextControl->SetAlignment(B_ALIGN_LEFT, B_ALIGN_LEFT);

	fIncrementTextControl = new BTextControl(B_TRANSLATE("Amount to increment tolerance after iteration"), incrementText.String(), NULL);
	fIncrementTextControl->SetAlignment(B_ALIGN_LEFT, B_ALIGN_LEFT);

	fToleranceTextControl = new BTextControl(B_TRANSLATE("Initial tolerance to use for nearby check"), toleranceText.String(), NULL);
	fToleranceTextControl->SetAlignment(B_ALIGN_LEFT, B_ALIGN_LEFT);

	fOkButton = new BButton(B_TRANSLATE("OK"), new BMessage(MSG_INPUT_OK));
	fOkButton->SetEnabled(true);

	BButton* cancelButton = new BButton(B_TRANSLATE("Cancel"), new BMessage(B_QUIT_REQUESTED));

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

	ResizeToPreferred();
	BRect parentRect = target->Frame();
	MoveTo(parentRect.left + ((parentRect.Width() - Frame().Width()) / 2.0),
		parentRect.top + ((parentRect.Height() - Frame().Height()) / 2.0));

	if (target != NULL)
		AddToSubset(target);
	else
		SetFeel(B_FLOATING_APP_WINDOW_FEEL);
}

bool
STLRepairWindow::QuitRequested()
{
	fTarget.SendMessage(MSG_INPUT_CANCEL);
	return true;
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
