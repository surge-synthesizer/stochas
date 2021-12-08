/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "CommonComponents.h"
#include "NotePanel.h"



/*===========================================================================================
ToggleCpt implementations
============================================================================================*/
ToggleCpt::ToggleCpt(SeqGlob *glob, int id, CptNotify *notify, String name) :
   Component(name),
   mId(id),
   mAlwaysNotify(false),
   mNotify(notify),
   mItemsPerRow(0xFFFF),
   mGlob(glob)
{}

void ToggleCpt::recalcGrid()
{
   // calculate mNumRows, mNumItemsOnEachRow, mNumItemsOnLastRow
   int t=mItems.size();
   
   mNumItemsOnLastRow = t % mItemsPerRow;
   mNumRows = t / mItemsPerRow + (mNumItemsOnLastRow ? 1 : 0);
   if (mNumRows > 1) {
      mNumItemsOnEachRow = mItemsPerRow;
   }
   else {
      mNumItemsOnEachRow = t;
   }
   if (!mNumItemsOnLastRow)
      mNumItemsOnLastRow = mNumItemsOnEachRow;
}

void ToggleCpt::paintText(Graphics &g, Rectangle<float> r, const String & txt)
{
   g.drawText(txt, r, Justification::centred);
}

void ToggleCpt::setMaxItemsPerRow(int max)
{
   if (max == 0)
      max = 0xFFFF;
   mItemsPerRow = max;
   recalcGrid();
}

void ToggleCpt::addItem(int id, String text, bool selected)
{
   Item it;
   it.id = id;
   it.text = text;
   it.selected = selected;
   mItems.add(it);
   recalcGrid();
}

void ToggleCpt::clearItems()
{
   mItems.clearQuick();
   recalcGrid();
}

void ToggleCpt::setCurrentItem(int id, bool val, bool triggerNotify)
{
   // only one item can be selected at a time.
   // so if there are multiple items, and val is true, unselect the others
   Item tmp;
   bool found = false;
   bool doNotify = false;
   int notifyVal = 0;
   jassert(mItems.size());
   if (mItems.size() == 1) {
      // single item. set it to val
      tmp = mItems[0];
      jassert(tmp.id == id);
      if (tmp.selected != val) {// is it changing?
         doNotify = true;
         tmp.selected = val;
      }
      found = true;
      // if setting to true, we notify with the id, otherwise with 0
      notifyVal = val ? tmp.id : 0;
      // replace the item in the array with a new item
      mItems.setUnchecked(0, tmp);
   }
   else {
      // multiple items
      for (int i = 0; i < mItems.size(); i++) {
         tmp = mItems[i];
         if (tmp.id == id) {
            if (tmp.selected != val) { // if this item is changing value
               doNotify = true;
               tmp.selected = val;
            }
            notifyVal = tmp.id;
            found = true;
         } else // not our target item, it always gets turned off
            tmp.selected = false; 

         // replace the item in the array with a new item
         mItems.setUnchecked(i, tmp);
      }
   }

   if (doNotify || mAlwaysNotify) {
      repaint();

      if (triggerNotify && mNotify)
         mNotify->cptValueChange(mId, notifyVal);
   }

   // item was not found
   jassert(found);
}

bool ToggleCpt::isCurrent(int id)
{
   Item tmp;
   for (int i = 0; i < mItems.size(); i++) {
      tmp = mItems[i];
      if (tmp.id == id) {
         return tmp.selected;
      }
   }

   // doesn't exist
   jassertfalse;
   return false;
}

void ToggleCpt::setAlwaysNotify(bool val)
{
   mAlwaysNotify = val;
}

void ToggleCpt::paint(Graphics & g)
{
   int intwidth, intheight;
   double width, height;
   double rem, remh;
   juce::Colour c, sel, unsel;

   // background
   //g.fillAll(mGlob->mEditorState->getColorFor(EditorState::background));

   if (!mItems.size())
      return; // leave it blank
   sel = mGlob->mEditorState->getColorFor(EditorState::toggleOn);
   unsel = mGlob->mEditorState->getColorFor(EditorState::toggleOff);

   Rectangle<int> r = getLocalBounds();
   //g.fillAll(mGlob->mEditorState->getColorFor(EditorState::background));

   // divide into number of blocks
   width = r.getWidth() / (double)mNumItemsOnEachRow;
   intwidth = (int)width;
   height = r.getHeight() / (double)mNumRows;
   intheight = (int)height;
   rem = remh =0;
   r.reduce(1, 1); // pull 1 pixel of all sides so we can see background a little
   Rectangle<int> one;
   Rectangle<int> oneRow;
   
   
   for (int curRow = 0; curRow < mNumRows; curRow++) {      
      // remove a row from top
      oneRow = r.removeFromTop(intheight+(int)remh);
      int startidx = curRow * mNumItemsOnEachRow;
      int count;
      if (curRow == mNumRows - 1)
         count = mNumItemsOnLastRow;
      else
         count = mNumItemsOnEachRow;
      if (remh >= 1.0)
         remh = 0;
      remh += (height-intheight);

      // add items to that row
      for (int i = startidx; i < startidx+count; i++) {
         one = oneRow.removeFromLeft(intwidth + (int)rem);
         if (mItems[i].selected)
            c = sel;
         else
            c = unsel;
         g.setColour(c);
         //g.fillRect(one.toFloat());
         one.reduce(1, 1); //!!!
         g.fillRoundedRectangle(one.toFloat(), 3.0f);
         g.setColour(c.contrasting(.5));
         paintText(g, one.toFloat(), mItems[i].text);
         // this nonsense is so that we take up the whole width
         if (rem >= 1.0)
            rem = 0;
         rem += (width - intwidth);
      }
      
   }

}

void ToggleCpt::mouseDown(const MouseEvent & event)
{
   bool isSel;
   int x, y;
   float width; // item width
   float height; // item height

   Rectangle<int> r = getLocalBounds();
   jassert(mItems.size());

   if (mItems.size() == 1)
      isSel = !mItems[0].selected; // toggle
   else
      isSel = true;

   // determine which item was clicked
   // divide into number of blocks
   width = (float)r.getWidth() / mNumItemsOnEachRow;
   height = (float)r.getHeight() / mNumRows;

   x = (int)((float)event.getMouseDownX() / width);
   y = (int)((float)event.getMouseDownY() / height);

   int itemSelected = (y*mNumItemsOnEachRow) + x;
   if (itemSelected < mItems.size()) {

      if(event.mods.isAnyModifierKeyDown()) {
         // if ctrl/cmd/shift/alt is being held while click,
         // we don't change items, we just raise event
         mNotify->cptItemClickWithModifier(mId, itemSelected,event.mods.getCurrentModifiers());
      }
      else {
         setCurrentItem(mItems[itemSelected].id, isSel, true);
      }
   }
   
}


void ToggleCptWithLabel::setLabel(int id, const String & txt)
{
   mLabels.set(id, txt);
   repaint();
}

void ToggleCptWithLabel::clearLabels()
{
   mLabels.clear();
   repaint();
}

void ToggleCptWithLabel::clearLabel(int id)
{
   mLabels.remove(id);
   repaint();
}

void ToggleCptWithLabel::paint(Graphics & g)
{
   ToggleCpt::paint(g);

   int intwidth;
   double width;
   double rem;
   juce::Colour c;
   c = juce::Colours::red;

   if (!mItems.size())
      return; // leave it blank


   Rectangle<int> r = getLocalBounds();
   // divide into number of blocks
   width = r.getWidth() / (double)mItems.size();
   intwidth = (int)width;
   rem = 0;
   r.reduce(1, 1); // pull 1 pixel of all sides so we can see background a little
   Rectangle<int> one;
   for (int i = 0; i < mItems.size(); i++) {
      one = r.removeFromLeft(intwidth + (int)rem);
      if (mLabels.contains(mItems[i].id)) {         
         one.removeFromLeft(one.getWidth() * 2 / 3);
         one.removeFromBottom(one.getHeight() / 3);
         one.removeFromTop(2);
         one.removeFromRight(2);
         g.setColour(c);
         g.setFont(Font(10.0, Font::bold));
         //g.fillRect(one.toFloat());
         g.fillRoundedRectangle(one.toFloat(), 2.0f);
         g.setColour(c.contrasting(.5));
         g.drawText(mLabels[mItems[i].id], one, Justification::centred);
      }
      // this nonsense is so that we take up the whole width
      if (rem >= 1.0)
         rem = 0;
      rem += (width - intwidth);
   }

}

void ToggleCptWithLabel::paintText(Graphics & g, Rectangle<float> r, const String & txt)
{
   r.removeFromRight(r.getWidth() / 4);
   g.drawText(txt, r, Justification::centred);
}

ToggleCptWithLabel::ToggleCptWithLabel(SeqGlob * glob, int id, CptNotify * notify, String name)
   :ToggleCpt(glob, id, notify, name)
{
}


/*===========================================================================================
ButtonCpt implementations
============================================================================================*/

void ButtonCpt::mouseDown(const MouseEvent &)
{
   if (mNotify)
      mNotify->cptValueChange(mId, 0);
}

ButtonCpt::ButtonCpt(SeqGlob *glob, int id, CptNotify *notify, String name) :
   Component(name),
   mId(id),
   mGlob(glob),
   mNotify(notify)
{

}

void ButtonCpt::paint(Graphics & g)
{
   juce::Colour c;
   // background
   //g.fillAll(mGlob->mEditorState->getColorFor(EditorState::background));
   // see if color is overriden
   if (mOverrideColor != Colour())
      c = mOverrideColor;
   else
      c = mGlob->mEditorState->getColorFor(EditorState::button);

   Rectangle<int> r = getLocalBounds();
   r.reduce(1, 1); // pull 1 pixel of all sides so we can see background a little
   g.setColour(c);
   //g.fillRect(r.toFloat());
   g.fillRoundedRectangle(r.toFloat(), 3.0f);
   g.setColour(c.contrasting(.5));
   g.drawText(mText, r.toFloat(), Justification::centred);
}

void ButtonCpt::setText(String txt)
{
   mText = txt;
   repaint();
}

void ButtonCpt::overrideColor(juce::Colour c)
{
   mOverrideColor = c;
}


/*===========================================================================================
NumberCpt implementations
============================================================================================*/
NumberCpt::NumberCpt(SeqGlob *glob, int id, CptNotify *notify, String name) : 
   Component(name), mGlob(glob), mId(id),
   mValue(0), mDefaultValue(0), mInterval(1),mLowBound(0),mHighBound(0),
   mNotify(notify), mInteracting(false),mDragValue(0),mIntSense(1), 
   mAxis(unknown),mEnabled(true), mTextEdit(0), mTextReplacer(0)
{

}

void NumberCpt::mouseDrag(const MouseEvent & event)
{
   EditorState *e = mGlob->mEditorState;
   if (!mEnabled)
      return;

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
   if(mAxis==vertical) // want up mouse to = positive value
      mouseDist = -event.getDistanceFromDragStartY() / (e->getMouseSense()*mIntSense);
   else // right = positive
      mouseDist = event.getDistanceFromDragStartX() / (e->getMouseSense()*mIntSense);

   
   mDragValue = mValue + (mouseDist * mInterval);
   if (mDragValue > mHighBound)
      mDragValue = mHighBound;
   else if (mDragValue < mLowBound)
      mDragValue = mLowBound;
   
   if(mNotify)
      mNotify->cptValueChanging(mId, mDragValue);

   repaint();
}

void NumberCpt::mouseDown(const MouseEvent &event)
{
   if (!mEnabled)
      return;
   // shift-click resets to default
   if(event.mods.isAnyModifierKeyDown())
      setValue(mDefaultValue, true);
   else {
      mInteracting = true;
      mDragValue = mValue;
   }
   repaint();
}

void NumberCpt::mouseUp(const MouseEvent &)
{
   if (!mEnabled)
      return;
   if (mInteracting) {
      mInteracting = false;
      mAxis = unknown;
      setValue(mDragValue, true);
      repaint();
   }
}

void NumberCpt::mouseDoubleClick(const MouseEvent & )
{
   if (!mEnabled)
      return;
   //setValue(mDefaultValue, true);
   mTextEdit = new InlineEditor(this);   
   
   
   repaint();
}

String NumberCpt::getTextualValue(bool stripped)
{
   String txt;
   int val = mInteracting ? mDragValue : mValue;
   if (mTextReplacer && mTextReplacer->getNumberCptCustomText(mId, val, txt)) {
      // done
   }
   else
   if (mReplacements.contains(val)) {
      txt = mReplacements[val];
   }
   else {
      txt = String::formatted("%d", val);
      if(!stripped)
         txt.append(mSuffix, mSuffix.length());
   }

   return txt;

}

void NumberCpt::paint(Graphics & g)
{
   String txt;
   EditorState *e = mGlob->mEditorState;
   juce::Colour c;
   
   if (mInteracting) {
      c = e->getColorFor(EditorState::interact);

   } else if(mEnabled) {
         c = e->getColorFor(EditorState::noteEditable);
   } else {
      // non-editable note
      c = e->getColorFor(EditorState::noteLocked);
   }

   txt = getTextualValue();
   
   // background
   //g.fillAll(e->getColorFor(EditorState::background));
   // current color
   g.setColour(c);
   Rectangle<int> r = getLocalBounds();
   r.reduce(1, 1); // pull 1 pixel of all sides so we can see background a little
   g.fillRoundedRectangle(r.toFloat(), 3.0f);
   //g.fillRect(r.toFloat());
   g.setColour(c.contrasting(.5));
   g.drawText(txt, r.toFloat(), Justification::centred);
}

void NumberCpt::enable(bool en)
{
   mEnabled = en;

}

void NumberCpt::setSpec(int lo, int hi, int interval, int defValue, String suffix)
{
   mLowBound = lo;
   mHighBound = hi;
   mInterval = interval;
   mSuffix = suffix; 
   mDefaultValue = defValue;


   int numitems = (mHighBound - mLowBound + 1) / mInterval;
   if (numitems > 50)
      mIntSense = 1;
   else if (numitems > 10)
      mIntSense = 2;
   else
      mIntSense = 3;

}

void NumberCpt::setStringRep(int val, String repl)
{
   mReplacements.set(val, repl);
}

void NumberCpt::setCustomText(CustomText * replacer)
{
   mTextReplacer = replacer;
}

int NumberCpt::getValue()
{
   return mValue;
}

void NumberCpt::setValue(int val, bool notify)
{
   bool r = (mValue != val);
   mValue = val;
   if (notify && mNotify)
      mNotify->cptValueChange(mId, mValue);
   if (r)
      repaint();
   
}

void NumberCpt::trySetValue(const String &val, bool notify)
{
   // this will ignore a suffix (ie read numbers until it encounters a non-number)
   int newval = val.getIntValue();

   
   if (newval == 0 && !val.containsOnly("0")) {
      // non-numeric text was entered, see if it matches any of the replacements
      String clean;
      // strip spaces
      clean = val.removeCharacters(" ");
      clean=clean.toLowerCase();
      if (mTextReplacer) {
         // brute force approach
         String txt;
         for (int i = mLowBound; i <= mHighBound; i++) {
            mTextReplacer->getNumberCptCustomText(mId, i, txt);
            txt = txt.removeCharacters(" ");
            txt = txt.toLowerCase();
            if (txt.compare(clean) == 0) {
               // found
               setValue(i, notify);
               return;
            }
         }         
      }
      
      if (mReplacements.size()) {
         String txt;
         HashMap<int, juce::String>::Iterator it(mReplacements);
         while (it.next()) {
            txt = it.getValue().removeCharacters(" ");
            txt = txt.toLowerCase();
            if (txt.compare(clean) == 0) {
               setValue(it.getKey(), notify);
               return;
            }
         }         
      }

      // textual value was not matched in either
      return;
   }

   if (newval >= mLowBound && newval <= mHighBound) {
      if (mInterval > 1 && (newval - mLowBound) % mInterval != 0)
         return;
      setValue(newval, notify);
   }
   
}


void NumberCpt::InlineEditor::textEditorReturnKeyPressed(TextEditor &)
{
   mParent->trySetValue(this->getText(), true);
   mParent->removeChildComponent(this);
   delete this;
}

void NumberCpt::InlineEditor::textEditorEscapeKeyPressed(TextEditor &)
{
   mParent->removeChildComponent(this);
   delete this;
}

void NumberCpt::InlineEditor::textEditorFocusLost(TextEditor &)
{
   mParent->trySetValue(getText(), true);
   mParent->removeChildComponent(this);
   delete this;
}

void NumberCpt::InlineEditor::mouseDown(const MouseEvent & event)
{
   if (event.eventComponent == static_cast<Component *>(this))
      return; // don't want to close the text box if they are clicking in it

   // mouse down on another component up the chain
   mParent->trySetValue(getText(), true);
   mParent->removeChildComponent(this);
   delete this;
}

NumberCpt::InlineEditor::~InlineEditor()
{
   if (mTop)
      mTop->removeMouseListener(this);
}

NumberCpt::InlineEditor::InlineEditor(NumberCpt * parent)
{
   mTop = parent;
   while (mTop->getParentComponent() != nullptr)
      mTop = mTop->getParentComponent();
   mTop->addMouseListener(this, true);

   mParent = parent;
   addListener(this);
   parent->addAndMakeVisible(this);
   fixDynamicTextEditBox(mParent->mGlob->mEditorState, *this);
   setBounds(mParent->getLocalBounds());
   setText(parent->getTextualValue(true), false);
   setSelectAllWhenFocused(true);
   grabKeyboardFocus();
}


/*===========================================================================================
GrooveCpt implementations
============================================================================================*/
void GrooveCpt::paint(Graphics &g)
{
   // TODO this code is a mess
   int i;
   float dist, pos;
   Rectangle<float> b = getLocalBounds().toFloat(); // rect containing all
   Rectangle<float> h = b; // rect containing our graph and our step numbers
   // occupies top 2/3rds of control
   h.removeFromBottom(b.getHeight() / 3);
   Colour c=mGlob->mEditorState->getColorFor(EditorState::background);
   Colour measureColor = mGlob->mEditorState->getColorFor(EditorState::beatStart);
   Colour grvColor = mGlob->mEditorState->getColorFor(EditorState::grooveEdit);
   Colour lc= c.contrasting(.5f);
   
   g.setColour(c);   
   g.fillAll();
   g.setColour(lc);
   g.drawRect(b.toFloat(), 1.0f); // border around whole
   Line<float> ln;
   ln.setStart(h.getBottomLeft());
   ln.setEnd(h.getBottomRight());
   g.drawLine(ln);
   dist = h.getWidth() / (SEQ_DEFAULT_NUM_STEPS*2);
   pos = 0.0f;
   for (i = 0; i < (SEQ_DEFAULT_NUM_STEPS * 2); i++) {

      Rectangle<float> txtRect; // for drawing step number
      txtRect.setSize(dist * 2, h.getHeight() / 4);
      // for dashed line
      float dls[2] = { 4, 2 };

      pos += dist;
      
      ln.setEnd(pos, h.getHeight());

      if (i % 2 != 0) { // in between line
         ln.setStart(pos, h.getHeight()/2);
         g.drawDashedLine(ln, dls, 2, 1.0f);
      }
      else { // main line
         int val;
         ln.setStart(pos, h.getHeight() / 2);
         if (i % 8 == 0) {
            g.setColour(measureColor);
            g.drawLine(ln, 4.0f);
            g.setColour(lc);
         }
         else {
            g.drawLine(ln, 1.0f);
         }
         txtRect.setPosition(pos-dist, h.getHeight() / 8);
         g.drawText(String::formatted("%d", (i/2) + 1),txtRect,Justification::centred,false);

         
         if (i/2 == mChgCpt) // value is changing
            val = mChgVal;
         else // actual value
            val = mNums[i / 2]->getValue();
         
         if (val && mEnabled) {
            float width = val*dist / 50;
            if (width < 0)
               width *= -1;
            Rectangle<float> box;
            box.setBounds(ln.getStartX(), 
               ln.getStartY(), width, h.getHeight()/2);
            box.removeFromTop(h.getHeight() / 8);
            if (val < 0) {
               box.setPosition(box.getX() - box.getWidth(), box.getY());
            }
            g.setColour(grvColor);
            g.fillRect(box);
            g.setColour(lc);

         }
      }
   }
   
}

void GrooveCpt::resized()
{
   Rectangle<float> r = getLocalBounds().toFloat();
   Rectangle<float> tmp;
   float dist=r.getWidth() / SEQ_DEFAULT_NUM_STEPS;
   r.removeFromTop(r.getHeight()*2 / 3);   
   //r.removeFromBottom(r.getHeight() / 2);
   for (int i = 0; i < SEQ_DEFAULT_NUM_STEPS; i++) {
      tmp = r.removeFromLeft(dist);
      tmp.reduce(4.0f, 2.0f);
      mNums[i]->setBounds(tmp.toNearestInt());
   }

}

void GrooveCpt::cptValueChange(int cptId, int)
{
   if (mNotify)
      mNotify->cptValueChange(mId, cptId);
   mChgCpt = -1;
   repaint();
}

void GrooveCpt::cptValueChanging(int cptId, int val)
{
   mChgCpt = cptId;
   mChgVal = val;
   repaint();
}

GrooveCpt::GrooveCpt(int id, SeqGlob * glob, CptNotify *notify) :
   mGlob(glob), mNotify(notify), mId(id),  mEnabled(true), mChgCpt(-1), mChgVal(0)
{
   mNums = new NumberCpt *[SEQ_DEFAULT_NUM_STEPS];
   for (int i = 0; i < SEQ_DEFAULT_NUM_STEPS; i++) {
      mNums[i] = new NumberCpt(glob, i, this);
      addAndMakeVisible(*mNums[i]);
      mNums[i]->setSpec(-50, 50, 1, 0, "%");
      mNums[i]->setStringRep(0, "Off");
   }
   
}

GrooveCpt::~GrooveCpt()
{
   for (int i = 0; i < SEQ_DEFAULT_NUM_STEPS; i++)
      delete mNums[i];
   delete mNums;
}

void GrooveCpt::setValue(int idx, int value)
{
   jassert(idx >= 0 && idx <= SEQ_DEFAULT_NUM_STEPS);
   mNums[idx]->setValue(value, false);
}

int GrooveCpt::getValue(int idx)
{
   jassert(idx >= 0 && idx <= SEQ_DEFAULT_NUM_STEPS);
   return mNums[idx]->getValue();
}

void GrooveCpt::enable(bool en)
{
   mEnabled = en;
   for (int i = 0; i < SEQ_DEFAULT_NUM_STEPS; i++) {
      mNums[i]->setVisible(en);
   }
   repaint();
}

/*===========================================================================================
TabPanelCpt implementations
============================================================================================*/

TabPanelCpt::TabPanelCpt(SeqGlob * glob, bool gradientFill) :
   mGlob(glob), mGradient(gradientFill)
{
}

void
TabPanelCpt::paint(Graphics &g)
{
   if(mGradient)
      gradientFill(*mGlob->mEditorState, this, g);
   else
      g.fillAll(mGlob->mEditorState->getColorFor(EditorState::background));

   
   
}

/*===============================================================================================
SeqModalDialog
=================================================================================================*/
SeqModalDialog::SeqModalDialog(SeqGlob * glob, int id, CptNotify *parent, int width, int height) :
   mParent(parent), mId(id),   mWidth(width),  mHeight(height), mPanel(glob, this), mGlob(glob)
{
   Colour txtColor, bgColor, hlColor;
   addAndMakeVisible(mPanel);
}



void SeqModalDialog::forceResize()
{
   resizedInner(&mPanel);
}

void SeqModalDialog::notifyParent(int val)
{
   mParent->cptValueChange(mId, val);
}

ToggleButton *
SeqModalDialog::addToggle(const String &txt, int grp, Component *parent, int cptId)
{
   if (!parent)
      parent = &mPanel;
   ToggleButton *ret = new ToggleButton();
   ret->setComponentID(String::formatted("%d", cptId));   
   parent->addAndMakeVisible(ret);
   ret->setButtonText(txt);
   ret->setRadioGroupId(grp);
   ret->addListener(this);
   ret->setWantsKeyboardFocus(false);
   return ret;
}

ComboBox *
SeqModalDialog::addCombo(const String &txt, Component *parent, int cptId)
{

   if (!parent)
      parent = &mPanel;
   ComboBox *ret = new ComboBox();
   parent->addAndMakeVisible(ret);
   ret->setComponentID(String::formatted("%d", cptId));
   ret->setEditableText(false);
   ret->setJustificationType(Justification::centredLeft);
   ret->setTextWhenNothingSelected(txt);
   ret->setTextWhenNoChoicesAvailable("(no choices)");
   ret->setWantsKeyboardFocus(false);
   ret->addListener(this);
   return ret;
}

Label *
SeqModalDialog::addStdLabel(const String &txt, Component *parent)
{
   if (!parent)
      parent = &mPanel;
   Label *ret = new Label(String(), txt);
   
   parent->addAndMakeVisible(ret);
   ret->setFont(Font(15.00f, Font::plain));
   ret->setJustificationType(Justification::centredLeft);
   ret->setEditable(false, false, false);
   
   return ret;
}

TextButton * SeqModalDialog::addStdButton(const String & txt, Component * parent, int cptId)
{
   TextButton *ret = new TextButton("");
   if (!parent)
      parent = &mPanel;
   ret->setComponentID(String::formatted("%d", cptId));
   
   parent->addAndMakeVisible(ret);   
   ret->setButtonText(txt);
   ret->setWantsKeyboardFocus(false);
   ret->addListener(this);
   return ret;
}

TextEditor * SeqModalDialog::addStdTextEditor(Component * parent, int cptId)
{
   if (!parent)
      parent = &mPanel;
   TextEditor *ret = new TextEditor(String());
   ret->setComponentID(String::formatted("%d", cptId));
   
   
   parent->addAndMakeVisible(ret);
   ret->setFont(Font(15.00f, Font::plain));
   ret->setMultiLine(false, false);
   
   return ret;
}

void SeqModalDialog::addToInner(int cptId, Component & cpt, int zOrd)
{
   cpt.setComponentID(String::formatted("%d", cptId));
   mPanel.addChildComponent(cpt, zOrd);   
}

void SeqModalDialog::removeFromInner(Component * cpt)
{
   mPanel.removeChildComponent(cpt);
}

void SeqModalDialog::openDialog()
{
   fixButtonColors();
   setBounds(getParentComponent()->getBounds());
   toFront(true);
   setVisible(true);
   enterModalState();
}

void SeqModalDialog::closeDialog(bool hitOk)
{
   endDialog(hitOk); // call derived
   setVisible(false);
   exitModalState(0);
}

void SeqModalDialog::fixButtonColors()
{
   //fixColors(this);
}

void SeqModalDialog::resized()
{
   Rectangle<int> r;
   int removeV;

   // panel that contains all
   r = getLocalBounds();

   r.reduce((r.getWidth()-mWidth)/2, 0); // trim from all sides
   // need to position it nearer the top
   removeV = r.getHeight() - mHeight;
   if (removeV > 50) {
      r.removeFromTop(50);
      removeV -= 50;
      r.removeFromBottom(removeV);
   }
   else {
      r.reduce(0, removeV / 2);
   }
   
   // this will force a resize on mPanel which will call the resizeInner
   mPanel.setBounds(r);

}

void SeqModalDialog::paint(Graphics &g)
{
   setOpaque(false);
   g.setColour(juce::Colour((uint8)0, (uint8)0, (uint8)0, (uint8)90));
   g.fillAll();
}

void SeqModalDialog::cptValueChange(int cptId, int value)
{
   notify(cptId, value);
}

void SeqModalDialog::buttonClicked(Button *b)
{
   notify(b->getComponentID().getIntValue(),0);
}

void SeqModalDialog::comboBoxChanged(ComboBox *box)
{
   notify(box->getComponentID().getIntValue(), box->getSelectedItemIndex());
}

void SeqModalDialog::mouseDown(const MouseEvent &)
{
   //mouse click on outside of inner panel
   closeDialog(false);
}

SeqModalDialog::InnerPanel::InnerPanel(SeqGlob * glob, SeqModalDialog *parent) :
   mGlob(glob), mParent(parent)
{
}

void SeqModalDialog::InnerPanel::resized()
{
   mParent->resizedInner(this);
}

void SeqModalDialog::InnerPanel::paint(Graphics &g)
{
   Rectangle<int> b;
   // fill
   Colour c = mGlob->mEditorState->getColorFor(EditorState::background);
   g.setColour(c);
   g.fillAll();
   // border
   c = mGlob->mEditorState->getColorFor(EditorState::border);
   b = getLocalBounds();
   g.setColour(c);
   g.drawRect(b.toFloat(), 2.0);

}
