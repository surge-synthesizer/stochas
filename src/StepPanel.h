/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef STEPPANEL_H_
#define STEPPANEL_H_

#include "SequenceData.h"
#include "EditorState.h"
#include "Constants.h"
#include "CommonComponents.h"

/**
This component represents a single cell on the sequencer grid
*/
class StepCpt : public Component {   
public:
   // these guys refer to a cell in the actual sequence data (that the processor sees)
   int mRow;
   int mCol;
   SeqGlob *mGlob;
   // if we are in the act of dragging, this will hold the temporary value (otherwise -1)
   char mTempValue;
   StepCpt();
   void paint(Graphics &g) override;
   // determine the color of the cell and the text content
   // depending on mode, value, etc
   // returns false if the cell is not going to be visible (in this case the 
   // out parameters won't be touched)
   bool getEffectiveColorAndText(juce::Colour &c, juce::String &txt, int layer=-1);
   // returns true if step is visible in current view (within row and column bounds, step page, etc)
   bool isStepInRange(int layer = -1);
};

/** This is the small cursor that is at the right of the selected cell and is used
to drag the width of the cell left or right
*/
class StepCursorCpt : public Component {
public:
   StepCpt *mCpt; // which cpt we are associated with or null if none
   int mLenDelta; // length delta (in num of cells)
   StepCursorCpt();
   void paint(Graphics &g) override;
};


// handles chord paint

class SeqChordHandler {
   SeqGlob *mGlob;
   struct Interval {
      int semitones; // number of semitones UP from the root note
      int rowNum;    // will be populated with row numbers or -1 if not valid
   };
   Interval mInts[SEQ_MAX_CHORD_COUNT];
   int mUsed;
   int mCol;
   int mRootRow;
   int mInversion;
   bool mInCell;   
   void recalcNotes();

public:
   SeqChordHandler(SeqGlob *glob);

   void decreaseInversion();

   void increaseInversion();

   // clear all intervals. used when rebuilding chord structure
   void clearIntervals();
   // add an interval (which is number of semitones from a root note)
   void addInterval(int semitones);

   int numUsedIntervals();

   void leaveCell();

   // returns true if cursor is in a cell and we have chord data
   bool inCell();

   // given the row where the root note is, determine the row idx's of the
   // rest of the intervals in the chord
   // col is not used to make the determination, just stored
   void enterCell(int rootRow, int col);


   // return the row number for an interval index, or -1 if it's
   // not used for that interval.
   // only valid after a call to determineRows
   int getRowForInterval(int idx);

   int getCol();
   int getRootRow();
};

// steppanel can notify when mouse is over a specific row (notepanel *wink* this means you)
class StepRowNotify {
public:
   virtual void setRow(int row) = 0;
   virtual ~StepRowNotify(){}
};
#define MOUSE_STARTVAL_INVALID -99
/**
This is the panel that hosts the grid cells (only)
*/
class StepPanel : public Component, public KeyListener {
   // this is ONLY used for:
   // - checking whether window has focus
   // - set alert text when chain add fails
   Component *mMainEditor; 
   SeqGlob *mGlob;
   int mId;
   CptNotify *mNotify;
   int mPage;
   bool mMacro;
   StepCpt mGrid[SEQ_MAX_ROWS*SEQ_MAX_STEPS];
   StepCursorCpt mLengthEditCursor;
   int mSelGridItem;
   bool mSelIsChanging;
   int mLastLayer;            // will be set to whichever the most previous layer was, or -1 if none
                              // used to show ghosting of previous layer
   int mCurLayer;
   SeqMouseAxis mAxis;       // which axis we are dragging on

   // this keeps track of original value when mouse down event first happens
   // will be -99 if nothings going on
   char mMouseStartVal;

   // keep track of starting item for chain drag
   StepCpt *mChainStartItem;
   StepCpt *mChainEndItem;
   
   bool mChainCustom; // ctrl drag   we create a negative line
   bool mChainNegTgt; // if ctrl shift dragging we popup custom
   StepRowNotify *mRowNotify;
   ActionBroadcaster mBroadcaster; // for sending messages to plugineditor

   int mCurPosition; // current play position

   bool mDoingMultiSelect; // set to true while dragging multiselect

   bool keyPressed(const KeyPress &key, Component *originatingComponent) override;

   void paintOverChildren(Graphics &g) override;
   // mouse drag handles drag events on any of the grid cells
   void mouseDrag(const MouseEvent &event) override;
   // mouse down grabs original value so we know what it's offset is
   void mouseDown(const MouseEvent &event) override;
   // mouse up finalizes value
   void mouseUp(const MouseEvent &event) override;
   void mouseEnter(const MouseEvent &event) override;
   void mouseExit(const MouseEvent &event) override;
   void resized() override;   

   void paintGhostedLayer(Graphics &g);
   void paintChains(Graphics &g);
   void paintSingleChain(Graphics &g, int x, int y, bool current);
   void paintSelectedCells(Graphics &g);
   void paintLengthBars(Graphics &g);

   // if xaxis is false, we are shifting on y axis, if positive is false we are shifting left/up (depending on axis)
   void shiftSelectedCells(bool xaxis, bool positive);
   void deleteSelectedCells();
public:
   void setRowNotify(StepRowNotify *notify);
   SeqChordHandler mChordHandler;

   // reposition the length edit cursor to correspond to selected cell
   // It will hide it if the selected cell is not currently visible,
   // or has a prob of 0, or for many other reasons which can't currently be named
   void moveLengthEditCursor();

   // make sure that no cell is selected (does not repaint) implicitly calls moveLengthEditCursor
   // NOTE that this doesn't affect multi-selection
   // TODO - integrate multi-selection and single-selection better
   void unselectAll();

   StepPanel(SeqGlob *glob, int id,Component *mainCpt, CptNotify *notify, String name = "undefined" /* will be set later*/);

   // call this when layer is changing (or not. ie if layer button is clicked)
   // it is used to keep track of ghosting stuff
   void layerHasChanged();
   
   // check to see if play position has changed and refresh if so
   void check();
};

#endif
