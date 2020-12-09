/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef EDITOR_STATE_H_
#define EDITOR_STATE_H_

#include "SequenceData.h"
/**
The Editor State keeps track of current editor settings (does not keep track of any data in the 
sequencer, just current editor settings such as whether we are in velocity edit mode, how sensitive
the mouse is, etc)
This will also return needed colors for various things (in case we want to allow theming later)
*/



class EditorState {
public:
   EditorState();
   ~EditorState();
   enum EditMode {
      editingSteps,
      editingVelocity,
      editingChain,
      editingOffset
   };

   // note the order of these need to correspond with ordering (and elements) in Colors.h
   enum coloredElements {
      background=0,
      stepDisabled,      // eg if we have less steps than 16
      stepOff,           // step is off (ie 0 prob)
      stepNeverProb,     // probability of never (only useful for chain)
      stepLowProb,       // low prob
      stepMediumProb,    // med prob
      stepHighProb,      // high prob
      stepLowVel,        // low velocity
      stepMedVel,        // medium velocity
      stepHighVel,       // high velocity
      playIndicatorOff,
      playIndicatorOn,
      noteEditable,
      noteLocked,
      toggleOff,        // for things on the main ui that have a toggle state
      toggleOn,
      button,           // other buttons
      interact,         // if we are interacting with something with the mouse
      border,           // any borders around things
      measureStart,
      beatStart,
      grooveEdit,
      ghostedNotes,     // ghosted notes from previous layer
      cellSelection,    // cursor around selected cells, and length cursor
      chainPositive,    // chain/dependency positive color
      chainNegative,    // chain/dependency negative color
      chainPositiveSel,    // chain/dependency positive color selected
      chainNegativeSel,    // chain/dependency negative color selected
      alertBackground,  // for the alert popup
      recordActive,     // record button when recording or standby
      recordPassive,    // record button when not recording
      //--- add more here^^^
      Count             // number of elements
   };

   int mUserColors[coloredElements::Count];
   
   // edit mode - we can either be editing steps, or velocity (or some other thing)
   EditMode getEditMode();
   void setEditMode(EditMode newMode);
   // return the color to be used for a specific element
   juce::Colour getColorFor(coloredElements e);
   // given current velocity, returns next velocity value that would be needed when mouse is clicked
   // if next is false it cycles backwards
   char velocityCycleNext(char curVel, bool next=true);
   // given current probability, returns next prob value when mouse is clicked
   // either in mono or poly mode
   // if next is false it cycles backwards
   char probCycleNext(char curprob, bool mono, bool next=true);

   int getCurrentLayer() { return mCurrentLayer; }
   void setCurrentLayer(int lay) {
      jassert(lay >= 0 && lay < SEQ_MAX_LAYERS);
      mCurrentLayer = lay;
   }
   // sets which step number is leftmost (panning left and right)
   // (0..n where n=max steps) or -1 for macro view
   void setVisibleStep(int step);

   // determines which step is visible on the left most side
   // (0..n where n=max steps) or -1 for macro view
   int getVisibleStep();

   /*=============================================================================================
     The following items are persisted to a config file
     =============================================================================================*/

   // mouse sensitivity for dragging stuff. 1..SEQ_MOUSE_SENSE_MAX where 1 is highest and n is lower
   int getMouseSense();
   void setMouseSense(int val);

   // default velocity for newly placed steps
   char getDefaultVelocity();
   void setDefaultVelocity(char val);

   // lowest octave number (could be -2, -1, 0)
   int getLowestOctave();
   void setLowestOctave(int val);

   // default probability for newly placed steps
   char getDefaultProbability(bool mono);
   void setDefaultProbability(char val, bool mono);

   // color theme where 0=dark and 1=light
   int getColorTheme();
   void setColorTheme(int val);

   enum mouseRightClickAction {
      deleteCell=0,
      cycleDown
   };
   mouseRightClickAction getMouseRightClickAction();
   void setMouseRightClickAction(mouseRightClickAction action);
   bool getSelectedCells(int *lowCol=0, int *hiCol=0, int *lowRow=0, int *hiRow=0);
   void setSelectedCells(int lowCol, int hiCol, int lowRow, int hiRow);
   // sets highCol and highRows to these values
   void enlargeSelectedCells(int col, int row);
   // makes sure that highrow is > low row, etc
   void normalizeSelectedCells();
   void clearSelectedCells();
   // return the number of multi-selected cells
   int getNumSelectedCells();
   // return true if the cell at the location is multi-selected
   bool isCellSelected(int col, int row);
   bool isShiftReversed();
   void setShiftReversed(bool isReversed);
   void setFileDirectory(const String &dir);
   String getFileDirectory();
   bool getKeyboardDisabled();

   // value here is divided by 1000
   // THREAD SAFETY WARNING! This 'get' function is called from
   // the audio thread, and the 'set' is called from the ui thread
   // no attempt has been made to ensure atomicity but it should be fine.
   void setPPQOffset(int offset);
   int getPPQOffset();

   void setScaleFactor(int factor);
   int getScaleFactor();

private:
   // read or write settings to the config file
   void configSerialization(bool read);
   // read/write individual int setting (min max ensure that values read from file are not out of range)
   // def is default value
   void configGetSetInt(juce::PropertiesFile *pf, bool read, int &val, 
      const char *key, int min, int max, int def);

   void configGetSetString(juce::PropertiesFile *pf, bool read, String &val,
      const char *key, const String &defaultVal);

   // load user colors from file into user slot
   // create the skin file if it doesn't exist
   void loadColorsFromFile();
   

   // config values
   int mShiftReversed;
   int mMouseSense;
   int mDefaultVelocity;
   int mLowestOctave;
   int mDefaultMono;
   int mDefaultPoly;
   mouseRightClickAction mRightClickAction;
   int mColorTheme; // 0=dark, 1=light
   String mDefaultFileDir;

   EditMode mEditMode;   
   int mCurrentLayer;
   int mVisibleStep;
   bool mSelectedCells;
   int mSelectedLowRow;
   int mSelectedHighRow;
   int mSelectedLowCol;
   int mSelectedHighCol;
   int mKeyboardDisabled;
   int mPPQOffset;
   int mScaleFactor;
public:
};


class SeqAudioProcessor;

// This holds pointers to our big objects to make it easy to pass around
// TODO not sure if it really belongs here
class SeqGlob {
public:
   EditorState *mEditorState;
   SeqDataBuffer *mSeqBuf;
   SeqProcessorNotifier *mAudNotify;

   // for sending data from UI thread to audio thread (processor)
   // the processor holds the actual object that this refers to
   SeqFifo *mProcessNotify;

   // only use this with atomics, don't access it from ui thread
   SeqAudioProcessor *mProcessor; // this is ONLY used to set the unlock bit in the processor
                                  // by calling the unlock method

   // this is used as a callback from SequenceData whenever swap() is called.
   // It's used to notify the processor that data has changed (so that it can
   // tell the daw that a save is needed)
   static void changeNotify(void *p) {
      ((SeqGlob*)(p))->mProcessNotify->addToFifo(SEQ_NOTIFY_HOST, 0, 0);      
   }

   SeqGlob(SeqDataBuffer *d, EditorState *es, SeqProcessorNotifier *n, SeqFifo *fif, SeqAudioProcessor *p) :
      mEditorState(es), mSeqBuf(d), mAudNotify(n), mProcessNotify(fif), mProcessor(p) 
   {
      // tell it to call changeNotify when some data changes
      mSeqBuf->setChangeNotify(changeNotify, this);
   }

   ~SeqGlob() {
      mSeqBuf->setChangeNotify(0,0);
   }

};

// this is used to generate a "truly random" seed to be used for seeding other
// random generators
int64 generateNewRootSeed();

// another shared funtion I don't know where to put
void fixColors(EditorState *e, Component * cpt);
// call this from "editorShown" when a label gets converted to a text edit box
// to fix colors and optionally limit max length
void fixDynamicTextEditBox(EditorState *e, TextEditor &te, int maxlen = 0);

void gradientFill(EditorState &e, Component *c, Graphics &g);
#endif
