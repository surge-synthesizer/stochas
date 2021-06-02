/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef NOTEPANEL_H_
#define NOTEPANEL_H_

#include "SequenceData.h"
#include "EditorState.h"
#include "Constants.h"
#include "CommonComponents.h"
#include "StepPanel.h"

/**
This represents a single Note Name cell on the left
*/
class NotePanel;
class NoteCpt : public Component, public Label::Listener /*, public Button::Listener*/{   
   friend class NotePanel;
   NotePanel *mParent;
   // we need to override this solely so that we can set the
   // length restriction on the edit box
   class MyLabel : public Label {
   public:
      NotePanel *mParent;
      virtual void 	editorShown(TextEditor *ed);
   };
   MyLabel mText;

   // override so we can get mouseDown and mouseUp
   class MyImageButton : public ImageButton {
   public:
      MyImageButton(NoteCpt *par) : ImageButton() {
         addMouseListener(par,false);
      }
   };   
   MyImageButton mBtnPlay;
   int mId; // which one it is
   int8_t mTempValue;
   SeqGlob *mGlob; // note that this is not available from the ctor
   void labelTextChanged(Label *labelThatHasChanged) override;
   void resized() override;
   void setupImage();
   void paint(Graphics &g) override;
   // these are used for the button mousedown/mouseup
   void mouseUp(const MouseEvent &event) override;
   void mouseDoubleClick(const MouseEvent &event) override;
   void mouseDown(const MouseEvent &event) override;

   /* This is the text box that comes up when the user double clicks
   on the item. Similar to code in NumberCpt
   */
   class InlineEditor : public TextEditor, public TextEditor::Listener {

      /** Called when the user presses the return key. Accept the edit */
      void textEditorReturnKeyPressed(TextEditor&) override;

      /** Called when the user presses the escape key. Abort the edit */
      void textEditorEscapeKeyPressed(TextEditor&) override;

      /** Called when the text editor loses focus. Accept the edit */
      void textEditorFocusLost(TextEditor&) override;

      // mouse listener added to get mouse click outside this component
      // which does the same as make it lose focus (saves the contents)
      void mouseDown(const MouseEvent& event) override;
      void mouseDoubleClick(const MouseEvent&) override {
         // do nothing. we want to throw away the double click on this text box
         // otherwise it ends up causing an extra double click which might select
         // one word (if there is a space) instead of keeping the full selection
      }
      Component *mTop;
      NoteCpt *mParent;
   public:
      ~InlineEditor();
      InlineEditor(NoteCpt *parent);
   };

   InlineEditor *mTextEdit;

public:
   String getTextualValue();
   void trySetValue(const String &txt);
   NoteCpt(String name = "note");
};

/**
This is the panel that hosts the note boxes on the left
*/
class NotePanel : public Component, public StepRowNotify {
   friend class NoteCpt;

   SeqGlob *mGlob;
   NoteCpt mNotes[SEQ_MAX_ROWS];
   SeqMouseAxis mAxis;       // which axis we are dragging on
   // mouse drag handles drag events on any of the grid cells
   void mouseDrag(const MouseEvent &event) override;
   void mouseDown(const MouseEvent &event) override;
   void mouseUp(const MouseEvent &event) override;
   void resized() override;

   // this keeps track of original value when mouse down event first happens
   int8_t mMouseStartVal;
   int mMouseOverRow;
public:
   NotePanel(SeqGlob *glob);
   
   // re-read settings and act appropriately
   void refreshAll(bool fixColorsToo=false);
  

   // Inherited via StepRowNotify
   // value of -1 means not over any row
   virtual void setRow(int row) override;

};

#endif
