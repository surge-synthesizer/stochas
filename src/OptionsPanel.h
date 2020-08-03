/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef OPTIONSPANEL_H_
#define OPTIONSPANEL_H_

#include "EditorState.h"
#include "Constants.h"
#include "CommonComponents.h"


/**
Layer specific options. Now on a tab (previously was in the main area)
*/
class OptionsPanel : public Component, public CptNotify, public ComboBox::Listener {
   SeqGlob *mGlob;
   ToggleCpt mMonoPoly;
   ToggleCpt mClockDiv;
   ToggleCpt mScaleStandCust;
   ToggleCpt mMuted;
   ToggleCpt mCombineOverlapsToggle;
   NumberCpt mPatLength;
   NumberCpt mDutyCycle;
   NumberCpt mMidiChannel;
   NumberCpt mStepsPerMeasure;   // default 16
   NumberCpt mNumRows;           // number of visible (and playable) rows
   NumberCpt mMaxPoly;
   NumberCpt mHumanizeVelo;
   NumberCpt mHumanizePos;
   NumberCpt mHumanizeLength;
   NumberCpt mPolyBias;

   




   Label mLblMonoPoly;
   Label mLblClockDiv;
   Label mLblScale;
   Label mLblPatLength;
   Label mLblDutyCycle;
   Label mLblMidiChannel;
   Label mLblStepsPerMeasure;
   Label mLblNumRows;
   Label mLblMaxPoly;
   Label mLblHumanVelo;
   Label mLblHumanPos;
   Label mLblHumanLength;
   Label mLblPolyBias;
   Label mLblCombineOverlaps;

   // these are shown in standard mode
   ComboBox mScaleList;
   ComboBox mKeyList;
   ComboBox mOctaveList;
   Label mLblKeyScaleOct;

   // these are shown in custom mode
   ButtonCpt mBtnLoadCustom;
   ButtonCpt mBtnSaveCustom;
   ButtonCpt mBtnStdCustom; // copy std to custom
   
   ButtonCpt mBtnMuteAll;


   // when we need to notify parent to repaint, etc.
   CptNotify *mParent;

   // setup colors etc
   void comboSetup(ComboBox *cb);
   // set selected item to one with text specified (ie find that item)
   void findItemInCombo(ComboBox *cb, String txt);
   void resized() override;
   void paint(Graphics &g) override;
   // called when one of the toggles changes
   void cptValueChange(int cptId, int id) override;
   void comboBoxChanged(ComboBox *comboBoxThatHasChanged) override;
   void fillOctaveList();
   void setupLabel(Label &lbl, const String &txt);
   bool allLayersAreMuted();
public:
   OptionsPanel(SeqGlob *glob, CptNotify *parent);

   // call this to re-read settings from sequence data (does not issue repaint)
   void refreshAll();
   
   
};

#endif
