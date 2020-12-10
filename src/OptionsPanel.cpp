/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "OptionsPanel.h"
#include "Scale.h"

OptionsPanel::OptionsPanel(SeqGlob *glob, CptNotify *parent) :
   Component("optionsPanel"), mGlob(glob),
   mMonoPoly(glob, SEQCTL_OPT_MP_TOGGLE, this, "tgMP"),
   mClockDiv(glob, SEQCTL_OPT_CLOCKDIV_TOGGLE, this, "tgClkDiv"),
   mScaleStandCust(glob, SEQCTL_OPT_SCALE_TOGGLE, this, "tgScale"),
   mMuted(glob, SEQCTL_OPT_MUTE_TOGGLE, this,"tgMuted"),
   mBtnMuteAll(glob, SEQCTL_OPT_MUTE_ALL, this, "btnMuteAll"),
   mCombineOverlapsToggle(glob, SEQCTL_COMBINE_TOGGLE, this, "combineToggle"),
   mPatLength(glob, SEQCTL_OPT_PATLEN_NUMCTL, this, "tgPL"),
   mDutyCycle(glob, SEQCTL_OPT_DUTYCY_NUMCTL, this, "tgDC"),
   mMidiChannel(glob, SEQCTL_OPT_MIDICH_NUMCTL, this, "tgMIDI"),
   mStepsPerMeasure(glob, SEQCTL_OPT_STEPSPERM_NUMCTL, this, "tgSPM"),
   mNumRows(glob, SEQCTL_OPT_NUMROWS_NUMCTL, this, "tgRows"),
   mMaxPoly(glob, SEQCTL_OPT_MAX_POLY, this, "numMaxPoly"),
   mHumanizeVelo(glob,SEQCTL_OPT_HUMAN_VELO,this, "numHumanVelo"),
   mHumanizePos(glob, SEQCTL_OPT_HUMAN_POS, this, "numHumanPos"),
   mHumanizeLength(glob, SEQCTL_OPT_HUMAN_LENGTH, this, "numHumanLength"),
   
   mPolyBias(glob, SEQCTL_OPT_POLYBIAS, this, "numPolyBias"),
   mScaleList("cbbScale"),
   mKeyList("cbbKey"),
   mOctaveList("cbbOct"),
   mBtnLoadCustom(glob, SEQCTL_OPT_LOAD_CUSTOM, this, "btnLdCust"),
   mBtnSaveCustom(glob, SEQCTL_OPT_SAVE_CUSTOM, this, "btnSvCust"),
   mBtnStdCustom(glob, SEQCTL_OPT_STD_TO_CUSTOM, this, "btnStdCust"),
   mParent(parent)
{
   int i=0, len=0;
   juce::Colour txtclr =
      mGlob->mEditorState->getColorFor(EditorState::background).contrasting(.5);

   // All labels
   setupLabel(mLblKeyScaleOct, "");
   setupLabel(mLblHumanPos, "Position Variance");
   setupLabel(mLblHumanLength, "Length Variance");
   setupLabel(mLblHumanVelo, "Velocity Variance");
   setupLabel(mLblCombineOverlaps, "Overlapped notes");
   setupLabel(mLblMonoPoly, "Prob. Mode");
   setupLabel(mLblMaxPoly, "Max Poly");
   setupLabel(mLblClockDiv, "Playback Speed");
   setupLabel(mLblScale, "Scale");
   setupLabel(mLblPatLength, "Total Steps");
   setupLabel(mLblDutyCycle, "Note-on Length");
   setupLabel(mLblMidiChannel, "Output Midi Chan");
   setupLabel(mLblStepsPerMeasure, "Steps Per Measure");
   setupLabel(mLblNumRows, "Visible Rows");
   setupLabel(mLblPolyBias, "Bias");

   // mono/poly setup and max poly setup
   mMonoPoly.addItem(SEQCTL_OPT_MP_TOGGLE_MONO, "Mono");
   mMonoPoly.addItem(SEQCTL_OPT_MP_TOGGLE_POLY, "Poly");
   addAndMakeVisible(mMonoPoly);
   mMaxPoly.setSpec(1, SEQ_MAX_ROWS, 1, SEQ_MAX_ROWS, "");
   addAndMakeVisible(mMaxPoly);   
   mPolyBias.setSpec(SEQ_POLY_BIAS_MIN, SEQ_POLY_BIAS_MAX, 1, 0, "");
   addAndMakeVisible(mPolyBias);
   
   
   // clock divider setup
   for (i = 0; i < SEQ_NUM_CLOCK_DIVS;i++){
      const char *val;
      int num;
      val=SeqScale::getClockDividerTextAndId(i, &num);
      mClockDiv.addItem(num, val);      
   }
   addAndMakeVisible(mClockDiv);
   
   
   // Scale std/custom setup
   mScaleStandCust.addItem(SEQCTL_OPT_SCALE_TOGGLE_STD, "Standard");
   mScaleStandCust.addItem(SEQCTL_OPT_SCALE_TOGGLE_CST, "Custom");
   addAndMakeVisible(mScaleStandCust);
   mScaleList.addListener(this);
   mKeyList.addListener(this);
   mOctaveList.addListener(this);
   // scale list
   comboSetup(&mScaleList);
   addAndMakeVisible(mScaleList);
   len = SeqScale::getNumScales();
   for (i = 0; i<len; i++)
      mScaleList.addItem(SeqScale::getScaleName(i), i + 1);
   // key list
   comboSetup(&mKeyList);
   addAndMakeVisible(mKeyList);
   len = SeqScale::getNumNoteNames();
   for (i = 0; i<len; i++)
      mKeyList.addItem(SeqScale::getNoteName(i), i + 1);
   // octave list
   comboSetup(&mOctaveList);
   addAndMakeVisible(mOctaveList);
   fillOctaveList();
   // custom buttons
   mBtnLoadCustom.setText("Load");
   addAndMakeVisible(mBtnLoadCustom);
   mBtnSaveCustom.setText("Save");
   addAndMakeVisible(mBtnSaveCustom);
   mBtnStdCustom.setText("Send to Custom");
   addAndMakeVisible(mBtnStdCustom);

   // mute
   mMuted.addItem(1, "Mute Layer");
   addAndMakeVisible(mMuted);

   mBtnMuteAll.setText("Mute All");
   addAndMakeVisible(mBtnMuteAll);

   // Pattern length setup
   mPatLength.setSpec(SEQ_MIN_STEPS, SEQ_MAX_STEPS, 1, SEQ_DEFAULT_NUM_STEPS, String());
   addAndMakeVisible(mPatLength);
   
   // duty cycle setup (105 is legato)
   mDutyCycle.setSpec(SEQ_DUTY_MIN, SEQ_DUTY_MAX, SEQ_DUTY_INTERVAL, SEQ_DUTY_DEFAULT, "%");
   addAndMakeVisible(mDutyCycle);
   
   // midi channel setup
   mMidiChannel.setSpec(1, 16, 1,1, "");
   addAndMakeVisible(mMidiChannel);
   
   // steps per measure setup
   mStepsPerMeasure.setSpec(SEQ_MIN_STEPS, SEQ_DEFAULT_NUM_STEPS, 1, SEQ_DEFAULT_NUM_STEPS,"");
   addAndMakeVisible(mStepsPerMeasure);
   
   // number of visible rows
   mNumRows.setSpec(SEQ_MIN_ROWS, SEQ_MAX_ROWS, 1, SEQ_MAX_VISIBLE_ROWS, "");
   addAndMakeVisible(mNumRows);
   

   // humanize
   mHumanizeVelo.setSpec(0, SEQ_MAX_HUMAN_VELOCITY, 1, 0, "%");
   addAndMakeVisible(mHumanizeVelo);
   mHumanizeLength.setSpec(0, SEQ_MAX_HUMAN_LENGTH, 1, 0, "%");
   addAndMakeVisible(mHumanizeLength);
   mHumanizePos.setSpec(0, SEQ_MAX_HUMAN_POSITION, 1, 0, "%");
   addAndMakeVisible(mHumanizePos);
   

   addAndMakeVisible(mCombineOverlapsToggle);
   mCombineOverlapsToggle.addItem(SEQCTL_COMBINE_TOGGLE_TRUNC, "Trim", true);
   mCombineOverlapsToggle.addItem(SEQCTL_COMBINE_TOGGLE_JOIN, "Join", false);
   

}

void OptionsPanel::setupLabel(Label & lbl, const String & txt)
{
   juce::Colour txtclr =
      mGlob->mEditorState->getColorFor(EditorState::background).contrasting(.5);
   lbl.setText(txt, juce::NotificationType::dontSendNotification);
   lbl.setColour(juce::Label::textColourId, txtclr);
   addAndMakeVisible(lbl);
}

bool OptionsPanel::allLayersAreMuted()
{
   
   int i;
   for (i = 0; i < SEQ_MAX_LAYERS; i++) {
      if (!mGlob->mSeqBuf->getUISeqData()->getLayer(i)->getMuted())
         break;
   }
   if (i == SEQ_MAX_LAYERS)
      return true;
   return false;
   
}



void OptionsPanel::resized()
{
   const int spacing = SEQ_SIZE_PROP_HEIGHT; // height of each
   const int gap = SEQ_SIZE_PROP_VSPACE;      // gap between
   const int tab = 120;     // size of text on left

   Rectangle<int> me = getLocalBounds();
   Rectangle<int> cur, rTxt, sko;

   Rectangle<int> midpanel, rightpanel;
   me.reduce(2,2); // border around all

   // 2 columns
   rightpanel =me.removeFromRight(me.getWidth()/2);
   midpanel = rightpanel.removeFromLeft(rightpanel.getWidth() / 2);
   rightpanel.removeFromLeft(20);
   midpanel.removeFromLeft(20);
   
   cur = me.removeFromTop(spacing);
   cur.removeFromLeft(tab);
   mMuted.setBounds(cur.removeFromLeft(cur.getWidth()/2));
   mBtnMuteAll.setBounds(cur);
   me.removeFromTop(gap);
   
   // mono/poly
   cur = me.removeFromTop(spacing);
   rTxt = cur.removeFromLeft(tab);
   mLblMonoPoly.setBounds(rTxt);

   rTxt = cur.removeFromLeft(95);
   mMonoPoly.setBounds(rTxt);
   cur.removeFromLeft(4);
   rTxt = cur.removeFromLeft(65);
   mLblMaxPoly.setBounds(rTxt);
   cur.removeFromLeft(4);
   rTxt = cur.removeFromLeft(50);
   mMaxPoly.setBounds(rTxt);

   cur.removeFromLeft(4);
   rTxt = cur.removeFromLeft(40);
   mLblPolyBias.setBounds(rTxt);

   rTxt = cur.removeFromRight(50);
   mPolyBias.setBounds(rTxt);
   me.removeFromTop(gap);

   // clock div
   cur = me.removeFromTop(spacing);
   rTxt = cur.removeFromLeft(tab);
   mLblClockDiv.setBounds(rTxt);
   mClockDiv.setBounds(cur);
   me.removeFromTop(gap);


   //
   // Right hand side
   //

   // scale
   cur = me.removeFromTop(spacing);
   rTxt = cur.removeFromLeft(tab);
   mLblScale.setBounds(rTxt);
   rTxt = cur.removeFromRight(120);
   rTxt.removeFromLeft(4);
   mScaleStandCust.setBounds(cur);
   mBtnStdCustom.setBounds(rTxt);
   me.removeFromTop(gap);

   // scale/key/oct list
   sko = cur = me.removeFromTop(spacing);
   rTxt = cur.removeFromLeft(tab); // for text
   mLblKeyScaleOct.setBounds(rTxt);
   mKeyList.setBounds(cur.removeFromLeft(42));
   cur.removeFromLeft(gap);
   mOctaveList.setBounds(cur.removeFromRight(42));
   cur.removeFromRight(gap);
   mScaleList.setBounds(cur);
   me.removeFromTop(gap);

   // save/load (same space as scale/key/oct)
   cur = sko;
   cur.removeFromLeft(tab);
   rTxt = cur.removeFromLeft(cur.getWidth()/2);
   mBtnLoadCustom.setBounds(rTxt);
   mBtnSaveCustom.setBounds(cur);
   me.removeFromTop(gap);


   // total steps 
   cur = midpanel.removeFromTop(spacing);
   rTxt = cur.removeFromLeft(tab);
   mLblPatLength.setBounds(rTxt);
   mPatLength.setBounds(cur);
   midpanel.removeFromTop(gap);

   // note on len
   cur = midpanel.removeFromTop(spacing);
   rTxt = cur.removeFromLeft(tab);
   mLblDutyCycle.setBounds(rTxt);
   mDutyCycle.setBounds(cur);
   midpanel.removeFromTop(gap);

   // number of rows 
   cur = midpanel.removeFromTop(spacing);
   rTxt = cur.removeFromLeft(tab);
   mLblNumRows.setBounds(rTxt);
   mNumRows.setBounds(cur);
   midpanel.removeFromTop(gap);

   //channel
   cur = midpanel.removeFromTop(spacing);
   rTxt = cur.removeFromLeft(tab);
   mLblMidiChannel.setBounds(rTxt);
   mMidiChannel.setBounds(cur);
   midpanel.removeFromTop(gap);
   
   //steps per measure
   cur = rightpanel.removeFromTop(spacing);
   rTxt = cur.removeFromLeft(tab);
   mLblStepsPerMeasure.setBounds(rTxt);
   mStepsPerMeasure.setBounds(cur);
   rightpanel.removeFromTop(gap);

   // humanize position
   cur = rightpanel.removeFromTop(spacing);
   rTxt = cur.removeFromLeft(tab);
   mLblHumanPos.setBounds(rTxt);
   mHumanizePos.setBounds(cur);
   rightpanel.removeFromTop(gap);
   // humanize length
   cur = rightpanel.removeFromTop(spacing);
   rTxt = cur.removeFromLeft(tab);
   mLblHumanLength.setBounds(rTxt);
   mHumanizeLength.setBounds(cur);
   rightpanel.removeFromTop(gap);
   // humanize velo
   cur = rightpanel.removeFromTop(spacing);
   rTxt = cur.removeFromLeft(tab);
   mLblHumanVelo.setBounds(rTxt);
   mHumanizeVelo.setBounds(cur);
   rightpanel.removeFromTop(gap);
   

   // overlap behavior
   cur = rightpanel.removeFromTop(spacing);
   rTxt = cur.removeFromLeft(tab);
   mLblCombineOverlaps.setBounds(rTxt);
   mCombineOverlapsToggle.setBounds(cur);
   rightpanel.removeFromTop(gap);


   
}

void OptionsPanel::paint(Graphics & g)
{
   EditorState &e = *mGlob->mEditorState;
   gradientFill(e, this, g);
   
}
void OptionsPanel::refreshAll()
{
   EditorState &e = *mGlob->mEditorState;   
   SequenceLayer *sl = mGlob->mSeqBuf->getUISeqData()->getLayer(e.getCurrentLayer());
   const char *scale;
   const char *key;
   int octave;
   if (isEnabled()) {
      // set current values

      // mono poly
      if (sl->isMonoMode()) {
         mMonoPoly.setCurrentItem(SEQCTL_OPT_MP_TOGGLE_MONO, true, false);
         mMaxPoly.setVisible(false);
         mLblMaxPoly.setVisible(false);
         mPolyBias.setVisible(false);
         mLblPolyBias.setVisible(false);
      }
      else {
         mMonoPoly.setCurrentItem(SEQCTL_OPT_MP_TOGGLE_POLY, true, false);
         mMaxPoly.setVisible(true);
         mLblMaxPoly.setVisible(true);
         mPolyBias.setVisible(true);
         mLblPolyBias.setVisible(true);
         mPolyBias.setValue(sl->getPolyBias(), false);
         mMaxPoly.setValue(sl->getMaxPoly(), false);
      }

      // clock div
      mClockDiv.setCurrentItem(sl->getClockDivider(), true, false);

      // std or custom
      if (sl->noteSourceIsCustom()) {
         mScaleStandCust.setCurrentItem(SEQCTL_OPT_SCALE_TOGGLE_CST, true, false);
         mBtnLoadCustom.setVisible(true);
         mBtnSaveCustom.setVisible(true);
         mBtnStdCustom.setVisible(false);
         mOctaveList.setVisible(false);
         mScaleList.setVisible(false);
         mKeyList.setVisible(false);
         mLblKeyScaleOct.setText("Note Information", juce::dontSendNotification);
      }
      else {
         mScaleStandCust.setCurrentItem(SEQCTL_OPT_SCALE_TOGGLE_STD, true, false);
         mBtnLoadCustom.setVisible(false);
         mBtnSaveCustom.setVisible(false);
         mBtnStdCustom.setVisible(true);
         mOctaveList.setVisible(true);
         mScaleList.setVisible(true);
         mKeyList.setVisible(true);
         mLblKeyScaleOct.setText("Key/Scale/Octave", juce::dontSendNotification);
         // key/scale/oct
         fillOctaveList(); // Octave list might change based on user pref for lowest octave
         sl->getKeyScaleOct(&scale, &key, &octave); // octave will be 0 based from here
         findItemInCombo(&mScaleList, scale);
         findItemInCombo(&mKeyList, key);
         // octave is 0 based, so just point to that item
         mOctaveList.setSelectedItemIndex(octave, juce::NotificationType::dontSendNotification);
      }

      // pat length
      mPatLength.setValue(sl->getNumSteps(), false);

      // duty
      mDutyCycle.setValue(sl->getDutyCycle(), false);

      // midi chan
      mMidiChannel.setValue(sl->getMidiChannel(), false);

      // steps per m
      mStepsPerMeasure.setValue(sl->getStepsPerMeasure(), false);

      // num rows
      mNumRows.setValue(sl->getMaxRows(), false);

      // muted
      mMuted.setCurrentItem(1, sl->getMuted() ? 1 : 0, false);

      // mute all - if all are muted change to unmute all
      if(allLayersAreMuted())
         mBtnMuteAll.setText("Unmute All");
      else // one or more are not muted
         mBtnMuteAll.setText("Mute All");


      // humanization
      mHumanizeLength.setValue(sl->getHumanLength(), false);
      mHumanizePos.setValue(sl->getHumanPosition(), false);
      mHumanizeVelo.setValue(sl->getHumanVelocity(), false);

      if(sl->isCombineMode())
         mCombineOverlapsToggle.setCurrentItem(SEQCTL_COMBINE_TOGGLE_JOIN, true, false);
      else
         mCombineOverlapsToggle.setCurrentItem(SEQCTL_COMBINE_TOGGLE_TRUNC, true, false);

   }
}

// this is called when a value is changed via the UI (only)
void OptionsPanel::cptValueChange(int cptId, int id)
{
   EditorState &e = *mGlob->mEditorState;
   SequenceLayer *sl = mGlob->mSeqBuf->getUISeqData()->getLayer(e.getCurrentLayer());

   switch (cptId)
   {
   case SEQCTL_OPT_CLOCKDIV_TOGGLE:
      sl->setClockDivider(id);
      break;
   case SEQCTL_OPT_MP_TOGGLE:
      if (id == SEQCTL_OPT_MP_TOGGLE_MONO)
         sl->setMonoMode(true);
      else
         sl->setMonoMode(false);
      break;
   case SEQCTL_OPT_SCALE_TOGGLE:
      jassert(id == SEQCTL_OPT_SCALE_TOGGLE_CST || id == SEQCTL_OPT_SCALE_TOGGLE_STD);
      sl->setNoteSource(id == SEQCTL_OPT_SCALE_TOGGLE_CST);
      break;
   case SEQCTL_OPT_PATLEN_NUMCTL: // steps per pattern
      if (sl->getNumSteps() != id) { // number of steps has changed
         sl->setNumSteps(id);
         if (id <= SEQ_DEFAULT_NUM_STEPS)
            e.setVisibleStep(0); // switch to page 1 view (as we might have been on "view all" mode which would cause it to stretch)
      }
      break;
   case SEQCTL_OPT_DUTYCY_NUMCTL:
      sl->setDutyCycle(id);
      break;
   case SEQCTL_OPT_MIDICH_NUMCTL:
      sl->setMidiChannel((char)id);
      break;
   case SEQCTL_OPT_STEPSPERM_NUMCTL:
      sl->setStepsPerMeasure(id);
      break;
   case SEQCTL_OPT_NUMROWS_NUMCTL:
      sl->setMaxRows(id);
      break;
   case SEQCTL_OPT_SAVE_CUSTOM:
   {
      // tell editor to show in save mode the dialog
      mParent->cptValueChange(SEQCTL_OPTION_PANEL, SEQCTL_OPTION_PANEL_SAVE);
      break;
   }
   case SEQCTL_OPT_STD_TO_CUSTOM:
   {
      // copy std notes to custom
      for (int i = 0; i < SEQ_MAX_ROWS - 1; i++) {
         sl->setNote(i, sl->getNote(i, false), true);
      }
      // set us to custom
      sl->setNoteSource(true);
      break;
   }
   case SEQCTL_OPT_LOAD_CUSTOM:
   {
      // tell parent to show the file dialog in load note mode
      mParent->cptValueChange(SEQCTL_OPTION_PANEL, SEQCTL_OPTION_PANEL_LOAD);
      break;
   }
   case SEQCTL_OPT_MAX_POLY:
      sl->setMaxPoly(id);
      break;
   case SEQCTL_OPT_POLYBIAS:
      sl->setPolyBias(id);
      break;
   case SEQCTL_OPT_MUTE_TOGGLE:
      sl->setMuted(id == 1);
      break;
   case SEQCTL_OPT_HUMAN_LENGTH:
      sl->setHumanLength(id);
      break;
   case SEQCTL_OPT_HUMAN_POS:
      sl->setHumanPosition(id);
      break;
   case SEQCTL_OPT_HUMAN_VELO:
      sl->setHumanVelocity(id);
      break;
   case SEQCTL_COMBINE_TOGGLE:
      if (id == SEQCTL_COMBINE_TOGGLE_JOIN)
         sl->setCombineMode(true);
      else
         sl->setCombineMode(false);
      break;
   case SEQCTL_OPT_MUTE_ALL:
   { // mute/unmute all
      int i;
      bool mute = !allLayersAreMuted();
      for (i = 0; i < SEQ_MAX_LAYERS; i++)
         mGlob->mSeqBuf->getUISeqData()->getLayer(i)->setMuted(mute);            
      break;
   }

   default:
      break;
   }

   mGlob->mSeqBuf->swap();
   // tell main ui to repaint. value 0 here means repaint
   mParent->cptValueChange(SEQCTL_OPTION_PANEL, 0);
}

void OptionsPanel::comboBoxChanged(ComboBox *)
{
   /*
   This is called whenever any of the combo's change.
   We need to set key/scale/oct to reflect the change
   */
   EditorState &e = *mGlob->mEditorState;
   SequenceLayer *sl = mGlob->mSeqBuf->getUISeqData()->getLayer(e.getCurrentLayer());
   String key, scale;
   int oct=0;
   scale= mScaleList.getText();
   key= mKeyList.getText();
   oct = mOctaveList.getSelectedItemIndex();
   // octave is 0 based now
   sl->setKeyScaleOct(scale.getCharPointer(), key.getCharPointer(), oct);
   mGlob->mSeqBuf->swap();
   // tell main ui to repaint. value 0 means repaint
   mParent->cptValueChange(SEQCTL_OPTION_PANEL, 0);
}

void OptionsPanel::fillOctaveList()
{
   /* The octave list needs to reflect the user's preference for starting octave.
   So it should start at the user's lowest octave number and go up from there
   */
   int lowOct=mGlob->mEditorState->getLowestOctave();
   mOctaveList.clear(juce::NotificationType::dontSendNotification);
   for (int i = 0; i < SEQ_NUM_OCTAVES; i++)
     mOctaveList.addItem(String::formatted("%d", i+lowOct), i + 1);
}

void
OptionsPanel::comboSetup(ComboBox *cb)
{
   cb->setWantsKeyboardFocus(false);
   cb->setColour(juce::ComboBox::ColourIds::backgroundColourId,
      mGlob->mEditorState->getColorFor(EditorState::coloredElements::toggleOff));
   cb->setColour(juce::ComboBox::ColourIds::textColourId,
      mGlob->mEditorState->getColorFor(EditorState::coloredElements::toggleOff).contrasting(.5));
   cb->setColour(juce::ComboBox::ColourIds::outlineColourId,
      mGlob->mEditorState->getColorFor(EditorState::coloredElements::border));
   cb->setColour(juce::ComboBox::ColourIds::arrowColourId,
      mGlob->mEditorState->getColorFor(EditorState::coloredElements::border));
   cb->setColour(juce::ComboBox::ColourIds::buttonColourId,
      mGlob->mEditorState->getColorFor(EditorState::coloredElements::toggleOn));
}

void 
OptionsPanel::findItemInCombo(ComboBox *cb, String txt)
{
   int i;
   for (i = 0; i < cb->getNumItems(); i++) {
      if (cb->getItemText(i).compare(txt) == 0) {
         cb->setSelectedItemIndex(i, juce::NotificationType::dontSendNotification);
         break;
      }
   }
}
