/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/
#include "SettingsTab.h"



void SettingsTab::paint(Graphics &g)
{
   gradientFill(*mGlob->mEditorState, this, g);
   
}

void SettingsTab::resized()
{
   const int vspace = SEQ_SIZE_PROP_HEIGHT;
   const int tab = 120;
   const int space = SEQ_SIZE_PROP_VSPACE;
   const int vgap = SEQ_SIZE_PROP_VSPACE;

   // left region
   Rectangle<int> b1 = getLocalBounds();  // col 1

   // far right region
   Rectangle<int> b3 = b1.removeFromRight(b1.getWidth()/3); // col 3

   // middle region
   Rectangle<int> b2 = b1.removeFromRight(b1.getWidth()/2); // col 2
   Rectangle<int> left, top;

   b1.reduce(space, vgap);
   b2.reduce(space, vgap);
   b3.reduce(space, vgap);

   // mouse sense
   top = b1.removeFromTop(vspace);
   left = top.removeFromLeft(tab);
   mLblMouseSense.setBounds(left);
   top.removeFromLeft(space);
   mNumMouseSense.setBounds(top);
   b1.removeFromTop(vgap);

   // right click
   top = b1.removeFromTop(vspace);
   left = top.removeFromLeft(tab);
   mLblRightMouseAction.setBounds(left);
   top.removeFromLeft(space);
   mTglRightMouseAction.setBounds(top);
   b1.removeFromTop(vgap);

   // default prob mono
   top = b1.removeFromTop(vspace);
   left = top.removeFromLeft(tab);
   mLblDefaultMonoProb.setBounds(left);
   top.removeFromLeft(space);
   mNumDefaultMonoProb.setBounds(top);
   b1.removeFromTop(vgap);

   // default prob poly
   top = b1.removeFromTop(vspace);
   left = top.removeFromLeft(tab);
   mLblDefaultPolyProb.setBounds(left);
   top.removeFromLeft(space);
   mNumDefaultPolyProb.setBounds(top);
   b1.removeFromTop(vgap);
   
   // def velo
   top = b2.removeFromTop(vspace);
   left = top.removeFromLeft(tab);
   mLblDefaultVelo.setBounds(left);
   top.removeFromLeft(space);
   mNumDefaultVelo.setBounds(top);
   b2.removeFromTop(vgap);

   // lo oct
   top = b2.removeFromTop(vspace);
   left = top.removeFromLeft(tab);
   mLblLowestMidiOctave.setBounds(left);
   top.removeFromLeft(space);
   mTglLowestMidiOctave.setBounds(top);
   b2.removeFromTop(vgap);

   // color scheme
   top = b2.removeFromTop(vspace);
   left = top.removeFromLeft(tab);
   mLblColorScheme.setBounds(left);
   top.removeFromLeft(space);
   mTglColorScheme.setBounds(top);
   b2.removeFromTop(vgap);

   // shift beh
   top = b2.removeFromTop(vspace);
   left = top.removeFromLeft(tab);
   mLblShiftReversed.setBounds(left);
   top.removeFromLeft(space);
   mTglShiftReversed.setBounds(top);
   b2.removeFromTop(vgap);

   // pos offset
   top = b3.removeFromTop(vspace);
   left = top.removeFromLeft(tab);
   mLblPosOffset.setBounds(left);
   top.removeFromLeft(space);
   mNumPosOffset.setBounds(top);

   // ui scaling
   top = b3.removeFromTop(vspace);
   left = top.removeFromLeft(tab);
   mLblUIScale.setBounds(left);
   top.removeFromLeft(space);
   mNumUIScale.setBounds(top);

   // version
   top = b3.removeFromBottom(vspace);
   mLblVersionBuild.setBounds(top);

}

void SettingsTab::setupLabel(Label & lbl, const String & txt)
{
   Colour c=mGlob->mEditorState->getColorFor(EditorState::background).contrasting(.5f);
   addAndMakeVisible(lbl);
   lbl.setText(txt, juce::NotificationType::dontSendNotification);
   lbl.setColour(Label::ColourIds::textColourId, c);
   //lbl.setFont(Font(15.00f, Font::plain));
   lbl.setJustificationType(Justification::centredLeft);
   lbl.setEditable(false, false, false);
}

SettingsTab::SettingsTab(SeqGlob * glob, int id, CptNotify *notify) :
   Component("settingsTab"),
   mGlob(glob),
   mNotify(notify),
   mId(id),
   mNumMouseSense(glob,SEQCTL_SET_MOUSESENSE,this,"setMouseSense"),
   mTglRightMouseAction(glob, SEQCTL_SET_RTMOUSE,this,"setRtMouse"),
   mTglLowestMidiOctave(glob,SEQCTL_SET_OCTAVE,this,"setOctave"),
   mNumDefaultMonoProb(glob,SEQCTL_SET_DEFMONO,this,"setDefMono"),
   mNumDefaultPolyProb(glob,SEQCTL_SET_DEFPOLY,this,"setDefPoly"),
   mNumDefaultVelo(glob,SEQCTL_SET_DEFVELO,this,"setDefVelo"),
   mTglColorScheme(glob,SEQCTL_SET_COLOR,this,"setColor"),
   mTglShiftReversed(glob, SEQCTL_SET_SHIFTREV,this,"shiftRev"),
   mNumPosOffset(glob, SEQCTL_SET_POSOFFSET, this, "setPosOffset"),
   mNumUIScale(glob, SEQCTL_SET_UISCALE, this, "setUIScale")
{
   setupLabel(mLblMouseSense,"Mouse Sensitivity");
   setupLabel(mLblRightMouseAction,"Right Click");
   setupLabel(mLblLowestMidiOctave,"Lowest Octave");
   setupLabel(mLblDefaultMonoProb,"Default Mono");
   setupLabel(mLblDefaultPolyProb,"Default Poly");
   setupLabel(mLblDefaultVelo,"Default Velocity");
   setupLabel(mLblColorScheme,"Color Scheme");
   setupLabel(mLblShiftReversed, "Shift Key");
   setupLabel(mLblPosOffset, "Pos. Offset.");
   setupLabel(mLblUIScale, "UI Scale");
   String vs = String("Version: ");
   vs += Stochas::Build::FullVersionStr;
   setupLabel(mLblVersionBuild, vs);

   mNumMouseSense.setSpec(1,SEQ_MOUSE_SENSE_MAX, 1, SEQ_MOUSE_SENSE_DEFAULT,"");
   addAndMakeVisible(mNumMouseSense);

   mTglShiftReversed.addItem(0, "Normal", true);
   mTglShiftReversed.addItem(1, "Reversed", false);
   addAndMakeVisible(mTglShiftReversed);

   mTglRightMouseAction.addItem((int)EditorState::deleteCell, "Delete", true);
   mTglRightMouseAction.addItem((int)EditorState::cycleDown, "Cycle Down", false);
   addAndMakeVisible(mTglRightMouseAction);

   // lowest octave can be in the range -2 to 0
   mTglLowestMidiOctave.addItem(-2, "-2", false);
   mTglLowestMidiOctave.addItem(-1, "-1", true);
   mTglLowestMidiOctave.addItem(0, "0", false);
   addAndMakeVisible(mTglLowestMidiOctave);

   
   mNumDefaultMonoProb.setSpec(0,3,1,0,"");
   mNumDefaultMonoProb.setStringRep(0, SEQ_PROB_NEVER_TEXT);
   mNumDefaultMonoProb.setStringRep(1, SEQ_PROB_LOW_TEXT);
   mNumDefaultMonoProb.setStringRep(2, SEQ_PROB_MED_TEXT);
   mNumDefaultMonoProb.setStringRep(3, SEQ_PROB_HIGH_TEXT);
   

   addAndMakeVisible(mNumDefaultMonoProb);

   mNumDefaultPolyProb.setSpec(SEQ_PROB_NEVER, SEQ_PROB_ON, 1, 0, "%");
   mNumDefaultPolyProb.setStringRep(SEQ_PROB_ON, SEQ_PROB_ON_TEXT);
   addAndMakeVisible(mNumDefaultPolyProb);

   mNumDefaultVelo.setSpec(0, 127, 1, 0, "");
   addAndMakeVisible(mNumDefaultVelo);

   mTglColorScheme.addItem(0, "Dark", true);
   mTglColorScheme.addItem(1, "Light",false);
   mTglColorScheme.addItem(2, "Custom", false);
   addAndMakeVisible(mTglColorScheme);

   mNumPosOffset.setSpec(SEQ_POS_OFFSET_MIN, SEQ_POS_OFFSET_MAX, 1, 0, "");
   addAndMakeVisible(mNumPosOffset);

   mNumUIScale.setSpec(SEQ_UI_SCALE_MIN, SEQ_UI_SCALE_MAX, 1, 0, "");
   addAndMakeVisible(mNumUIScale);
   
}

// called when the tab becomes visible (user clicked on the tab)
// read all settings in
void SettingsTab::refreshAll()
{
   EditorState &em = *mGlob->mEditorState;
   int x;

   mNumPosOffset.setValue(em.getPPQOffset(), false);

   mNumUIScale.setValue(em.getScaleFactor(),false);

   x = em.getMouseSense(); // from 1 to 10 where 1 is highest sensitivity - so reverse
   mNumMouseSense.setValue(SEQ_MOUSE_SENSE_MAX-(x-1), false);

   if (em.isShiftReversed())
      mTglShiftReversed.setCurrentItem(1, true, false);
   else
      mTglShiftReversed.setCurrentItem(0, true, false);

   // mono
   x = em.getDefaultProbability(true);
   if(x <= SEQ_PROB_NEVER)
      mNumDefaultMonoProb.setValue(0, false);
   else if (x <= SEQ_PROB_LOW_VAL)
      mNumDefaultMonoProb.setValue(1, false);
   else if (x <= SEQ_PROB_MED_VAL)
      mNumDefaultMonoProb.setValue(2, false);
   else 
      mNumDefaultMonoProb.setValue(3, false);
   
   // poly
   mNumDefaultPolyProb.setValue(em.getDefaultProbability(false), false);
   mNumDefaultVelo.setValue(em.getDefaultVelocity(), false);

   x = em.getLowestOctave();
   mTglLowestMidiOctave.setCurrentItem(x, true, false);

   mTglRightMouseAction.setCurrentItem((int)em.getMouseRightClickAction(), true, false);
   mTglColorScheme.setCurrentItem(em.getColorTheme(), true, false);
   

}

// called when user changes a value
void SettingsTab::cptValueChange(int cptId, int value)
{
   EditorState &em = *mGlob->mEditorState;
   switch (cptId) {
   case SEQCTL_SET_MOUSESENSE:
      em.setMouseSense(SEQ_MOUSE_SENSE_MAX - (value - 1));
      break;
   case SEQCTL_SET_DEFMONO:
   {
      char x=0;
      switch (value) {
      case 0: x = SEQ_PROB_NEVER; break;
      case 1: x = SEQ_PROB_LOW_VAL; break;
      case 2: x = SEQ_PROB_MED_VAL; break;
      case 3: default: x = SEQ_PROB_HIGH_VAL; break;      
      }
      em.setDefaultProbability(x, true);
      break;
   }
   case SEQCTL_SET_DEFPOLY:
      em.setDefaultProbability((char)value, false);
      break;
   case SEQCTL_SET_DEFVELO:
      em.setDefaultVelocity((char)value);
      break;
   case SEQCTL_SET_OCTAVE:
      em.setLowestOctave(value);
      break;
   case SEQCTL_SET_RTMOUSE:
      em.setMouseRightClickAction((EditorState::mouseRightClickAction)value);
      break;
   case SEQCTL_SET_COLOR:
      em.setColorTheme(value);
      break;
   case SEQCTL_SET_SHIFTREV:
      em.setShiftReversed(value == 1);
      break;
   case SEQCTL_SET_POSOFFSET:
      em.setPPQOffset(value);
      break;
   case SEQCTL_SET_UISCALE:
      em.setScaleFactor(value);
      break;
   default:
      jassertfalse;
      break;
   }
   if (mNotify)
      mNotify->cptValueChange(mId, cptId);
}
