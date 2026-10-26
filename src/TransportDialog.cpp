/***************************************************************
 ** Copyright (C) 2026 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "TransportDialog.h"
#include "StandaloneSettings.h"

#define S_GAP     4 // gap between items
#define S_HEIGHT  24
#define S_BORDER  5  // size of border gap

void SeqTransportDialog::endDialog(bool hitOk)
{
   if (hitOk) {
      StandaloneSettings::setSendMidiClock(mSendClock->getToggleState());
      StandaloneSettings::setSendMidiTransport(mSendTransport->getToggleState());
   }
}

void SeqTransportDialog::resizedInner(Component *inner)
{
   Rectangle<int> r = inner->getLocalBounds();
   r.reduce(S_BORDER * 2, S_BORDER * 2);

   mLblMain->setBounds(r.removeFromTop(S_HEIGHT));
   r.removeFromTop(S_GAP);
   mSendClock->setBounds(r.removeFromTop(S_HEIGHT));
   r.removeFromTop(S_GAP);
   mSendTransport->setBounds(r.removeFromTop(S_HEIGHT));

   // buttons at the bottom
   Rectangle<int> btns = r.removeFromBottom(S_HEIGHT);
   mBtnOk->setBounds(btns.removeFromLeft(btns.getWidth() / 2 - S_GAP));
   btns.removeFromLeft(S_GAP * 2);
   mBtnCancel->setBounds(btns);
}

SeqTransportDialog::SeqTransportDialog(SeqGlob *glob, CptNotify *parent) :
   SeqModalDialog(glob, SEQCTL_TRANSPORTDIALOG, parent, 300, 160)
{
   mLblMain = std::unique_ptr<Label>(addStdLabel("Standalone Settings"));
   mLblMain->setFont(Font(16.0f, Font::bold));
   mLblMain->setJustificationType(Justification::centred);

   mSendClock = std::unique_ptr<ToggleButton>(addToggle("MIDI clock out", 0));
   mSendTransport = std::unique_ptr<ToggleButton>(addToggle("MIDI transport out (start/stop)", 0));

   mBtnOk = std::unique_ptr<TextButton>(addStdButton("OK", 0, CPTID_TRANSPORT_OK));
   mBtnCancel = std::unique_ptr<TextButton>(addStdButton("Cancel", 0, CPTID_TRANSPORT_CANCEL));
}

void SeqTransportDialog::doSetup()
{
   mSendClock->setToggleState(StandaloneSettings::getSendMidiClock(), dontSendNotification);
   mSendTransport->setToggleState(StandaloneSettings::getSendMidiTransport(), dontSendNotification);
}

void SeqTransportDialog::notify(int cptId, int)
{
   switch (cptId) {
   case CPTID_TRANSPORT_OK:
      closeDialog(true);
      break;
   case CPTID_TRANSPORT_CANCEL:
      closeDialog(false);
      break;
   default:
      break;
   }
}
