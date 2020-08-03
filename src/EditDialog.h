/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef EDITDIALOG_H_
#define EDITDIALOG_H_

#include "EditorState.h"
#include "Constants.h"
#include "CommonComponents.h"

#define CPTID_OK 10
#define CPTID_CANCEL 11
#define CPTID_ADJ_ABS_REL 12
#define CPTID_SELECTION_ACTION 13

/*
Panel that is displayed when Edit is clicked
The reason for pointers here is that I used the UI-builder in Projucer for some of this
*/
class SeqEditDialog : public SeqModalDialog
{
   // all pat components are on here
   std::unique_ptr<TabPanelCpt> mTab1;
   // all layer components are on here
   std::unique_ptr<TabPanelCpt> mTab2;
   // selection actions are here
   std::unique_ptr<TabPanelCpt> mTab3;

   // above two are on here
   std::unique_ptr<SeqTabbedCpt> mTabs;

   std::unique_ptr<Label> mLblMain;
   std::unique_ptr<Label> mLblPatLayer;
   std::unique_ptr<Label> mLblPatPattern;
   std::unique_ptr<Label> mLblLyrLayer;
   std::unique_ptr<Label> mLblDescription;

   std::unique_ptr<Label> mLblSelText;

   
   std::unique_ptr<ToggleButton> mPatClear;
   std::unique_ptr<ToggleButton> mPatCopyFrom;
   std::unique_ptr<ToggleButton> mLyrClear;
   std::unique_ptr<ToggleButton> mLyrCopyFrom;
   std::unique_ptr<ToggleButton> mLyrCopyScale;

   std::unique_ptr<ToggleButton> mSelClear;    // clear 
   std::unique_ptr<ToggleButton> mSelAdjVelo;  // adjust velocity
   std::unique_ptr<ToggleButton> mSelAdjProb;  // adjust probability
   std::unique_ptr<ToggleButton> mSelRepeat; // repeat till end of pattern

   std::unique_ptr<ComboBox> mSelAbsList;     // velo/prob - abs/relative
   std::unique_ptr<ComboBox> mSelAmtList;     // amount. for abs will be some fixed values, rel will be up/down

   std::unique_ptr<ComboBox> mPatLayerList;
   std::unique_ptr<ComboBox> mPatPatternList;
   std::unique_ptr<ComboBox> mLyrLayerList;

   std::unique_ptr<TextButton> mBtnOk;
   std::unique_ptr<TextButton> mBtnCancel;

   void endDialog(bool hitOk) override;
   void notify(int cptId, int value) override;
   void resizedInner(Component *inner) override;
   int getNumActiveSelected();
   bool mRecalcReady; // a hack
   enum Actions {
      none,
      clearPattern,
      copyPatternData,
      clearLayer,
      copyLayerData,
      copyNoteData,
      clearSelection,
      repeatSelection,
      probSelection,
      veloSelection
   };
   // what action we want
   Actions mStateAction;
   // what layer to operate on
   int mStateLayer;
   // what pattern to operate on
   int mStatePattern;
   // targets
   int mTargLayer;
   int mTargPat;

   enum AdjustmentType {
      invalid,
      absolute,            // a specific value
      relative             // a percentage
   };
   AdjustmentType mAdjType;
   int mAdjAmount;         // will be either a number or a percentage (or 0 if nothing selected)

   void fillSelAmountsList();
   void doSelectionAction();
   
public:
   // this will update things depending on what the user is doing
   void recalcState();
   // this will set up the dialog according to the most likely action the user wants
   void doSetup();
   
   ~SeqEditDialog();
   SeqEditDialog(SeqGlob *glob, CptNotify *parent);
   
};

#endif
