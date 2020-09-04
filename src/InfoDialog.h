/***************************************************************
 ** Copyright (C) 2020 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef INFODIALOG_H_
#define INFODIALOG_H_

#include "Constants.h"
#include "CommonComponents.h"

#define CPTID_OK 10
#define CPTID_HELP_LAUNCH 11
#define CPTID_WESITE_LAUNCH 12
/*
Panel that is displayed when Info is clicked
*/
class SeqInfoDialog : public SeqModalDialog
{
   ImageComponent mLogo;
   std::unique_ptr<Label> mLblMain;
   std::unique_ptr<TextEditor> mLblDescription;
   std::unique_ptr<TextButton> mBtnOk;
   std::unique_ptr<TextButton> mBtnHelp;
   std::unique_ptr<TextButton> mBtnWebsite;
  
  
   void endDialog(bool hitOk) override;
   void notify(int cptId, int value) override;
   void resizedInner(Component *inner) override;
   
public:
   
   ~SeqInfoDialog();
   SeqInfoDialog(SeqGlob *glob, CptNotify *parent);
   
};

#endif
