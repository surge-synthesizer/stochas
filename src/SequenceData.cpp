/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/
#include "SequenceData.h"
#include "EditorState.h" // just for random seed function
SeqMidiMapItem gDefaultMidiMapItems[SEQMIDI_NUM_DEFAULT_ITEMS] =
{
   // action, target, value, note, channel

   // change patterns for all
   { SEQMIDI_ACTION_CHGPAT,    SEQMIDI_TARGET_ALL,  1, SEQ_MIDI_NOTEON, 0, 1 },
   { SEQMIDI_ACTION_CHGPAT,    SEQMIDI_TARGET_ALL,  2, SEQ_MIDI_NOTEON, 1, 1 },
   { SEQMIDI_ACTION_CHGPAT,    SEQMIDI_TARGET_ALL,  3, SEQ_MIDI_NOTEON,2, 1 },
   { SEQMIDI_ACTION_CHGPAT,    SEQMIDI_TARGET_ALL,  4, SEQ_MIDI_NOTEON,3, 1 },
   { SEQMIDI_ACTION_CHGPAT,    SEQMIDI_TARGET_ALL,  5, SEQ_MIDI_NOTEON,4, 1 },
   { SEQMIDI_ACTION_CHGPAT,    SEQMIDI_TARGET_ALL,  6, SEQ_MIDI_NOTEON,5, 1 },
   { SEQMIDI_ACTION_CHGPAT,    SEQMIDI_TARGET_ALL,  7, SEQ_MIDI_NOTEON,6, 1 },
   { SEQMIDI_ACTION_CHGPAT,    SEQMIDI_TARGET_ALL,  8, SEQ_MIDI_NOTEON,7, 1 },

   // toggle mute for each layer
   { SEQMIDI_ACTION_MUTE,      1, SEQMIDI_VALUE_MUTE_TOGGLE, SEQ_MIDI_NOTEON,9,  1 },
   { SEQMIDI_ACTION_MUTE,      2, SEQMIDI_VALUE_MUTE_TOGGLE, SEQ_MIDI_NOTEON,10, 1 },
   { SEQMIDI_ACTION_MUTE,      3, SEQMIDI_VALUE_MUTE_TOGGLE, SEQ_MIDI_NOTEON,11, 1 },
   { SEQMIDI_ACTION_MUTE,      4, SEQMIDI_VALUE_MUTE_TOGGLE, SEQ_MIDI_NOTEON,12, 1 }
   

};


SeqProcessorNotifier::SeqProcessorNotifier(SeqProcessorNotifierHelper *hlpr) :
   mMidiEvent(0), mUINeedsUpdate(0), mRandomSeed(0), mNotifierHelper(hlpr)
{
   for (int i = 0; i < SEQ_MAX_LAYERS; i++)
      mPlayPosition[i].set(-1);

   // generate our initial random seed.
   // this will also happen every time playback starts or loops
   // unless the user has fixed it to a value   
   mRandomSeed.set(generateNewRootSeed());
   mRecordingState.set((int)PlayRecordState::off);
   mManualPlayingState.set((int)PlayRecordState::off);
}

int 
SeqProcessorNotifier::getPlayPosition(int layer)
{
   return mPlayPosition[layer].get();
   
}

int SeqProcessorNotifier::getCurrentPattern(int layer)
{
   return mCurrentPattern[layer].get();
}

bool 
SeqProcessorNotifier::getMidiEventOccurred(char *type, char *channel, char *number, char *value)
{
   int val = mMidiEvent.exchange(0);
   if (val) {
      if (type)
         *type = (0xFF & (val >> 24));
      if (channel)
         *channel = (0XFF & (val >> 16));
      if (number)
         *number = (0XFF & (val >> 8));
      if (value)
         *value = (val & 0XFF);
      return true;
   }
   else
      return false;
   
}


bool SeqProcessorNotifier::getMuteState(int layer)
{
   return mMuteState[layer].get()==1;
}

int64 SeqProcessorNotifier::getRandomSeed()
{
   return mRandomSeed.get();
}

bool SeqProcessorNotifier::getStepPlayedState(int layer, int position, int notenum)
{
   return mNotifierHelper->getStepPlayedState(layer,position,notenum);
}

void 
SeqProcessorNotifier::setMidiEventOccurred(char type, char channel, char number, char value)
{
   int sum = (type << 24) | (channel << 16) | (number << 8) | value;
   mMidiEvent.set(sum);
}

void 
SeqProcessorNotifier::setPlayPosition(int layer, int val)
{
   mPlayPosition[layer].set(val);
}

void SeqProcessorNotifier::setCurrentPattern(int layer, int val)
{
   mCurrentPattern[layer].set(val);
}

void SeqProcessorNotifier::setMuteState(int layer, bool val)
{
   mMuteState[layer].set(val ? 1 : 0);
}

void 
SeqProcessorNotifier::uiNeedsUpdate()
{
   mUINeedsUpdate.set(1);
}

void SeqProcessorNotifier::setRandomSeed(int64 seed)
{
   mRandomSeed.set(seed);
}
void SeqProcessorNotifier::addCompletedMidiNote(int number, int velocity, int len, int pos)
{
   int third;
   jassert(number < 128);
   jassert(velocity < 128);
   jassert(len < SEQ_MAX_STEPS); // length is 0 based
   jassert(pos < SEQ_MAX_STEPS);
   third = (len << 8) + pos;
   mCompletedNoteFifo.addToFifo(number, velocity, third);
}
void SeqProcessorNotifier::setRecordingState(PlayRecordState state)
{
   mRecordingState.set((int)state);
}
void SeqProcessorNotifier::setPlaybackState(PlayRecordState state)
{
   mManualPlayingState.set((int)state);
}
SeqProcessorNotifier::PlayRecordState
SeqProcessorNotifier::getRecordingState()
{
   return (PlayRecordState)mRecordingState.get();
}
SeqProcessorNotifier::PlayRecordState
SeqProcessorNotifier::getPlaybackState()
{
   return (PlayRecordState)mManualPlayingState.get();
}
bool
SeqProcessorNotifier::getCompletedMidiNote(int * number, int * velocity, int * len, int *pos)
{
   int third;
   bool r= mCompletedNoteFifo.readFromFifo(number, velocity, &third);
   if (r) {
      *pos = third & 0xFF;
      *len = third >> 8;
   }
   return  r;
}

void SeqProcessorNotifier::clearCompletedMidiNotes()
{
   mCompletedNoteFifo.clearFifo();
}

bool
SeqProcessorNotifier::doesUINeedUpdate()
{
   int r = mUINeedsUpdate.exchange(0);
   return r!=0;
}

/*============================================
SeqDataBuffer
*============================================*/
void SeqDataBuffer::swap()
{
   /*Theoretically, if the ui thread swaps this twice while the audio thread is
     still reading values, the audio thread might be picking up torn values, etc.
     In practice I don't believe this would happen due to: a) audio thread does
     quick reads of individual values, b) audio thread sets these in batches where
     more than one is being set at a time
   */

   int cur = mCurrent.get(); // the one the audio is looking at now
   int wanted = cur ? 0 : 1; // which one the audio needs to look at
                             // tell the audio to look at the other one
   mCurrent.set(wanted);
   
   // mBuffer[cur] contains info that we can use for UNDO
   mUndoBuffer = mBuffer[cur];

   // now copy data from the one the audio is looking at into the one
   // that the ui now has      
   mBuffer[cur] = mBuffer[wanted];

   // send notification that data is changing
   if (mCB && mCBHandle)
      mCB(mCBHandle);
  
}

void SeqDataBuffer::undo()
{
   // set UI buffer to undo buffer
   mBuffer[mCurrent.get() ? 0 : 1] = mUndoBuffer;
   swap(); // note that new undo buffer will contain state before undo
           // thus facilitating redo... how convenient!
}

void SeqDataBuffer::setChangeNotify(ChangeCallback cb, void * cbhandle)
{
   mCB = cb;
   mCBHandle = cbhandle;
}

/*============================================
SequenceData
*============================================*/

// initial list of midi mapping items
void SequenceData::setDefaultMidiMapItems()
{
   SeqMidiMapItem *mi;
   setMappingCount(SEQMIDI_NUM_DEFAULT_ITEMS);
   for (int i = 0; i < SEQMIDI_NUM_DEFAULT_ITEMS; i++) {
      mi = getMappingItem(i);
      *mi = gDefaultMidiMapItems[i];
   }

}

SequenceData::SequenceData() :
   mCurrentPattern(0),
   mSwing(0),
   mMidiPassthru(SEQ_MIDI_PASSTHRU_NONE),
   mMidiRespond(SEQCTL_MIDI_RESPOND_YES),
   mMidiMapCount(0),
   mRandomSeed(0),
   mOffsetTime(0),
   mAutoPlay(SEQ_PLAYMODE_AUTO),
   mStandaloneBPM(SEQ_DEFAULT_STANDALONE_BPM)
{
   clearGroove();
   setDefaultMidiMapItems();
}

double 
SequenceData::getStandaloneBPM()
{
   return mStandaloneBPM;
}
void 
SequenceData::setStandaloneBPM(double bpm)
{
   // value here is arbitrary
   jassert(bpm >= 1.0 && bpm <= 300.0);
   mStandaloneBPM = bpm;
}

SequenceLayer * 
SequenceData::getLayer(int layer)
{
   jassert(layer >= 0 && layer < SEQ_MAX_LAYERS);
   return &mLayers[layer];
}

int 
SequenceData::getCurrentPattern() 
{
   return mCurrentPattern;
}

void 
SequenceData::setCurrentPattern(int p) 
{
   jassert(p >= 0 && p < SEQ_MAX_PATTERNS);
   mCurrentPattern = p;
   for (int i = 0; i < SEQ_MAX_LAYERS; i++) {
      mLayers[i].mCurrentPattern = p;
   }
}

int SequenceData::getGroove(int idx) 
{
   jassert(idx >= 0 && idx <= SEQ_DEFAULT_NUM_STEPS);
   return mGroove[idx];

}

void 
SequenceData::setGroove(int idx, int val) 
{
   jassert(val >= -50 && val <= 50);
   jassert(idx >= 0 && idx <= SEQ_DEFAULT_NUM_STEPS);
   mGroove[idx] = val;
}

void 
SequenceData::clearLayer(int layer)
{
   jassert(layer >= 0 && layer <= SEQ_MAX_LAYERS);
   mLayers[layer].clear();
}

void 
SequenceData::clearPattern(int layer, int pattern)
{
   std::unique_ptr<SequenceLayer::Pattern> blank = std::unique_ptr<SequenceLayer::Pattern>(new SequenceLayer::Pattern());
   jassert(layer >= 0 && layer <= SEQ_MAX_LAYERS);
   jassert(pattern >= 0 && pattern <= SEQ_MAX_PATTERNS);
   mLayers[layer].mPats[pattern] = *blank;   
}

void
SequenceData::clearGroove()
{
   for (int i = 0; i < SEQ_DEFAULT_NUM_STEPS; i++)
      mGroove[i] = 0;
}

void
SequenceData::clearMapping()
{
   for (int i = 0; i < SEQMIDI_MAX_ITEMS; i++)
      mMidiMap[i].clear();
}

void
SequenceData::setMappingCount(int count) 
{
   jassert(count >= 0 && count < SEQMIDI_MAX_ITEMS);
   mMidiMapCount = count;

}

int 
SequenceData::getMappingCount() 
{
   return mMidiMapCount;
}

SeqMidiMapItem * 
SequenceData::getMappingItem(int ind) 
{
   jassert(ind >= 0 && ind < SEQMIDI_MAX_ITEMS);
   return &mMidiMap[ind];
}

void
SequenceData::setSwing(int val) 
{
   jassert(val >= -50 && val <= 50);
   mSwing = val;
}

int
SequenceData::getSwing() 
{
   return mSwing;
}

void
SequenceData::setMidiPassthru(int val) 
{
   switch (val) {
   case SEQ_MIDI_PASSTHRU_NONE:
   case SEQ_MIDI_PASSTHRU_UNHANDLED:
   case SEQ_MIDI_PASSTHRU_ALL:
      mMidiPassthru = val;
      break;
   default:
      jassertfalse;
      break;
   }
}

int
SequenceData::getMidiPassthru() 
{
   return mMidiPassthru;
}

void 
SequenceData::setMidiRespond(int val) 
{
   switch (val) {
   case SEQ_MIDI_RESPOND_NO:
   case SEQ_MIDI_RESPOND_YES:
      mMidiRespond = val;
      break;
   default:
      jassertfalse;
   }
}

int
SequenceData::getMidiRespond() 
{
   return mMidiRespond;
}

void 
SequenceData::copyLayer(int targLayer, int srcLayer)
{
   jassert(targLayer >= 0 && targLayer <= SEQ_MAX_LAYERS);
   jassert(srcLayer >= 0 && srcLayer <= SEQ_MAX_LAYERS);
   mLayers[targLayer] = mLayers[srcLayer];
}

void
SequenceData::copyScaleData(int targLayer, int srcLayer)
{
   jassert(targLayer >= 0 && targLayer <= SEQ_MAX_LAYERS);
   jassert(srcLayer >= 0 && srcLayer <= SEQ_MAX_LAYERS);
   //mLayers[targLayer] = mLayers[srcLayer];
   mLayers[targLayer].copyScaleData(mLayers[srcLayer]);
}

void
SequenceData::copyPatternData(int targLayer, int targPat, int srcLayer, int srcPat)
{
   jassert(targLayer >= 0 && targLayer <= SEQ_MAX_LAYERS);
   jassert(srcLayer >= 0 && srcLayer <= SEQ_MAX_LAYERS);
   jassert(targPat >= 0 && targPat <= SEQ_MAX_PATTERNS);
   jassert(srcPat >= 0 && srcPat <= SEQ_MAX_PATTERNS);
   mLayers[targLayer].mPats[targPat] = mLayers[srcLayer].mPats[srcPat];
}

int64 SequenceData::getRandomSeed()
{
   return mRandomSeed;
}

void SequenceData::setRandomSeed(int64 val)
{
   mRandomSeed = val;
}

int SequenceData::getOffsetTime()
{
   return mOffsetTime;
}

void SequenceData::setOffsetTime(int ms)
{
   mOffsetTime = ms;
}

int SequenceData::getAutoPlayMode()
{
   return mAutoPlay;
}

void SequenceData::setAutoPlayMode(int autoplay)
{
   mAutoPlay = autoplay;
}

/*============================================
SequenceLayer
*============================================*/
void
SequenceLayer::clear() {
   std::unique_ptr<Pattern> blankPat=std::unique_ptr<Pattern>(new Pattern());
   mMuted = false;
   mNumRows = SEQ_MAX_VISIBLE_ROWS;
   mNumSteps = SEQ_DEFAULT_NUM_STEPS;
   mIsMonoMode = true;
   mMaxPoly = SEQ_DEFAULT_MAX_POLY;
   mPolyBias = 0;
   mCurrentNoteSet = customSet;
   mCurrentPattern = 0;
   mClockDivider = SEQ_DEFAULT_CLOCK_DIV;
   mMidiChannel = SEQ_DEFAULT_MIDI_CHAN;
   mDutyCycle = SEQ_DUTY_DEFAULT;
   mStepsPerMeasure = SEQ_DEFAULT_NUM_STEPS;
   // this is the default scale
   setKeyScaleOct(SEQ_DEFAULT_SCALE, SEQ_DEFAULT_KEY, SEQ_DEFAULT_OCTAVE);
   // fill custom notes from standard notes initially
   mNoteSets[customSet] = mNoteSets[standardSet];
   for (int i = 0; i<SEQ_MAX_PATTERNS; i++)
      mPats[i] = *blankPat;
   
   strncpy(mLayerName, SEQ_DEFAULT_LAYER_NAME, SEQ_LAYER_NAME_MAXLEN);
   mLayerName[SEQ_LAYER_NAME_MAXLEN - 1] = 0;

   mHumanLen = 0;
   mHumanPos = 0;
   mHumanVelo = 0;
   mCombineMode = false;
}

void SequenceLayer::setMuted(bool muted)
{
   mMuted=muted;
}

bool SequenceLayer::getMuted()
{
return mMuted;
}

bool SequenceLayer::addChainSource(int row, int step, int sourceRow, int sourceStep,
   bool negtgt, bool negsrc, int pat)
{
   int i;
   int availableCell = -1;
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   jassert(step >= 0 && step < SEQ_MAX_STEPS);
   jassert(sourceRow >= 0 && sourceRow < SEQ_MAX_ROWS);
   jassert(sourceStep >= 0 && sourceStep < SEQ_MAX_STEPS);
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);

   SourceCell *cells = mPats[pat].mChains[step].cells;
   // we need to pick the next available cell unless there is a cell already representing
   // this relationship, in which case we use that (ie nothing changes with it unless neg/pos
   // is being changed
   for (i = 0; i < SEQ_MAX_CHAIN_SOURCES; i++) {
      if ((cells[i].flags & SEQ_CHAIN_FLAG_USED) == 0 &&
         availableCell==-1) {
         availableCell = i; // potentially use this (unless we find a cell representing the same data below)
      }
      else if ((cells[i].flags & SEQ_CHAIN_FLAG_USED) &&
         cells[i].col == (unsigned char)sourceStep &&
         cells[i].row == (unsigned char)sourceRow &&
         cells[i].targetrow == (unsigned char)row)
      {
         availableCell = i; // always use this
         break;
      }
   }
   if (availableCell==-1)
      return false; // none available

   cells[availableCell].flags = SEQ_CHAIN_FLAG_USED | 
      (negtgt ? SEQ_CHAIN_FLAG_NEGTGT : 0) |
      (negsrc ? SEQ_CHAIN_FLAG_NEGSRC : 0);
   cells[availableCell].col = (unsigned char)sourceStep;
   cells[availableCell].row = (unsigned char)sourceRow;
   cells[availableCell].targetrow = (unsigned char)row;
   return true;
}

int SequenceLayer::getNumChainSources(int row, int step, int pat)
{
   int i;
   int cnt = 0;
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   jassert(step >= 0 && step < SEQ_MAX_STEPS);
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);

   SourceCell *cells = mPats[pat].mChains[step].cells;
   for (i = 0; i < SEQ_MAX_CHAIN_SOURCES; i++) {
      if ((cells[i].flags & SEQ_CHAIN_FLAG_USED) && cells[i].targetrow == row) {
         cnt++;
      }
   }
   return cnt;

}

bool SequenceLayer::getChainSource(int row, int step, int * iterate,
   int * sourceRow, int * sourceCol, bool * negtgt, bool *negsrc, int pat)
{
   int i;
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   jassert(step >= 0 && step < SEQ_MAX_STEPS);
   jassert(*iterate >= -1 && *iterate <= SEQ_MAX_CHAIN_SOURCES);
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);

   if (*iterate == SEQ_MAX_CHAIN_SOURCES)
      return false;

   SourceCell *cells = mPats[pat].mChains[step].cells;
   (*iterate)++;
   for (i = *iterate; i < SEQ_MAX_CHAIN_SOURCES; i++) {
      if ((cells[i].flags & SEQ_CHAIN_FLAG_USED) &&
         cells[i].targetrow == row) {
         *iterate = i;
         *sourceRow = cells[i].row;
         *sourceCol = cells[i].col;
         *negtgt = (cells[i].flags & SEQ_CHAIN_FLAG_NEGTGT) ? true : false;
         *negsrc = (cells[i].flags & SEQ_CHAIN_FLAG_NEGSRC) ? true : false;
         return true;
      }
   }

   *iterate = i;
   return false;

}

bool SequenceLayer::getChainTarget(int row, int step, int * iterate, 
   int * targRow, int * targCol, bool * negtgt, bool *negsrc, int pat)
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   jassert(step >= 0 && step < SEQ_MAX_STEPS);
   jassert(*iterate >= -1 && *iterate <= SEQ_MAX_CHAIN_SOURCES * SEQ_MAX_STEPS);
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);

   if (*iterate == SEQ_MAX_CHAIN_SOURCES * SEQ_MAX_STEPS)
      return false;

   (*iterate)++;
   int i, j;
   // find where we were last
   i = (*iterate) / SEQ_MAX_CHAIN_SOURCES;
   j = (*iterate) % SEQ_MAX_CHAIN_SOURCES;
   goto iterate_jump;
   // iterate over chain sources for each step to find a match for row and step
   for (; i < SEQ_MAX_STEPS; i++) {      
      for (j=0; j < SEQ_MAX_CHAIN_SOURCES; j++) {
iterate_jump:
         SourceCell *cells = mPats[pat].mChains[i].cells;
         if ((cells[j].flags & SEQ_CHAIN_FLAG_USED) &&
            cells[j].row == row && cells[j].col == step) {
            *targRow = cells[j].targetrow;
            *targCol = i;
            *negtgt = (cells[j].flags & SEQ_CHAIN_FLAG_NEGTGT) ? true : false;
            *negsrc = (cells[j].flags & SEQ_CHAIN_FLAG_NEGSRC) ? true : false;
            return true;
         }
         else
            (*iterate)++;
      }
   }
   return false;
}

void SequenceLayer::clearChainSources(int row, int step, int pat)
{
   int i;
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   jassert(step >= 0 && step < SEQ_MAX_STEPS);
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);

   SourceCell *cells = mPats[pat].mChains[step].cells;
   
   for (i = 0; i < SEQ_MAX_CHAIN_SOURCES; i++) {
      if ((cells[i].flags & SEQ_CHAIN_FLAG_USED) &&
         cells[i].targetrow == row) {
         cells[i] = SourceCell();         
      }
   }

}


void
SequenceLayer::setKeyScaleOct(const char * scale, const char * key, int octave)
{
   jassert(octave >= 0 && octave < SEQ_NUM_OCTAVES);

   strncpy(mStdKeyName, key, SEQ_KEY_NAME_MAXLEN);
   mStdKeyName[SEQ_KEY_NAME_MAXLEN - 1] = 0;
   strncpy(mStdScaleName, scale, SEQ_SCALE_NAME_MAXLEN);
   mStdScaleName[SEQ_SCALE_NAME_MAXLEN - 1] = 0;
   mStdOctave = octave;

   // now fill in notes
   SeqScale iterator;
   iterator.startIterateNotesInScale(mStdScaleName, mStdKeyName, octave);

   for (int i = 0; i < SEQ_MAX_ROWS - 1; i++)
      mNoteSets[standardSet].notes[(SEQ_MAX_ROWS - 2) - i].note = iterator.getNextNote();
   mNoteSets[standardSet].notes[SEQ_MAX_ROWS - 1].note = -1; //"off" 

}

void 
SequenceLayer::getKeyScaleOct(const char ** scale, const char ** key, int * oct)
{
   *oct = mStdOctave;
   *key = mStdKeyName;
   *scale = mStdScaleName;
}

void 
SequenceLayer::copyScaleData(const SequenceLayer & src)
{
   int i;
   for (i = 0; i<noteSetCount; i++)
      mNoteSets[i] = src.mNoteSets[i];
   mCurrentNoteSet = src.mCurrentNoteSet;
   memcpy(mStdKeyName, src.mStdKeyName, SEQ_KEY_NAME_MAXLEN);
   memcpy(mStdScaleName, src.mStdScaleName, SEQ_SCALE_NAME_MAXLEN);
   mStdOctave = src.mStdOctave;
}

int 
SequenceLayer::getNumSteps() 
{
   return mNumSteps;
}

void 
SequenceLayer::setNumSteps(int val)
{
   jassert(val >= SEQ_MIN_STEPS && val <= SEQ_MAX_STEPS);
   mNumSteps = val;
   // check lengths of all steps to make sure they would not go past the end of the pattern
   for (int i = 0; i < mNumSteps; i++) {
      for (int j = 0; j < mNumRows; j++) {
         Cell *c = &mPats[mCurrentPattern].mRows[j].mSteps[i];
         if (/*i + */c->length >= mNumSteps-1) {
            setLength(j, i, (char)(mNumSteps - 1)); // (i + 1)));
         }
      }
   }

}

void 
SequenceLayer::setVel(int row, int step, char vel, int pat) 
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   jassert(step >= 0 && step < SEQ_MAX_STEPS);
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);
   mPats[pat].mRows[row].mSteps[step].velo = vel;
}

char 
SequenceLayer::getVel(int row, int step, int pat) 
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   jassert(step >= 0 && step < SEQ_MAX_STEPS);
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);
   return mPats[pat].mRows[row].mSteps[step].velo;
}

void 
SequenceLayer::setProb(int row, int step, char prob, int pat) 
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   jassert(step >= 0 && step < SEQ_MAX_STEPS);
   jassert(prob >= SEQ_PROB_OFF);
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);
   mPats[pat].mRows[row].mSteps[step].prob = prob;
   if (prob == SEQ_PROB_OFF) {
      // clear chains that target this step and source from this step
      // TODO very inefficient to have this code here
      for (int j = 0; j < SEQ_MAX_STEPS; j++) {
         for (int i = 0; i < SEQ_MAX_CHAIN_SOURCES; i++) {
            if( (mPats[pat].mChains[j].cells[i].flags & SEQ_CHAIN_FLAG_USED) &&
               mPats[pat].mChains[j].cells[i].col == step &&
               mPats[pat].mChains[j].cells[i].row == row) {
               mPats[pat].mChains[j].cells[i].flags = 0;
            }
         }
      }

      for (int i = 0; i < SEQ_MAX_CHAIN_SOURCES; i++) {         
         if ((mPats[mCurrentPattern].mChains[step].cells[i].flags & SEQ_CHAIN_FLAG_USED)
            && mPats[mCurrentPattern].mChains[step].cells[i].targetrow == row)
            mPats[mCurrentPattern].mChains[step].cells[i].flags = 0;
      }
   }
}

char 
SequenceLayer::getProb(int row, int step, int pat) 
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   jassert(step >= 0 && step < SEQ_MAX_STEPS);
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);

   return mPats[pat].mRows[row].mSteps[step].prob;
}

void 
SequenceLayer::setLength(int row, int step, char length, int pat) 
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   jassert(step >= 0 && step < SEQ_MAX_STEPS);
   jassert(length >= 0);
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);
   mPats[pat].mRows[row].mSteps[step].length = length;
}

char
SequenceLayer::getLength(int row, int step, int pat) 
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   jassert(step >= 0 && step < SEQ_MAX_STEPS);
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);

   return mPats[pat].mRows[row].mSteps[step].length;
}

void SequenceLayer::setOffset(int row, int step, char offset, int pat)
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   jassert(step >= 0 && step < SEQ_MAX_STEPS);
   jassert(offset >= -50 && offset <= 50);
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);
   mPats[pat].mRows[row].mSteps[step].offset = offset;
}

char SequenceLayer::getOffset(int row, int step, int pat)
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   jassert(step >= 0 && step < SEQ_MAX_STEPS);
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);

   return mPats[pat].mRows[row].mSteps[step].offset;
}

void SequenceLayer::clearCell(int row, int step)
{
   setLength(row, step, 0);
   setProb(row, step, SEQ_PROB_OFF);
   setVel(row, step, 0);
   setOffset(row, step, 0);
}

void SequenceLayer::copyCell(int targRow, int targStep, int srcRow, int srcStep)
{
   char srcProb;
   // clear out chains from target (if any)
   clearCell(targRow, targStep);

   srcProb = getProb(srcRow, srcStep);
   if (srcProb != SEQ_PROB_OFF) {
      setProb(targRow, targStep, srcProb);
      // copy chain sources and targets across
      int it = -1, srow = 0, scol = 0;
      bool negtgt = false;
      bool negsrc = false;
      while (getChainSource(srcRow, srcStep, &it, &srow, &scol, &negtgt, &negsrc)) {
         addChainSource(targRow, targStep, srow, scol, negtgt, negsrc);
      }
      it = -1;
      // effectively set the source cell to our new location (i know, hard to understand)
      while (getChainTarget(srcRow, srcStep, &it, &srow, &scol, &negtgt, &negsrc)) {
         addChainSource(srow, scol, targRow, targStep, negtgt, negsrc);
      }

      setLength(targRow, targStep, getLength(srcRow, srcStep));
      setVel(targRow, targStep, getVel(srcRow, srcStep));
      setOffset(targRow, targStep, getOffset(srcRow, srcStep));
   }
}

void 
SequenceLayer::setNote(int row, char val, bool custom) 
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   WhichNoteSet s = custom ? customSet : standardSet;
   mNoteSets[s].notes[row].note = val;
}

char 
SequenceLayer::getNote(int row, bool custom) 
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   WhichNoteSet s = custom ? customSet : standardSet;
   return mNoteSets[s].notes[row].note;
}

char *
SequenceLayer::getNoteName(int row) 
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   return mNoteSets[customSet].notes[row].noteName;
}

void 
SequenceLayer::setNoteName(int row, const char * name) 
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   memset(mNoteSets[customSet].notes[row].noteName, 0, SEQ_MAX_NOTELABEL_LEN);
   strncpy(mNoteSets[customSet].notes[row].noteName, name, SEQ_MAX_NOTELABEL_LEN);
}

// get note from "current" (either custom or standard)

char 
SequenceLayer::getCurNote(int row) 
{
   jassert(row >= 0 && row < SEQ_MAX_ROWS);
   return mNoteSets[mCurrentNoteSet].notes[row].note;
}

int 
SequenceLayer::getRowForNote(char note)
{
   NoteSet *set = &mNoteSets[mCurrentNoteSet];
   for (int i = SEQ_MAX_ROWS-1; i >= 0; i--) {
      if (set->notes[i].note == note)
         return i;
   }
   return -1;
}

// set us to point to either custom or standard

void
SequenceLayer::setNoteSource(bool custom) 
{
   mCurrentNoteSet = custom ? customSet : standardSet;
}

// determine whether we are in custom notes

bool
SequenceLayer::noteSourceIsCustom() 
{
   return mCurrentNoteSet == customSet;
}

int
SequenceLayer::getMaxRows() 
{
   return mNumRows;
}

void
SequenceLayer::setMaxRows(int val) 
{
   jassert(val >= SEQ_MIN_ROWS&& val <= SEQ_MAX_ROWS);
   mNumRows = val;
}

int
SequenceLayer::getMaxPoly() 
{
   return mMaxPoly;
}

void
SequenceLayer::setMaxPoly(int val) 
{
   jassert(val >= 1 && val <= SEQ_MAX_ROWS);
   mMaxPoly = val;
}

int SequenceLayer::getPolyBias()
{
   return mPolyBias;
}

void SequenceLayer::setPolyBias(int val)
{
   jassert(val >= SEQ_POLY_BIAS_MIN &&
      val <= SEQ_POLY_BIAS_MAX);
   mPolyBias = val;
}

bool
SequenceLayer::isMonoMode() 
{
   return mIsMonoMode;
}

void
SequenceLayer::setMonoMode(bool val) 
{
   mIsMonoMode = val;
}

bool SequenceLayer::isCombineMode()
{
   return mCombineMode;
}

void SequenceLayer::setCombineMode(bool val)
{
   mCombineMode = val;
}

int
SequenceLayer::getClockDivider() 
{
   return mClockDivider;
}

void
SequenceLayer::setClockDivider(int c) 
{
   jassert(c >= SEQ_MIN_CLOCK_DIV && c <= SEQ_MAX_CLOCK_DIV);
   mClockDivider = c;
}

char
SequenceLayer::getMidiChannel() 
{
   return mMidiChannel;
}

void
SequenceLayer::setMidiChannel(char val) 
{
   jassert(val >= 1 && val <= 16);
   mMidiChannel = val;
}

int
SequenceLayer::getStepsPerMeasure() 
{
   return mStepsPerMeasure;
}

void
SequenceLayer::setStepsPerMeasure(int val) 
{
   jassert(val >= SEQ_MIN_STEPS && val <= SEQ_DEFAULT_NUM_STEPS);
   mStepsPerMeasure = val;
}

int
SequenceLayer::getDutyCycle() 
{
   return mDutyCycle;
}

void
SequenceLayer::setDutyCycle(int val) 
{
   jassert(val >= SEQ_DUTY_MIN && val <= SEQ_DUTY_MAX);
   mDutyCycle = val;
}

void SequenceLayer::setHumanVelocity(int val)
{
   jassert(val >= 0 && val <= SEQ_MAX_HUMAN_VELOCITY);
   mHumanVelo = val;
}

int SequenceLayer::getHumanVelocity()
{
   return mHumanVelo;   
}

void SequenceLayer::setHumanPosition(int val)
{
   jassert(val >= 0 && val <= SEQ_MAX_HUMAN_POSITION);
   mHumanPos = val;
}

int SequenceLayer::getHumanPosition()
{
   return mHumanPos;
}

void SequenceLayer::setHumanLength(int val)
{
   jassert(val >= 0 && val <= SEQ_MAX_HUMAN_LENGTH);
   mHumanLen = val;
}

int SequenceLayer::getHumanLength()
{
   return mHumanLen;
}

const char * SequenceLayer::getLayerName()
{
   return mLayerName;
}

void SequenceLayer::setLayerName(const char * txt)
{
   strncpy(mLayerName, txt, SEQ_LAYER_NAME_MAXLEN);
   mLayerName[SEQ_LAYER_NAME_MAXLEN - 1] = 0;
}

void SequenceLayer::setPatternName(const char * txt, int pat)
{
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);
   strncpy(mPats[pat].mName, txt, SEQ_PATTERN_NAME_MAXLEN);
   mPats[pat].mName[SEQ_PATTERN_NAME_MAXLEN - 1] = 0;

}

const char * SequenceLayer::getPatternName(int pat)
{
   if (pat == -1)
      pat = mCurrentPattern;
   jassert(pat >= 0 && pat < SEQ_MAX_PATTERNS);
   return mPats[pat].mName;
}

/*============================================
SeqMidiMapItem
*============================================*/
void
SeqMidiMapItem::clear() 
{
   mAction = SEQMIDI_ACTION_INVALID;   // invalid
   mTarget = 0;   // invalid
   mValue = 0;    // invalid
   mType = 0;     // invalid
   mNote = 0;     // valid
   mChannel = 1;  // valid
}
