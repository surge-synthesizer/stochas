/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/
#ifndef SETTINGS_TAB_
#define SETTINGS_TAB_

#include "EditorState.h"
#include "Constants.h"
#include "CommonComponents.h"

class SettingsTab : public Component, public CptNotify {
   SeqGlob *mGlob;
   CptNotify *mNotify;
   int mId;
   /*
   default right mouse action
   mouse sense
   lowest midi octave number
   default cell prob for mono/poly
   default cell velocity
   color scheme (dark/light)
   */
   Label mLblMouseSense;
   Label mLblRightMouseAction;
   Label mLblLowestMidiOctave;
   Label mLblDefaultMonoProb;
   Label mLblDefaultPolyProb;
   Label mLblDefaultVelo;
   Label mLblColorScheme;
   Label mLblShiftReversed;
   Label mLblVersionBuild;
   Label mLblPosOffset;
   Label mLblUIScale;

   NumberCpt mNumMouseSense;
   ToggleCpt mTglRightMouseAction;
   ToggleCpt mTglLowestMidiOctave;
   NumberCpt mNumDefaultMonoProb;
   NumberCpt mNumDefaultPolyProb;
   NumberCpt mNumDefaultVelo;
   ToggleCpt mTglColorScheme;
   ToggleCpt mTglShiftReversed;
   NumberCpt mNumPosOffset;
   NumberCpt mNumUIScale;

   void cptValueChange(int cptId, int value) override;
   void paint(Graphics &) override;
   void resized() override;
   void setupLabel(Label &lbl, const String &txt);
public:
   SettingsTab(SeqGlob *glob,int id, CptNotify *notify);

   // read settings from edit state. call before tab becomes visible
   void refreshAll();

};

#endif

