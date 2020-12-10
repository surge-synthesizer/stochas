/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Persist.h"

/* Given a number of steps, return number of pages needed to accomodate that
 number of steps
 */
static int
numStepPages(int steps)
{
   if (steps <= SEQ_DEFAULT_NUM_STEPS) // no buttons are needed
      return 0;
   else // more than 16. take into account pages with less than 16 steps.
      return (steps / SEQ_DEFAULT_NUM_STEPS) + (steps % SEQ_DEFAULT_NUM_STEPS == 0 ? 0 : 1);
}

SeqAudioProcessorEditor::SeqAudioProcessorEditor(SeqAudioProcessor &p)
    : AudioProcessorEditor(&p), mProcessor(p),
      mGlob(&p.mData, p.mEditorState, &p.mNotifier, &p.mIncomingData, &p),
      mStepHolder("stepholder"),                         // holds the step pane and the play pane as a child
      mStepPanel(&mGlob, SEQCTL_STEP_PANEL, this, this), // main step pane
      mNoteHolder("noteholder"),                         // holds the note pane as a child
      mNotePanel(&mGlob),                                // note pane
      mStepScrollbar(true),                              // is vertical
      mStepHScrollbar(false),                            // is horizontal
      mMidiIndicator("midiIndicator"),
      mPlayPanel(&mGlob),                                              // play position indicator
      mPatPlayPanel(&mGlob),                                           // pattern play panel
      mEditToggle(&mGlob, SEQCTL_EDIT_TOGGLE, this, "editModeSelect"), // which mode we are looking at: prob/velo/opt
      mHelpBtn(&mGlob, SEQCTL_HELP_BUTTON, this, "helpButton"),        // help button
      mEditBtn(&mGlob, SEQCTL_EDIT_BUTTON, this, "editButton"),        // button that brings up edit dialog
      mUndoBtn(&mGlob, SEQCTL_UNDO_BUTTON, this, "undoButton"),        // button that brings up edit dialog
      mRecordBtn(&mGlob, SEQCTL_RECORD_BUTTON, this, "recordButton"),
      mPlayBtn(&mGlob, SEQCTL_PLAY_BUTTON, this, "playButton"),
      mPatternSelect(&mGlob, SEQCTL_PATSEL_TOGGLE, this, "patternSelect"),      // select current pattern
      mSectionSelect(&mGlob, SEQCTL_SECTION_TOGGLE, this, "sectionSelect"),     // select section when number of steps exceeds 16
      mMainTabs(SEQCTL_TABS, this, TabbedButtonBar::Orientation::TabsAtBottom), // main tabs at bottom
      mBPM(&mGlob, SEQCTL_STANDALONE_BPM_BUTTON, this, "standaloneBPM"),
      mTabGroove(&mGlob, true),                             // tab that holds groove stuff
      mGroove(SEQCTL_GROOVE, &mGlob, this),                 // groove control
      mClearGrooveBtn(&mGlob, SEQCTL_GRV_CLR_BUTTON, this), // clear groove/copy swing to groove
      mBtnLoadGroove(&mGlob, SEQCTL_GRV_LOAD_BUTTON, this),
      mBtnSaveGroove(&mGlob, SEQCTL_GRV_SAVE_BUTTON, this),
      mGrooveSwing(&mGlob, SEQCTL_SWING_NUMBER, this), // swing amount
      mTabSettings(&mGlob, SEQCTL_SETTINGS_TAB, this), // tab that holds settings
      mTabChord(&mGlob, true),                         // tab that holds chord stuff
      mTglChords(&mGlob, SEQCTL_CHORD_TOGGLE, this, "tglChord"),
      mOptionsPanel(&mGlob, this), // layer options panel
      mTabPatchOpts(&mGlob, true), // patch options
      mMidiPass(&mGlob, SEQCTL_MIDI_PASSTHRU, this, "midiPass"),
      mMidiRespond(&mGlob, SEQCTL_MIDI_RESPOND, this, "midiRespond"),
      mPlaybackMode(&mGlob, SEQCTL_PLAYBACK_MODE, this, "playbackMode"),
      mMidiMap(&mGlob, SEQCTL_MIDI_MAP_BUTTON, this, "midiMap"),
      mBtnLoadPatch(&mGlob, SEQCTL_LOAD_PATCH, this, "loadPatch"),
      mBtnSavePatch(&mGlob, SEQCTL_SAVE_PATCH, this, "savePatch"),
      mRandomToggle(&mGlob, SEQCTL_RANDOM_TOGGLE, this, "randomToggle"),
      mUISizePanic(&mGlob, SEQCTL_SIZE_PANIC, this, "sizePanic"),
      mHelpBanner(&mGlob),                             // banner at bottom that indicates alert or context help
      mLayerToggle(&mGlob, SEQCTL_LAYER_TOGGLE, this), // for selecting which layer we are on
      mEditDialog(&mGlob, this),                       // our edit dialog itself
      mInfoDialog(&mGlob, this),
      mFileChooser(&mGlob, this),
      mChainDialog(&mGlob, this),
      mMidiLightCountDown(0),                         // for keeping the midi light lit for a specified time
      mTimeDivider(0),                                // for updating help text at bottom
      mRecStateCache(SeqProcessorNotifier::standby),  // force it to be different than default
      mPlayStateCache(SeqProcessorNotifier::standby), // force it to be different than default
      mMidiDlg(&mGlob, this)
{

   int tmp = 0;

   setLookAndFeel(&mLookAndFeel);
   // for help text
   mStepScrollbar.setName("mainScroll");
   mStepHScrollbar.setName("mainHScroll");
   setName("main");
   mGroove.setName("grv");
   mGrooveSwing.setName("grvSwing");
   mClearGrooveBtn.setName("grvClr");
   mBtnLoadGroove.setName("grvLoadMidi");
   mBtnSaveGroove.setName("grvSaveMidi");

   addChildComponent(mMidiDlg);

   addChildComponent(mFileChooser);
   addChildComponent(mChainDialog);

   //=============================StepPanel and PlayPanel which are within their own holders
   addAndMakeVisible(mStepHolder);
   mStepHolder.addAndMakeVisible(mStepPanel);
   mStepPanel.setRowNotify(&mNotePanel);

   addAndMakeVisible(mPPHolder);
   mPPHolder.addAndMakeVisible(mPlayPanel);

   //=============================NotePanel (is contained in note holder)
   addAndMakeVisible(mNoteHolder);
   mNoteHolder.addAndMakeVisible(mNotePanel);
   addAndMakeVisible(mMidiIndicator);

   //=============================View modes toggle
   mEditToggle.addItem(SEQCTL_EDIT_TOGGLE_PROB, "Prob");
   mEditToggle.addItem(SEQCTL_EDIT_TOGGLE_VELO, "Velo");
   mEditToggle.addItem(SEQCTL_EDIT_TOGGLE_CHAIN, "Chain");
   mEditToggle.addItem(SEQCTL_EDIT_TOGGLE_OFFSET, "Shift");

   // set up the current values from the Edit model
   switch (mGlob.mEditorState->getEditMode())
   {
   case EditorState::editingSteps:
      tmp = SEQCTL_EDIT_TOGGLE_PROB;
      break;
   case EditorState::editingVelocity:
      tmp = SEQCTL_EDIT_TOGGLE_VELO;
      break;
   case EditorState::editingChain:
      tmp = SEQCTL_EDIT_TOGGLE_CHAIN;
      break;
   case EditorState::editingOffset:
      tmp = SEQCTL_EDIT_TOGGLE_OFFSET;
      break;
   default:
      jassert(false);
      break;
   }
   mEditToggle.setCurrentItem(tmp, true, false);
   addAndMakeVisible(mEditToggle);
   mEditLabel.setText("View", juce::dontSendNotification);
   //   mEditLabel.setColour(juce::Label::textColourId, txtclr);
   addAndMakeVisible(mEditLabel);

   //=============================Layers selector
   mLayerLabel.setText("Layer", juce::dontSendNotification);
   tmp = mGlob.mEditorState->getCurrentLayer();
   for (int i = 0; i < SEQ_MAX_LAYERS; i++)
   {
      String t = String::formatted("%d", i + 1);
      mLayerToggle.addItem(i, t, tmp == i);
   }
   // we want to get notified even when user clicks same layer twice
   // this allows us to get rid of ghosting
   mLayerToggle.setAlwaysNotify(true);
   addAndMakeVisible(mLayerToggle);
   addAndMakeVisible(mLayerLabel);
   addAndMakeVisible(mLblLayerName);
   mLblLayerName.setName("lblLayerName");
   mLblLayerName.setEditable(true);
   mLblLayerName.addListener(this);
   addAndMakeVisible(mLblPatternName);
   mLblPatternName.setEditable(true);
   mLblPatternName.addListener(this);
   mLblPatternName.setName("lblPatternName");

   //============================Standalone BPM
   if (mProcessor.wrapperType == AudioProcessor::wrapperType_Standalone)
   {
      mLblBPM.setText("BPM", juce::dontSendNotification);
      addAndMakeVisible(mLblBPM);
      mLblBPM.setName("lblBPM");
      mBPM.setSpec(1, 300, 1, 1, "");
      addAndMakeVisible(mBPM);
   }

   //=============================Help Button
   mHelpBtn.setText("Info");
   addAndMakeVisible(mHelpBtn);

   //============================= Edit Button
   mEditBtn.setText("Edit");
   addAndMakeVisible(mEditBtn);

   mUndoBtn.setText("Undo");
   addAndMakeVisible(mUndoBtn);

   mRecordBtn.setText("Record");
   addAndMakeVisible(mRecordBtn);

   mPlayBtn.setText("Play");
   addAndMakeVisible(mPlayBtn);

   //=============================Midi indicator light
   mMidiIndicator.mGlob = &mGlob;

   // general purpose timer that runs at 30hz
   startTimer(0, 1000 / 30 /*30hz*/);

   // ============================= Pattern selector and play panel
   for (int i = 1; i < SEQ_MAX_PATTERNS + 1; i++)
      mPatternSelect.addItem(i, String::formatted("%d", i));
   addAndMakeVisible(mPatternSelect);
   mPatternLabel.setText("Pattern Select", juce::dontSendNotification);
   //mPatternLabel.setColour(juce::Label::textColourId, txtclr);
   addAndMakeVisible(mPatternLabel);
   mPatternSelect.setCurrentItem(mGlob.mSeqBuf->getUISeqData()->getCurrentPattern() + 1, true, false);
   addAndMakeVisible(mPatPlayPanel);

   // ============================== Section selector
   mSectionLabel.setText("Steps", juce::dontSendNotification);
   addAndMakeVisible(mSectionSelect);
   addAndMakeVisible(mSectionLabel);

   // ============================== Bottom tabs
   addAndMakeVisible(mMainTabs);
   mMainTabs.addTab("Layer Options", juce::Colour(), &mOptionsPanel, false);
   mMainTabs.addTab("Patch Options", juce::Colour(), &mTabPatchOpts, false);
   mMainTabs.addTab("Groove/Swing", juce::Colour(), &mTabGroove, false);
   mMainTabs.addTab("Chords", juce::Colour(), &mTabChord, false);
   mMainTabs.addTab("Settings", juce::Colour(), &mTabSettings, false);

   // ============================== Tab contents for bottom tabs

   //========================Groove
   mTabGroove.addAndMakeVisible(mGroove);
   mGrooveSwing.setSpec(SEQ_MIN_SWING, SEQ_MAX_SWING, 1, 0, "%");
   mTabGroove.addAndMakeVisible(mGrooveSwing);
   mGrooveSwingLabel.setText("Swing", juce::NotificationType::dontSendNotification);
   mTabGroove.addAndMakeVisible(mGrooveSwingLabel);
   mGrooveGrooveLabel.setText("Groove", juce::NotificationType::dontSendNotification);
   mTabGroove.addAndMakeVisible(mGrooveGrooveLabel);
   mGrooveHelpLabel.setText("Swing is active. Set swing to 0 to use groove instead.",
                            juce::NotificationType::dontSendNotification);
   mTabGroove.addChildComponent(mGrooveHelpLabel);
   mBtnLoadGroove.setText("Import from MIDI File");
   mBtnSaveGroove.setText("Export to MIDI File");
   mTabGroove.addAndMakeVisible(mBtnLoadGroove);
   mTabGroove.addAndMakeVisible(mBtnSaveGroove);
   mClearGrooveBtn.setText(""); // will be set later
   mTabGroove.addAndMakeVisible(mClearGrooveBtn);

   //==========================Patch tab
   mTabPatchOpts.addAndMakeVisible(mLblMidiPass);
   mLblMidiPass.setText("Passthru MIDI data", juce::dontSendNotification);
   mTabPatchOpts.addAndMakeVisible(mLblMidiRespond);
   mLblMidiRespond.setText("Respond to MIDI", juce::dontSendNotification);
   mMidiPass.addItem(SEQCTL_MIDI_PASSTHRU_ALL, "All", true);
   mMidiPass.addItem(SEQCTL_MIDI_PASSTHRU_UNHND, "Unhandled", false);
   mMidiPass.addItem(SEQCTL_MIDI_PASSTHRU_NONE, "None", false);
   mTabPatchOpts.addAndMakeVisible(mMidiPass);
   mMidiRespond.addItem(SEQCTL_MIDI_RESPOND_YES, "Yes", true);
   mMidiRespond.addItem(SEQCTL_MIDI_RESPOND_NO, "No", false);
   mTabPatchOpts.addAndMakeVisible(mMidiRespond);
   mTabPatchOpts.addAndMakeVisible(mMidiMap);
   mTabPatchOpts.addAndMakeVisible(mBtnLoadPatch);
   mTabPatchOpts.addAndMakeVisible(mBtnSavePatch);
   mTabPatchOpts.addAndMakeVisible(mRandomToggle);
   mRandomToggle.addItem(SEQCTL_RANDOM_TOGGLE_VARYING, "Variable", true);
   mRandomToggle.addItem(SEQCTL_RANDOM_TOGGLE_STABLE, "Stable", false);
   mLblRandomization.setText("Randomization", juce::dontSendNotification);
   mTabPatchOpts.addAndMakeVisible(mLblRandomization);
   mMidiMap.setText("MIDI Mapping...");
   mBtnLoadPatch.setText("Load Patch...");
   mBtnSavePatch.setText("Save Patch...");

   mLblPlaybackMode.setText("Playback mode", juce::dontSendNotification);
   mTabPatchOpts.addAndMakeVisible(mLblPlaybackMode);
   mPlaybackMode.addItem(SEQCTL_PLAYBACK_MODE_AUTO, "Auto", true);
   mPlaybackMode.addItem(SEQCTL_PLAYBACK_MODE_INSTANT, "Instant", false);
   mPlaybackMode.addItem(SEQCTL_PLAYBACK_MODE_Q_STEP, "Step", false);
   mPlaybackMode.addItem(SEQCTL_PLAYBACK_MODE_Q_BEAT, "Beat", false);
   mPlaybackMode.addItem(SEQCTL_PLAYBACK_MODE_Q_MEAS, "Measure", false);
   mTabPatchOpts.addAndMakeVisible(mPlaybackMode);

   // ============================== Chord tab
   mTabChord.addAndMakeVisible(mTglChords);
   mTglChords.setMaxItemsPerRow(5);
   {
      mTglChords.addItem(-1, "Off");
      int numchords = SeqScale::getNumChords();
      for (int i = 0; i < numchords; i++)
      {
         mTglChords.addItem(i, SeqScale::getChordName(i));
      }
   }
   mLblChords.setText("Note: Use up/down arrow keys to set chord inversions", juce::dontSendNotification);
   mTabChord.addAndMakeVisible(mLblChords);

   // ============ size panic btn (will be visible for a short period of time if ui is rescaled)
   mUISizePanic.setText("Restore UI To Default Size");
   addChildComponent(mUISizePanic);
   if(mGlob.mEditorState->getScaleFactor() > 100.0) {
      mUISizePanic.setVisible(true);
      startTimer(3, 2000); // make invisible after 2 sec
   } else {
      mUISizePanic.setVisible(false);
   }

   // ============================== Help and alert banner
   addAndMakeVisible(mHelpBanner);

   // ============================== Edit dialog
   addChildComponent(mEditDialog);

   // ============================== Info dialog
   addChildComponent(mInfoDialog);

   // ================================= Scrollbar that moves rows up and down
   mStepScrollbar.setRangeLimits(Range<double>(0, 1), juce::dontSendNotification);
   mStepScrollbar.setAutoHide(true);
   mStepScrollbar.addListener(this);
   addAndMakeVisible(mStepScrollbar);

   // ================================== Scrollbar that moves rows left/right
   mStepHScrollbar.setRangeLimits(Range<double>(0, 1), juce::dontSendNotification);
   mStepHScrollbar.setAutoHide(true);
   mStepHScrollbar.addListener(this);
   addAndMakeVisible(mStepHScrollbar);

   // set main size. Note that this is done almost last, because it causes a paint to happen.
   // We don't want to paint on the items above until the above already happened, otherwise they
   // don't get another paint apparently
   setSize(SEQ_SIZE_MAIN_W, SEQ_SIZE_MAIN_H);

   fixButtonColors();

   // now that we have our size we can do this
   updateUI();
}

SeqAudioProcessorEditor::~SeqAudioProcessorEditor()
{
   // avoid an assert
   setLookAndFeel(nullptr);

   stopTimer(0);
}

void SeqAudioProcessorEditor::paint(Graphics &g)
{
   g.fillAll(mGlob.mEditorState->getColorFor(EditorState::background));
}

void SeqAudioProcessorEditor::resized()
{

   Desktop::getInstance().setGlobalScaleFactor(mGlob.mEditorState->getScaleFactor() / 100.0);

   // position the grid panel. leave room on left
   // for notes, on top and bottom for whatever is needed
   Rectangle<int> me = getLocalBounds();
   const int spc = SEQ_SIZE_PROP_HEIGHT;
   const int gap = SEQ_SIZE_PROP_VSPACE;
   Rectangle<int> gridctrl, lower, left, upper, tmp, tmp2, options,
       midiInd, patToggle, banner, scrl, scrlH, pph;

   // ORDER OF THESE IS IMPORTANT
   // the remove* functions alter the original rect

   // allow for a full border around the whole thing
   me.reduce(SEQ_SIZE_MAIN_BORDER, SEQ_SIZE_MAIN_BORDER);

   // Area above the main grid (help btn, layer select)
   upper = me.removeFromTop(20);

   // small buffer above the grid
   me.removeFromTop(SEQ_SIZE_MAIN_BORDER);

   // Bigger area below the main grid
   lower = me.removeFromBottom(me.getHeight() -
                               ((SEQ_SIZE_CELL_H * SEQ_MAX_VISIBLE_ROWS) + SEQ_SIZE_PP_H + SEQ_SIZE_HSCROLL));

   // top of that is the toggles for seq controls
   lower.removeFromTop(SEQ_SIZE_MAIN_BORDER);
   gridctrl = lower.removeFromTop(30);
   lower.removeFromTop(8); // buffer between grid and controls below

   // help banner at bottom
   banner = lower.removeFromBottom(40);
   banner.removeFromTop(SEQ_SIZE_MAIN_BORDER);

   // horizontal scrollbar (below main grid outside stepholder)
   scrlH = me.removeFromBottom(SEQ_SIZE_HSCROLL);
   scrlH.removeFromLeft(SEQ_SIZE_NOTE_W);

   scrlH.removeFromRight(SEQ_SIZE_VSCROLL);
   tmp = scrlH.removeFromRight(240);
   mSectionSelect.setBounds(tmp);
   tmp = scrlH.removeFromRight(35);
   mSectionLabel.setBounds(tmp);

   // notes at the left of the grid
   left = me.removeFromLeft(SEQ_SIZE_NOTE_W);

   scrl = me.removeFromRight(SEQ_SIZE_VSCROLL);

   // play panel holder
   pph = me.removeFromTop(SEQ_SIZE_PP_H);
   mPPHolder.setBounds(pph);

   // main grid (options panel follows)
   mStepHolder.setBounds(me);

   // scroll bar for vertical
   mStepScrollbar.setBounds(scrl);
   mStepHScrollbar.setBounds(scrlH);

   // midi indicator is above notes panel
   midiInd = left.removeFromTop(SEQ_SIZE_PP_H);
   mMidiIndicator.setBounds(midiInd);
   // notes panel
   mNoteHolder.setBounds(left);

   // pattern label and pattern select
   tmp = gridctrl.removeFromLeft(80);
   patToggle = gridctrl.removeFromLeft(400);
   tmp2 = patToggle.removeFromTop(10);
   tmp.removeFromTop(10);
   mPatternSelect.setBounds(patToggle);
   mPatPlayPanel.setBounds(tmp2);
   mPatternLabel.setBounds(tmp);
   tmp = gridctrl.removeFromLeft(100);
   tmp.removeFromTop(10);
   mLblPatternName.setBounds(tmp);

   // View label and view toggle
   tmp = gridctrl.removeFromRight(180);
   mEditToggle.setBounds(tmp);
   gridctrl.removeFromRight(4);
   tmp = gridctrl.removeFromRight(40);
   mEditLabel.setBounds(tmp);

   // help,edit,undo, record buttons
   // for standalone mode, the bpm as well
   tmp = upper.removeFromRight(50);
   mHelpBtn.setBounds(tmp);
   tmp = upper.removeFromRight(50);
   mEditBtn.setBounds(tmp);
   tmp = upper.removeFromRight(50);
   mUndoBtn.setBounds(tmp);
   tmp = upper.removeFromRight(64);
   mRecordBtn.setBounds(tmp);
   tmp = upper.removeFromRight(64);
   mPlayBtn.setBounds(tmp);
   if (mProcessor.wrapperType == AudioProcessor::wrapperType_Standalone)
   {
      tmp = upper.removeFromRight(40);
      mBPM.setBounds(tmp);
      tmp = upper.removeFromRight(32);
      mLblBPM.setBounds(tmp);
   }

   // section selector (which steps are visible)
   /*
   upper.removeFromRight(10);
   tmp = upper.removeFromRight(310);
   mSectionSelect.setBounds(tmp);
   tmp = upper.removeFromRight(35);
   mSectionLabel.setBounds(tmp);
   */

   // layer selection
   mUISizePanic.setBounds(upper); // will be visible briefly if ui is oversized
   tmp = upper.removeFromLeft(40);
   mLayerLabel.setBounds(tmp);
   tmp = upper.removeFromLeft(200);
   mLayerToggle.setBounds(tmp);
   upper.removeFromLeft(4);
   tmp = upper.removeFromLeft(200); // dont have much room here
   mLblLayerName.setBounds(tmp);

   // bottom tabs
   mMainTabs.setBounds(lower);

   // help banner
   mHelpBanner.setBounds(banner);

   // tab contents--------------------------------------------

   // groove
   {
      Rectangle<int> tmp3;
      const int grvTab = 70;
      const int grvSpc = SEQ_SIZE_PROP_HEIGHT;
      tmp = mTabGroove.getLocalBounds();
      tmp.reduce(SEQ_SIZE_MAIN_BORDER, SEQ_SIZE_MAIN_BORDER);

      tmp2 = tmp.removeFromTop(grvSpc); // swing row

      // save grv
      tmp3 = tmp2.removeFromRight(150);
      mBtnSaveGroove.setBounds(tmp3);
      // load grv
      tmp2.removeFromRight(2);
      tmp3 = tmp2.removeFromRight(150);
      mBtnLoadGroove.setBounds(tmp3);

      // clear
      tmp2.removeFromRight(2);
      tmp3 = tmp2.removeFromRight(150);
      mClearGrooveBtn.setBounds(tmp3);

      // swing
      tmp3 = tmp2.removeFromLeft(grvTab);
      mGrooveSwingLabel.setBounds(tmp3);
      tmp3 = tmp2.removeFromLeft(70);
      mGrooveSwing.setBounds(tmp3);
      mGrooveHelpLabel.setBounds(tmp2);

      tmp.removeFromTop(SEQ_SIZE_PROP_VSPACE);

      tmp3 = tmp.removeFromLeft(grvTab);
      tmp3 = tmp3.removeFromTop(25);
      mGrooveGrooveLabel.setBounds(tmp3);

      mGroove.setBounds(tmp);
   }

   // patch options
   {
      Rectangle<int> tmp3, rightSide;
      const int tab = 130;

      tmp = mTabPatchOpts.getLocalBounds();
      tmp.reduce(SEQ_SIZE_PROP_VSPACE, SEQ_SIZE_PROP_VSPACE);
      rightSide = tmp.removeFromRight(tmp.getWidth() / 2);
      rightSide.removeFromLeft(4);

      // respond to midi
      tmp2 = tmp.removeFromTop(spc);
      tmp3 = tmp2.removeFromRight(140);
      tmp3.removeFromLeft(20);
      mMidiMap.setBounds(tmp3);
      tmp3 = tmp2.removeFromLeft(tab);
      mLblMidiRespond.setBounds(tmp3);
      mMidiRespond.setBounds(tmp2);

      // passthru midi
      tmp.removeFromTop(gap);
      tmp2 = tmp.removeFromTop(spc);
      tmp3 = tmp2.removeFromLeft(tab);
      mLblMidiPass.setBounds(tmp3);
      mMidiPass.setBounds(tmp2);

      // randomization
      tmp.removeFromTop(gap);
      tmp2 = tmp.removeFromTop(spc);
      tmp3 = tmp2.removeFromLeft(tab);
      mLblRandomization.setBounds(tmp3);
      mRandomToggle.setBounds(tmp2);

      // playback mode
      tmp.removeFromTop(gap);
      tmp2 = tmp.removeFromTop(spc);
      tmp3 = tmp2.removeFromLeft(tab);
      mLblPlaybackMode.setBounds(tmp3);
      mPlaybackMode.setBounds(tmp2);

      // save load (on right)
      tmp2 = rightSide.removeFromTop(spc);
      tmp3 = tmp2.removeFromRight(140);
      mBtnLoadPatch.setBounds(tmp3);
      rightSide.removeFromTop(gap);
      tmp2 = rightSide.removeFromTop(spc);
      tmp3 = tmp2.removeFromRight(140);
      mBtnSavePatch.setBounds(tmp3);
   }

   // chord
   {

      tmp = mTabChord.getLocalBounds();
      tmp.reduce(SEQ_SIZE_PROP_VSPACE, SEQ_SIZE_PROP_VSPACE);
      tmp2 = tmp.removeFromTop(spc);
      tmp.removeFromTop(gap);
      mTglChords.setBounds(tmp);
      mLblChords.setBounds(tmp2);
   }
}

// this determines how many buttons to show for switching between sections in the sequence
void SeqAudioProcessorEditor::setSectionSelectItems()
{
   SequenceLayer *sl = mGlob.mSeqBuf->getUISeqData()->getLayer(mGlob.mEditorState->getCurrentLayer());
   String txt;
   int steps = sl->getNumSteps();
   int curpage;
   bool between = false; // will be set to true if we are between two pages

   // which step is leftmost visible
   int leftStep = mGlob.mEditorState->getVisibleStep();
   // determine visible page and whether we are between 2 pages
   if (leftStep != -1)
   { // -1 means zoomed out
      curpage = leftStep / SEQ_DEFAULT_NUM_STEPS;
      if (leftStep % SEQ_DEFAULT_NUM_STEPS != 0)
         between = true;
   }
   else
      curpage = -1;

   // how many panels will there be? (excluding the "all" panel which is dealt with later)
   int panels = numStepPages(steps);

   if (curpage != 0 && curpage >= panels)
   {
      // our current page is out of range, adjust it
      mGlob.mEditorState->setVisibleStep(0);
      curpage = 0;
   }

   mSectionSelect.clearItems();
   if (panels)
   {
      for (int i = 0; i < panels; i++)
      {
         int start = (i * SEQ_DEFAULT_NUM_STEPS);
         int count;
         if (i < panels - 1 || steps % SEQ_DEFAULT_NUM_STEPS == 0)
            count = SEQ_DEFAULT_NUM_STEPS;
         else
            count = steps % SEQ_DEFAULT_NUM_STEPS;
         txt = String::formatted("%d-%d", start + 1, start + count);
         mSectionSelect.addItem(i, txt, i == curpage && !between);
         start += SEQ_DEFAULT_NUM_STEPS;
      }

      mSectionSelect.addItem(-1, "All", -1 == curpage);
   }
   mSectionLabel.setVisible(panels != 0);
}

// move step panel so that desired steps in the range of rows and columns are visible
// also moves note panel
void SeqAudioProcessorEditor::setStepRangeVisible()
{
   SeqDataBuffer *sd = mGlob.mSeqBuf;
   SequenceLayer *sl = sd->getUISeqData()->getLayer(mGlob.mEditorState->getCurrentLayer());
   double upper;
   double thumb = 0.0;
   // number of columns currently visible (will only change if we are zoomed out)
   int visibleColumns = SEQ_DEFAULT_NUM_STEPS;

   // horizontal panning
   // get the series of steps that are currently visible. -1 = all visible
   int ls = mGlob.mEditorState->getVisibleStep();
   if (ls == -1)
   { // show all steps (scale the width appropriately)
      // determine width needed for each step
      double wn = (double)mStepHolder.getWidth() / (double)sl->getNumSteps();
      // total width needed for all steps
      double tw = wn * SEQ_MAX_STEPS;
      mPlayPanel.setBounds(0, 0, (int)tw, SEQ_SIZE_PP_H);
      mStepPanel.setBounds(0, 0, (int)tw, (SEQ_SIZE_CELL_H * sl->getMaxRows()));
      visibleColumns = sl->getNumSteps();
      //} else if(p>-1) {
      //   // pan the step panel horizontally so that the first step
      //   // represents the first step on the desired page
      //   mPlayPanel.setBounds(-(p*SEQ_DEFAULT_NUM_STEPS*SEQ_SIZE_CELL_W), 0,
      //      SEQ_SIZE_CELL_W * SEQ_MAX_STEPS, SEQ_SIZE_PP_H);
      //   mStepPanel.setBounds(-(p*SEQ_DEFAULT_NUM_STEPS*SEQ_SIZE_CELL_W), 0,
      //      SEQ_SIZE_CELL_W * SEQ_MAX_STEPS, (SEQ_SIZE_CELL_H * sl->getMaxRows()));
      //   // Where on the scrollbar should the thumb be, given number of pages and which page we are on
      //   thumb = (double)p / (double)numStepPages(sl->getNumSteps());
   }
   else
   {
      // somewhere in between
      // preserve old scrollbar thumb position
      //thumb = mStepHScrollbar.getCurrentRangeStart();
      thumb = (double)ls / (double)sl->getNumSteps();

      // pan appropriately according to leftmost visible cell
      mPlayPanel.setBounds(-(ls * SEQ_SIZE_CELL_W),
                           0,
                           SEQ_SIZE_CELL_W * SEQ_MAX_STEPS,
                           SEQ_SIZE_PP_H);

      mStepPanel.setBounds(-(ls * SEQ_SIZE_CELL_W),
                           0,
                           SEQ_SIZE_CELL_W * SEQ_MAX_STEPS,
                           (SEQ_SIZE_CELL_H * sl->getMaxRows()));
   }

   // determine range
   // for horz, if we are zoomed out we don't need the scrollbar

   upper = visibleColumns / (double)sl->getNumSteps();
   // set horizontal scrollbar range
   mStepHScrollbar.setCurrentRange(Range<double>(0, upper), juce::dontSendNotification);
   if (upper < 1.0)
      mStepHScrollbar.setCurrentRangeStart(thumb, juce::dontSendNotification);

   // vertical panning

   // preserve old scrollbar thumb position
   thumb = mStepScrollbar.getCurrentRangeStart();

   // resize note panel to reflect number of rows available.
   // this is important as it determines how the scrollbar behaves
   mNotePanel.setBounds(0, 0,
                        mNoteHolder.getWidth(), sl->getMaxRows() * SEQ_SIZE_CELL_H);

   // range is 0 to 1
   upper = (double)SEQ_MAX_VISIBLE_ROWS / (double)sl->getMaxRows();
   mStepScrollbar.setCurrentRange(Range<double>(0, upper), juce::dontSendNotification);
   // restore scrollbar position and scroll there (notify)
   // only do this if there is actually a scrollbar visible
   if (upper < 1.0)
      mStepScrollbar.setCurrentRangeStart(thumb, juce::sendNotification);
}

// dont call this from paint or daw will not refresh properly!
void SeqAudioProcessorEditor::setGrooveItems()
{
   SequenceData *sd = mGlob.mSeqBuf->getUISeqData();
   int sw = sd->getSwing();
   // swing is active so disable gruuv
   mGroove.enable(sw == 0);

   for (int i = 0; i < SEQ_DEFAULT_NUM_STEPS; i++)
      mGroove.setValue(i, sd->getGroove(i));

   mGrooveSwing.setValue(sd->getSwing(), false);

   mGrooveHelpLabel.setVisible(sw != 0);
   if (sw)
      mClearGrooveBtn.setText("Copy swing to groove");
   else
      mClearGrooveBtn.setText("Clear Groove");
}

void SeqAudioProcessorEditor::cptValueChange(int cptId, int id)
{
   EditorState::EditMode em = (EditorState::EditMode)0;
   SeqDataBuffer *sd = mGlob.mSeqBuf;
   SequenceData *s = sd->getUISeqData();

   switch (cptId)
   {
   case SEQCTL_EDIT_TOGGLE:
      // the toggle that selects the current page editing mode has been selected
      switch (id)
      {
      case SEQCTL_EDIT_TOGGLE_PROB:
         em = EditorState::editingSteps;
         break;
      case SEQCTL_EDIT_TOGGLE_VELO:
         em = EditorState::editingVelocity;
         break;
      case SEQCTL_EDIT_TOGGLE_CHAIN:
         em = EditorState::editingChain;
         break;
      case SEQCTL_EDIT_TOGGLE_OFFSET:
         em = EditorState::editingOffset;
         break;
      default:
         jassert(false);
      }
      // set current editing mode
      mGlob.mEditorState->setEditMode(em);
      updateUI();
      repaint();
      break;
   case SEQCTL_PATSEL_TOGGLE:
      // switching to a different pattern
      s->setCurrentPattern(id - 1);
      sd->swap();
      // since we are switching patterns, unselect any selected steps
      mStepPanel.unselectAll();
      updateUI();
      repaint();
      break;
   case SEQCTL_SECTION_TOGGLE:
   {
      int leftStep = -1;
      // switching to a different range of visible cells
      if (id != -1)
         leftStep = id * SEQ_DEFAULT_NUM_STEPS;
      mGlob.mEditorState->setVisibleStep(leftStep);
      // reposition or hide the length editing cursor as appropriate
      mStepPanel.moveLengthEditCursor();
      setStepRangeVisible();
      repaint();
      break;
   }
   case SEQCTL_LAYER_TOGGLE:
      // switching to a different layer
      mGlob.mEditorState->setCurrentLayer(id);
      // we are on a different layer, so the currently selected cell may not be appropriate
      mStepPanel.unselectAll();
      // we need to make sure the ghosting stuff is properly set up
      mStepPanel.layerHasChanged();
      // do a full update
      updateUI();
      repaint();
      break;
   case SEQCTL_OPTION_PANEL:
      switch (id)
      {
      case SEQCTL_OPTION_PANEL_REPAINT:
         // options panel had an item change that requires a repaint in this guy
         mStepPanel.unselectAll(); // ensure nothing selected
         updateUI();
         repaint();
         break;
      case SEQCTL_OPTION_PANEL_LOAD:
         showFileChooser(SEQ_FILE_LOAD_NOTES);
         // will trigger a notify SEQCTL_FILEDIALOG if hit ok
         break;
      case SEQCTL_OPTION_PANEL_SAVE:
         showFileChooser(SEQ_FILE_SAVE_NOTES);
         // will trigger a notify SEQCTL_FILEDIALOG if hit ok
         break;
      }

      break;
   case SEQCTL_EDITDIALOG: // user clicked OK in edit dialog
      // edit dialog did some action which may affect current state
      // just do every refresh known to man
      mStepPanel.unselectAll();     // ensure nothing selected
      mStepPanel.layerHasChanged(); // update ghosting stuff
      updateUI();                   // all updates
      repaint();
      break;
   case SEQCTL_GROOVE:
   {
      // one of the groove values is changing so update sequence data
      int v = mGroove.getValue(id);
      s->setGroove(id, v);
      // changing groove value resets swing to 0 (swing will already be
      // 0 though because otherwise gruuv would be disabled)
      s->setSwing(0);
      sd->swap();
      updateUI();
      repaint();
      break;
   }
   case SEQCTL_SWING_NUMBER:
      // user has changed the value of swing
      {
         int swingVal = mGrooveSwing.getValue();
         s->setSwing(swingVal);
         sd->swap();
         updateUI();
         repaint();
         break;
      }
   case SEQCTL_HELP_BUTTON:
   {
      mInfoDialog.openDialog();
      break;
   }
   case SEQCTL_EDIT_BUTTON:
      // edit was clicked
      mEditDialog.doSetup();
      mEditDialog.openDialog();
      break;

   case SEQCTL_UNDO_BUTTON:
      sd->undo();
      updateUI();
      repaint();
      break;

   case SEQCTL_RECORD_BUTTON:
      // toggle record state
      mGlob.mProcessNotify->addToFifo(SEQ_SET_RECORD_MODE, 0, 0);
      break;
   case SEQCTL_PLAY_BUTTON:
      // toggle play state if we are in right mode
      mGlob.mProcessNotify->addToFifo(SEQ_SET_PLAY_START_STOP, 0, 0);
      break;
   case SEQCTL_STANDALONE_BPM_BUTTON:
   {
      // set a new bpm
      int t = mBPM.getValue();
      s->setStandaloneBPM((double)t);
      sd->swap();
      // tell the processor that bpm has changed so it can do a recalc
      mGlob.mProcessNotify->addToFifo(SEQ_STANDALONE_SET_TEMPO, 0, 0);
      break;
   }
   case SEQCTL_GRV_CLR_BUTTON: // clear or copy to groove was clicked
      clearGrooveOrCopySwing();
      updateUI();
      repaint();
      break;
   case SEQCTL_GRV_LOAD_BUTTON:
      showFileChooser(SEQ_FILE_LOAD_MIDI);
      updateUI();
      repaint();
      break;
   case SEQCTL_GRV_SAVE_BUTTON:
      showFileChooser(SEQ_FILE_SAVE_MIDI);
      updateUI();
      repaint();
      break;
   case SEQCTL_TABS: // tab selection at bottom has changed
      // turn off chord
      chordSelect(-1);
      break;
   case SEQCTL_SETTINGS_TAB:
      // one of the items on the settings tab changed. id will be the id on that tab of the item that changed
      switch (id)
      {
      case SEQCTL_SET_UISCALE:
         resized();
         // this is cheezy I know, but I am not sure what hosts will not resize themselves properly
         setAlertText(SEQ_RESIZE_MSG);
         break;
      case SEQCTL_SET_COLOR:
         // fix tab colors
         fixButtonColors();
         repaint();
         break;
      default:
         repaint();
      }
      break;
   case SEQCTL_MIDI_MAP_BUTTON:
      // read sequence data into the dialog
      mMidiDlg.seqDataToLocal();
      mMidiDlg.openDialog();
      break;
   case SEQCTL_MIDI_PASSTHRU:
      if (id == SEQCTL_MIDI_PASSTHRU_ALL)
         s->setMidiPassthru(SEQ_MIDI_PASSTHRU_ALL);
      else if (id == SEQCTL_MIDI_PASSTHRU_NONE)
         s->setMidiPassthru(SEQ_MIDI_PASSTHRU_NONE);
      else if (id == SEQCTL_MIDI_PASSTHRU_UNHND)
         s->setMidiPassthru(SEQ_MIDI_PASSTHRU_UNHANDLED);
      else
         jassertfalse;
      sd->swap();
      refreshPatchOptions();
      break;
   case SEQCTL_MIDI_RESPOND:
      if (id == SEQCTL_MIDI_RESPOND_YES)
      {
         s->setMidiRespond(SEQ_MIDI_RESPOND_YES);
      }
      else if (id == SEQCTL_MIDI_RESPOND_NO)
      {
         s->setMidiRespond(SEQ_MIDI_RESPOND_NO);
      }
      sd->swap();
      refreshPatchOptions();
      break;
   case SEQCTL_RANDOM_TOGGLE:
      if (id == SEQCTL_RANDOM_TOGGLE_STABLE)
      {
         // fetch the last generated seed from engine.
         // this will be a good number even if playback was not
         // started yet
         int64 seed = mGlob.mAudNotify->getRandomSeed();
         s->setRandomSeed(seed);
         sd->swap();
      }
      else if (id == SEQCTL_RANDOM_TOGGLE_VARYING)
      {
         // set to 0 so a new one gets always generated
         s->setRandomSeed(0);
         sd->swap();
      }
      else
         jassertfalse;
      break;
   case SEQCTL_SAVE_PATCH:
      showFileChooser(SEQ_FILE_SAVE_PATCH);
      break;
   case SEQCTL_LOAD_PATCH:
      showFileChooser(SEQ_FILE_LOAD_PATCH);
      break;
   case SEQCTL_STEP_PANEL:
      // stepPanel could send some notification if it wanted
      break;
   case SEQCTL_CHORD_TOGGLE:
      chordSelect(id);
      break;
   case SEQCTL_FILEDIALOG: // file dialog had "OK" hit on it, access members on it to determine what to do
      respondFileChooser();
      repaint();
      break;
   case SEQCTL_ADDCHAINDIALOG:
      repaint();
      break;
   case SEQCTL_PLAYBACK_MODE:
      if (id == SEQCTL_PLAYBACK_MODE_AUTO)
         s->setAutoPlayMode(SEQ_PLAYMODE_AUTO);
      else if (id == SEQCTL_PLAYBACK_MODE_INSTANT)
         s->setAutoPlayMode(SEQ_PLAYMODE_INSTANT);
      else if (id == SEQCTL_PLAYBACK_MODE_Q_STEP)
         s->setAutoPlayMode(SEQ_PLAYMODE_STEP);
      else if (id == SEQCTL_PLAYBACK_MODE_Q_BEAT)
         s->setAutoPlayMode(SEQ_PLAYMODE_BEAT);
      else if (id == SEQCTL_PLAYBACK_MODE_Q_MEAS)
         s->setAutoPlayMode(SEQ_PLAYMODE_MEASURE);
      else
         jassertfalse;
      sd->swap();
      refreshPatchOptions();
      break;
   case SEQCTL_SIZE_PANIC:
      // user hit the size panic button. restore the ui to original size
      mGlob.mEditorState->setScaleFactor(100);
      resized();
      updateUI(); // force setting tab to update the setting for scale factor
      // this is cheezy I know, but I am not sure what hosts will not resize themselves properly
      setAlertText(SEQ_RESIZE_MSG);
      break;
   default:
      jassertfalse;
   }
}

void SeqAudioProcessorEditor::cptItemClickWithModifier(int cptId, int id, juce::ModifierKeys mods)
{

   if (cptId == SEQCTL_LAYER_TOGGLE)
   {
      bool mute = false;
      bool solo = false;
      int i;
      int whichSolo = -1;
      int muteCt = 0;
      SequenceData *sd = mGlob.mSeqBuf->getUISeqData();
      if (mods.isShiftDown())
         mute = true;
      else if (mods.isCtrlDown() || mods.isCommandDown())
         solo = true;
      // determine if any are soloed
      for (i = 0; i < SEQ_MAX_LAYERS; i++)
      {
         SequenceLayer *sl = sd->getLayer(i);
         if (sl->getMuted())
            muteCt++;
         else
            whichSolo = i;
      }

      if (muteCt != SEQ_MAX_LAYERS - 1)
         whichSolo = -1; // no solo condition exists

      // mute or unmute as logic dictates
      for (i = 0; i < SEQ_MAX_LAYERS; i++)
      {
         SequenceLayer *sl = sd->getLayer(i);
         if (mute && id == i)              // mute/unmute selected
            sl->setMuted(!sl->getMuted()); // toggle
         else if (solo)
         {
            // we solo the selected one (mute all others)
            // unless they are toggling the one that's currently soloed
            // in which case all are unmuted

            if (id == whichSolo)
            { // toggling the one that's currently soloed
               sl->setMuted(false);
            }
            else
            { // solo selected
               sl->setMuted(id != i);
            }
         }
      }
      mGlob.mSeqBuf->swap();
      updateUI();
      repaint();
   }
}

void SeqAudioProcessorEditor::chordSelect(int id)
{
   int cnt;
   int *ints;
   mStepPanel.mChordHandler.clearIntervals();
   if (id != -1)
   { // -1 is our "off"
      ints = SeqScale::getChordIntervals(id, &cnt);
      for (int i = 0; i < cnt; i++)
      {
         mStepPanel.mChordHandler.addInterval(ints[i]);
      }
   }

   // in case we were not coming directly from mTglChords mouse down event
   if (mTglChords.getNumItems())
      mTglChords.setCurrentItem(id, true, false);
}

void SeqAudioProcessorEditor::fixButtonColors()
{
   // this will be called if color scheme is changed
   fixColors(mGlob.mEditorState, this);
   mNotePanel.refreshAll(true);
}

void SeqAudioProcessorEditor::showFileChooser(int mode)
{
   mFileChooser.doSetup(mode);
   mFileChooser.openDialog();
}

void SeqAudioProcessorEditor::respondFileChooser()
{
   EditorState &e = *mGlob.mEditorState;
   SequenceLayer *sl = mGlob.mSeqBuf->getUISeqData()->getLayer(e.getCurrentLayer());

   switch (mFileChooser.mMode)
   {
   case SEQ_FILE_SAVE_NOTES:
   {
      String noteLines;
      File f(mFileChooser.mFileName);
      File noteFile = f.withFileExtension("stnote");
      for (int i = 0; i < SEQ_MAX_ROWS - 1; i++)
      {
         char *nn = sl->getNoteName(i);
         char n = sl->getCurNote(i);
         noteLines.append(String::formatted("%d \"", (int)n), 5);
         noteLines.append(nn, SEQ_MAX_NOTELABEL_LEN);
         noteLines.append("\"\n", 2);
      }
      noteFile.replaceWithText(noteLines);
   }

   break;
   case SEQ_FILE_LOAD_NOTES:
   {
      StringArray noteLines;
      File noteFile(mFileChooser.mFileName);
      noteFile.readLines(noteLines);
      for (int i = 0; i < SEQ_MAX_ROWS - 1; i++)
      {
         StringArray pair;
         String theName;
         int nval;
         if (noteLines.size() > i)
         {
            pair = StringArray::fromTokens(noteLines[i], true);
            nval = pair[0].getIntValue();
            theName = pair[1].removeCharacters("\"");
            sl->setNoteName(i, theName.getCharPointer());
            if (nval < 128 && nval > -1)
               sl->setNote(i, (char)nval, true);
         }
      }
      mGlob.mSeqBuf->swap(); // since we may have called setNote
   }
   break;
   case SEQ_FILE_LOAD_MIDI:
      loadMidiGroove(mFileChooser.mFileName);
      break;
   case SEQ_FILE_SAVE_MIDI:
      saveMidiGroove(mFileChooser.mFileName);
      break;
   case SEQ_FILE_LOAD_PATCH:
      loadPatch(mFileChooser.mFileName);
      break;
   case SEQ_FILE_SAVE_PATCH:
      savePatch(mFileChooser.mFileName);
      break;
   default:
      jassertfalse;
   }
   updateUI();
}

void SeqAudioProcessorEditor::mouseWheelMove(const MouseEvent &, const MouseWheelDetails &wheel)
{
   EditorState &e = *mGlob.mEditorState;
   SequenceLayer *sl = mGlob.mSeqBuf->getUISeqData()->getLayer(e.getCurrentLayer());
   /* see https://forum.juce.com/t/what-is-the-scaling-of-mousewheeldetails-deltay/18582
   in particular:
   Windows  120/256 — (see juce_win32_Windowing.cpp:1868)
   Linux  50/256 — see juce_linux_Windowing.cpp:2213)
   */

   // we seem to get a separate event every half click
   int numrows = sl->getMaxRows();
   if (numrows > SEQ_MAX_VISIBLE_ROWS)
   { // don't mess with it if there is no scrollbar
      double clicks = wheel.deltaY * 256.0 / 120.0;
      // get the value relative to rows
      double cur = mStepScrollbar.getCurrentRangeStart() * numrows;
      // apply the delta
      cur -= clicks;
      // turn back to a fraction
      cur /= numrows;

      mStepScrollbar.setCurrentRangeStart(cur, juce::sendNotification);
   }
}

void SeqAudioProcessorEditor::scrollBarMoved(ScrollBar *sb, double newRangeStart)
{
   EditorState &e = *mGlob.mEditorState;
   SequenceLayer *sl = mGlob.mSeqBuf->getUISeqData()->getLayer(e.getCurrentLayer());
   if (sb == &mStepScrollbar)
   {
      int y = -roundToInt(newRangeStart * sl->getMaxRows()) * SEQ_SIZE_CELL_H;
      mNotePanel.setTopLeftPosition(Point<int>(0, y));
      int x = mStepPanel.getPosition().getX();
      mStepPanel.setTopLeftPosition(Point<int>(x, y));
   }
   else
   { // horizontal scroll
      // determine which step should be leftmost
      e.setVisibleStep(roundToInt(newRangeStart * sl->getNumSteps()));
      // reposition or hide the length editing cursor as appropriate
      mStepPanel.moveLengthEditCursor();
      setStepRangeVisible();
      setSectionSelectItems(); // higlight or unhighlight one of the page change buttons
      repaint();
   }
}

void SeqAudioProcessorEditor::loadMidiGroove(const String &filename)
{
   SeqDataBuffer *sd = mGlob.mSeqBuf;
   SequenceData *s = sd->getUISeqData();

   // changing groove value resets swing to 0
   s->setSwing(0);

   short tpqn;
   setAlertText("");
   File f(filename);
   std::unique_ptr<FileInputStream> stream = f.createInputStream();
   if (!stream)
   {
      setAlertText("Failed to open MIDI file for reading");
      return;
   }

   MidiFile file;
   if (!file.readFrom(*stream) || file.getNumTracks() == 0)
   {
      setAlertText("Failed to read data from file. Possibly it is not a MIDI file, or it has no valid tracks.");
      return;
   }

   tpqn = file.getTimeFormat();
   if (tpqn < 0)
   {
      setAlertText("MIDI file is wrong time format (must be ticks-per-quarter-note format)");
      return;
   }

   // determine how many ticks per 16th note (step)
   int tps = tpqn / 4;
   int numtracks = file.getNumTracks();

   // loop thru all data in all tracks, extracting note-on events.
   // this will effectively mean that the groove is whatever the last
   // note-on events were seen
   for (int curtrack = 0; curtrack < numtracks; curtrack++)
   {
      const MidiMessageSequence *seq = file.getTrack(curtrack);
      int len = seq->getNumEvents();
      for (int i = 0; i < len; i++)
      {
         const MidiMessageSequence::MidiEventHolder *e = seq->getEventPointer(i);
         if (e->message.isNoteOn())
         {
            // determine what step position it has an affinity for
            double ts = seq->getEventTime(i); // in ticks
            int step;
            int step_before = (int)(ts / tps); // step before
            int distance = (int)ts % tps;      // distance to the right of that step
            if (distance <= tps / 2)
            {
               step = step_before; // closer to the step before
            }
            else
            {
               step = step_before + 1; // closer to the step after
               distance -= tps;        // convert to a negative number representing the distance
            }

            step %= SEQ_DEFAULT_NUM_STEPS; // should be range of 0..15
            distance = juce::roundToInt((double)distance / (double)tps * 100.0);
            // distance will always be in the range -50 to 50

            s->setGroove(step, distance);
         }
      }
   }

   sd->swap();
}

void SeqAudioProcessorEditor::saveMidiGroove(const String &filename)
{
   SeqDataBuffer *sd = mGlob.mSeqBuf;
   SequenceData *s = sd->getUISeqData();

   setAlertText("");
   File f(filename);
   if (!f.deleteFile())
   {
      setAlertText("Failed to open MIDI file for writing");
      return;
   }
   std::unique_ptr<FileOutputStream> stream = f.createOutputStream();
   if (!stream)
   {
      setAlertText("Failed to open MIDI file for writing");
      return;
   }

   MidiMessage msgToPutInFile;
   MidiMessageSequence seq;
   MidiFile file;
   file.setTicksPerQuarterNote(960);

   // ticks per step
   int tps = 960 / 4;
   int pos = 0;
   // loop thru groove items twice so that the second time thru
   // we can get a negative groove on step 1 if it exists
   for (int i = 0; i < SEQ_DEFAULT_NUM_STEPS * 2; i++)
   {
      int grv = s->getGroove(i % SEQ_DEFAULT_NUM_STEPS);

      if (i)
      { // all except first item (which will be at 0 for the first measure)
         // initial position for new note
         pos = i * tps;
         // apply groove
         pos += ((grv * tps) / 100);
         // prevent ambiguity as to which step when loading back up
         if (grv == -50)
            pos++;
         else if (grv == 50)
            pos--;

         // note off for old item
         msgToPutInFile = MidiMessage::noteOff(1, i - 1, (uint8)0);
         msgToPutInFile.setTimeStamp(pos);
         seq.addEvent(msgToPutInFile);
      }
      msgToPutInFile = MidiMessage::noteOn(1, i, (uint8)127);
      msgToPutInFile.setTimeStamp(pos);
      seq.addEvent(msgToPutInFile);
   }

   file.addTrack(seq);
   if (!file.writeTo(*stream))
   {
      setAlertText("Failed to write to file");
   }
}

void SeqAudioProcessorEditor::editorShown(Label *lbl, TextEditor &te)
{
   int ml = 0;
   if (lbl == &mLblLayerName)
      ml = SEQ_LAYER_NAME_MAXLEN;
   else if (lbl == &mLblPatternName)
      ml = SEQ_PATTERN_NAME_MAXLEN;
   fixDynamicTextEditBox(mGlob.mEditorState, te, ml);
}

void SeqAudioProcessorEditor::labelTextChanged(Label *labelThatHasChanged)
{
   SeqDataBuffer *sd = mGlob.mSeqBuf;
   SequenceData *s = sd->getUISeqData();
   SequenceLayer *lay = s->getLayer(mGlob.mEditorState->getCurrentLayer());
   String txt = labelThatHasChanged->getText();

   if (labelThatHasChanged == &mLblLayerName)
   {
      lay->setLayerName(txt.getCharPointer());
   }
   else if (labelThatHasChanged == &mLblPatternName)
   {
      // since the ui intertwines layers and pattens, change name on all layers
      for (int i = 0; i < SEQ_MAX_LAYERS; i++)
      {
         lay = s->getLayer(i);
         lay->setPatternName(txt.getCharPointer());
      }
   }
   sd->swap();
}

void SeqAudioProcessorEditor::loadPatch(const String &fn)
{

   SeqPersist persist;
   std::unique_ptr<XmlElement> xml = XmlDocument::parse(File(fn));
   setAlertText("");
   if (!xml)
   {
      setAlertText("Failed to open/read file");
      return;
   }

   if (persist.retrieve(mGlob.mSeqBuf->getUISeqData(), xml.get()))
   {
      mGlob.mSeqBuf->swap();
      // for standalone mode, tempo may be different in what we are loading. send notification
      mGlob.mProcessNotify->addToFifo(SEQ_STANDALONE_SET_TEMPO, 0, 0);
   }
   else
      setAlertText("Failed to read file. May be wrong format, or wrong version.");
}

void SeqAudioProcessorEditor::savePatch(const String &fn)
{
   setAlertText("");

   SeqPersist persist;
   const XmlElement &xml = persist.store(mGlob.mSeqBuf->getAudSeqData());

   if (!xml.writeTo(File(fn).withFileExtension("stochas"), juce::XmlElement::TextFormat()))
      setAlertText("Failed to write to file");
}

void SeqAudioProcessorEditor::setAlertText(const String &txt)
{
   mHelpBanner.setAlert(txt);
   if (txt.length()) // make it go away after some time
      startTimer(2, 30000);
}

void SeqAudioProcessorEditor::timerCallback(int timerID)
{
   switch (timerID)
   {
   case 0:
      mainTimer();
      break;
   case 2: // clear help text
      mHelpBanner.setAlert("");
      break;
   case 3: // remove ui size panic button
      mUISizePanic.setVisible(false);
      break;
   }
}
void SeqAudioProcessorEditor::checkForRecordedNotes()
{
   char prob;
   int midiRecNum, midiRecVel, midiRecLen, midiRecPos;
   SeqDataBuffer *sd = mGlob.mSeqBuf;
   SequenceData *s = sd->getUISeqData();
   SequenceLayer *lyr = s->getLayer(mGlob.mEditorState->getCurrentLayer());
   int row;
   bool changed = false;
   prob = mGlob.mEditorState->getDefaultProbability(lyr->isMonoMode());

   while (mGlob.mAudNotify->getCompletedMidiNote(&midiRecNum, &midiRecVel, &midiRecLen, &midiRecPos))
   {
      // determine if note falls into active row (todo: getRowForNote is inefficient function!)
      row = lyr->getRowForNote((char)midiRecNum);
      if (row != -1)
      {
         lyr->setProb(row, midiRecPos, prob);
         lyr->setLength(row, midiRecPos, (char)midiRecLen);
         lyr->setVel(row, midiRecPos, (char)midiRecVel);
         changed = true;
      }
   }

   if (changed)
      sd->swap();
}

void SeqAudioProcessorEditor::mainTimer()
{
   // this timer is running at 30hz

   char type, chan, num, val;

   // tell the play panel to see whether one of the play lights needs to be lit
   mPlayPanel.check();

   // current pattern that's playing
   mPatPlayPanel.check();

   // mute state on layers
   setMuteUnmuteLayers();

   // if we have recorded incoming midi notes, update the layer's current pattern
   checkForRecordedNotes();

   // tell the step panel to see if it needs to update
   // the grid to reflect which steps have played and also show new midi notes
   // that may have been recorded (will update when step position changes)
   mStepPanel.check();

   // check to see whether record state has changed
   SeqProcessorNotifier::PlayRecordState rs = mGlob.mAudNotify->getRecordingState();
   if (rs != mRecStateCache)
   {
      mRecStateCache = rs;
      switch (rs)
      {
      case SeqProcessorNotifier::on:
         mRecordBtn.setText("Recording");
         mRecordBtn.overrideColor(mGlob.mEditorState->getColorFor(EditorState::recordActive));
         break;
      case SeqProcessorNotifier::off:
         mRecordBtn.setText("Record");
         mRecordBtn.overrideColor(mGlob.mEditorState->getColorFor(EditorState::recordPassive));
         break;
      case SeqProcessorNotifier::standby:
         mRecordBtn.setText("Standby");
         mRecordBtn.overrideColor(mGlob.mEditorState->getColorFor(EditorState::recordActive));
         break;
      }
   }

   rs = mGlob.mAudNotify->getPlaybackState();
   if (rs != mPlayStateCache)
   {
      mPlayStateCache = rs;
      switch (rs)
      {
      case SeqProcessorNotifier::on:
         mPlayBtn.setText("Playing");
         mPlayBtn.overrideColor(mGlob.mEditorState->getColorFor(EditorState::playIndicatorOn));
         break;
      case SeqProcessorNotifier::off:
         mPlayBtn.setText("Play");
         mPlayBtn.overrideColor(mGlob.mEditorState->getColorFor(EditorState::playIndicatorOff));
         break;
      case SeqProcessorNotifier::standby:
         mPlayBtn.setText("Standby");
         mPlayBtn.overrideColor(mGlob.mEditorState->getColorFor(EditorState::playIndicatorOn));
         break;
      }
   }

   // was a midi event received?
   // if so tell the indicator to go on and handle it also.
   // otherwise countdown to it's going off
   if (mGlob.mAudNotify->getMidiEventOccurred(&type, &chan, &num, &val))
   {
      if (!mMidiLightCountDown)
      {
         mMidiIndicator.setOn(true);
      }
      mMidiLightCountDown = 5;
      mMidiDlg.midiMsgReceived(type, chan, num, val);
   }
   else
   {
      if (mMidiLightCountDown)
      {
         mMidiLightCountDown--;
         if (!mMidiLightCountDown)
            mMidiIndicator.setOn(false);
      }
   }

   // see if mouse help text, etc needs updating
   // current playing pattern, etc
   // we will only do this about 3 times per second
   if (mTimeDivider > 10)
   {
      Component *cpt;
      Point<int> pt;
      pt = getMouseXYRelative();
      cpt = getComponentAt(pt);
      if (cpt)
         mHelpBanner.lookupAndSetText(cpt);

      // general ui need update?
      if (mGlob.mAudNotify->doesUINeedUpdate())
      {
         updateUI();
         repaint();
      }

      mTimeDivider = 0;
   }
   else
   {
      mTimeDivider++;
   }
}

void SeqAudioProcessorEditor::updateUI()
{
   SequenceLayer *lay = mGlob.mSeqBuf->getUISeqData()->getLayer(mGlob.mEditorState->getCurrentLayer());
   double bpm = mGlob.mSeqBuf->getUISeqData()->getStandaloneBPM();
   mLblLayerName.setText(lay->getLayerName(), juce::dontSendNotification);
   mLblPatternName.setText(lay->getPatternName(), juce::dontSendNotification);
   // at some point we could make this higher res
   mBPM.setValue((int)bpm, juce::dontSendNotification);

   switch (mGlob.mEditorState->getEditMode())
   {
   case EditorState::editingSteps:
      mStepPanel.setName("stepPanelStepMode");
      break;
   case EditorState::editingChain:
      mStepPanel.setName("stepPanelChainMode");
      break;
   case EditorState::editingVelocity:
      mStepPanel.setName("stepPanelVeloMode");
      break;
   case EditorState::editingOffset:
      mStepPanel.setName("stepPanelOffsMode");
      break;
   default:
      jassertfalse;
      break;
   }
   mMainTabs.setTabName(0,
                        String::formatted("Layer %d Options", mGlob.mEditorState->getCurrentLayer() + 1));

   mOptionsPanel.refreshAll(); // populate controls from current data
   mNotePanel.refreshAll();
   mTabSettings.refreshAll();
   mPlayPanel.refreshAll(); // reconfigure number of steps visible
   refreshPatchOptions();

   // initialize groove panel with current values
   setGrooveItems();
   // ensure that the buttons that let you select a visible range of steps is accurate for current
   // number of steps
   setSectionSelectItems();
   // ensure valid steps are visible depending on which range of steps we are looking at
   setStepRangeVisible();

   // make sure pattern select button reflects the current pattern
   mPatternSelect.setCurrentItem(mGlob.mSeqBuf->getUISeqData()->getCurrentPattern() + 1, true, false);
}

void SeqAudioProcessorEditor::clearGrooveOrCopySwing()
{
   SeqDataBuffer *sd = mGlob.mSeqBuf;
   SequenceData *s = sd->getUISeqData();
   // if sw is non zero, we have a swing value, therefore the button
   // will be for copying swing into groove. Otherwise it will be for clearing groove
   int sw = s->getSwing();
   if (sw)
   { // copy swing into groove was clicked
      for (int i = 0; i < SEQ_DEFAULT_NUM_STEPS; i++)
      {
         if (i % 2 == 1)
         {
            s->setGroove(i, sw);
         }
         else
         {
            s->setGroove(i, 0);
         }
      }
      s->setSwing(0);
   }
   else
   { // clear groove was clicked
      s->clearGroove();
   }
   sd->swap();
}

void SeqAudioProcessorEditor::refreshPatchOptions()
{
   SeqDataBuffer *sd = mGlob.mSeqBuf;
   SequenceData *s = sd->getUISeqData();
   int val;
   val = s->getMidiRespond();
   switch (val)
   {
   case SEQ_MIDI_RESPOND_NO:
      mMidiRespond.setCurrentItem(SEQCTL_MIDI_RESPOND_NO, true, false);
      mMidiMap.setVisible(false);
      break;
   case SEQ_MIDI_RESPOND_YES:
      mMidiRespond.setCurrentItem(SEQCTL_MIDI_RESPOND_YES, true, false);
      mMidiMap.setVisible(true);
      break;
   default:
      jassertfalse;
   }

   val = s->getMidiPassthru();
   switch (val)
   {
   case SEQ_MIDI_PASSTHRU_NONE:
      mMidiPass.setCurrentItem(SEQCTL_MIDI_PASSTHRU_NONE, true, false);
      break;
   case SEQ_MIDI_PASSTHRU_UNHANDLED:
      mMidiPass.setCurrentItem(SEQCTL_MIDI_PASSTHRU_UNHND, true, false);
      break;
   case SEQ_MIDI_PASSTHRU_ALL:
      mMidiPass.setCurrentItem(SEQCTL_MIDI_PASSTHRU_ALL, true, false);
      break;
   default:
      jassertfalse;
      break;
   }

   val = s->getAutoPlayMode();
   switch (val)
   {
   case SEQ_PLAYMODE_AUTO:
      mPlaybackMode.setCurrentItem(SEQCTL_PLAYBACK_MODE_AUTO, true, false);
      mPlayBtn.setVisible(false);
      break;
   case SEQ_PLAYMODE_INSTANT:
      mPlaybackMode.setCurrentItem(SEQCTL_PLAYBACK_MODE_INSTANT, true, false);
      mPlayBtn.setVisible(true);
      break;
   case SEQ_PLAYMODE_STEP:
      mPlaybackMode.setCurrentItem(SEQCTL_PLAYBACK_MODE_Q_STEP, true, false);
      mPlayBtn.setVisible(true);
      break;
   case SEQ_PLAYMODE_BEAT:
      mPlaybackMode.setCurrentItem(SEQCTL_PLAYBACK_MODE_Q_BEAT, true, false);
      mPlayBtn.setVisible(true);
      break;
   case SEQ_PLAYMODE_MEASURE:
      mPlaybackMode.setCurrentItem(SEQCTL_PLAYBACK_MODE_Q_MEAS, true, false);
      mPlayBtn.setVisible(true);
      break;
   default:
      jassertfalse;
   }

   int64 val64 = s->getRandomSeed();
   if (!val64)
   {
      // a value of 0 means generate a new seed every time playback starts
      mRandomToggle.setCurrentItem(SEQCTL_RANDOM_TOGGLE_VARYING, true, false);
   }
   else
   {
      // the value is set to something other than zero
      mRandomToggle.setCurrentItem(SEQCTL_RANDOM_TOGGLE_STABLE, true, false);
   }
}

void SeqAudioProcessorEditor::setMuteUnmuteLayers()
{
   // trying to avoid unnecessary repaints here
   bool m, h;
   for (int i = 0; i < SEQ_MAX_LAYERS; i++)
   {
      m = mGlob.mAudNotify->getMuteState(i);
      h = mLayerToggle.hasLabel(i);
      if (m && !h)
         mLayerToggle.setLabel(i, "M");
      else if (!m && h)
         mLayerToggle.clearLabel(i);
   }
}

void SeqAudioProcessorEditor::actionListenerCallback(const String &message)
{
   // there has got to be an easier way to do this...
   StringArray tokens;
   String type;
   tokens.addTokens(message, "|", "");
   if (tokens.size())
   {
      type = tokens[0];
   }

   // this listens for async events
   if (type.compare("chainAdd") == 0)
   {
      mChainDialog.doSetup(tokens[1].getIntValue(), tokens[2].getIntValue(),
                           tokens[3].getIntValue(), tokens[4].getIntValue());
      mChainDialog.openDialog();
   }
}
