/* Copyright (C) 2020 Andrew Shakinovsky
*/
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
   std::unique_ptr<Label> mLblMain;
   std::unique_ptr<Label> mLblDescription;
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
