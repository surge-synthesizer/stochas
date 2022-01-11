/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "EditDialog.h"

// id of dropdown for adjustment (Selection based) refers to items in this list
// (subtract one to make zero based)
struct Adjustment {
   const char *text;
   int value;   
};

Adjustment gRelVeloValues[] = {
   {"-50%", -50 },
   {"-25%", -25 },
   {"-5%", -5},
   {"+5%", 5},
   {"+25%",25},
   {"+50%",50},
   {0,0}
};

Adjustment gAbsVeloValues[] = {
   { "10",10 },
   { "20",20 },
   { "30",30 },
   { "40",40 },
   { "50",50 },
   { "60",60 },
   { "70",70 },
   { "80",80 },
   { "90",90 },
   { "100",100 },
   { "110",110 },
   { "120",120 },
   {"127",127},   
   { 0,0 }
};

Adjustment gRelProbValues[] = {
   {"Half",50},
   {"Double", 200},
   { 0,0 }
};

Adjustment gAbsProbValues[] = {
   { "0% (Never)",-999 }, // since it doesn't like using an id of 0
   { "25% (Low)",25 },
   { "50% (Med)",50 },
   { "On (High)",100 },
   { 0,0 }
};

#define ADJ_REL_VELO 0
#define ADJ_ABS_VELO 1
#define ADJ_REL_PROB 2
#define ADJ_ABS_PROB 3

Adjustment *gAllAdj[4] = {
   gRelVeloValues,
   gAbsVeloValues,
   gRelProbValues,
   gAbsProbValues
};

// called when the dialog is ending
void SeqEditDialog::endDialog(bool hitOk)
{
   SequenceData *db = mGlob->mSeqBuf->getUISeqData();
   
   // recalcState must have been called before we get here
   if (hitOk) {
      // perform the desired action here
      switch (mStateAction) {
      case Actions::clearLayer:
         db->clearLayer(mTargLayer);         
         break;
      case Actions::clearPattern:
         db->clearPattern(mTargLayer, mTargPat);
         break;
      case Actions::copyLayerData:
         db->copyLayer(mTargLayer, mStateLayer);
         break;
      case Actions::copyNoteData:
         db->copyScaleData(mTargLayer, mStateLayer);
         break;
      case Actions::copyPatternData:
         db->copyPatternData(mTargLayer, mTargPat, mStateLayer, mStatePattern);
         break;
      case Actions::clearSelection:
      case Actions::repeatSelection:
      case Actions::probSelection:
      case Actions::veloSelection:
         doSelectionAction();
         break;

      case Actions::none:
      default:
         break;
      }
      // do it
      mGlob->mSeqBuf->swap();
      notifyParent(0);      
   }   
   
}


#define S_GAP     4 // gap between items
#define S_HEIGHT  22
#define S_BORDER  2  // size of border gap
#define S_TAB     80 // tab size (where labels are involved)
#define S_LISTWIDTH 50
// this is called when the inner box is resized (ie we need to do our resize here)
void SeqEditDialog::resizedInner(Component * inner)
{
   Rectangle<int> r;
   Rectangle<int> bottom, top, btns, tmp, tmp2;

   // inner panel
   r = inner->getLocalBounds();   
   r.reduce(S_BORDER*2, S_BORDER*2); // have a bit of a space all around

                                 // space for description and buttons at bottom
   bottom = r.removeFromBottom(100);

   // space for the label at the top
   top = r.removeFromTop(S_HEIGHT);
   mLblMain->setBounds(top);

   // leave some space between tabs and other
   //r.reduce(S_BORDER, S_GAP);
   r.removeFromTop(S_GAP);
   r.removeFromBottom(S_GAP);

   // remaining area is occupied by tabs
   mTabs->setBounds(r);

   btns = bottom.removeFromBottom(S_HEIGHT);
   // description panel
   bottom.removeFromBottom(S_GAP);
   mLblDescription->setBounds(bottom);

   // buttons
   tmp = btns.removeFromRight(60);
   mBtnCancel->setBounds(tmp);
   btns.removeFromRight(S_GAP);
   tmp = btns.removeFromRight(60);
   mBtnOk->setBounds(tmp);

   // these will be relative to mPanel, mTab1, mTab2

   //
   // tab 1
   //
   r = mTab1->getLocalBounds();
   r.reduce(S_BORDER, S_BORDER);
   // pattern clear radio
   top = r.removeFromTop(S_HEIGHT);
   mPatClear->setBounds(top);
   // copy from radio
   r.removeFromTop(S_GAP);
   top = r.removeFromTop(S_HEIGHT);
   mPatCopyFrom->setBounds(top);

   // layer and pattern list on one line
   r.removeFromTop(S_GAP);
   top = r.removeFromTop(S_HEIGHT);
   tmp = top.removeFromLeft(S_TAB);
   mLblPatLayer->setBounds(tmp);

   tmp = top.removeFromLeft(S_LISTWIDTH);
   mPatLayerList->setBounds(tmp);
   top.removeFromLeft(S_TAB / 2); // spacer
   tmp = top.removeFromLeft(S_TAB);
   mLblPatPattern->setBounds(tmp);
   tmp = top.removeFromLeft(S_LISTWIDTH);
   mPatPatternList->setBounds(tmp);

   //
   // Tab 2
   //
   r = mTab2->getLocalBounds();
   r.reduce(S_BORDER, S_BORDER);

   // layer clear radio
   top = r.removeFromTop(S_HEIGHT);
   mLyrClear->setBounds(top);

   // layer copy from radio
   r.removeFromTop(S_GAP);
   top = r.removeFromTop(S_HEIGHT);
   mLyrCopyFrom->setBounds(top);
   // copy scale radio
   r.removeFromTop(S_GAP);
   top = r.removeFromTop(S_HEIGHT);
   mLyrCopyScale->setBounds(top);
   // layer list
   r.removeFromTop(S_GAP);
   top = r.removeFromTop(S_HEIGHT);
   tmp = top.removeFromLeft(S_TAB);
   mLblLyrLayer->setBounds(tmp);
   top.setWidth(S_LISTWIDTH);
   mLyrLayerList->setBounds(top);

   //
   // Tab 3
   //
   r = mTab3->getLocalBounds();
   r.reduce(S_BORDER, S_BORDER);
   mLblSelText->setBounds(r);
   top = r.removeFromTop(S_HEIGHT);
   mSelClear->setBounds(top);
   r.removeFromTop(S_GAP);
   top = r.removeFromTop(S_HEIGHT);
   mSelRepeat->setBounds(top);
   r.removeFromTop(S_GAP);

   tmp = r.removeFromTop((S_HEIGHT+S_GAP)*2);
   tmp2 = tmp.removeFromRight(250);
   top= tmp.removeFromTop(S_HEIGHT);
   mSelAdjVelo->setBounds(top);
   tmp.removeFromTop(S_GAP);
   top = tmp.removeFromTop(S_HEIGHT);
   mSelAdjProb->setBounds(top);
   tmp.removeFromTop(S_GAP);
   

   tmp2.reduce(0, S_HEIGHT / 2);
   tmp = tmp2.removeFromLeft(115);
   tmp2.removeFromLeft(20);
   tmp2.removeFromRight(10);
   mSelAbsList->setBounds(tmp);
   mSelAmtList->setBounds(tmp2);
   
   
   

}

int SeqEditDialog::getNumActiveSelected()
{
   int count = 0;
   int lowCol, lowRow, hiCol, hiRow, row, step;
   mGlob->mEditorState->getSelectedCells(&lowCol, &hiCol, &lowRow, &hiRow);
   SequenceLayer *lay = mGlob->mSeqBuf->getUISeqData()
         ->getLayer(mGlob->mEditorState->getCurrentLayer());
   for (row = lowRow; row <= hiRow; row++) {
      for (step = lowCol; step <= hiCol; step++) {
         if (lay->getProb(row, step) >= 0)
            count++;
      }
   }

   return count;
}

SeqEditDialog::SeqEditDialog(SeqGlob * glob, CptNotify *parent) :
   SeqModalDialog(glob, SEQCTL_EDITDIALOG,parent, 460,280), mRecalcReady(false)
{
   Colour txtColor, bgColor, hlColor;
   
   bgColor = mGlob->mEditorState->getColorFor(EditorState::background);   
   txtColor = bgColor.contrasting(0.5f);

   // big label at top
   mLblMain = std::unique_ptr<Label>(addStdLabel( "Actions for current layer/pattern")); // will be changed as appropriate
   mLblMain->setFont(Font(20.0f, Font::plain));
   mLblMain->setJustificationType(Justification::centred);
   
   // tabs
   mTabs = std::unique_ptr<SeqTabbedCpt>(new SeqTabbedCpt(0, this, TabbedButtonBar::TabsAtTop));
   addToInner(0, *mTabs);
   mTabs->setVisible(true);
   mTabs->setTabBarDepth(27);
   mTab1 = std::unique_ptr<TabPanelCpt>(new TabPanelCpt(glob, false));
   mTabs->addTab("Pattern", bgColor, mTab1.get(), false);
   mTab2 = std::unique_ptr<TabPanelCpt>(new TabPanelCpt(glob, false));
   mTabs->addTab("Layer", bgColor, mTab2.get(), false);
   mTab3 = std::unique_ptr<TabPanelCpt>(new TabPanelCpt(glob, false));
   mTabs->addTab("Selection", bgColor, mTab3.get(), false);
   mTabs->setCurrentTabIndex(0);
   mTabs->setWantsKeyboardFocus(false);
   

   // all toggles
   mPatClear =    std::unique_ptr<ToggleButton>(addToggle( "Clear all cell data from the current pattern", 1, mTab1.get()));
   mPatCopyFrom = std::unique_ptr<ToggleButton>(addToggle("Copy cell data from another pattern", 1, mTab1.get()));
   mLyrClear =    std::unique_ptr<ToggleButton>(addToggle("Clear all data from the current layer", 2, mTab2.get()));
   mLyrCopyFrom = std::unique_ptr<ToggleButton>(addToggle("Copy all data from another layer", 2, mTab2.get()));
   mLyrCopyScale =std::unique_ptr<ToggleButton>(addToggle("Copy scale information from another layer", 2, mTab2.get()));
   
   // all lists
   mPatLayerList =   std::unique_ptr<ComboBox>(addCombo("Select Layer", mTab1.get()));
   mPatPatternList = std::unique_ptr<ComboBox>(addCombo("Select Pattern",mTab1.get()));
   mLyrLayerList =   std::unique_ptr<ComboBox>(addCombo("Select Layer", mTab2.get()));

   // all labels
   mLblPatLayer =    std::unique_ptr<Label>(addStdLabel("Layer", mTab1.get()));
   mLblPatPattern=   std::unique_ptr<Label>(addStdLabel("Pattern", mTab1.get()));
   mLblLyrLayer=     std::unique_ptr<Label>(addStdLabel("Layer", mTab2.get()));

   // selection
   mSelClear =    std::unique_ptr<ToggleButton>(addToggle("Clear", 3, mTab3.get(), CPTID_SELECTION_ACTION));
   mSelAdjVelo =  std::unique_ptr<ToggleButton>(addToggle("Adjust Velocity",3, mTab3.get(), CPTID_SELECTION_ACTION));
   mSelAdjProb =  std::unique_ptr<ToggleButton>(addToggle("Adjust Probability", 3, mTab3.get(), CPTID_SELECTION_ACTION));
   mSelRepeat =   std::unique_ptr<ToggleButton>(addToggle("Repeat to end of pattern",3,mTab3.get(), CPTID_SELECTION_ACTION));

   // when selection here changes, the amount combo needs to change
   mSelAbsList = std::unique_ptr<ComboBox>(addCombo("Select",mTab3.get(), CPTID_ADJ_ABS_REL));
   mSelAbsList->addItem("Relative", (int)AdjustmentType::relative);
   mSelAbsList->addItem("Absolute", (int)AdjustmentType::absolute);
   

   mSelAmtList = std::unique_ptr<ComboBox>(addCombo("Amount", mTab3.get()));
   mLblSelText = std::unique_ptr<Label>(addStdLabel("No cells are selected. Use shift-drag to select cells.", mTab3.get()));

   // big descr
   mLblDescription = std::unique_ptr<Label>(addStdLabel(""));
   mLblDescription->setJustificationType(Justification::topLeft);
   mLblDescription->setMinimumHorizontalScale(1.0f);

   // buttons
   mBtnOk = std::unique_ptr<TextButton>(addStdButton("Ok",0,CPTID_OK));
   mBtnCancel = std::unique_ptr<TextButton>(addStdButton("Cancel", 0, CPTID_CANCEL));

   mRecalcReady = true;
}

// this is called when any of the added components have changed.
// value may or may not be applicable
void SeqEditDialog::notify(int cptId, int )
{
   switch (cptId) {
   case CPTID_OK:
      recalcState();
      closeDialog(true);
      break;
   case CPTID_CANCEL:
      closeDialog(false);
      break;
   case CPTID_SELECTION_ACTION:
   case CPTID_ADJ_ABS_REL:
      fillSelAmountsList();
      recalcState();
      break;
      
   default:
      recalcState();
      break;
   }

   
}

void SeqEditDialog::fillSelAmountsList()
{
   int list;
   bool velo = false;
   AdjustmentType type;
   mSelAmtList->clear();
   Adjustment *cur;

   if (mSelAdjVelo->getToggleState())
      velo = true;
   else if (mSelAdjProb->getToggleState())
      velo = false;
   else return; // some other toggle which is not applicable

   // this should always have a selection as far as I know
   jassert(mSelAbsList->getSelectedId() != 0);

   type = (AdjustmentType)mSelAbsList->getSelectedId();
   if (type == absolute && velo)
      list = ADJ_ABS_VELO;
   else if (type == absolute)
      list = ADJ_ABS_PROB;
   else if (type == relative && velo)
      list = ADJ_REL_VELO;
   else
      list = ADJ_REL_PROB;

   cur = gAllAdj[list];
   while (cur->text) {
      mSelAmtList->addItem(cur->text, cur->value);
      cur++;
   }
   
   
}

void SeqEditDialog::doSelectionAction()
{
   SequenceData *sd = mGlob->mSeqBuf->getUISeqData();
   SequenceLayer *sl = sd->getLayer(mGlob->mEditorState->getCurrentLayer());
   int startx, endx, starty, endy;
   int x, y;

   mGlob->mEditorState->getSelectedCells(&startx, &endx, &starty, &endy);
   int width = 1 + (endx - startx); // for repeat functionality
   int totsteps = sl->getNumSteps(); // for repeat
   //int8_t defVel = mGlob->mEditorState->getDefaultVelocity();
   double mult=0.0;
   int8_t set=-1;     // if non-neg we are setting an absolute value to this
   int amt = mAdjAmount;
   if (amt == -999) // doesn't like using 0 as an item id
      amt = 0; 
   if (mAdjType == relative)
      mult = amt / 100.0;
   else
      set = (int8_t)amt;

   
   for (x = startx; x <= endx; x++) {
      for (y = starty; y <= endy; y++) {
         switch (mStateAction) {
         case Actions::clearSelection:
            // duplication exists. we also clear cells in plugineditor in response to del key
            sl->clearCell(y, x);
            break;
         case Actions::repeatSelection: {
            int t = x+width;
            while (t < totsteps) {
               sl->copyCell(y, t, y, x);
               t += width;
            }
            break;
         }
         case Actions::probSelection:
         { // set probability
            int8_t old = sl->getProb(y, x);
            if (set >-1) { // if absolute
               // note that this only changes the value of cells that are already on
               // if this is not desired, you will need to have the commented logic below
               // that ensures velocity has a value
               if (old != SEQ_PROB_OFF) {
                  sl->setProb(y, x, set);
               }
               //if (!old)
               //   sl->setVel(y, x, defVel);
            } else { //relative - multiply current by mult
               if (old != SEQ_PROB_OFF) {
                  int v=(int)((double)old * mult);
                  if (v > SEQ_PROB_ON)
                     v = SEQ_PROB_ON;
                  sl->setProb(y, x, (int8_t)v);
               }
            }
               break;
         }
         case Actions::veloSelection:
            if (sl->getProb(y, x) != SEQ_PROB_OFF) {
               if (set>-1) // absolute
                  sl->setVel(y, x, set);
               else { // relative - get current and multiply
                  int c = sl->getVel(y, x);
                  int v = (int)((double)c * mult);
                  c += v; // adjust that value
                  if (c > 127)
                     c = 127;
                  sl->setVel(y, x, (int8_t)c);
               }
            }
            break;
         default:
            jassertfalse;
         }
      }
   }
   // swap will be called by callee
}

void SeqEditDialog::recalcState()
{

   int cl=mGlob->mEditorState->getCurrentLayer();
   int cp=mGlob->mSeqBuf->getUISeqData()->getLayer(cl)->getCurrentPattern();
   mTargLayer = cl;
   mTargPat = cp;
   
   int tabidx;
   String txt;
   const char *velprob = 0;
   bool selValid = mGlob->mEditorState->getSelectedCells();

   if (!mRecalcReady)
      return; // still setting up

   int numSelected = getNumActiveSelected();

   mLblPatLayer->setVisible(false);
   mLblPatPattern->setVisible(false);
   mLblLyrLayer->setVisible(false);
   mPatLayerList->setVisible(false);
   mPatPatternList->setVisible(false);
   mLyrLayerList->setVisible(false);


   mLblSelText->setVisible(!selValid);
   mSelClear->setVisible(selValid);
   mSelRepeat->setVisible(selValid);
   mSelAdjProb->setVisible(selValid);
   mSelAdjVelo->setVisible(selValid);
   mSelAbsList->setVisible(false);
   mSelAmtList->setVisible(false);


   tabidx = mTabs->getCurrentTabIndex();
   switch (tabidx) {
   case 0: // Pattern operations
      if (mPatClear->getToggleState()) {
         mStateAction = Actions::clearPattern;
         mStatePattern = mPatPatternList->getSelectedId() - 1;
      }
      else if (mPatCopyFrom->getToggleState()) {
         mStateAction = Actions::copyPatternData;
         mPatLayerList->setVisible(true);
         mPatPatternList->setVisible(true);
         mLblPatLayer->setVisible(true);
         mLblPatPattern->setVisible(true);
         mStateLayer = mPatLayerList->getSelectedId() - 1;
         mStatePattern = mPatPatternList->getSelectedId() - 1;
         if (mStateLayer == cl && mStatePattern == cp)
            mStateAction = Actions::none; // can't copy over itself
      }
      else
         mStateAction=Actions::none;
      
      break;
   case 1: // Layer operations
      
      if (mLyrClear->getToggleState()) {
         mStateAction = Actions::clearLayer;
      }
      else if (mLyrCopyFrom->getToggleState()) {
         mStateAction = Actions::copyLayerData;
         mLyrLayerList->setVisible(true);
         mLblLyrLayer->setVisible(true);
      }
      else if (mLyrCopyScale->getToggleState()) {
         mStateAction = Actions::copyNoteData;
         mLyrLayerList->setVisible(true);
         mLblLyrLayer->setVisible(true);
      } 
      else
         mStateAction = Actions::none;      

      mStateLayer = mLyrLayerList->getSelectedId() - 1;
      mStatePattern = - 1;
      if (mStateAction != Actions::none && cl == mStateLayer)
         mStateAction = Actions::none;

      break;
   case 2: // selection tab operations

      mAdjType = (AdjustmentType)mSelAbsList->getSelectedId();
      mAdjAmount = mSelAmtList->getSelectedId();      
      mStateAction = Actions::none;

      if (numSelected) {
         if (mSelClear->getToggleState()) {
            mStateAction = clearSelection;
         }
         else if (mSelRepeat->getToggleState()) {
            mStateAction = repeatSelection;
         }
         else if (mSelAdjVelo->getToggleState()) {
            if (mAdjAmount)
               mStateAction = veloSelection;
            mSelAbsList->setVisible(true);
            mSelAmtList->setVisible(true);

         }
         else if (mSelAdjProb->getToggleState()) {
            if (mAdjAmount)
               mStateAction = probSelection;
            mSelAbsList->setVisible(true);
            mSelAmtList->setVisible(true);

         }
      }
      break;
   default:
      mStateAction = Actions::none;
   }

   // update description
   switch (mStateAction) {
   case copyPatternData:
      txt = String::formatted("ALL cell data will be copied from layer %d pattern %d to layer %d pattern %d",
         mStateLayer + 1, mStatePattern + 1, cl+1,cp+1);
      break;
   case clearPattern:
      txt = String::formatted("ALL cells in pattern %d will be cleared", cp+1);
      break;
   case clearLayer:
      txt = String::formatted("ALL data from layer %d will be cleared. "
         "The layer will be reset to it's default state, including notes, and options", cl+1);
      break;
   case copyLayerData:
      txt = String::formatted("ALL data from layer %d will be copied to layer %d. "
         "This includes all pattern data, notes, and layer options",
         mStateLayer+1, cl+1);
      break;
   case copyNoteData:
      txt = String::formatted("Scale data will be copied from layer %d to layer %d. "
         "Scale setting will be set to match. If custom notes are used, they will be copied",
         mStateLayer+1, cl+1);
      break;
   case clearSelection:
      txt = String::formatted("%d cells will be cleared", numSelected);
      break;
   case repeatSelection:
      txt = String::formatted("%d cells will be repeated until the end of the pattern", numSelected);
         
      break;
   case veloSelection:
      velprob = "velocity";
   case probSelection: {
      const char *absrel;
      if (!velprob)
         velprob = "probability";
   
      if (mAdjType == relative)
         absrel = "adjusted by ";
      else
         absrel = "set to ";

      txt << "The " << velprob << " of " << numSelected << 
         " cells will be " << absrel << mSelAmtList->getText();
      break;

   }
   default:
      txt = "No action will be taken. Please select a valid set of options.";
   }
   mLblDescription->setText(txt, NotificationType::dontSendNotification);
   
}

void SeqEditDialog::doSetup()
{
   // this does initial setup of the dialog whenever the user accesses it.

   int i;
   int curlyr = mGlob->mEditorState->getCurrentLayer();
   int curpat = mGlob->mSeqBuf->getUISeqData()->getLayer(curlyr)->getCurrentPattern();

   mPatLayerList->clear();
   mLyrLayerList->clear();
   for (i = 0; i < SEQ_MAX_LAYERS; i++) {
      mLyrLayerList->addItem(String::formatted("%d", i + 1), i + 1);
      mPatLayerList->addItem(String::formatted("%d", i + 1), i + 1);
   }
   // this one defaults to this layer, or the last layer if there is one before
   mLyrLayerList->setSelectedItemIndex(curlyr ? curlyr-1 : curlyr+1);
   mPatLayerList->setSelectedItemIndex(curlyr);

   mPatPatternList->clear();
   for (i = 0; i < SEQ_MAX_PATTERNS; i++) {
      mPatPatternList->addItem(String::formatted("%d", i + 1), i + 1);
   }
   // default to previous pattern (or next pattern if we are on 0)
   mPatPatternList->setSelectedItemIndex(curpat ? curpat - 1 : curpat+1);

   mSelClear->setToggleState(true, NotificationType::dontSendNotification);

   mPatClear->setToggleState(true, NotificationType::dontSendNotification);
   mLyrClear->setToggleState(true, NotificationType::dontSendNotification);



   if(mGlob->mEditorState->getNumSelectedCells() > 1)
      mTabs->setCurrentTabIndex(2);
   else
      mTabs->setCurrentTabIndex(0);

   // clear selection on this
   mSelAmtList->setSelectedId(0);
   mSelAbsList->setSelectedId((int)AdjustmentType::relative);

   recalcState();
}

SeqEditDialog::~SeqEditDialog()
{
}
