/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "StepPanel.h"
#include "PluginEditor.h"
/*===========================================================================================
StepCpt implementations
============================================================================================*/
StepCpt::StepCpt() : Component("singleStep"),   mTempValue(MOUSE_STARTVAL_INVALID)
{
}

void
StepCpt::paint(Graphics &g)
{
   String txt;
   juce::Colour c;
   EditorState *e = mGlob->mEditorState;
   int curLayer = e->getCurrentLayer();
   // background
   g.fillAll(e->getColorFor(EditorState::background));

   // if it's visible
   if(getEffectiveColorAndText(c,txt)) {
      Rectangle<int> r = getLocalBounds();
      Rectangle<int> b;
      SeqDataBuffer *s = mGlob->mSeqBuf;
      SequenceLayer *d = s->getUISeqData()->getLayer(curLayer);
      int spm = 0, spb = 0; // steps per measure/beat
      int marker = 0; // 1=start of measure, 2=start of beat

      // determine whether this is a "marker" cell (quarter note, etc)
      spm=d->getStepsPerMeasure();
      if (spm % 4 == 0)
         spb = 4;
      else if (spm % 3 == 0)
         spb = 3;
      else if (spm % 2 == 0) // anything divisible by 2 make 2 beats
         spb = spm/2;
      else
         spb = spm;

      if (mCol % spm == 0) // start of measure
         marker = 1;
      else if (mCol % spb == 0) // start of beat
         marker = 2;


      // there will be a background colored line on the bottom and right
      r.removeFromRight(1);
      r.removeFromBottom(1);
      if (marker) { // check to see if it's a boundary step that needs special attention
         if (marker == 1) { // start of measure
            b = r.removeFromLeft(4);
            g.setColour(e->getColorFor(EditorState::measureStart));
         }
         else { // start of "beat"
            b = r.removeFromLeft(2);
            g.setColour(e->getColorFor(EditorState::beatStart));
         }
         g.fillRect(b.toFloat());
      }
      
      // current color
      g.setColour(c);
      g.fillRect(r.toFloat());

      r.reduce(1, 1);
      // see how much space we have
      float tw = g.getCurrentFont().getStringWidthFloat(txt);
      if (tw <= r.toFloat().getWidth()) {
         // draw text if it fits
         g.setColour(c.contrasting(.5));
         g.drawText(txt, r.toFloat(), Justification::centred);
      }


      // if we are playing and 
      // if the step played recently, make an indicator
      if (mGlob->mAudNotify->getPlayPosition(curLayer) != -1 &&
         mGlob->mAudNotify->getStepPlayedState(curLayer, mCol, mRow)) {
         r=r.removeFromRight(5);
         g.setColour(c.contrasting(.5));
         g.fillRect(r.toFloat());
      }

   }
}

bool
StepCpt::getEffectiveColorAndText(juce::Colour &c, juce::String &txt, int layer)
{
   EditorState *e = mGlob->mEditorState;
   SeqDataBuffer *s = mGlob->mSeqBuf;
   SequenceLayer *d = s->getUISeqData()->getLayer(layer == -1 ? e->getCurrentLayer() : layer);
   char probVal = d->getProb(mRow, mCol);
   
   if (isStepInRange(layer)) {      
      EditorState::EditMode mode = e->getEditMode();
      char val;
      if (mTempValue != MOUSE_STARTVAL_INVALID)   // currently being edited
         val = mTempValue;
      else if (mode == EditorState::editingVelocity)
         val = d->getVel(mRow, mCol);
      else if (mode == EditorState::editingOffset)
         val = d->getOffset(mRow, mCol);
      else
         val = probVal;
      

      switch (mode) {
      case EditorState::editingSteps:
      case EditorState::editingChain:
         // val is probability. if 0, it's off

         // colors will be same for mono/poly right now
         if (val == SEQ_PROB_OFF)
            c = e->getColorFor(EditorState::stepOff);
         else if(val==SEQ_PROB_NEVER)
            c = e->getColorFor(EditorState::stepNeverProb);
         else if (val <= SEQ_PROB_LOW_VAL)
            c = e->getColorFor(EditorState::stepLowProb);
         else if (val <= SEQ_PROB_MED_VAL)
            c = e->getColorFor(EditorState::stepMediumProb);
         else if (val <= SEQ_PROB_ON)
            c = e->getColorFor(EditorState::stepHighProb);

         if (d->isMonoMode()) {
            if (val == SEQ_PROB_OFF)
               txt = "";
            else if (val == SEQ_PROB_NEVER)
               txt = SEQ_PROB_NEVER_TEXT;
            else if (val <= SEQ_PROB_LOW_VAL)
               txt = SEQ_PROB_LOW_TEXT;
            else if (val <= SEQ_PROB_MED_VAL)
               txt = SEQ_PROB_MED_TEXT;
            else if (val <= SEQ_PROB_HIGH_VAL)
               txt = SEQ_PROB_HIGH_TEXT;
         }
         else // poly mode
            if (val != SEQ_PROB_OFF) {
               if (val == SEQ_PROB_ON)
                  txt = SEQ_PROB_ON_TEXT;
               else
                  txt = String().formatted("%d%%",val);
            }


         break;
      case EditorState::editingVelocity:
         if (val ||probVal != SEQ_PROB_OFF)
            txt = String().formatted("%d", val);
         if (probVal != SEQ_PROB_OFF) {
            if(val < 43)
               c = e->getColorFor(EditorState::coloredElements::stepLowVel);
            else if(val < 85)
               c = e->getColorFor(EditorState::coloredElements::stepMedVel);
            else
               c = e->getColorFor(EditorState::coloredElements::stepHighVel);
         }
         else
            c = e->getColorFor(EditorState::coloredElements::stepOff);

         break;
      case EditorState::editingOffset:
         if (val || probVal != SEQ_PROB_OFF) {
            txt << (val < 0 ? "<" : "")
               << (int)val
               << (val > 0 ? ">" : "");            
         }
            
         if (probVal != SEQ_PROB_OFF) {
            // use the colors for lowvel and high vel to indicate whether or not it's shifted
            if (val)
               c = e->getColorFor(EditorState::coloredElements::stepHighVel);
            else
               c = e->getColorFor(EditorState::coloredElements::stepLowVel);
         }
         else {
            c = e->getColorFor(EditorState::coloredElements::stepOff);
         }

         break;
      default:
         jassert(false);
         break;
      }

      // if we are dragging
      if (this->mTempValue != MOUSE_STARTVAL_INVALID) {
         c = e->getColorFor(EditorState::interact);
      }

      return true;
   } else
      return false; // not visible


}

bool StepCpt::isStepInRange(int layer)
{
   /*Determine if the cell is within bounds for the layer.
   Each layer can store max rows and max cols, however the user may have
   set the layer to be less rows or cols. This returns false if the cell is outside
   that range.*/
   EditorState *e = mGlob->mEditorState;
   SeqDataBuffer *s = mGlob->mSeqBuf;
   SequenceLayer *d = s->getUISeqData()->getLayer(layer == -1 ? e->getCurrentLayer() : layer);
   if (mCol < d->getNumSteps() && mRow >= SEQ_MAX_ROWS - d->getMaxRows())
      return true;

   return false;
}

void StepPanel::unselectAll()
{
   mSelGridItem = -1;
   moveLengthEditCursor();
   // remove multi select
   mGlob->mEditorState->clearSelectedCells();
}

/*===========================================================================================
StepPanel implementations
============================================================================================*/
StepPanel::StepPanel(SeqGlob *glob, int id, Component *mainCpt, CptNotify *notify, String name) :
   Component(name),
   mMainEditor(mainCpt),
   mGlob(glob),
   mId(id),
   mNotify(notify),
   mLengthEditCursor(),
   mSelGridItem(-1),
   mLastLayer(-1),
   mCurLayer(0), // first layer
   mAxis(unknown),
   mMouseStartVal(MOUSE_STARTVAL_INVALID),
   mChainStartItem(0),
   mChainEndItem(0),
   mChainCustom(false),
   mChainNegTgt(false),
   mRowNotify(0),
   mDoingMultiSelect(false),
   mChordHandler(glob),
   mCurPosition(-1)
   
{
   // this is to notify the main window when a custom chain is being added
   ActionListener *al = dynamic_cast<ActionListener *>(mainCpt);
   jassert(al);
   mBroadcaster.addActionListener(al);

   setWantsKeyboardFocus(true);
   for (int i = 0; i < SEQ_MAX_ROWS*SEQ_MAX_STEPS; i++) {
      addAndMakeVisible(mGrid[i]);
      mGrid[i].mGlob = mGlob;
   }
   // assign row/col to each cell for ease of use when we get events
   for (int x = 0; x < SEQ_MAX_STEPS; ++x) {
      for (int y = 0; y < SEQ_MAX_ROWS; ++y) {
         mGrid[x + SEQ_MAX_STEPS * y].mRow = y;
         mGrid[x + SEQ_MAX_STEPS * y].mCol = x;
      }
   }

   // this is for enlarging or shrinking the length of a cell
   addAndMakeVisible(mLengthEditCursor);
   
   // we want this component to listen for events from it's child
   // components. It shouldn't get any of it's own as the children completely
   // cover it. heaven forbid it does (see mousedrag)!
   addMouseListener(this, true);
   addKeyListener(this);

}

void StepPanel::layerHasChanged()
{
   int cur = mGlob->mEditorState->getCurrentLayer();
   if (mCurLayer != -1) { // already seen a layer      
      if (cur == mCurLayer) // clicked same layer twice (don't ghost)
         mLastLayer = -1;
      else
         mLastLayer = mCurLayer; // shift current to last      
   }
   mCurLayer = cur;
}

void StepPanel::check()
{
   /* This is to update the grid with highlighted cells that 
      have played to indicate which cells are playing.
      also to show new steps that might have been recorded
   */
   int pp;
   pp = mGlob->mAudNotify->getPlayPosition(mGlob->mEditorState->getCurrentLayer());
   if (pp != mCurPosition) {
      repaint();
      mCurPosition = pp;
   }
}

static inline char
getNewMonoVal(char curVal, int delta)
{
   static int vals[] = {
      SEQ_PROB_OFF,
      SEQ_PROB_NEVER,
      SEQ_PROB_LOW_VAL,
      SEQ_PROB_MED_VAL,
      SEQ_PROB_HIGH_VAL
   };
   static const char num = sizeof(vals) / sizeof(int);
   int now = 0, i;

   delta = delta > (num - 1) ? (num - 1) : delta < -(num - 1) ? -(num - 1) : delta;

   for (i = 0; i < num; i++) {
      if (curVal <= vals[i]) {
         now = i;
         break;
      }
   }
   if (i == num)
      now = i - 1;

   // apply delta
   now += delta;
   // clamp to possible vals
   now = now<0 ? 0 : now>(num - 1) ? (num - 1) : now;
   return (char)vals[now];
}

bool StepPanel::keyPressed(const KeyPress & key, Component * /*originatingComponent*/)
{
   if (mGlob->mEditorState->getKeyboardDisabled())
      return false;

   if (key == KeyPress::deleteKey) {

      deleteSelectedCells();      
      unselectAll();
      repaint();
      return true;
   }
   else if (key == KeyPress::downKey || key==KeyPress::numberPad2) {
      if (mChordHandler.inCell()) {
         mChordHandler.decreaseInversion();
         repaint();
         return true;
      }
      else {
         shiftSelectedCells(false, true);
         repaint();
         return true;
      }
   }
   else if (key == KeyPress::upKey || key == KeyPress::numberPad8) {
      if (mChordHandler.inCell()) {
         mChordHandler.increaseInversion();
         repaint();
         return true;
      }
      else {
         shiftSelectedCells(false, false);
         repaint();
         return true;
      }
   }
   else if (key == KeyPress::leftKey || key == KeyPress::numberPad4) {

      shiftSelectedCells(true, false);
      repaint();
      return true;

   }
   else if (key == KeyPress::rightKey || key == KeyPress::numberPad6) {

      shiftSelectedCells(true, true);
      repaint();
      return true;

   }
   
   return false;
}

// this paint overlays the child controls
void StepPanel::paintOverChildren(Graphics & g)
{
   // draw ghosted layer first if applicable   
   if (mLastLayer != -1) {
      paintGhostedLayer(g);
   }

   // draw chains if in chain mode
   if (mGlob->mEditorState->getEditMode() == EditorState::editingChain) {
      paintChains(g);      
   }
      
   // draw the selected cell(s)
   paintSelectedCells(g);

   // draw length bars on cells that have length > 0
   paintLengthBars(g);

   // show the chord cursors
   if (mGlob->mEditorState->getEditMode() == EditorState::editingSteps && mChordHandler.inCell()) {
      StepCpt *sc;
      Rectangle<int> b;
      int c = mChordHandler.getCol();
      int rr = mChordHandler.getRootRow();
      // show the root
      sc = &mGrid[(rr*SEQ_MAX_STEPS) + c];
      b = sc->getBoundsInParent();
      g.setColour(mGlob->mEditorState->getColorFor(EditorState::coloredElements::cellSelection).withAlpha(.6f));
      g.fillRect(b.toFloat());
      g.setColour(mGlob->mEditorState->getColorFor(EditorState::coloredElements::cellSelection).withAlpha(.2f));
      // show chord rows
      for (int i = 0; i < mChordHandler.numUsedIntervals(); i++) {
         int r = mChordHandler.getRowForInterval(i);
         if (r != -1) {
            sc = &mGrid[(r*SEQ_MAX_STEPS) + c];
            b = sc->getBoundsInParent();
            g.fillRect(b.toFloat());
         }
      }
   }

}

void
StepPanel::mouseDrag(const MouseEvent &event)
{
   SeqDataBuffer *buf = mGlob->mSeqBuf;
   EditorState *e = mGlob->mEditorState;
   SequenceLayer *data = buf->getUISeqData()->getLayer(e->getCurrentLayer());
   if (&mLengthEditCursor == event.eventComponent) {
      // length cursor, calc drag
      int mouseH = event.getDistanceFromDragStartX();
      int cellWidth = getWidth() / SEQ_MAX_STEPS;
      mLengthEditCursor.mLenDelta = mouseH / cellWidth;
      repaint();
   }
   else {
      jassert(event.eventComponent->getName().compare("singleStep")==0);
      StepCpt *c;
      if (mDoingMultiSelect) { // multiselect mode is active
         MouseEvent mm=event.getEventRelativeTo(this);
         c = static_cast<StepCpt * > (getComponentAt(mm.getPosition()));
         if (c && c->getName().compare("singleStep") == 0) {
            // change state of multi selection (multi selection may not be normalized after this)
            e->enlargeSelectedCells(c->mCol, c->mRow);
            repaint();
         }
      } else {
         c = static_cast<StepCpt *>(event.eventComponent);
         if (mMouseStartVal != MOUSE_STARTVAL_INVALID) {
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

            int newval = -1;
            // we want distance such that a positive number is moving up and a negative is
            // moving down
            //int mouseDist = (-event.getDistanceFromDragStartY() / e->getMouseSense());

            int mouseDist;
            if (mAxis == vertical) // want up mouse to = positive value
               mouseDist = -event.getDistanceFromDragStartY() / (e->getMouseSense());
            else // right = positive
               mouseDist = event.getDistanceFromDragStartX() / (e->getMouseSense());

            if (mouseDist) {
               if (e->getEditMode() == EditorState::editingVelocity) {
                  // clamp to 127
                  newval = mMouseStartVal + mouseDist;
                  newval = newval > 127 ? 127 : newval < 0 ? 0 : newval;
               }
               else if (e->getEditMode() == EditorState::editingOffset) {
                  // clamp to -50 to +50
                  newval = mMouseStartVal + mouseDist;
                  newval = newval > 50 ? 50 : newval < -50 ? -50 : newval;
               }
               else if (data->isMonoMode()) {
                  // since there are limited possible values in monoMode, make the mouse less sensitive
                  mouseDist /= 5;
                  // there are 4 possible values so the max we'll move from current val is 3 (or -3)      
                  newval = getNewMonoVal(mMouseStartVal, mouseDist);
               }
               else { // poly mode   
                      // clamp to 100      
                  newval = mMouseStartVal + mouseDist;
                  newval = newval > SEQ_PROB_ON ? SEQ_PROB_ON : 
                     newval < SEQ_PROB_OFF ? SEQ_PROB_OFF : newval;
               }

               // write to a temporary value on the step itself. This will be written to seq on mouseUp
               // also used during paint
               c->mTempValue = (char)newval;
            } 
            repaint();
         }
         else {  // mouse start val not -1               
            if (mChainStartItem) {
               // dragging for chain, get item we are over
               MouseEvent mm = event.getEventRelativeTo(this);
               c = static_cast<StepCpt * > (getComponentAt(mm.getPosition()));
               if (c && c->getName() == "singleStep") {
                  mChainEndItem = c;
               }
               repaint();
            }
         }
      } // if selecting (right mouse) or not  
      
   } // if length cursor or not

}

void StepPanel::mouseDown(const MouseEvent & event)
{
   EditorState *e = mGlob->mEditorState;
   // get rid of marquee selection
   mDoingMultiSelect = false;
   e->clearSelectedCells();
   bool shifted = false;
   if (event.mods.isShiftDown())
      shifted = true;
   if(e->isShiftReversed())
      shifted = !shifted;


   mChainStartItem = mChainEndItem = 0;

   if (&mLengthEditCursor == event.eventComponent) {
      // length cursor, start drag operation
      mLengthEditCursor.mLenDelta = 0;
   }
   else if(event.eventComponent->getName().compare("singleStep") == 0){
      jassert(mMouseStartVal == MOUSE_STARTVAL_INVALID);
      StepCpt *c = static_cast<StepCpt *>(event.eventComponent);
      SeqDataBuffer *buf = mGlob->mSeqBuf;
      SequenceLayer *data = buf->getUISeqData()->getLayer(e->getCurrentLayer());

      // regardless of anything else we select the cell
      e->setSelectedCells(c->mCol, c->mCol, c->mRow, c->mRow);

      if (shifted && !event.mods.isCommandDown()) { 
         // multi select mode is active
         mDoingMultiSelect = true;
      }

      int newSelItem = (c->mRow*SEQ_MAX_STEPS) + c->mCol;
      if (c->mCol >= data->getNumSteps())
         goto end; // past the end of our pattern, so don't bother doing anything

      if (e->getEditMode() == EditorState::editingChain) {
         // in chain mode, we drag to create arrows between cells
         if (data->getProb(c->mRow, c->mCol) != SEQ_PROB_OFF) {
            mChainStartItem = c;
            mChainCustom = event.mods.isCommandDown() && event.mods.isShiftDown();
            mChainNegTgt = event.mods.isCommandDown() && !event.mods.isShiftDown();

         }
      }

      // select a new item if it's different than current
      if (mSelGridItem != newSelItem) {
         mSelGridItem = newSelItem;
         mSelIsChanging = true; // used by mouse up event
      }
      else
         mSelIsChanging = false;

      // in drag mode, we swipe up and down to edit values
      // keep track of original value so that mouse drag can be offset from this
      if (e->getEditMode() == EditorState::editingVelocity) {
         if (data->getProb(c->mRow, c->mCol) != SEQ_PROB_OFF) // only allow velocity edit on "on" notes
            mMouseStartVal = data->getVel(c->mRow, c->mCol);
      }
      else if (e->getEditMode() == EditorState::editingOffset) {
         if (data->getProb(c->mRow, c->mCol) != SEQ_PROB_OFF) // only allow offset edit on "on" notes
            mMouseStartVal = data->getOffset(c->mRow, c->mCol);
      }
      else if (e->getEditMode() == EditorState::editingSteps) {
         mMouseStartVal = data->getProb(c->mRow, c->mCol);

         // force paint to think that it's changing to make the ui look more responsive
         // (that's all) TODO do this always if selsize==1 rather otherwise it doesnt work as desired when shifted
         if (mMouseStartVal == SEQ_PROB_OFF && !shifted)
            mGrid[mSelGridItem].mTempValue = SEQ_PROB_OFF;
      }

   }
      

end:;
   repaint();

}

void StepPanel::mouseUp(const MouseEvent & event)
{
   bool rightMouse = event.mods.isRightButtonDown();
   String cn = event.eventComponent->getName();
   EditorState *e = mGlob->mEditorState;

   SeqDataBuffer *buf = mGlob->mSeqBuf;
   SequenceLayer *data = buf->getUISeqData()->getLayer(e->getCurrentLayer());
   if (&mLengthEditCursor == event.eventComponent) {
      // length cursor
      if (mLengthEditCursor.mLenDelta || rightMouse) {
         StepCpt *c = mLengthEditCursor.mCpt;
         jassert(c != nullptr);
         int totWidth = data->getNumSteps();
         int len = data->getLength(c->mRow, c->mCol);
         if (rightMouse) // right mouse click resets length to 0
            len = 0;
         else {
            len += mLengthEditCursor.mLenDelta;
            if (len < 0) // if dragging left, we wrap around
               len = totWidth + len;// -1;
               //len = 0; // default length of 1 cell
            else if (len > totWidth - 1) {
               len = (len % (totWidth - 1)) - 1;
            }
         }

         data->setLength(c->mRow, c->mCol, (char)len);

         // also set lengths to match if chord mode is active and any of the notes
         // in the chord with this root are turned on
         if (e->getEditMode() == EditorState::editingSteps && mChordHandler.numUsedIntervals()) {
            for (int i = 0; i < mChordHandler.numUsedIntervals(); i++) {
               int r = mChordHandler.getRowForInterval(i);
               if (r != -1 && data->getProb(r, c->mCol) != SEQ_PROB_OFF) {
                  data->setLength(r, c->mCol, (char)len);
               }
            }
         }


         mLengthEditCursor.mLenDelta = 0;
         jassert(mSelGridItem != -1);
         // this cursor needs to move to the end of the length portion
         moveLengthEditCursor();

         // make sure the cell is selected still if it's on
         mGlob->mEditorState->setSelectedCells(c->mCol, c->mCol, c->mRow, c->mRow);

         buf->swap();
      }
      repaint();
   }
   else if(cn == "singleStep") {
      StepCpt *c = static_cast<StepCpt *>(event.eventComponent);

      if (mDoingMultiSelect) {
         // user has just executed a multi-select (shift drag)
         // ensure that the rectangle is not negative in any way
         mGlob->mEditorState->normalizeSelectedCells();
         mDoingMultiSelect = false;
      }

      if (mChainStartItem) { // if the mouseDown was on an item and we are in chain mode
         if (mChainStartItem == mChainEndItem || mChainEndItem == 0) {
            if (rightMouse) {
               if (data->getNumChainSources(mChainStartItem->mRow, mChainStartItem->mCol)) {
                  // remove all because they right-clicked on a single item
                  data->clearChainSources(mChainStartItem->mRow, mChainStartItem->mCol);
                  buf->swap();
               }
               else if (data->getProb(mChainStartItem->mRow, mChainStartItem->mCol) == SEQ_PROB_NEVER) {
                  // special case where they right clicked in chain mode on a cell that
                  // is in "never" state that has no chains entering it. Delete it.
                  data->clearCell(mChainStartItem->mRow, mChainStartItem->mCol);
                  buf->swap();
               }
               
            }
         }
         else if (mChainEndItem) { // two different items, add a link
            // have start and end, so add a chain item if it's allowed
            if (mChainEndItem->mCol < data->getNumSteps()) {
               if (data->getProb(mChainEndItem->mRow, mChainEndItem->mCol) == SEQ_PROB_OFF) {
                  // create a value there
                  data->setVel(mChainEndItem->mRow, mChainEndItem->mCol, e->getDefaultVelocity());
                  data->setLength(mChainEndItem->mRow, mChainEndItem->mCol, 0); // default length
                  data->setProb(mChainEndItem->mRow, mChainEndItem->mCol, SEQ_PROB_NEVER);
               }
               if (data->addChainSource(mChainEndItem->mRow, mChainEndItem->mCol,
                  mChainStartItem->mRow, mChainStartItem->mCol,mChainNegTgt, false)) {
                  buf->swap();
                  c = mChainEndItem; // so that this items ends up being selected (below where we select item)                                    

                  // if they ctrl drag, we make it negative and also popup the dialog
                  // so they can further customize it
                  if (mChainCustom) {
                     String f=String::formatted("chainAdd|%d|%d|%d|%d", mChainStartItem->mRow,
                        mChainStartItem->mCol, mChainEndItem->mRow, mChainEndItem->mCol);
                     mBroadcaster.sendActionMessage(f);
                  }
               } else {
                  // failed to add due to reaching limit
                  String txt=String::formatted("Maximum number of chains (%d) for this column has been reached", SEQ_MAX_CHAIN_SOURCES);
                  ((SeqAudioProcessorEditor*)mMainEditor)->setAlertText(txt);
               }
            }
         }
         mChainStartItem = mChainEndItem = 0;
         repaint();
      } // if mChainStartItem

      char newProb, newVel, newOffs;
      char oldProb, oldVel, oldOffs;
      oldVel = newVel = data->getVel(c->mRow, c->mCol);
      oldProb = newProb = data->getProb(c->mRow, c->mCol);
      oldOffs = newOffs = data->getOffset(c->mRow, c->mCol);
      mAxis = unknown;
      if (mMouseStartVal != MOUSE_STARTVAL_INVALID &&
         e->getNumSelectedCells() < 2) { // if we are interacting with a valid cell
         // if a drag has occurred where a value actually changed
         if (c->mTempValue != MOUSE_STARTVAL_INVALID && c->mTempValue != mMouseStartVal) {
            // a drag has occurred, tempvalue will hold the new value
            if (e->getEditMode() == EditorState::editingVelocity)
               newVel = c->mTempValue;
            else if (e->getEditMode() == EditorState::editingOffset)
               newOffs = c->mTempValue;
            else if (e->getEditMode() == EditorState::editingSteps)
               newProb = c->mTempValue;
         }
         else {
            if (rightMouse) {
               // if it's a right click (not a drag which is handled above)
               // we either cycle down or delete
               if (e->getEditMode() == EditorState::editingVelocity) {
                  if (e->getMouseRightClickAction() == EditorState::deleteCell) {
                     newVel = 0;
                  }
                  else {
                     newVel = e->velocityCycleNext(newVel, false);
                  }

               }
               else if (e->getEditMode() == EditorState::editingOffset) {
                  newOffs = 0;
               }
               else if (e->getEditMode() == EditorState::editingSteps) {
                  if (e->getMouseRightClickAction() == EditorState::deleteCell) {
                     newProb = SEQ_PROB_OFF;
                  }
                  else {
                     newProb = e->probCycleNext(oldProb, data->isMonoMode(), false);
                  }

               }
            }
            else {
               // if no drag occurred, or if the user dragged but changed their mind      
               // cycle thru some values            
               if (e->getEditMode() == EditorState::editingVelocity) {
                  if (!mSelIsChanging) // only cycle once it has the focus
                     newVel = e->velocityCycleNext(oldVel);
               }
               else if (e->getEditMode() == EditorState::editingSteps) {// editing probability
                     // only cycle if it was 0, or it has the focus already
                  if (!mSelIsChanging || oldProb == SEQ_PROB_OFF)
                     newProb = e->probCycleNext(oldProb, data->isMonoMode());
               }
            }
         }

         //
         // Set new values
         //
         data->setProb(c->mRow, c->mCol, newProb);
         data->setVel(c->mRow, c->mCol, newVel);
         data->setOffset(c->mRow, c->mCol, newOffs);

         if (e->getEditMode() == EditorState::editingSteps) {
            // if it was off going to on, set velocity to default
            // if on going to off set to 0
            if (oldProb == SEQ_PROB_OFF && newProb != SEQ_PROB_OFF) {
               newVel = e->getDefaultVelocity();
               data->setVel(c->mRow, c->mCol, newVel);
               data->setLength(c->mRow, c->mCol, 0); // default length
            }
            else if (newProb == SEQ_PROB_OFF) {
               newVel = 0;
               data->clearCell(c->mRow, c->mCol);               
            }
         }

         // paint chords if applicable
         if (e->getEditMode() == EditorState::editingSteps && mChordHandler.numUsedIntervals()) {
            for (int i = 0; i < mChordHandler.numUsedIntervals(); i++) {
               int r = mChordHandler.getRowForInterval(i);
               if (r != -1) {
                  data->setProb(r, c->mCol, newProb);
                  data->setVel(r, c->mCol, newVel);
                  data->setLength(r, c->mCol, 0);
               }
            }
         }

         buf->swap();

      } //mMouseStartVal != MOUSE_STARTVAL_INVALID etc...

      // move or hide the length edit cursor, if hiding,
      // hide it because selected cell is effectively off
      moveLengthEditCursor();

      // if we have only selected one cell and it's off, just unselect it
      if (e->getNumSelectedCells() == 1 && data->getProb(c->mRow, c->mCol) == SEQ_PROB_OFF)
         e->clearSelectedCells();
         
      c->mTempValue = MOUSE_STARTVAL_INVALID;
      mMouseStartVal = MOUSE_STARTVAL_INVALID;

      repaint();

   }  // interacting with length cursor or not

   
}

void StepPanel::mouseEnter(const MouseEvent & event)
{
   // so that up/down/delete work
   // grabbing focus causes problems in the DAW. it causes the stochas window to come to the front
   // concealing other windows which were above it just by moving the mouse over it
   // fix this, by checking if main editor component has focus, and only grabbing it then.
   // the side effect of this is that if the daw has the focus and you are in chord paint mode, the 
   // keyboard up/down get sent to the daw
   if(mMainEditor->hasKeyboardFocus(true))
      grabKeyboardFocus();

   if (event.eventComponent->getName().compare("singleStep") == 0) {
      StepCpt *c = static_cast<StepCpt *>(event.eventComponent);
      // if we have any chord intervals, calculate them
      if (mChordHandler.numUsedIntervals())      
         mChordHandler.enterCell(c->mRow, c->mCol);         
      if (mRowNotify)
         mRowNotify->setRow(c->mRow);
      repaint();
   }    

   

   
}

void StepPanel::mouseExit(const MouseEvent &)
{
   if (mRowNotify)
      mRowNotify->setRow(-1);
   mChordHandler.leaveCell();
   repaint();
   
   
}

void
StepPanel::resized()
{
   // notepanel has been sized according to how many rows are visible, so we need
   // to move each cell so the bottom n cells are on the visible portion

   int invisibleHeight; // height of invisible area above bottom rows
   invisibleHeight = (SEQ_MAX_ROWS*SEQ_SIZE_CELL_H) - getHeight();

   // steppanel is hosted inside another component
   int stepX = getWidth() / SEQ_MAX_STEPS;
   int idx;
   for (int x = 0; x < SEQ_MAX_STEPS; ++x) {
      for (int y = 0; y < SEQ_MAX_ROWS; ++y) {
         // creates the rectangle     (x,         y,         width, height)
         Rectangle<int> elementBounds(
            x * stepX, 
            (y * SEQ_SIZE_CELL_H) - invisibleHeight,
            stepX,
            SEQ_SIZE_CELL_H);
         // set the size and position of the Toggle light to this rectangle.
         idx = x + SEQ_MAX_STEPS * y;
         mGrid[idx].setBounds(elementBounds);   
      }
   }
   moveLengthEditCursor();

}

void 
StepPanel::paintGhostedLayer(Graphics &g)
{
   /* This draws notes that were on the previously selected layer
   // TODO BUG FIX HERE: if the previous layer has a different scale than current layer,
   // the ghosted notes are not in their proper places. we could either do the math to
   // get the correct placement, or just bail in that case.
   // Also, when one layer has less steps than a different layer, we should repeat them.
   // Also when a layer has a different time sig?
   */
   jassert(mLastLayer >= 0 && mLastLayer < SEQ_MAX_LAYERS);
   juce::Colour ghostColor = 
      mGlob->mEditorState->getColorFor(EditorState::ghostedNotes).withAlpha(0.25f);
   SequenceLayer *data = mGlob->mSeqBuf->getUISeqData()->getLayer(mLastLayer);
   for (int i = 0; i < SEQ_MAX_ROWS*SEQ_MAX_STEPS; i++) {
      StepCpt *cpt = &mGrid[i];
      String txt;
      if (cpt->isStepInRange(mLastLayer) && cpt->isStepInRange() &&  // is it visible?
         data->getProb(cpt->mRow, cpt->mCol) != SEQ_PROB_OFF) {     // is it on?

         Rectangle<int>lenRect = cpt->getBoundsInParent();
         int steplen = data->getLength(cpt->mRow, cpt->mCol);
         if (steplen) {
            // extend it
            lenRect.setWidth(lenRect.getWidth() * (steplen));
         }
         g.setColour(ghostColor);
         g.drawRoundedRectangle(lenRect.toFloat(), 2.0, 5.0);
      } // if visible and turned on
   } // for each grid item

}


static void drawCurve(Point<float> start, Point<float> end, Graphics &g, bool upward, bool bold,
   Colour lineColor, Colour blobColor)
{
   const float curvature = 11.0; // how curvy (smaller number = bigger curve)
   const float arrowsize = 7.0;
   Point<float> cp;
   
   Path p;
   float side;
   if (start.getX() > end.getX())
      side = -1;
   else
      side = 1;
   if (upward)
      side *= -1;
   float thickness;
   if (bold)
      thickness = 4.0f;
   else
      thickness = 2.5f;

   Line<float> ln(start, end);
   float linelen = ln.getLength();
   if (!linelen)
      return; // do nothing

   Point<float> arrowStart = ln.getEnd();
   // adjust for arrowhead;
   if (linelen >=5) {
      linelen -= 5;
      end = ln.getPointAlongLine(linelen);
      ln = Line<float>(start, end);
   }
   // mid point on the line
   Point<float> mid = ln.getPointAlongLine(linelen / 2);

   // 90 degree line from that point
   float angle = ln.getAngle(); //subtr 90 deg
   Line<float> ctrlLine = 
      Line<float>::fromStartAndAngle(mid, linelen / curvature, angle + (1.5708f * side));

   // end of that line becomes control point for curve
   cp = ctrlLine.getEnd();

   // Calculate arrowhead (note that the values were derived by trial and error)
   
   ctrlLine = Line<float>::fromStartAndAngle(arrowStart, arrowsize, angle + (-2.7f * side));
   Point<float> arrA = ctrlLine.getEnd();
   ctrlLine = Line<float>::fromStartAndAngle(arrowStart, arrowsize, angle + (2.5f * side));
   Point<float> arrB = ctrlLine.getEnd();

   // draw the curve
   g.setColour(lineColor);
   PathStrokeType stroke(thickness, 
      PathStrokeType::JointStyle::curved, PathStrokeType::EndCapStyle::rounded);

   p.startNewSubPath(start);
   p.quadraticTo(cp, end);
   g.strokePath(p, stroke);

   
   // draw the arrow head
   stroke = PathStrokeType(thickness * 2.0f / 3.0f, PathStrokeType::JointStyle::mitered, PathStrokeType::EndCapStyle::rounded);

   p.clear();
   p.addTriangle(arrowStart, arrA, arrB);
   g.strokePath(p, stroke);
   g.fillPath(p);

   // draw the blob in the source cell that indicates whether it's negative source flag
   g.setColour(blobColor);
   Rectangle<float> r;   
   r.setSize(11, 11);
   r.setCentre(ln.getStart());
   g.fillEllipse(r);
   g.setColour(Colours::black.brighter(.5));
   g.drawEllipse(r, 2.0f);
   
   
}

void
StepPanel::paintChains(Graphics &g)
{
   int startX, endX, startY, endY;
   EditorState *e = mGlob->mEditorState;
   
   // do all non-selected chains first
   for (int y = 0; y < SEQ_MAX_ROWS; y++) {
      for (int x = 0; x < SEQ_MAX_STEPS; x++) {
         paintSingleChain(g, x, y, false);            
      }
   }

   // do highlighted last so its on top
   if (e->getSelectedCells(&startX, &endX, &startY, &endY)) {
      if (startX == endX && startY == endY) {
         paintSingleChain(g, startX, startY, true);
      }
   }
   
   // do the chain that is being added currently
   if (mChainStartItem && mChainEndItem) {
      juce::Colour selColor, srcBlobColor, posColor;
      Point<int> start = mChainStartItem->getBounds().getCentre();
      Point<int> end = mChainEndItem->getBounds().getCentre();
      bool up = false;
      if (mChainStartItem->mRow > SEQ_MAX_ROWS - 4 || mChainEndItem->mRow > SEQ_MAX_ROWS - 4)
         up = true;
      if(mChainNegTgt)
         posColor = e->getColorFor(EditorState::chainNegative);
      else
         posColor = e->getColorFor(EditorState::chainPositive);
      selColor = posColor.withAlpha(0.5f);
      srcBlobColor = posColor.withAlpha(0.5f);
      drawCurve(start.toFloat(), end.toFloat(), g, up, true, selColor, srcBlobColor);
   }
}

void StepPanel::paintSingleChain(Graphics & g, int x, int y, bool current)
{
   StepCpt *chStart, *chEnd;
   chEnd = &mGrid[x + SEQ_MAX_STEPS *y];
   if (chEnd->isStepInRange()) {
      EditorState *e = mGlob->mEditorState;
      juce::Colour posColor, negColor, posSel, negSel, selColor, srcBlobColor;
      posColor = e->getColorFor(EditorState::chainPositive);
      negColor = e->getColorFor(EditorState::chainNegative);
      posSel = e->getColorFor(EditorState::chainPositiveSel);
      negSel = e->getColorFor(EditorState::chainNegativeSel);
      srcBlobColor = posColor.withAlpha(0.5f);
      SequenceLayer *data = mGlob->mSeqBuf->getUISeqData()->getLayer(e->getCurrentLayer());
      // TODO optimize?
      int sx, sy;
      bool negtgt;
      bool negsrc;
      int iterate = -1;
      while (data->getChainSource(y, x, &iterate, &sy, &sx, &negtgt, &negsrc)) {
         chStart = &mGrid[sx + SEQ_MAX_STEPS * sy];
         if (chStart->isStepInRange()) {
            bool up = false;
            Point<int> start = chStart->getBounds().getCentre();
            Point<int> end = chEnd->getBounds().getCentre();
            if (current) { // chains leading to currently selected cell
               if (negtgt)
                  selColor = negSel;
               else
                  selColor = posSel;

               if (negsrc)
                  srcBlobColor = negSel;
               else
                  srcBlobColor = posSel;
            }
            else {
               if (negtgt)
                  selColor = negColor;
               else
                  selColor = posColor;

               if (negsrc)
                  srcBlobColor = negColor;
               else
                  srcBlobColor = posColor;
            }

            if (y > SEQ_MAX_ROWS - 4 || sy > SEQ_MAX_ROWS - 4)
               up = true;
            drawCurve(start.toFloat(), end.toFloat(), g, up, current, selColor, srcBlobColor);

         }
      }
   }
}


void 
StepPanel::paintSelectedCells(Graphics &g)
{
   EditorState &e = *mGlob->mEditorState;
   int startX, endX, startY, endY;
   
   if (e.getSelectedCells(&startX, &endX, &startY, &endY)) {
      juce::Colour overlay =
         e.getColorFor(EditorState::cellSelection).withAlpha(0.6f);
      StepCpt *cpttl, *cptbr;

      Rectangle<int> b;
      cpttl = &mGrid[startX + SEQ_MAX_STEPS *startY];
      cptbr = &mGrid[endX + SEQ_MAX_STEPS * endY];
      b = cpttl->getBoundsInParent().getUnion(cptbr->getBoundsInParent());
      g.setColour(overlay);
      g.drawRoundedRectangle(b.toFloat(), 2.0, 5.0);
   }
}

void 
StepPanel::paintLengthBars(Graphics & g)
{
   /* This paints the length bars, and also highlights the length bar
   // if it's the selected cell
   */
   EditorState *e = mGlob->mEditorState;
   Colour overlay = e->getColorFor(EditorState::cellSelection).withAlpha(0.6f);   
   SequenceLayer *data = mGlob->mSeqBuf->getUISeqData()->getLayer(e->getCurrentLayer());

   // dont show length bars on "off" row (seq_max_rows-1)
   for (int i = 0; i < (SEQ_MAX_ROWS-1)*SEQ_MAX_STEPS; i++) {
      StepCpt *cpt = &mGrid[i];
      juce::Colour c;
      juce::String txt;
      if (cpt->getEffectiveColorAndText(c, txt) &&  // is it visible?
         data->getProb(cpt->mRow, cpt->mCol)!= SEQ_PROB_OFF) {     // is it on?
         Rectangle<int> b= cpt->getBoundsInParent();
         // for our length extender (if any)
         Rectangle<int>lenRect;
         
         // how many steps before we need to wrap
         int maxBeforeWrap=data->getNumSteps() - cpt->mCol - 1;
         int wrapped = 0;
         // determine dimensions of the length extender
         int steplen = data->getLength(cpt->mRow, cpt->mCol);
         if (steplen > maxBeforeWrap) {
            wrapped = steplen - maxBeforeWrap;
            steplen = maxBeforeWrap;
         }

         if (steplen > 0) { // length to right of cell
            lenRect = b;
            lenRect.removeFromTop(lenRect.getHeight() / 2);
            // move to end of cell
            lenRect.setX(lenRect.getX() + lenRect.getWidth());
            lenRect.setWidth(b.getWidth() * steplen);
            // color is same as cell but with alpha
            g.setColour(c.withAlpha(0.4f));
            g.fillRect(lenRect.toFloat());
         }

         if (wrapped) { // length that wrapped around to beginning
            lenRect = b;
            lenRect.removeFromTop(lenRect.getHeight() / 2);
            // move to end of cell
            lenRect.setX(0);
            lenRect.setWidth(b.getWidth() * wrapped);
            // color is same as cell but with alpha
            g.setColour(c.withAlpha(0.4f));
            g.fillRect(lenRect.toFloat());            
         }

         // highlight selected item's length bar and cursor for moving it
         if (i == mSelGridItem) {
            int totWidth = data->getNumSteps();
            steplen = data->getLength(cpt->mRow, cpt->mCol);
            // add or subtract the delta
            steplen += mLengthEditCursor.mLenDelta;
            if (steplen < 0) // if dragging left, we wrap around
               steplen = totWidth + steplen; // -1;
            else if (steplen > totWidth - 1) {
               steplen = (steplen % (totWidth - 1)) - 1;
            }
            int wrapSelected = 0;
            if (steplen > 0) { // steplen will be 0 or a positive value
               if (steplen > maxBeforeWrap) {
                  wrapSelected = steplen - maxBeforeWrap;
                  steplen = maxBeforeWrap;                  
               }

               g.setColour(overlay);
               // fill in space occupied by the length bar
               lenRect = b;
               lenRect.removeFromTop(lenRect.getHeight() / 2);
               // move to end of cell
               lenRect.setX(lenRect.getX() + lenRect.getWidth());
               lenRect.setWidth(b.getWidth() * steplen);
               g.fillRect(lenRect.toFloat());

               if (wrapSelected) {
                  lenRect = b;
                  lenRect.removeFromTop(lenRect.getHeight() / 2);
                  lenRect.setX(0);
                  lenRect.setWidth(b.getWidth() * wrapSelected);
                  g.fillRect(lenRect.toFloat());
               }

               // fill in space occupied by the top of the length edit cursor
               lenRect.removeFromLeft(lenRect.getWidth() - mLengthEditCursor.getWidth());
               lenRect.setY(lenRect.getY() - (b.getHeight() / 2));
               g.fillRect(lenRect.toFloat());
               
               
            }
         } // if grid item is the selected one      
      } // if visible and turned on
   } // for each grid item


}

void StepPanel::shiftSelectedCells(bool xaxis, bool positive)
{
   SeqDataBuffer *buf = mGlob->mSeqBuf;
   EditorState *e = mGlob->mEditorState;
   SequenceLayer *sl = buf->getUISeqData()->getLayer(e->getCurrentLayer());
   int startx, endx, starty, endy;
   int x, y, tmp;
   // direction to move
   int xdir = 1;
   int ydir = 1;
   int copyFromX = 0;
   int copyFromY = 0;
   
   if (!e->getNumSelectedCells())
      return;

   mGlob->mEditorState->getSelectedCells(&startx, &endx, &starty, &endy);
   
   if (xaxis) { // shifting left or right      
      if (positive) { // shifting right
         if (endx >= sl->getNumSteps()-1)
            return; // can't do it
         endx++; // expand the area right
         xdir = -1;
         // swap these since we are going right to left
         tmp = startx;
         startx = endx;
         endx = tmp;
      }
      else {
         if (startx < 1)
            return; // can't
         startx--; // expand left
      }
      copyFromX = xdir;
   } else { // shifting up or down
      if (positive) { // down
         if (endy >= SEQ_MAX_ROWS-1)
            return; // no no
         endy++; // expand downward
         ydir = -1;
         // swap these since we are going bottom to top
         tmp = starty;
         starty = endy;
         endy = tmp;
      }
      else { // up
         if (starty < SEQ_MAX_ROWS - sl->getMaxRows()+1)
            return; // no
         starty--; // expand up
      }
      copyFromY = ydir;
   }
   

   for (x = startx; x != endx+xdir; x+=xdir) {
      for (y = starty; y != endy+ydir; y+=ydir) {
         if ((x == endx && copyFromX) || (y == endy && copyFromY)) {
            sl->clearCell(y, x);
         }
         else {
            sl->copyCell(y, x, y + copyFromY, x + copyFromX);
         }
      }
   }

   // shift the cell selection
   mGlob->mEditorState->getSelectedCells(&startx, &endx, &starty, &endy);
   if (xaxis) {
      if (positive) {
         endx++;
         startx++;
      }
      else {
         endx--;
         startx--;
      }
   }
   else {
      if (positive) {
         endy++;
         starty++;
      }
      else {
         starty--;
         endy--;
      }
   }

   // move selection to match new location
   mGlob->mEditorState->setSelectedCells(startx, endx, starty, endy);

   mGlob->mSeqBuf->swap();

}

void StepPanel::deleteSelectedCells()
{
   SequenceData *sd = mGlob->mSeqBuf->getUISeqData();
   SequenceLayer *sl = sd->getLayer(mGlob->mEditorState->getCurrentLayer());
   int startx, endx, starty, endy;
   int x, y;

   mGlob->mEditorState->getSelectedCells(&startx, &endx, &starty, &endy);

   for (x = startx; x <= endx; x++) {
      for (y = starty; y <= endy; y++) {
         sl->clearCell(y, x);
      }
   }
   mGlob->mSeqBuf->swap();
}

void StepPanel::setRowNotify(StepRowNotify * notify)
{
   mRowNotify = notify;
}


void
StepPanel::moveLengthEditCursor()
{
   /*This handy function is a catch-all for getting the length edit cursor sorted out
     and put where it needs to be (or hidden) depending on whether anything's selected,
     whether what is selected is visible, and has a value
    */
   
   if (mSelGridItem !=-1) {
      EditorState *e = mGlob->mEditorState;
      SeqDataBuffer *buf = mGlob->mSeqBuf;
      SequenceLayer *data = buf->getUISeqData()->getLayer(e->getCurrentLayer());
      StepCpt *c = &mGrid[mSelGridItem];      
      if (c->mCol < data->getNumSteps() &&
         data->getProb(c->mRow, c->mCol) != SEQ_PROB_OFF &&
         data->getCurNote(c->mRow) != -1) { // don't allow length edit on "off" row
         // how long length can be before we need to wrap around
         int maxBeforeWrap = data->getNumSteps() - c->mCol - 1;
         // length will be relative to left edge (in num cells)
         int length = data->getLength(c->mRow, c->mCol);
         if (length > maxBeforeWrap) {
            length -= maxBeforeWrap;
            length--;
         }
         else
            length = c->mCol + length;

         // move length edit control to correct position which is on the right side
         // of the current cell
         Rectangle<int> dragCursorRect;
         dragCursorRect = c->getBounds();
         // enlarge for the length of the cell
         dragCursorRect.setX(c->getWidth()*length);
         dragCursorRect.removeFromLeft(dragCursorRect.getWidth() - SEQ_SIZE_LENCURSOR);

         mLengthEditCursor.setBounds(dragCursorRect);
         mLengthEditCursor.setVisible(true);
         mLengthEditCursor.mCpt = c;
         goto end;
      }
   }
   
   // not visible currently
   mLengthEditCursor.setVisible(false);
   mLengthEditCursor.mCpt = nullptr;
end:;
}

/*===========================================================================================
StepCursorCpt implementations
============================================================================================*/
StepCursorCpt::StepCursorCpt() : Component("lengthCursor"), mCpt(0),mLenDelta(0)
{
   setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
}

void StepCursorCpt::paint(Graphics &)
{   
}
/*===========================================================================================
SeqChordHandler implementations
============================================================================================*/
void
SeqChordHandler::recalcNotes() {
   SequenceLayer *lay = mGlob->mSeqBuf->getUISeqData()->getLayer(mGlob->mEditorState->getCurrentLayer());
   int row;
   int i;
   int noteNum;

   int rootNote = lay->getCurNote(mRootRow);
   int rowNote;

   // first clear them
   for (i = 0; i < mUsed; i++)
      mInts[i].rowNum = -1;

   // no chord on the "off" item
   if (rootNote == -1)
      return;

   // find the rows that make up the intervals
   for (row = 0; row < SEQ_MAX_ROWS; row++) {
      rowNote = lay->getCurNote(row);
      for (i = 0; i < mUsed; i++) {
         if (i >(mUsed - mInversion) - 1) {
            noteNum = (mInts[i].semitones - 12) + rootNote;
         }
         else {
            noteNum = mInts[i].semitones + rootNote;
         }
         if (noteNum == rowNote) {
            if (mInts[i].rowNum == -1) {
               mInts[i].rowNum = row;
            }
            
            else if(std::abs(mRootRow-row) < std::abs(mRootRow- mInts[i].rowNum)){
               // if this is closer to root row than what we had choose it instead
               // since we can wrap around 
               mInts[i].rowNum = row;
            }
            //break;
         }
      }
   }
}

SeqChordHandler::SeqChordHandler(SeqGlob * glob) : mGlob(glob), mInversion(0), mInCell(false) {
   clearIntervals();
}

void SeqChordHandler::decreaseInversion() {
   if (mInversion > 0) {
      mInversion--;
      if (mInCell)
         recalcNotes();

   }
}

void SeqChordHandler::increaseInversion() {
   if (mInversion < mUsed) {
      mInversion++;
      if (mInCell)
         recalcNotes();
   }
}

// clear all intervals. used when rebuilding chord structure

void SeqChordHandler::clearIntervals() {
   mUsed = 0;
   mInversion = 0;
   //clearRows();
}

// add an interval (which is number of semitones from a root note)

void
SeqChordHandler::addInterval(int semitones) {
   if (mUsed >= SEQ_MAX_CHORD_COUNT) {
      jassertfalse;
      return;
   }
   mInts[mUsed].semitones = semitones;
   mUsed++;
}

int SeqChordHandler::numUsedIntervals() {
   return mUsed;
}

void SeqChordHandler::leaveCell() {
   mInCell = false;
}

// returns true if cursor is in a cell and we have chord data

bool SeqChordHandler::inCell() {
   if (!mUsed)
      return false;
   return mInCell;
}

// given the row where the root note is, determine the row idx's of the
// rest of the intervals in the chord
// col is not used to make the determination, just stored

void SeqChordHandler::enterCell(int rootRow, int col) {
   jassert(rootRow >= 0 && rootRow < SEQ_MAX_ROWS);
   mInCell = true;
   if (!mUsed)
      return;
   mCol = col;
   mRootRow = rootRow;
   recalcNotes();
}

// return the row number for an interval index, or -1 if it's
// not used for that interval.
// only valid after a call to determineRows

int SeqChordHandler::getRowForInterval(int idx) {
   jassert(idx >= 0 && idx < SEQ_MAX_ROWS);
   return mInts[idx].rowNum;
}

int SeqChordHandler::getCol() {
   return mCol;
}

int SeqChordHandler::getRootRow() {
   return mRootRow;
}
