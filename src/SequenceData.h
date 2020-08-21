/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef SEQUENCEDATA_H_
#define SEQUENCEDATA_H_

/**
This maintains a model of the current state. It includes everything needed to save/load
a patch or preset. It does not include user preferences. It does not include current state
of midi-switched values (eg midi pattern change). That state is stored in stocha engine
and overrides the state here (so you will not see it here)

Do not add pointers to any of these!
*/


#include "Constants.h"
#include "Scale.h"

class SequenceData;

// this was gotten and modified from the example on juce.com
#define FIFO_SIZE 16
class SeqFifo
{
public:
   struct DataMember {
      int value1;
      int value2;
      int value3;
   };
   SeqFifo() : abstractFifo(FIFO_SIZE) {}
   // add a piece of data to the fifo, return true if success
   bool addToFifo(int value1, int value2, int value3) {
      bool ret = false;
      int start1, size1, start2, size2;
      abstractFifo.prepareToWrite(1, start1, size1, start2, size2);
      // since we are only writing one element we don't care about size2
      if (size1 > 0) {
         ret = true;
         myBuffer[start1].value1 = value1;
         myBuffer[start1].value2 = value2;
         myBuffer[start1].value3 = value3;
      }
      abstractFifo.finishedWrite(size1 + size2);
      return ret;
   }
   // read a piece of data from fifo, return true if success
   bool readFromFifo(int *value1, int *value2, int *value3)
   {
      bool ret = false;
      int start1, size1, start2, size2;
      abstractFifo.prepareToRead(1, start1, size1, start2, size2);
      if (size1 > 0) {
         *value1 = myBuffer[start1].value1;
         *value2 = myBuffer[start1].value2;
         *value3 = myBuffer[start1].value3;
         ret = true;
      }
      abstractFifo.finishedRead(size1 + size2);
      return ret;
   }

   void clearFifo()
   {
      abstractFifo.reset();
   }
private:
   AbstractFifo abstractFifo;
   DataMember myBuffer[FIFO_SIZE];
};

/* The info in this struct is specific to each layer. 
*/
class SequenceLayer {
   friend class SequenceData;

   // single cell of data
   struct Cell {
      // probability -1 - 100
      // non-negative value denotes that the cell is "on"
      // in poly mode a value of 100 means always play
      // a value of 0 means never play and can only be activated by chains
      char prob;     
      // velocity 0-127
      char velo;
      // length in number of cells 
      // 0 which is default, means single step, so 1 here means 2 steps)
      char length;   
      char offset; // a range from -50 to +50 that indicates how much to move the cell in time
      Cell() :prob(-1), velo(0), length(0), offset(0) {}
   };

   // row of cells (horiz)
   struct Row {
      Cell mSteps[SEQ_MAX_STEPS];
   };

   // for chain information
   struct SourceCell {
      unsigned char col;      // source info
      unsigned char row;
      unsigned char targetrow; // which row it applies to
      char flags;
      SourceCell() : col(0), row(0), targetrow(0), flags(0) {}
   };
   // each col has a chainsource which represents cells in other
   // columns that determine notes to play in this col. each cell in the col
   // shares this pool
   struct ChainSource {
      SourceCell cells[SEQ_MAX_CHAIN_SOURCES];
   };


   // each layer has a set of patterns. All the patterns for a layer
   // share the note name/values, num rows and num steps
   struct Pattern {
      Row mRows[SEQ_MAX_ROWS];  

      // each step (column) in the pattern gets a pool of these to share
      ChainSource mChains[SEQ_MAX_STEPS];

      char mName[SEQ_PATTERN_NAME_MAXLEN];
      Pattern() {
         strncpy(mName, SEQ_DEFAULT_PAT_NAME, SEQ_PATTERN_NAME_MAXLEN);
         mName[SEQ_PATTERN_NAME_MAXLEN - 1] = 0;
      }
   };

   // all pattern data
   Pattern mPats[SEQ_MAX_PATTERNS];

   // Keep track of which midi note will play, and what the user wants to name it
   // in the case of custom
   struct Note {
      char note;
      char noteName[SEQ_MAX_NOTELABEL_LEN];
      Note() : note(0) {
         memset(noteName, 0, SEQ_MAX_NOTELABEL_LEN);
      }
   };

   // A set of current notes for the layer
   struct NoteSet {
      Note notes[SEQ_MAX_ROWS];
   };

   // note sets (custom and standard)
   enum WhichNoteSet {
      customSet = 0,
      standardSet,
      noteSetCount
   };
   // we store a standard and a custom set.
   // the standard set is overwritten each time the user changes key/scale/oct
   // the custom set can be customized
   NoteSet mNoteSets[noteSetCount];


   // number of current rows (may be less than max)
   int mNumRows;
   // number of current steps (may be less than max)
   int mNumSteps;
   // which mode we are in (mono/poly)
   bool mIsMonoMode;
   // max poly
   int mMaxPoly;
   // poly bias
   int mPolyBias;
   // Which note set are we using (standard/custom)
   WhichNoteSet mCurrentNoteSet;
   // current pattern we are on (note that this is set via the parent right now
   // ie all layers are linked. it might change at some point.)
   int mCurrentPattern;

   // the speed of playback in a range 1/16 up to 4 (1/16,1/8,1/4...1,2,4)
   // this number represents the numerator where denominator is 16
   // so a value of 16 is default (use constants)
   int mClockDivider;

   // the midi channel that this layer plays on
   char mMidiChannel;

   // how long a note will play on each step. ie 100% will play that whole step, 50% will play half
   // 101% is legato (will play a little longer)
   // note that if length of a cell is > 0, then this cycle applies to the last cell-length that plays
   int mDutyCycle;

   // number of steps in each measure
   // this allows altering of the timing to eg 3/4
   int mStepsPerMeasure;

   // for standard scale
   // this stores info of the standard scale that was selected
   char mStdKeyName[SEQ_KEY_NAME_MAXLEN];
   char mStdScaleName[SEQ_SCALE_NAME_MAXLEN];
   char mLayerName[SEQ_LAYER_NAME_MAXLEN];
   int  mStdOctave;
   bool mMuted;

   // variance in %
   int mHumanLen;
   int mHumanVelo;
   int mHumanPos;
   bool mCombineMode;

   /* DONT ADD POINTERS or other external structures that may contain them! */

public:
   SequenceLayer() { clear(); }
   
   // clear all layer data to initial state
   void clear();

   void setMuted(bool muted);
   bool getMuted();

   // add a chain source step to a step.
   // if the item already exists (matching src and target), it is modified to the new bool values
   // returns false if no more slots available
   // negtgt - do not trigger the target cell if chain triggers
   // negsrc - chain will trigger if source cell did NOT trigger
   bool addChainSource(int row, int step, int sourceRow, int sourceStep, bool negtgt, bool negsrc, int pat=-1);

   // return number of chain sources for a cell
   int getNumChainSources(int row, int step, int pat=-1);

   // iteratively get the next chain source step.
   // pass -1 as the value for iterate for the first item. it will be set
   // to the last item retrieved so that the next call will get the following
   // returns false when no more exist
   bool getChainSource(int row, int step, int *iterate, int *sourceRow, int *sourceCol, bool *negtgt, bool *negsrc, int pat=-1);
   bool getChainTarget(int row, int step, int *iterate, int *targRow, int *targCol, bool *negtgt, bool *negsrc, int pat = -1);

#if 0 // not used. we dont want persistence layer to know too much about internals
   // for persisting only. returns true if it's in use
   // (flag is SEQ_CHAIN_FLAG_USED)
   bool getChainSourceRaw(int pat, int step, int idx, int *col, int *row, int *targRow, int *flags);
#endif

   // clear all chain sources tied to a cell on a pattern
   // if pattern not specifies, uses current
   void clearChainSources(int row, int step, int pat=-1);

   // set current key scale and octave where octave is internal 0-based
   // this will also fill in note names and values
   void setKeyScaleOct(const char *scale, const char *key, int octave);

   // get current key scale and octave (octave is internal 0-based)
   void getKeyScaleOct(const char **scale, const char **key, int *oct);

   // copy key/scale/octave data and note vals from another layer
   void copyScaleData(const SequenceLayer &src);

   // set velocity for a cell in the current pattern
   void setVel(int row, int step, char vel, int pat=-1);

   // get a velocity for a cell, optionally specifying pattern
   // (current will be used if not specified)
   char getVel(int row, int step, int pat = -1);

   // set probability for a cell (prob of -1 means turn it off)
   void setProb(int row, int step, char prob, int pat=-1);

   // get probability for a cell, optionally specifying pattern 
   // (current will be used if not specified)
   // if the value is negative it means the cell is off
   char getProb(int row, int step, int pat = -1);

   // set cell length. a value >0 means play for that many more cell lengths
   // (ie a value of 2 means play 3 cell lengths, the last of which is
   // shortened by the duty cycle)
   void setLength(int row, int step, char length, int pat=-1);

   // get length of a cell, optionally specifying a pattern
   // (current will be used if not specified)
   char getLength(int row, int step, int pat = -1);

   // set cell offset in time. A value of -50 means start playing it half a step sooner
   // a value of 50 means play it half a step later
   void setOffset(int row, int step, char length, int pat = -1);

   // get cell offset in time. value of -50 to 50. Default 0
   char getOffset(int row, int step, int pat = -1);

   // clear all data from cell
   void clearCell(int row, int step);
   // copy all data from cell to cell
   void copyCell(int targRow, int targStep, int srcRow, int srcStep);

   // set a note value to a midi note value
   // if custom is true, it is set on the custom note buffer,
   // otherwise on the standard
   void setNote(int row, char val, bool custom);

   // get a midi note value for a row. if custom is true, it retrieves
   // from custom buffer, otherwise standard
   char getNote(int row, bool custom);

   // get the custom name (label) of a note row
   // this is a user specified name of length SEQ_MAX_NOTELABEL_LEN
   char *getNoteName(int row);

   // set a custom note name (label) for a row
   // this is a user specified name of length SEQ_MAX_NOTELABEL_LEN
   void setNoteName(int row, const char *name);

   // get midi note from "current" (either custom or standard)
   char getCurNote(int row);

   // !!! TESTING this is inefficient
   // returns -1 if not exist
   int getRowForNote(char note);

   // set us to point to either custom or standard
   // which determines which notes play
   void setNoteSource(bool custom);

   // determine whether we are in custom notes mode
   bool noteSourceIsCustom();

   // maximum number of rows that are available on this layer
   int getMaxRows();

   // maximum number of rows that are available on this layer
   void setMaxRows(int val);

   // maximum polyphony on this layer (obviously only applies to poly mode)
   int getMaxPoly();
   
   // maximum polyphony on this layer (obviously only applies to poly mode)
   void setMaxPoly(int val);

   // get the poly bias. this is how much to add to each probability
   int getPolyBias();
   // set poly bias
   void setPolyBias(int val);

   // number of steps available on this layer. this is the number of steps 
   // that are played on each pattern in this layer
   int getNumSteps();

   // set the number of steps in all patterns on this layer
   void setNumSteps(int val);

   // determine if we are in mono mode (as opposed to poly)
   bool isMonoMode();

   // determine if we are in mono mode (as opposed to poly)
   void setMonoMode(bool val);

   // combine mode means that if the same note on the same channel starts
   // to play while it's already playing, we just lengthen the other note (tie them together)
   bool isCombineMode();
   void setCombineMode(bool val);

   // get clock divider for layer. This value is the numerator
   // where SEQ_CLOCK_DENOM is the denominator. The playback speed
   // is multiplied by that fraction
   int getClockDivider();

   // set the value of the clock divider numerator
   // where SEQ_CLOCK_DENOM is the denominator. The playback speed
   // is multiplied by that fraction
   void setClockDivider(int c);

   // determine the midi channel used for this layer's playback
   char getMidiChannel();

   // determine the midi channel used for this layer's playback
   void setMidiChannel(char val);
   
   // determine how many steps are in each measure. This defines
   // the time signature. eg 12 steps would be 3/4
   int getStepsPerMeasure();

   // determine how many steps are in each measure. This defines
   // the time signature. eg 12 steps would be 3/4
   void setStepsPerMeasure(int val);

   // determine how long to play a note for a step. this is 
   // a percentage, where 100 means play for the whole duration 
   // of that step. applies to the last cell that would play
   // for that step (eg if length is > 0)
   int getDutyCycle();

   // determine how long to play a note for a step. this is 
   // a percentage, where 100 means play for the whole duration 
   // of that step. applies to the last cell that would play
   // for that step (eg if length is > 0)
   void setDutyCycle(int val);

   // humanize velocity in %
   void setHumanVelocity(int val);
   int getHumanVelocity();

   // humanize position in % (up to 50
   // which would move it by half step max)
   void setHumanPosition(int val);
   int getHumanPosition();

   // humanize length in %
   // this, like duty cycle, only affects the last step's worth
   // of length
   void setHumanLength(int val);
   int getHumanLength();

   const char *getLayerName();
   void setLayerName(const char *txt);
   void setPatternName(const char *txt, int pat = -1);
   const char *getPatternName(int pat = -1);
};


/* Store a midi mapping. This is outside here because it's also used in the midi
   mapping dlg for internal storage
*/
struct SeqMidiMapItem {
   char mAction;        // See SEQMIDI_ACTION_*
   char mTarget;        // A value of 1-4 or SEQMIDI_TARGET_ALL
   char mValue;         // See SEQMIDI_VALUE_* varies depending on action
   char mType;          // type of msg see SEQ_MIDI_*
   char mNote;          // midi note/cc to recognize 0-127
   char mChannel;       // midi channel to recognize 1-16
   SeqMidiMapItem(char act, char targ, char val, char type, char note, char chan) :
      mAction(act), mTarget(targ), mValue(val), mType(type), mNote(note), mChannel(chan) {}
   SeqMidiMapItem()  { clear(); }
   // reset map item to it's default state
   void clear();
};

// access to an array of default mappings that are in effect
// on a new blank patch. user can reset to this array
// will have count  == SEQMIDI_NUM_DEFAULT_ITEMS
extern SeqMidiMapItem gDefaultMidiMapItems[];

/** Store all current sequence data for all patterns with helpers

   **IMPORTANT** do not put any pointers in here, since we do the buffer swap
   and copy this whole structure. 

*/
class SequenceData {

   SequenceLayer mLayers[SEQ_MAX_LAYERS];

   // which is the current pattern we are on
   int mCurrentPattern;

   // groove values are -50 to 50
   int mGroove[SEQ_DEFAULT_NUM_STEPS];

   // a value from -50 to 50 that represents how far to shift every even numbered 16th note
   int mSwing;

   // see SEQ_MIDI_PASSTHRU_*
   int mMidiPassthru;
   // see SEQ_MIDI_RESPOND_*
   int mMidiRespond;

   int mMidiMapCount;
   SeqMidiMapItem mMidiMap[SEQMIDI_MAX_ITEMS];

   int64 mRandomSeed;

   // global offset time in ms - either positive or negative
   int mOffsetTime;

   // whether to start playback with DAW (default)
   int mAutoPlay;

   // standalone mode bpm
   double mStandaloneBPM;

   // **DONT ADD POINTERS!**

   // set midi map items to default (see gDefaultMidiMapItems)
   void setDefaultMidiMapItems();
public:
   SequenceData();
   SequenceLayer *getLayer(int layer);
   // Right now there is a current pattern that applies to all layers
   // from midi, individual layers can be set to different patterns
   int getCurrentPattern();
   void setCurrentPattern(int p);
   // get a groove value for a specific position
   // which can be 0..SEQ_DEFAULT_NUM_STEPS
   int getGroove(int idx);
   // set a groove value for a specific position
   // which can be 0..SEQ_DEFAULT_NUM_STEPS
   void setGroove(int idx, int val);
   // clear all data for a layer
   void clearLayer(int layer);
   // clear all pattern data for a specific layer
   void clearPattern(int layer, int pattern);
   // clear all groove data
   void clearGroove();

   // clear the current midi mapping
   // does not alter the count of mapping items, just
   // sets all of them to a default (invalid) state
   void clearMapping();
   // set how many midi mapping items are used
   // can be 0..SEQMIDI_MAX_ITEMS
   void setMappingCount(int count);
   // return the number of midi mapping items that are in use
   // note that some of them might be invalid
   int getMappingCount();

   // get a midi mapping item for read or write
   // any can be set up to SEQMIDI_MAX_ITEMS, however, only
   // 0..mappingCount are actually used
   SeqMidiMapItem *getMappingItem(int ind);

   // set swing value. If swing is set to a non-zero value, it is
   // used. Otherwise groove values (if any) are used
   void setSwing(int val);

   // get current swing value
   int getSwing();

   // set the state of midi passthru
   // possible values: SEQ_MIDI_PASSTHRU_*
   void setMidiPassthru(int val);

   // get the state of midi passthru
   // possible values: SEQ_MIDI_PASSTHRU_*
   int getMidiPassthru();

   // set whether we respond to incoming midi
   // possible values SEQ_MIDI_RESPOND_*
   void setMidiRespond(int val);

   // determine whether we respond to incoming midi
   // possible values SEQ_MIDI_RESPOND_*
   int getMidiRespond();

   // copy all layer data from one layer to another
   void copyLayer(int targLayer, int srcLayer);

   // copy all scale data from one layer to another. includes whether std or custom
   void copyScaleData(int targLayer, int srcLayer);

   // copy all pattern data from one place to another
   void copyPatternData(int targLayer, int targPat, int srcLayer, int srcPat);
   
   // return the current random seed. If this is 0 it means a new random number
   // is generated every time playback starts
   int64 getRandomSeed();

   // set the random seed that will be saved with the patch. if set to 0 it means
   // generate a new one every time playback starts
   void setRandomSeed(int64 val);

   // get global offset time in milliseconds. Positive value means it plays later
   // negative value means it plays sooner
   int getOffsetTime();
   void setOffsetTime(int ms);

   // whether we automatically play when DAW transport starts/stops
   int getAutoPlayMode();
   void setAutoPlayMode(int autoplay);

   double getStandaloneBPM();
   void setStandaloneBPM(double bpm);

};

// about 1.2 MB right now
//char(*__kaboom)[sizeof(SequenceData)] = 1;

typedef void(*ChangeCallback)(void *);
/** This is to swap out buffers so that we can modify the sequence data
    from the UI thread, then swap it into the audio thread
*/
class SeqDataBuffer {
   Atomic<int> mCurrent; // which one the audio thread is looking at
   SequenceData mBuffer[2]; // double buffer
   ChangeCallback mCB;
   void *mCBHandle;
   SequenceData mUndoBuffer;
public:
   SeqDataBuffer() : mCurrent(0), mCB(0) , mCBHandle(0) {}
   // called from UI thread (only) to point audio thread to new buffer
   void swap();

   // this copies undo buffer to ui side and implicitly does a swap
   void undo();

   void setChangeNotify(ChangeCallback cb, void *cbhandle);

   // called from audio thread to get the data it needs.
   // The audio thread must never alter the data
   // also, the audio thread should not assume that subsequent calls to getAudSeqData
   // will return the same physical object (ie, don't call it multiple times in a function
   // and don't store the value beyond a function)
   SequenceData *getAudSeqData() {
      return &mBuffer[mCurrent.get()];
   }

   // called from UI thread when it needs to get current data.
   // it may alter the data, and then call swap when it wants
   // to make that available to the audio thread
   SequenceData *getUISeqData() {
      return &mBuffer[mCurrent.get() ? 0 : 1];
   }
};

/* used only so that seqprocessornotifier can get access to 
   stochaengine objects inside processor
 */
class SeqProcessorNotifierHelper {
public:
   virtual bool getStepPlayedState(int layer, int position, int notenum) = 0;
};

/* Used to send data from processor to UI thread
The audio processor needs to be able to notify the editor of a few things without locking. 
This object allows it to do so. 
All get functions are used by UI thread. All set functions are ONLY used by audio thread
*/
class SeqProcessorNotifier {
   // current play position, or -1 if not playing
   Atomic<int> mPlayPosition[SEQ_MAX_LAYERS];
   // current pattern that's playing or -1 if not playing
   Atomic<int> mCurrentPattern[SEQ_MAX_LAYERS];
   Atomic<int> mMuteState[SEQ_MAX_LAYERS];
   // set to non-zero by audio thread if midi event was received
   Atomic<int> mMidiEvent;

   Atomic<int> mUINeedsUpdate;
   Atomic<int64> mRandomSeed;
   Atomic<int> mRecordingState;
   Atomic<int> mManualPlayingState;
   SeqProcessorNotifierHelper *mNotifierHelper;

   //This is for passing complete midi notes (note, step position, velocity, length) 
   // to the UI thread so that it can add them to the SequenceData
   SeqFifo mCompletedNoteFifo;
public:
   SeqProcessorNotifier(SeqProcessorNotifierHelper *hlpr);   
   //
   // UI Thread access only
   //
   enum PlayRecordState {
      off,
      on,
      standby
   };
   PlayRecordState getRecordingState();

   // just reuse this tri-state enum
   PlayRecordState getPlaybackState();

   // The UI thread can see if complete midi notes are available from the processor.
   // 
   bool getCompletedMidiNote(int *number, int *velocity, 
      int *len /*in steps*/, 
      int *pos /*on grid*/ );

   // clear all recorded notes. call this when recording starts
   // to make sure no old notes are left in the buffer
   void clearCompletedMidiNotes();

   //  UI asks if he should be updated?
   // calling this inherently resets the flag to false
   bool doesUINeedUpdate();
   
   // return the current play position for the given layer
   int getPlayPosition(int layer);

   // currently active (playing) pattern
   int getCurrentPattern(int layer);

   // get the current midi event (or 0 if there is none)
   // parameters are optional and return the values for the event
   // (see setMidiEventOccurred)
   bool getMidiEventOccurred(char *type, char *channel, char *number, char *value);

   bool getMuteState(int layer);

   // get the random seed that's currently being used by the processor
   int64 getRandomSeed();

   // given a step position in the grid and a layer, return whether or
   // not that position played in the last cycle
   // TODO is this safe without atomics? Probably doesn't matter as
   // we are just using it for ui display
   bool getStepPlayedState(int layer, int position, int notenum);


   //
   // Processor thread access only
   //
   
   // a midi event was received (will happen when ANY event is received)
   // note that subsequent calls might overwrite previous value if the UI thread
   // has not emptied that old value
   // type - one of SEQ_MIDI_*
   // channel
   // number - cc or note number
   // value - cc value or velocity
   void setMidiEventOccurred(char type, char channel, char number, char value);

   // set a new playposition (-1 = off)
   void setPlayPosition(int layer, int val);

   // set current pattern that's playing
   void setCurrentPattern(int layer, int val);
   
   void setMuteState(int layer, bool val);

   
   // Engine says to update the whole ui
   void uiNeedsUpdate();

   // set the random seed that is currently being used
   void setRandomSeed(int64 seed);
   
   // The processor will send completed notes as they are received
   void addCompletedMidiNote(int number, int velocity, 
      int len /*in steps*/, int pos /*on the grid*/);

   void setRecordingState(PlayRecordState state);

   void setPlaybackState(PlayRecordState playing);
};

#endif
