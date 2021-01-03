/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "MidiDialog.h"
#include "Scale.h"

SeqMidiDialog::SeqMidiDialog(SeqGlob * glob, CptNotify * parent) :
   SeqModalDialog(glob, 666, parent, 600, 350), mGlob(glob), mLearningRow(-1)
{


   mTable = std::unique_ptr<ListBox>(new ListBox("tblMidi", this));

   Colour c = mGlob->mEditorState->getColorFor(EditorState::background);
   mTable->setColour(juce::TableListBox::ColourIds::backgroundColourId, c);
   addToInner(0, *mTable);
   mTable->setVisible(true);
   mTable->setRowHeight(25);
   mTable->setColour(ListBox::outlineColourId, mGlob->mEditorState->getColorFor(EditorState::border));
   mTable->setOutlineThickness(1);

   mLblAction = std::unique_ptr<Label>(addStdLabel("Action"));
   mLblTarget = std::unique_ptr<Label>(addStdLabel("Target"));
   mLblValue = std::unique_ptr<Label>(addStdLabel("Value"));
   mLblChan = std::unique_ptr<Label>(addStdLabel("Chan"));
   mLblNote = std::unique_ptr<Label>(addStdLabel("Msg"));
   mLblHeader = std::unique_ptr<Label>(addStdLabel("MIDI Mapping"));
   mLblHeader->setFont(Font(20.0, Font::plain));
   mLblHeader->setJustificationType(Justification::centred);

   mBtnClose = std::unique_ptr<TextButton>(addStdButton("Close", nullptr, SEQCTL_MIDI_OK));
   mBtnClear = std::unique_ptr<TextButton>(addStdButton("Clear", nullptr, SEQCTL_MIDI_CLEAR));
   mBtnReset = std::unique_ptr<TextButton>(addStdButton("Reset to Default", nullptr, SEQCTL_MIDI_RESET));
   mBtnAdd = std::unique_ptr<TextButton>(addStdButton("Add New", nullptr, SEQCTL_MIDI_ADD));

}

SeqMidiDialog::~SeqMidiDialog() {

}


int SeqMidiDialog::getNumRows()
{   
   return mMapping.size();
}

void SeqMidiDialog::paintListBoxItem(int , Graphics & g, int , int , bool )
{
   Colour sel, bg;
   bg = mGlob->mEditorState->getColorFor(EditorState::background);
   g.setColour(bg);
   g.fillAll();
}

Component * SeqMidiDialog::refreshComponentForRow(int rowNumber, bool , 
   Component * existingComponentToUpdate)
{
   SeqMidiRow *row = nullptr;
   // if this is null, we need to create one (which the model takes ownership of)
   if (existingComponentToUpdate == nullptr) {
      row = new SeqMidiRow(mGlob, this);
      existingComponentToUpdate = row;
   }
   else
      row = (SeqMidiRow*)existingComponentToUpdate;

   // update it with current row's content if it's a valid row
   if (rowNumber < mMapping.size()) {
      row->setRowNumber(rowNumber);
   }

   return existingComponentToUpdate;
   
}

void SeqMidiDialog::refreshAll()
{
   if (mMapping.size() >= SEQMIDI_MAX_ITEMS)
      mBtnAdd->setEnabled(false);
   else
      mBtnAdd->setEnabled(true);
   mTable->updateContent();
   //repaint();
}




void SeqMidiDialog::endDialog(bool ) {
   // gather mMapping back to sequence data (there is no ok button)
   SequenceData *sd = mGlob->mSeqBuf->getUISeqData();   
   int len = mMapping.size();
   int targetind = 0;
   sd->clearMapping();
   SeqMidiMapItem *mi;
   SeqMidiMapItem curmi;
   for (int i = 0; i < len; i++) {
      curmi = mMapping.getUnchecked(i);
      // only add valid ones
      if (curmi.mAction != SEQMIDI_ACTION_INVALID) {
         mi = sd->getMappingItem(targetind);
         // values < 0 represent note off, so adjust
         if (curmi.mNote < 0) {
            curmi.mNote += 128;
            curmi.mType = SEQ_MIDI_NOTEOFF;
         }
         *mi = curmi;
         targetind++;
      }      
   }
   sd->setMappingCount(targetind);
   mGlob->mSeqBuf->swap();

   // tel the processor that it needs to refresh midi map data
   mGlob->mProcessNotify->addToFifo(SEQ_REFRESH_MAP_MSG, 0, 0);

   mLearningRow = -1;
}

// this is called when one of the main dialog buttons are pressed
void SeqMidiDialog::notify(int id, int ) 
{
   mLearningRow = -1;

   switch (id) {
   case SEQCTL_MIDI_OK:
      closeDialog(true);
      break;
   case SEQCTL_MIDI_CLEAR:
      mMapping.clear();
      refreshAll();
      break;
   case SEQCTL_MIDI_RESET:
      // reset to some default state
   {
      mMapping.clear();
      SeqMidiMapItem mi;
      for (int i = 0; i < SEQMIDI_NUM_DEFAULT_ITEMS; i++) {
         mi=gDefaultMidiMapItems[i];
         mMapping.add(mi);
      }
      refreshAll();
      break;
   }
      
   case SEQCTL_MIDI_ADD:
      if (mMapping.size() < SEQMIDI_MAX_ITEMS) {
         SeqMidiMapItem mi;
         mi.mType = SEQ_MIDI_NOTEON; // default type unless bias is selected
         mMapping.add(mi);
      }
      refreshAll();
      mTable->scrollToEnsureRowIsOnscreen(mMapping.size() - 1);
      break;
   default:
      jassertfalse;
   }
   
}

void SeqMidiDialog::resizedInner(Component * inner) {
   Rectangle<int> r = inner->getLocalBounds();
   r.reduce(SEQ_SIZE_MAIN_BORDER, SEQ_SIZE_MAIN_BORDER);
   Rectangle<int>bottom, b, header;
   header = r.removeFromTop(30);
   mLblHeader->setBounds(header);
   header = r.removeFromTop(30);
   bottom = r.removeFromBottom(30);
   bottom.removeFromTop(SEQ_SIZE_PROP_VSPACE);
   mTable->setBounds(r);

   b=bottom.removeFromRight(100);
   b.removeFromLeft(SEQ_SIZE_PROP_VSPACE);   
   mBtnClose->setBounds(b);

   b = bottom.removeFromRight(100);
   b.removeFromLeft(SEQ_SIZE_PROP_VSPACE);
   mBtnClear->setBounds(b);
   
   b = bottom.removeFromRight(130);
   b.removeFromLeft(SEQ_SIZE_PROP_VSPACE);
   mBtnReset->setBounds(b);
   
   b = bottom.removeFromRight(100);
   b.removeFromLeft(SEQ_SIZE_PROP_VSPACE);
   mBtnAdd->setBounds(b);

   Rectangle<int> tmp;
   r = header;
   r.reduce(1, 1);
   tmp = r.removeFromLeft(SEQMIDI_WIDTH_DEL);
   r.removeFromLeft(2);
   
   tmp = r.removeFromLeft(SEQMIDI_WIDTH_ACTION);
   mLblAction->setBounds(tmp);
   r.removeFromLeft(2);
   tmp = r.removeFromLeft(SEQMIDI_WIDTH_TARGET);
   mLblTarget->setBounds(tmp);
   r.removeFromLeft(2);
   tmp = r.removeFromLeft(SEQMIDI_WIDTH_VALUE);
   mLblValue->setBounds(tmp);
   r.removeFromLeft(2);
   tmp = r.removeFromLeft(SEQMIDI_WIDTH_CHAN);
   mLblChan->setBounds(tmp);
   r.removeFromLeft(2);
   tmp = r.removeFromLeft(SEQMIDI_WIDTH_NOTE);
   mLblNote->setBounds(tmp);
   


}

void SeqMidiDialog::seqDataToLocal()
{
   SequenceData *sd = mGlob->mSeqBuf->getUISeqData();
   // read seqdata to local. seqdata should be valid
   int len=sd->getMappingCount();
   mMapping.clear();
   SeqMidiMapItem *mi;
   for (int i = 0; i < len; i++) {
      mi = sd->getMappingItem(i);
      if (mi->mType == SEQ_MIDI_NOTEOFF) {
         mi->mNote -= 128;
         mi->mType = SEQ_MIDI_NOTEON; // probably not needed
      }
      mMapping.add(*mi);
   }

   refreshAll();
}

void SeqMidiDialog::midiMsgReceived(char type, char chan, char num, char /*val*/)
{
   // we only care about SEQ_MIDI_NOTEON/OFF or SEQ_MIDI_CC
   if (type != SEQ_MIDI_NOTEON && type != SEQ_MIDI_NOTEOFF && type != SEQ_MIDI_CC)
      return;

   if (mLearningRow!=-1) {
      SeqMidiMapItem mi;
      mi = mMapping.getUnchecked(mLearningRow);

      // using a CC for a non-cc type action is not accepted
      // using a note for a cc-type is not accepted either
      if((mi.mAction == SEQMIDI_ACTION_PBIAS && type ==SEQ_MIDI_CC) ||
         (mi.mAction != SEQMIDI_ACTION_PBIAS && (type == SEQ_MIDI_NOTEON||type==SEQ_MIDI_NOTEOFF))) {

         mi.mChannel = chan;         
         if (type == SEQ_MIDI_NOTEOFF) {
            type = SEQ_MIDI_NOTEON; // probably not needed
            num -= 128;
         } 
         mi.mNote = num;
         mi.mType = type;
         mMapping.setUnchecked(mLearningRow, mi);
         mLearningRow = -1;
         refreshAll();
      }
      
   }
   
}




////////////////////////////////////////////////////////////////////////////////////////
/// MIDI ROW STUFF /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
SeqMidiRow::SeqMidiRow(SeqGlob * glob, SeqMidiDialog * dlg) :
   mGlob(glob),
   mDlg(dlg),
   mRowNumber(-1),
   mBtnLearn(glob, SEQMIDI_CPT_LEARN, this, "midiDlgLearn"),
   mNumChannel(glob, SEQMIDI_CPT_CHAN, this, "midiDlgChan"),
   mNumNote(glob, SEQMIDI_CPT_NOTE, this, "midiDlgNote") // this will change to our new note cpt
{

   juce::Colour c = mGlob->mEditorState->getColorFor(EditorState::button);
   juce::Colour cd = mGlob->mEditorState->getColorFor(EditorState::toggleOn);

   //
   // Delete button
   // (see setRowNumber)
   //Image img;
   //img = ImageCache::getFromMemory(SeqImageX::xmark464_png, SeqImageX::xmark464_pngSize);
   //mBtnDelete.setImages(false, true, true, img, 1.0, c, img, 1.0, c, img, 1.0, cd);
   mBtnDelete.addListener(this);
   addAndMakeVisible(mBtnDelete);

   //
   // Learn button
   //
   mBtnLearn.addItem(1, "Learn", false);
   addAndMakeVisible(mBtnLearn);

   //
   // Action combo
   //
   mCBAction.addListener(this);
   mCBAction.setTextWhenNothingSelected("Select Action");
   setComboColor(&mCBAction);
   mCBAction.setWantsKeyboardFocus(false);
   addActionsToCombo(mCBAction);   
   addAndMakeVisible(mCBAction);

   // 
   // Target combo (layers 1..4 and all layers)
   //
   mCBTarget.addListener(this);
   mCBTarget.setWantsKeyboardFocus(false);
   for (int i = 0; i<SEQ_MAX_LAYERS; i++)
      mCBTarget.addItem(String::formatted("Layer %d", i + 1), i + 1);
   mCBTarget.addItem("All Layers", SEQMIDI_TARGET_ALL);
   mCBTarget.setTextWhenNoChoicesAvailable("Not Applicable");
   mCBTarget.setTextWhenNothingSelected("Select Target");
   setComboColor(&mCBTarget);
   addAndMakeVisible(mCBTarget);

   //
   // Value combo
   //
   mCBValue.addListener(this);
   mCBValue.setWantsKeyboardFocus(false);
   mCBValue.setTextWhenNothingSelected("Select Value");
   setComboColor(&mCBValue);
   addAndMakeVisible(mCBValue);

   //
   // channel number
   //
   mNumChannel.setSpec(1, 16, 1, 1, "");
   addAndMakeVisible(mNumChannel);

   //
   // Note/cc number
   // (negative values: note off, add 128)
   mNumNote.setSpec(-128, 127, 1, 0, "");
   mNumNote.setCustomText(this);
   addAndMakeVisible(mNumNote);


}


// for delete button

void
SeqMidiRow::buttonClicked(Button * btn) {
   jassert(btn == (Button*)&mBtnDelete);
   jassert(mRowNumber < mDlg->mMapping.size());
   mDlg->mMapping.remove(mRowNumber);
   mDlg->refreshAll();
}

void
SeqMidiRow::resized() {
   Rectangle<int> r = getLocalBounds();
   Rectangle<int> tmp;
   r.reduce(1, 1);
   tmp = r.removeFromLeft(SEQMIDI_WIDTH_DEL);
   mBtnDelete.setBounds(tmp);
   r.removeFromLeft(2);
   tmp = r.removeFromLeft(SEQMIDI_WIDTH_ACTION);
   mCBAction.setBounds(tmp);
   r.removeFromLeft(2);
   tmp = r.removeFromLeft(SEQMIDI_WIDTH_TARGET);
   mCBTarget.setBounds(tmp);
   r.removeFromLeft(2);
   tmp = r.removeFromLeft(SEQMIDI_WIDTH_VALUE);
   mCBValue.setBounds(tmp);
   r.removeFromLeft(2);
   tmp = r.removeFromLeft(SEQMIDI_WIDTH_CHAN);
   mNumChannel.setBounds(tmp);
   r.removeFromLeft(2);
   tmp = r.removeFromLeft(SEQMIDI_WIDTH_NOTE);
   mNumNote.setBounds(tmp);
   mBtnLearn.setBounds(r);
}

void SeqMidiRow::addActionsToCombo(ComboBox & cb, bool addall)
{
   cb.addItem("Change Pattern", SEQMIDI_ACTION_CHGPAT);
   cb.addItem("Mute/Unmute", SEQMIDI_ACTION_MUTE);
   cb.addItem("Set Speed", SEQMIDI_ACTION_SPEED);
   cb.addItem("Transpose", SEQMIDI_ACTION_TRANS);
   cb.addItem("Set Num Steps", SEQMIDI_ACTION_STEPS);
   cb.addItem("Set Poly Bias", SEQMIDI_ACTION_PBIAS);
   cb.addItem("Playback", SEQMIDI_ACTION_PLAYBACK);
   cb.addItem("Record", SEQMIDI_ACTION_RECORD);
   if(addall) // this would not be added when the list is being used as a target
      cb.addItem("Reset Action", SEQMIDI_ACTION_RESET);
   
}


void
SeqMidiRow::cptValueChange(int cptId, int value) {
   
   switch (cptId) {
   case SEQMIDI_CPT_LEARN:
      if (value)
         mDlg->mLearningRow = mRowNumber;
      else
         mDlg->mLearningRow = -1;
      mDlg->refreshAll(); // will turn the others off
      break;
   case SEQMIDI_CPT_CHAN: {
      SeqMidiMapItem m=mDlg->mMapping.getUnchecked(mRowNumber);
      m.mChannel = (char)value;
      mDlg->mMapping.setUnchecked(mRowNumber, m);
      break;
   }
   case SEQMIDI_CPT_NOTE: {
      SeqMidiMapItem m = mDlg->mMapping.getUnchecked(mRowNumber);
      m.mNote = (char)value;
      mDlg->mMapping.setUnchecked(mRowNumber, m);
      break;
   }
   default:
      jassertfalse;
   }

}

void SeqMidiRow::comboBoxChanged(ComboBox * comboBox) {
   if (mRowNumber == -1)
      return;

   SeqMidiMapItem m = mDlg->mMapping.getUnchecked(mRowNumber);
   
   if (comboBox == &mCBAction) {
      // action changed
      // update the action
      m.mAction = (char)(mCBAction.getSelectedId());
      if (m.mAction == SEQMIDI_ACTION_PBIAS) {
         m.mType = SEQ_MIDI_CC;
         m.mValue = SEQMIDI_VALUE_VARIABLE;
         mNumNote.setSpec(0, 127, 1, 0, "");
      }
      else {
         m.mType = SEQ_MIDI_NOTEON;
         // value will not make sense in new context so just set to 0
         m.mValue = 0;
         mNumNote.setSpec(-128, 127, 1, 0, "");
      }
      if (mCBTarget.getSelectedId() == 0) {
         m.mTarget = SEQMIDI_TARGET_ALL;
      }
      mNumNote.repaint(); // in case it's switching to/from cc
      mDlg->mMapping.setUnchecked(mRowNumber, m);
      // force a rework of combos and update other values
      setRowNumber(mRowNumber);

   }
   else if (comboBox == &mCBTarget) {
      // target changed
      m.mTarget = (char)(mCBTarget.getSelectedId());
      mDlg->mMapping.setUnchecked(mRowNumber, m);
      
   }
   else if (comboBox == &mCBValue) {
      // value changed
      m.mValue = (char)(mCBValue.getSelectedId());
      mDlg->mMapping.setUnchecked(mRowNumber, m);

   }
}


void
SeqMidiRow::fillValueListBasedOnAction()
{
   int action = mCBAction.getSelectedId();
   mCBValue.clear();
   mCBValue.setEnabled(true);
   mCBTarget.setEnabled(true);
   switch (action) {
   case SEQMIDI_ACTION_PLAYBACK:
      mCBValue.addItem("Start", SEQMIDI_VALUE_PLAYBACK_START);
      mCBValue.addItem("Stop", SEQMIDI_VALUE_PLAYBACK_STOP);
      mCBValue.addItem("Toggle", SEQMIDI_VALUE_PLAYBACK_TOGGLE);
      mCBTarget.setSelectedId(SEQMIDI_TARGET_ALL, juce::dontSendNotification);
      mCBTarget.setEnabled(false);
      break;
   case SEQMIDI_ACTION_RECORD:
      mCBValue.addItem("Start", SEQMIDI_VALUE_RECORD_START);
      mCBValue.addItem("Stop", SEQMIDI_VALUE_RECORD_STOP);
      mCBValue.addItem("Toggle", SEQMIDI_VALUE_RECORD_TOGGLE);
      mCBTarget.setSelectedId(SEQMIDI_TARGET_ALL, juce::dontSendNotification);
      mCBTarget.setEnabled(false);
      break;
   case SEQMIDI_ACTION_CHGPAT:      
      for (int i = 0; i < SEQ_MAX_PATTERNS; i++) {
         mCBValue.addItem(String::formatted("%d", i + 1), i + 1);
      }
      mCBValue.addItem("Next", SEQMIDI_VALUE_PAT_NEXT);
      mCBValue.addItem("Previous", SEQMIDI_VALUE_PAT_PREV);
      break;
   case SEQMIDI_ACTION_MUTE:
      mCBValue.addItem("Mute", SEQMIDI_VALUE_MUTE_MUTE);
      mCBValue.addItem("Unmute", SEQMIDI_VALUE_MUTE_UNMUTE);
      mCBValue.addItem("Toggle", SEQMIDI_VALUE_MUTE_TOGGLE);
      break;
   case SEQMIDI_ACTION_SPEED:
      for (int i = 0; i < SEQ_NUM_CLOCK_DIVS; i++) {
         const char *val;
         int num;
         val = SeqScale::getClockDividerTextAndId(i, &num);
         mCBValue.addItem(val, num);
      }
      mCBValue.addItem("Double", SEQMIDI_VALUE_SPD_DBL);
      mCBValue.addItem("Half", SEQMIDI_VALUE_SPD_HALF);      
      break;
   case SEQMIDI_ACTION_TRANS:
      for (int id = SEQMIDI_VALUE_TRANSPOSE_MIN; id <= SEQMIDI_VALUE_TRANSPOSE_MAX; id++) {
         mCBValue.addItem(SeqScale::getTransposeText(id), id);
      }
      break;
   case SEQMIDI_ACTION_STEPS:
      for (int i = SEQMIDI_VALUE_NS_MIN; i <= SEQMIDI_VALUE_NS_MAX; i++)
         mCBValue.addItem(String::formatted("%d Steps", i), i);
      mCBValue.addItem(String::formatted("%d Steps", SEQMIDI_VALUE_NS_TWENTYFOUR), SEQMIDI_VALUE_NS_TWENTYFOUR);
      mCBValue.addItem(String::formatted("%d Steps",SEQMIDI_VALUE_NS_TWO), SEQMIDI_VALUE_NS_TWO);
      mCBValue.addItem(String::formatted("%d Steps", SEQMIDI_VALUE_NS_THREE), SEQMIDI_VALUE_NS_THREE);
      mCBValue.addItem(String::formatted("%d Steps", SEQMIDI_VALUE_NS_FOUR), SEQMIDI_VALUE_NS_FOUR);
      
      break;
   case SEQMIDI_ACTION_RESET:
       // add all except the reset action
      addActionsToCombo(mCBValue,false);
      break;
   case SEQMIDI_ACTION_PBIAS:
      // no items
      mCBValue.addItem("CC Value", SEQMIDI_VALUE_VARIABLE);
      mCBValue.setEnabled(false);
      break;
   case 0:
      break; // no item is selected
   default:
      jassertfalse;
   }
}


void 
SeqMidiRow::setRowNumber(int val) 
{
   jassert(val < mDlg->mMapping.size());
   

   mRowNumber = val;
   SeqMidiMapItem m= mDlg->mMapping.getUnchecked(val);
   mCBAction.setSelectedId(m.mAction,juce::dontSendNotification);
   mCBTarget.setSelectedId(m.mTarget, juce::dontSendNotification);
   // value list varies between actions
   fillValueListBasedOnAction();
   mCBValue.setSelectedId(m.mValue, juce::dontSendNotification);

   mNumNote.setValue(m.mNote, false);
   mNumChannel.setValue(m.mChannel, false);
   
   if (mDlg->mLearningRow == val)
      mBtnLearn.setCurrentItem(1, true, false);
   else
      mBtnLearn.setCurrentItem(1, false, false);

   // in case color scheme is changing (via refreshComponentForRow, via updateContent in fixColors common cpt)
   setComboColor(&mCBAction);
   setComboColor(&mCBTarget);
   setComboColor(&mCBValue);

   juce::Colour c = mGlob->mEditorState->getColorFor(EditorState::button);
   juce::Colour cd = mGlob->mEditorState->getColorFor(EditorState::toggleOn);

   //
   // Delete button colors
   //
   Image img;
   img = ImageCache::getFromMemory(SeqImageX::xmark464_png, SeqImageX::xmark464_pngSize);
   mBtnDelete.setImages(false, true, true, img, 1.0, c, img, 1.0, c, img, 1.0, cd);
}

void
SeqMidiRow::setComboColor(ComboBox * cb) {
   juce::Colour bg = mGlob->mEditorState->getColorFor(EditorState::background);
   juce::Colour border = mGlob->mEditorState->getColorFor(EditorState::background);
   juce::Colour fg = bg.contrasting(.5f);
   cb->setColour(ComboBox::ColourIds::backgroundColourId, bg);
   cb->setColour(ComboBox::ColourIds::textColourId, fg);
   cb->setColour(ComboBox::ColourIds::arrowColourId, fg);
   cb->setColour(ComboBox::ColourIds::outlineColourId, border);
}

bool SeqMidiRow::getNumberCptCustomText(int id, int value, String & repl)
{
   // override from NumberCpt to replace the text with something meaningful
   char buf[SEQ_NOTE_NAME_MAXLEN];
   jassert(id == SEQMIDI_CPT_NOTE);
   jassert(value >= -128 && value < 128);
   int action = mCBAction.getSelectedId();
   if (action == SEQMIDI_ACTION_PBIAS) {
      repl << "CC" << value;
   }
   else {
      const char *suff = "";
      if (value < 0) {
         value += 128;
         suff = " (off)";
      }
      int lowOct = mGlob->mEditorState->getLowestOctave();
      repl = SeqScale::getMidiNoteName((char)value, lowOct, buf);
      repl << suff;
   }
   return true;
}


