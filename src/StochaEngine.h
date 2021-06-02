/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/
#ifndef STOCHAENGINE_H_
#define STOCHAENGINE_H_

#include "Constants.h"
#include "SequenceData.h"
#include "SeqRandom.h"
#include <bitset>


/**
The engine handles the determination of midi notes to play.
It will be called some time before a play position is reached.
It will check the sequence data,
and it will possibly send a midi note to a midi queue
*/

class StochaEngine {

   /* Store an array of which notes played for the last cycle. This is used
      to determine dependencies (chaining)
    */
   
   typedef std::bitset<SEQ_MAX_ROWS> DepSource;
   DepSource mDependencySource[SEQ_MAX_STEPS];
   
   /* represent a single mapping and what it does.
      A mapping is a user defined value that says "when i hit note A4 I want to change pattern in layer x"
      
   */
   struct MidiMappingItem {
      int mChannel;  // 1 based channel
      int mAction; // see SEQMIDI_ACTION_*
      int mValue;  // will depend on action. 
      int mType;     // see SEQ_MIDI_* for one of the types
      MidiMappingItem *mNext;
      MidiMappingItem() : mChannel(0), mAction(SEQMIDI_ACTION_INVALID), mValue(0), 
         mType(0), mNext(nullptr) {}
   };

   // array of items where each item may be a linked list.
   // this is indexed by the incoming value to make for speedy finding.
   MidiMappingItem *mMapping[128];
   bool mMappingIsValid;

   // for midi mapping, we are either overriding a value or using it's default
   struct MidiOverride {
      bool mOverriden;
      int mValue;
      MidiOverride() : mOverriden(false), mValue(0) {}
      inline void override(int newval) { mValue = newval; mOverriden = true; }
      inline void clear() { mValue = 0; mOverriden = false; }
      inline int get(int defaultVal) { if (mOverriden) return mValue; return defaultVal; }
   };

   // these are available via midi as well as automation
   MidiOverride mOverridePattern;   // value will be pattern number (0 based)
   MidiOverride mOverrideMute;      // value will be SEQMIDI_VALUE_MUTE_MUTE or _UNMUTE (only)
   MidiOverride mOverrideSpeed;     // value will be speed value
   MidiOverride mOverrideTranspose; // value will be semitones to transpose
   MidiOverride mOverrideNumSteps;  // value will be num steps
   MidiOverride mOverridePolyBias;  // value will be the bias amount

   // these are available via automation
   MidiOverride mOverrideStepsPerMeasure;
   MidiOverride mOverrideDutyCycle;
   MidiOverride mOverridePosVariance;
   MidiOverride mOverrideVeloVariance;
   MidiOverride mOverrideLengthVariance;
   MidiOverride mOverrideOutputChannel;
   MidiOverride mOverrideMaxPoly;
   MidiOverride mOverrideSwing;
   
   /*
   Represents a midi event that will happen in the future
   */
   struct StochaEvent {
      int mNumSamples;     // number of samples into the future that it needs to occur
                           // this will be negative if the slot is available
      int8_t mNote;          // the note value.
      int8_t mVelo;          // note velocity. If 0, it's a note off
      int8_t mChan;          // value from 1 to 16
      StochaEvent *mCorrespondingNoteOff; // if this is a note on event, this will point at it's note off
      StochaEvent() { clear(); }
      void clear() {
         mNumSamples = -1;
         mNote = -1;
         mVelo = -1;
         mChan = -1;
         mCorrespondingNoteOff = nullptr;
      }
      
   };
   // these two are just to avoid excessive looping in getMidiEvent
   int mNumActiveNoteOnEvents;
   int mNumActiveNoteOffEvents;

   // this is the last step that was registered (might not have occurred yet)
   int mCurrentStepPosition;
   // this is the step position that is used for visually informing the user what position is current
   double mRealStepPosition;
   SeqDataBuffer *mSeq;
   StochaEvent mEvents[SEQ_MAX_MIDI_EVENTS]; 
   int mLayer;
   struct SelectedItem {
      int rowToPlay;
      bool mandatory;
   };
   SelectedItem mMulti[SEQ_MAX_ROWS]; // buffer for multiple random selection

   uint64 mOldSeed;
   uint64 mOldSeq;
   // random number generator
   SeqRandom mRand;
   
   // cheesy way of handling jumps back in time
   double mOldStepPosInTrack;

   // this is where playback is offset from. Normally it is offset from 0.0
   // which is the beginning of the track, but if manual playback is on, it 
   // might be some later position
   double mPlayStartPosition;

   // add a midi note to the queue (note on and off). Len is in number of samples
   // returns false if it could not be added (not enough space)
   // if velo is 0 then only a note-off will be added (numSamples is ignored in this case)
   bool addMidiEvent(int startSamples, int8_t note,int8_t velo, int8_t chan, int numSamples);

   /*
   This, when called will cause all future note-on messages to be deleted, and will move
   all note-off message such that they will occur right away. getMidi should be called
   some time after this, and those events processed.
   This will be called when playback stops and when pattern change occurs
   the parameter, if false, will not change note-off events to occur now
   */
   void quiesceMidi(bool moveNoteOffs=true);

   /*
   This is called some time before a grid step occurs (could be up to half a step before it occurs).
   samples_per_step - how many samples are in each grid step
   position - the position in the sequence (grid step) that is about to occur (in steps)
   samples_until - number of samples before the grid step occurs
   returns false if one or more midi notes that needed to be inserted could not be
   inserted
   */
   bool playPositionChange(int samples_per_step, int position, int samples_until);

   /*
   Generate a single note to play. returns row number, or -1 if no note should be played
   */
   int getRandomSingle(int position);
   /*
   Generate multiple notes. Returns number of notes to play and mMulti is filled with row numbers of these notes.
   will not produce more than maxpoly
   */
   int getRandomMulti(int position, int maxpoly);

   // perform an action
   void performMidiMapAction(int action, int value);
   void rebuildMappingSchema();
   // trim down used values to a number of values (max polyphony)
   int trimPoly(int maxpoly, int used);
   // compact the multi array down. returns new size
   void compactArray(int used);
   bool isMandatory(int col, int row, bool *off, int pattern);

public:
   StochaEngine();
   ~StochaEngine();
   void init(SeqDataBuffer *s,int layer);

   // set the random seed which is used to calculate other seeds
   // that are used to generate random values
   void setRandomSeed(uint64 seed, uint64 seqno);

   /*
   This returns the current step position in the range 0..n where n is the current number of steps
   in the pattern. it may also return -1 if there is no current step position (eg playback stopped).
   This is used to inform the user where we are.
   If fraction is specified it returns a value between 0 and 99 indicating what % of the way thru the
   step we are (for midi record)
   */
   int getCurrentStepPosition(int *fraction = 0);

   /* this returns step position in the whole track (ie doesn't flip back to 0 when numsteps is reached */
   int getCurrentOverallPosition(int *fraction = 0);

   //get effective number of steps (with overriden value)
   int getNumSteps();

   /* This returns the current pattern that is playing. it will either be current pattern,
      or it will be the one that is effectively set from a midi action
   */
   int getPlayingPattern();

   /*
   Return the current mute state of the layer
   */
   bool getMuteState();

   /*
   Call this when playback has stopped. this will quiesce the midi so you should call
   getMidiEvent afterwards
   Also, this will clear our dependency tracker
   */
   void playbackStopped();

   /*
   Call this to set manual playback start position. will be automatically reset to 0 when
   playbackStopped is called
   */
   void setPlaybackStartPosition(double pos);

   /*
   call to process audio block. please set ALL members of mPosInfo before
   each time this is called. After calling, call getMidiEvent until all are gotten,
   then call doneBlock
   returns false if some midi events could not be added
   */
   bool processBlock(double beatPosition,    // which quarter measure we are on
      double sampleRate,      // current sample rate
      int numSamplesInBlock,  // number of samples in the block
      double BPM,             // current bpm
      double bpb            // beats per bar
#ifdef CUBASE_HACKS
      , double beatPositionActual //for cubase - could be negative
#endif
   );
   
   /*
   this will return true if there is a midi event that needs to happen now (within this block)
   if so, pos will be set to the offset in samples into the block where it needs to occur
   and msg values will be set to the desired values.
   blocksize should specify the size of the current block of samples so that we can know
   whether this event will occur in this block
   keep calling it until it returns false, which will mean that no more events should occur
   in this block
   */
   bool getMidiEvent(int numSamplesInBlock, int *pos, int8_t *note, int8_t *velo, int8_t *chan);
   
   /*
   call after getting midi events for the block.
   This will advance any active events forward.
   */
   void doneBlock(int numSamplesInBlock);

   /*
   call this to tell the stocha of some incoming midi data that may need to be used to set
   the temporary play state (ie honor the midi mappings). The temporary play state is active
   while the daw is playing, and should get reset when it stops (see below).
   When this is called, it checks to see if it has an internal schema of the midi mapping.
   if not, it creates one and uses it.
   type = SEQ_MIDI_*
   number = note number or cc number
   chan = channel
   val = if cc, the cc value
   returns true if the data was handled (meaning it was found in the mapping)
   */
   bool incomingMidiData(int type, int8_t number, int8_t chan, int8_t val);

   /*
   call this to reset the internal schema of the mapping state. This will cause a new one
   to be rebuilt based on sequence data midi mapping. (ie when incomingMidiData is called next)
   this should be called when a new mapping comes into effect
   */
   void resetMappingSchema();

   /* Call this to reset temporarily modified values. these are the values that are modified based
   on the midi mapping, and having received midi data that matches that mapping. this should be called
   whenever playing stops.
   */
   void resetMidiControl();

   /* given a note row and a step number, return whether that
      note recently played at that step.
      THIS IS PROBABLY NOT THREAD SAFE, but for the usage of it
      (which is to update the display) it doesn't really matter
    */
   bool getStepPlayedState(int position, int notenum);

   /* incoming automation value. override it.
   */
   void setAutomationParameterValue(int paramId, int value);
};

#endif
