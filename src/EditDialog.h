/* Copyright (C) 2016 Andrew Shakinovsky
*/
#ifndef EDITDIALOG_H_
#define EDITDIALOG_H_

#include <JuceHeader.h>
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
   ScopedPointer<TabPanelCpt> mTab1;
   // all layer components are on here
   ScopedPointer<TabPanelCpt> mTab2;
   // selection actions are here
   ScopedPointer<TabPanelCpt> mTab3;

   // above two are on here
   ScopedPointer<SeqTabbedCpt> mTabs;

   ScopedPointer<Label> mLblMain;
   ScopedPointer<Label> mLblPatLayer;
   ScopedPointer<Label> mLblPatPattern;
   ScopedPointer<Label> mLblLyrLayer;
   ScopedPointer<Label> mLblDescription;

   ScopedPointer<Label> mLblSelText;

   
   ScopedPointer<ToggleButton> mPatClear;
   ScopedPointer<ToggleButton> mPatCopyFrom;
   ScopedPointer<ToggleButton> mLyrClear;
   ScopedPointer<ToggleButton> mLyrCopyFrom;
   ScopedPointer<ToggleButton> mLyrCopyScale;

   ScopedPointer<ToggleButton> mSelClear;    // clear 
   ScopedPointer<ToggleButton> mSelAdjVelo;  // adjust velocity
   ScopedPointer<ToggleButton> mSelAdjProb;  // adjust probability
   ScopedPointer<ToggleButton> mSelRepeat; // repeat till end of pattern

   ScopedPointer<ComboBox> mSelAbsList;     // velo/prob - abs/relative
   ScopedPointer<ComboBox> mSelAmtList;     // amount. for abs will be some fixed values, rel will be up/down

   ScopedPointer<ComboBox> mPatLayerList;
   ScopedPointer<ComboBox> mPatPatternList;
   ScopedPointer<ComboBox> mLyrLayerList;

   ScopedPointer<TextButton> mBtnOk;
   ScopedPointer<TextButton> mBtnCancel;

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
