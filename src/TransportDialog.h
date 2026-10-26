/***************************************************************
 ** Copyright (C) 2026 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef TRANSPORTDIALOG_H_
#define TRANSPORTDIALOG_H_

#include "Constants.h"
#include "CommonComponents.h"

#define CPTID_TRANSPORT_OK 10
#define CPTID_TRANSPORT_CANCEL 11

/*
Panel that is displayed when the transport settings (cog) button is clicked.
Standalone only. Settings are saved (see StandaloneSettings) when OK is hit.
*/
class SeqTransportDialog : public SeqModalDialog
{
   std::unique_ptr<Label> mLblMain;
   std::unique_ptr<ToggleButton> mSendClock;
   std::unique_ptr<ToggleButton> mSendTransport;
   std::unique_ptr<TextButton> mBtnOk;
   std::unique_ptr<TextButton> mBtnCancel;

   void endDialog(bool hitOk) override;
   void notify(int cptId, int value) override;
   void resizedInner(Component *inner) override;

public:
   SeqTransportDialog(SeqGlob *glob, CptNotify *parent);
   // load current settings into the controls (call before opening)
   void doSetup();
};

#endif
