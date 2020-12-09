/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "PluginProcessor.h"
#include "OptionsPanel.h"
#include "StepPanel.h"
#include "NotePanel.h"
#include "PlayPanel.h"
#include "HelpBanner.h"
#include "CommonComponents.h"
#include "EditorState.h"
#include "Constants.h"
#include "EditDialog.h"
#include "InfoDialog.h"
#include "SettingsTab.h"
#include "MidiDialog.h"
#include "FileDialog.h"
#include "ChainDialog.h"

/*
Main UI Class
*/
class SeqAudioProcessorEditor  : public AudioProcessorEditor, 
   public CptNotify, public MultiTimer, public ScrollBar::Listener,
   public Label::Listener, public ActionListener
{
   LookAndFeel_V3 mLookAndFeel; // maintain the v3 look and feel
   SeqAudioProcessor& mProcessor;
   // ================see the SeqAudioProcessorEditor ctor for more comments on each of these
   SeqGlob mGlob;
   Component mStepHolder;  
   StepPanel mStepPanel;
   Component mNoteHolder;  
   NotePanel mNotePanel;
   Component mPPHolder; 
   ScrollBar mStepScrollbar;
   ScrollBar mStepHScrollbar;
   PlayLightCpt mMidiIndicator;
   PlayPanel mPlayPanel;
   PatternPlayPanel mPatPlayPanel;
   ToggleCpt mEditToggle;  
   Label mEditLabel;       
   ButtonCpt mHelpBtn;
   ButtonCpt mEditBtn;
   ButtonCpt mUndoBtn;
   ButtonCpt mRecordBtn;
   ButtonCpt mPlayBtn;
   ToggleCpt mPatternSelect;
   Label mPatternLabel;
   
   ToggleCpt mSectionSelect;
   SeqTabbedCpt mMainTabs;
   Label mLblLayerName;
   Label mLblPatternName;
   Label mLblBPM; // standalone mode only
   NumberCpt mBPM;  // standalone mode only
   //----------------------------- groove tab
   TabPanelCpt mTabGroove;
   GrooveCpt mGroove;
   ButtonCpt mClearGrooveBtn;
   ButtonCpt  mBtnLoadGroove;
   ButtonCpt  mBtnSaveGroove;
   NumberCpt mGrooveSwing;
   Label mGrooveSwingLabel;
   Label mGrooveGrooveLabel;
   Label mGrooveHelpLabel;

   //-----------------------------settings tab
   SettingsTab mTabSettings;

   //-----------------------------chord tab
   TabPanelCpt mTabChord;
   ToggleCpt mTglChords;
   Label mLblChords;

   //-----------------------------layer options tab
   OptionsPanel mOptionsPanel;

   //-----------------------------patch options tab
   TabPanelCpt mTabPatchOpts;
   Label mLblMidiPass;
   Label mLblPlaybackMode;
   Label mLblMidiRespond;
   ToggleCpt mMidiPass;
   ToggleCpt mMidiRespond;
   ToggleCpt mPlaybackMode;
   ButtonCpt mMidiMap;
   ButtonCpt mBtnLoadPatch;
   ButtonCpt mBtnSavePatch;

   Label mLblRandomization;
   ToggleCpt mRandomToggle;

   

   SeqHelpBanner mHelpBanner;
   ToggleCptWithLabel mLayerToggle;
   Label mLayerLabel;
   ButtonCpt mUISizePanic;          // zorder should be higher than layer label (does this do it?)
   Label mSectionLabel;
   SeqEditDialog mEditDialog;
   SeqInfoDialog mInfoDialog;
   SeqFileDialog mFileChooser;
   SeqChainDialog mChainDialog;

   int mMidiLightCountDown;
   int mTimeDivider;
   // just to avoid excessive repaints
   SeqProcessorNotifier::PlayRecordState mRecStateCache;
   SeqProcessorNotifier::PlayRecordState mPlayStateCache;

   SeqMidiDialog mMidiDlg;
   
   // will be false if this is not the first time they have shown the UI for
   // this instance of the plugin
   bool mFirstTimeEditor;
   // set active chord for painting, -1 is off
   void chordSelect(int id);
   void fixButtonColors();
   void showFileChooser(int mode);
   // will be called when ok is hit on the file chooser
   void respondFileChooser();
   void mouseWheelMove(const MouseEvent &event, const MouseWheelDetails &wheel) override;

   void scrollBarMoved(ScrollBar *scrollBarThatHasMoved, double newRangeStart) override;
   void loadMidiGroove(const String &filename);
   void saveMidiGroove(const String &filename);
   /** Called when a Label goes into editing mode and displays a TextEditor. */
   void editorShown(Label*, TextEditor&) override;
   void labelTextChanged(Label* labelThatHasChanged)  override;
   void loadPatch(const String &fn);
   void savePatch(const String &fn);
   void checkForRecordedNotes();
public:
   // set alert text which stays up for a few seconds
   void setAlertText(const String &txt);
   
    SeqAudioProcessorEditor (SeqAudioProcessor&);
    ~SeqAudioProcessorEditor();
    void paint (Graphics&) override;
    void resized() override;

    // display the correct choices for selecting a range of steps to be displayed.
    // this fixes the toggle to the correct number and sets the toggle values
    void setSectionSelectItems();

    // make sure that the currently selected step range (as selected by the toggle) is visible
    void setStepRangeVisible();

    // make sure the groove reflects whats in sequence data
    void setGrooveItems();

    // used by child components to get to the sequencer data
    inline SeqGlob *getGlob() { return &mGlob; }

    // registered to get any toggle changes on any of the toggles
    void cptValueChange(int cptId, int id) override;

    // for the layer select to mute/solo
    void cptItemClickWithModifier(int cptId, int id, juce::ModifierKeys mods) override;

    // this timer polls to see if anything needs updating on the ui
    void timerCallback(int timerID) override;
    void mainTimer();
// this will ensure that the UI matches the model and there are no invalid
    // selections in the UI. Should be called before a repaint, after something
    // might have changed
    void updateUI();

    // as it says...
    void clearGrooveOrCopySwing();

    // refresh items on the patch tab
    void refreshPatchOptions();

    void setMuteUnmuteLayers();

private:
   // don't know what this is right now so I leave it here
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SeqAudioProcessorEditor)

      // Inherited via ActionListener
      virtual void actionListenerCallback(const String & message) override;
};


// todo remove this at some point
void logMessage(String msg);

#endif  // PLUGINEDITOR_H_INCLUDED
