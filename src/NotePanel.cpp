/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "NotePanel.h"
#include "PluginEditor.h"
/*===========================================================================================
NoteCpt implementations
============================================================================================*/
void NoteCpt::labelTextChanged(Label * lbl)
{
   String s;
   // update our model with the new info
   EditorState *e = mGlob->mEditorState;
   char noteBuf[SEQ_MAX_NOTELABEL_LEN];
   SequenceLayer *data = mGlob->mSeqBuf->getUISeqData()->getLayer(e->getCurrentLayer());
   memset(noteBuf, 0, SEQ_MAX_NOTELABEL_LEN);
   s= lbl->getText(false);
   strncpy(noteBuf, s.getCharPointer(), SEQ_MAX_NOTELABEL_LEN);
   data->setNoteName(mId, noteBuf);

}

void NoteCpt::resized()
{
   Rectangle<int> r = getLocalBounds();
   Rectangle<int>b;
   b=r.removeFromLeft(r.getHeight());
   b.reduce(1, 1);
   mBtnPlay.setBounds(b);
   r.removeFromRight(SEQ_SIZE_NOTE_SUB_W);
   mText.setBounds(r);
   
}
String NoteCpt::getTextualValue()
{
   String ret;
   EditorState *e = mGlob->mEditorState;
   int8_t n;
   char noteBuf[SEQ_NOTE_NAME_MAXLEN];
   SequenceLayer *data = mGlob->mSeqBuf->getUISeqData()->getLayer(e->getCurrentLayer());
   n = data->getCurNote(mId);   
   ret = SeqScale::getMidiNoteName(n, e->getLowestOctave(), noteBuf);
   return ret;
}
void NoteCpt::trySetValue(const String & txt)
{
   char noteBuf[SEQ_NOTE_NAME_MAXLEN];
   EditorState *e = mGlob->mEditorState;
   SequenceLayer *data = mGlob->mSeqBuf->getUISeqData()->getLayer(e->getCurrentLayer());
   
   String cleaned = txt.removeCharacters(" ").toUpperCase();
   String comp;
   for (int i = 0; i < 128; i++) {
      comp = SeqScale::getMidiNoteName((int8_t)i, e->getLowestOctave(), noteBuf);
      comp = comp.removeCharacters(" ").toUpperCase();
      if (comp.compare(cleaned) == 0) {
         data->setNote(mId,(int8_t) i, true);
         mGlob->mSeqBuf->swap();
         break;
      }
   }
}
NoteCpt::NoteCpt(String name) : Component(name), mParent(0), mBtnPlay(this), mTempValue(-1), mTextEdit(0)
{
   // mGlob is not available yet!
   addAndMakeVisible(mText);
   mText.setName("txtName");
   mText.setText("---", juce::dontSendNotification);   
   mText.setEditable(false, true, false);
   mText.addListener(this);
   addAndMakeVisible(mBtnPlay);

   //mBtnPlay.addListener(this);
}

void NoteCpt::setupImage()
{
   // separate because it needs glob
   juce::Colour c = mGlob->mEditorState->getColorFor(EditorState::button);
   juce::Colour cd = mGlob->mEditorState->getColorFor(EditorState::toggleOn);
   Image img;
   img = ImageCache::getFromMemory(SeqImageX::play_png, SeqImageX::play_pngSize);
   mBtnPlay.setImages(false, true, true, img, 1.0, c, img, 1.0, c, img, 1.0, cd);
}

void
NoteCpt::paint(Graphics & g)
{
   EditorState *e = mGlob->mEditorState;
   juce::Colour c;
   juce::Colour selClr;
   int8_t n;
   bool locked = true;
   char noteBuf[SEQ_NOTE_NAME_MAXLEN];
   SequenceLayer *data = mGlob->mSeqBuf->getUISeqData()->getLayer(e->getCurrentLayer());
   n = data->getCurNote(mId);
   if (mParent->mMouseOverRow == mId)
      selClr = e->getColorFor(EditorState::coloredElements::interact);
   else
      selClr = e->getColorFor(EditorState::coloredElements::background);
   if (data->noteSourceIsCustom() && n != -1) {
      locked = false;

      // if we are currently dragging the cell
      if (mTempValue != -1) {
         c = e->getColorFor(EditorState::interact);
         n = mTempValue;
      }
      else {
         c = e->getColorFor(EditorState::noteEditable);
      }
   }
   else {
      // non-editable note
      c = e->getColorFor(EditorState::noteLocked);
   }

   // background
   g.fillAll(e->getColorFor(EditorState::background));
   // current color
   g.setColour(c);
   Rectangle<int> r = getLocalBounds();
   Rectangle<int> rtmp;
   //r.removeFromRight(1);
   r.removeFromBottom(1);
   g.fillRect(r.toFloat());

   // current row indicator
   rtmp=r.removeFromRight(3);
   g.setColour(selClr);
   g.fillRect(rtmp.toFloat());
   

   // play button
   r.removeFromLeft(r.getHeight());   
   g.setColour(c.contrasting(.5));
   if (!locked) {
      r = r.removeFromRight(SEQ_SIZE_NOTE_SUB_W);
   }

   // midi note name

   g.drawText(SeqScale::getMidiNoteName(n,e->getLowestOctave(), noteBuf), r.toFloat(), 
      locked? Justification::centred : Justification::right);
}

void NoteCpt::mouseUp(const MouseEvent & event)
{
   if (event.eventComponent == &mBtnPlay) {
      mGlob->mProcessNotify->addToFifo(SEQ_MIDI_NOTEOFF, mGlob->mEditorState->getCurrentLayer(), mId);
   }
}

void NoteCpt::mouseDoubleClick(const MouseEvent &)
{
   // if mText (the label edit) is visible, then we are editable
   if (mText.isVisible()) {
      mTextEdit = new InlineEditor(this);
      repaint();
   }
}

void NoteCpt::mouseDown(const MouseEvent & event)
{
   if (event.eventComponent == &mBtnPlay) {
      
      /* right now the protocol is:
         value1 - midi type
         value2 - layer
         value3 - note number to play         
      */
      mGlob->mProcessNotify->addToFifo(SEQ_MIDI_NOTEON,mGlob->mEditorState->getCurrentLayer(), mId);
   }
}

void NoteCpt::MyLabel::editorShown(TextEditor * ed)
{

   fixDynamicTextEditBox(mParent->mGlob->mEditorState, *ed, SEQ_MAX_NOTELABEL_LEN);
   
}

void NoteCpt::InlineEditor::textEditorReturnKeyPressed(TextEditor &)
{
   mParent->trySetValue(this->getText());
   mParent->removeChildComponent(this);
   delete this;
}

void NoteCpt::InlineEditor::textEditorEscapeKeyPressed(TextEditor &)
{
   mParent->removeChildComponent(this);
   delete this;
}

void NoteCpt::InlineEditor::textEditorFocusLost(TextEditor &)
{
   mParent->trySetValue(getText());
   mParent->removeChildComponent(this);
   delete this;
}

void NoteCpt::InlineEditor::mouseDown(const MouseEvent & event)
{
   if (event.eventComponent == static_cast<Component *>(this))
      return; // don't want to close the text box if they are clicking in it

              // mouse down on another component up the chain
   mParent->trySetValue(getText());
   mParent->removeChildComponent(this);
   delete this;
}

NoteCpt::InlineEditor::~InlineEditor()
{
   if (mTop)
      mTop->removeMouseListener(this);
}

NoteCpt::InlineEditor::InlineEditor(NoteCpt * parent)
{
   mTop = parent;
   while (mTop->getParentComponent() != nullptr)
      mTop = mTop->getParentComponent();
   mTop->addMouseListener(this, true);

   mParent = parent;
   addListener(this);
   parent->addAndMakeVisible(this);
   fixDynamicTextEditBox(mParent->mGlob->mEditorState, *this);
   Rectangle<int>r = mParent->getLocalBounds().removeFromRight(SEQ_SIZE_NOTE_SUB_W);
   setBounds(r);
   setSelectAllWhenFocused(true);
   setText(parent->getTextualValue(), false);
   grabKeyboardFocus();
   
}


/*===========================================================================================
NotePanel implementations
============================================================================================*/
NotePanel::NotePanel(SeqGlob *glob) : Component("notePanel"), mGlob(glob), 
   mAxis(unknown), 
   mMouseStartVal(-1),
   mMouseOverRow(-1)
{
   for (int i = 0; i < SEQ_MAX_ROWS; i++) {
      addAndMakeVisible(mNotes[i]);
      mNotes[i].mId = i;
      mNotes[i].mGlob = mGlob;
      mNotes[i].mParent = this;
      mNotes[i].mText.mParent = this;
      mNotes[i].setupImage();
   }

   // listen for events on the child note items
   addMouseListener(this, true);

}


void NotePanel::refreshAll(bool fixColorsToo)
{
   EditorState *e = mGlob->mEditorState;
   SequenceLayer *data = mGlob->mSeqBuf->getUISeqData()->getLayer(e->getCurrentLayer());
   juce::Colour c = e->getColorFor(EditorState::coloredElements::noteEditable).contrasting(.5f);
   bool cust = data->noteSourceIsCustom();
   char nbuf[SEQ_MAX_NOTELABEL_LEN + 1];
   nbuf[SEQ_MAX_NOTELABEL_LEN] = 0;
   for (int i = 0; i < SEQ_MAX_ROWS; i++) {
      if (fixColorsToo)
         mNotes[i].setupImage();
      int8_t n = data->getCurNote(mNotes[i].mId);
      if (cust && n != -1) {
         const char *cs;
         cs = data->getNoteName(i);
         strncpy(nbuf, cs, SEQ_MAX_NOTELABEL_LEN);
         mNotes[i].mText.setVisible(true);        
         mNotes[i].mText.setColour(Label::ColourIds::textColourId, c);
         mNotes[i].mText.setText(nbuf, juce::NotificationType::dontSendNotification);
      }
      else {
         mNotes[i].mText.setVisible(false);
         
      }

      if(n != -1)
         mNotes[i].mBtnPlay.setVisible(true);
      else
         mNotes[i].mBtnPlay.setVisible(false);
   }
   repaint();

}

void NotePanel::setRow(int row)
{
   // value of -1 means not over any row
   jassert(row >= -1 && row < SEQ_MAX_ROWS);
   int old = mMouseOverRow;
   mMouseOverRow = row;
   if (old != row) {
      if (old > -1)
         mNotes[old].repaint();
      if (row > -1)
         mNotes[row].repaint();
   }
}

void NotePanel::mouseDrag(const MouseEvent & event)
{
   if (event.eventComponent->getName() == "note") {
      NoteCpt *c = static_cast<NoteCpt *>(event.eventComponent);
      EditorState *e = mGlob->mEditorState;
      int newval = -1;

      // first determine axis of motion
      if (mAxis == unknown) {
         int mdx, mdy;
         mdx = event.getDistanceFromDragStartX();
         mdy = event.getDistanceFromDragStartY();
         mdx *= mdx < 0 ? -1 : 1;
         mdy *= mdy < 0 ? -1 : 1;
         if (mdx < SEQ_MOUSE_AXIS_SENSE && mdy < SEQ_MOUSE_AXIS_SENSE)
            return;
         if (mdx > mdy)
            mAxis = horizontal;
         else
            mAxis = vertical;
      }

      int mouseDist;
      if (mAxis == vertical) // want up mouse to = positive value
         mouseDist = -event.getDistanceFromDragStartY() / (e->getMouseSense());
      else // right = positive
         mouseDist = event.getDistanceFromDragStartX() / (e->getMouseSense());


      if (mMouseStartVal != -1) {
         // clamp to 127
         newval = mMouseStartVal + mouseDist;
         newval = newval > 127 ? 127 : newval < 0 ? 0 : newval;
         c->mTempValue = (int8_t)newval;
         repaint();
      }
   }
}

void NotePanel::mouseDown(const MouseEvent & event)
{
   if (event.eventComponent->getName() == "note") {
      NoteCpt *c = static_cast<NoteCpt *>(event.eventComponent);
      SeqDataBuffer *buf = mGlob->mSeqBuf;
      SequenceLayer *data = buf->getUISeqData()->getLayer(mGlob->mEditorState->getCurrentLayer());
      int8_t n = data->getNote(c->mId, true);



      // only allow edit when custom notes are present
      if (data->noteSourceIsCustom() && n != -1)
         mMouseStartVal = n;
   }
}

void NotePanel::mouseUp(const MouseEvent & event)
{
   if (event.eventComponent->getName() == "note") {
      NoteCpt *c = static_cast<NoteCpt *>(event.eventComponent);
      SeqDataBuffer *buf = mGlob->mSeqBuf;
      SequenceLayer *data = buf->getUISeqData()->getLayer(mGlob->mEditorState->getCurrentLayer());

      mAxis = unknown;

      if (mMouseStartVal != -1 && c->mTempValue != -1) {
         data->setNote(c->mId, c->mTempValue, true);
         buf->swap();
         repaint();
         mMouseStartVal = -1;
         c->mTempValue = -1;
      }
   }
}

void NotePanel::resized()
{
   // notepanel has been sized according to how many rows are visible, so we need
   // to move each cell so the bottom n cells are on the visible portion

   int invisibleHeight; // height of invisible area above bottom rows
   invisibleHeight = (SEQ_MAX_ROWS*SEQ_SIZE_CELL_H) - getHeight();

   int width = getWidth();

   for (int y = 0; y < SEQ_MAX_ROWS; ++y) {
      // creates the rectangle     (x,         y,         width, height)
      Rectangle<int> elementBounds(0, (y * SEQ_SIZE_CELL_H)-invisibleHeight, width, SEQ_SIZE_CELL_H);
      // set the size and position of the Toggle light to this rectangle.
      mNotes[y].setBounds(elementBounds);
   }
}

