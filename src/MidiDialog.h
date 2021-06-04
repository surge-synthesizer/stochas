/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef MIDIDIALOG_H_
#define MIDIDIALOG_H_

#include "EditorState.h"
#include "Constants.h"
#include "CommonComponents.h"


// widths
#define SEQMIDI_WIDTH_DEL     20
#define SEQMIDI_WIDTH_ACTION  130
#define SEQMIDI_WIDTH_TARGET  130
#define SEQMIDI_WIDTH_VALUE   130
#define SEQMIDI_WIDTH_CHAN    40
#define SEQMIDI_WIDTH_NOTE    70

// cpt id's (some might be unused)
//#define SEQMIDI_CPT_ACTION    1
//#define SEQMIDI_CPT_TARGET    2
//#define SEQMIDI_CPT_VALUE     3
#define SEQMIDI_CPT_CHAN      4
#define SEQMIDI_CPT_NOTE      5
//#define SEQMIDI_CPT_DELETE    6
#define SEQMIDI_CPT_LEARN     7

class SeqMidiDialog;

// this is the ui for a single row of the list box
class SeqMidiRow : public Component, public ComboBox::Listener, 
   public CptNotify, public Button::Listener,
   public NumberCpt::CustomText {
   SeqGlob *mGlob;
   SeqMidiDialog *mDlg;
   int mRowNumber; // currently represented row number
   ImageButton mBtnDelete;
   ToggleCpt mBtnLearn;
   ComboBox mCBAction;
   ComboBox mCBTarget;
   ComboBox mCBValue;
   NumberCpt mNumChannel;
   NumberCpt mNumNote;  
   // for our own cpts
   void cptValueChange(int cptId, int value) override;
   // for combo boxes
   void comboBoxChanged(ComboBox * comboBox) override;
   // for delete button
   void buttonClicked(Button* btn) override;
   void resized() override;
   void addActionsToCombo(ComboBox &cb, bool addall=true);
   void fillValueListBasedOnAction();
   void setComboColor(ComboBox *cb);

   // implement customtext for numbercpt
   bool getNumberCptCustomText(int id, int value, String &repl) override;

public:
   SeqMidiRow(SeqGlob *glob, SeqMidiDialog *dlg);
   // this refreshes the row with data from the data array corresponding to that row.
   // note that the row number needs to be a valid row
   void setRowNumber(int val);      
};

class SeqMidiDialog : public SeqModalDialog, public ListBoxModel {
   friend class SeqMidiRow;
   std::unique_ptr<ListBox> mTable;
   std::unique_ptr<TextButton> mBtnAdd;
   std::unique_ptr<TextButton> mBtnClose;
   std::unique_ptr<TextButton> mBtnClear;
   std::unique_ptr<TextButton> mBtnReset;
   std::unique_ptr<Label> mLblAction;
   std::unique_ptr<Label> mLblTarget;
   std::unique_ptr<Label> mLblValue;
   std::unique_ptr<Label> mLblChan;
   std::unique_ptr<Label> mLblNote;
   std::unique_ptr<Label> mLblHeader;
   SeqGlob *mGlob;
   int mLearningRow;

   // Store working data
   // note that the interpretation of mNote is different in the dialog from the sequence data
   // in the dialog its -128 to 127 with negative values representing note-off
   // it's adjusted coming in and out
   Array<SeqMidiMapItem> mMapping;

   // implement SeqModalDialog...
   void notify(int id, int val) override;
   void resizedInner(Component *inner) override;
   void endDialog(bool hitOk) override;
   
   // implement ListBoxModel...
   int getNumRows() override;
   void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, 
      bool rowIsSelected) override;
   Component* refreshComponentForRow(int rowNumber, bool isRowSelected, 
      Component* existingComponentToUpdate) override;
   void refreshAll();
   // add standard actions to combo
   
 public:
   // call this when the dialog comes up
   // copy current sequence data to local (the reverse happens in endDialog)
   void seqDataToLocal();
   // a midi msg is received (are we learning?)
   void midiMsgReceived(int8_t type, int8_t chan, int8_t num, int8_t val);
   

   SeqMidiDialog(SeqGlob *glob, CptNotify *parent);
   ~SeqMidiDialog();
};

#endif
