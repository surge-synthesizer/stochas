#ifndef SETTINGS_TAB_
#define SETTINGS_TAB_
#include <JuceHeader.h>
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

   NumberCpt mNumMouseSense;
   ToggleCpt mTglRightMouseAction;
   ToggleCpt mTglLowestMidiOctave;
   NumberCpt mNumDefaultMonoProb;
   NumberCpt mNumDefaultPolyProb;
   NumberCpt mNumDefaultVelo;
   ToggleCpt mTglColorScheme;
   ToggleCpt mTglShiftReversed;
   NumberCpt mNumPosOffset;

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

