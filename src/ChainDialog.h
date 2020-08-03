/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef CHAINDIALOG_H_
#define CHAINDIALOG_H_

#include "EditorState.h"
#include "Constants.h"
#include "CommonComponents.h"

#define CPTID_OK        10
#define CPTID_SRC       11
#define CPTID_TARG      12

/*
Panel that is displayed when Ctrl drag for chain
*/
class SeqChainDialog : public SeqModalDialog
{
   int mTgtRow;
   int mTgtStep;
   int mSrcRow;
   int mSrcStep;

   // all pat components are on here
   std::unique_ptr<TabPanelCpt> mTab1;
   // all layer components are on here
   std::unique_ptr<TabPanelCpt> mTab2;
   // selection actions are here
   std::unique_ptr<TabPanelCpt> mTab3;

   // above two are on here
   std::unique_ptr<SeqTabbedCpt> mTabs;
   std::unique_ptr<Label> mLblDescription;
   std::unique_ptr<Label> mLblSource;
   std::unique_ptr<Label> mLblTarget;
   std::unique_ptr<ToggleCpt> mSource;
   std::unique_ptr<ToggleCpt> mTarget;
   std::unique_ptr<TextButton> mBtnOk;
   
   void endDialog(bool hitOk) override;
   void notify(int cptId, int value) override;
   void resizedInner(Component *inner) override;
   
   void setDesc();
public:
   // this will set up the dialog according to the most likely action the user wants
   void doSetup(int srcRow, int srcCol, int destRow, int destCol);
   
   ~SeqChainDialog();
   SeqChainDialog(SeqGlob *glob, CptNotify *parent);
   
};

#endif
