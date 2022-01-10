/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "StochaEngine.h"
#define INVALIDPOS -9999
#define INVALIDREALPOS -1.0
//#include <juce_core/logging/juce_Logger.h>

bool StochaEngine::addMidiEvent(int startSamples, int8_t note, int8_t velo, int8_t chan, int numSamples)
{
   int transp,i;
   StochaEvent *a = nullptr, *b = nullptr, *cur = nullptr;
   SequenceData *s = mSeq->getAudSeqData();
   SequenceLayer *sd =  s->getLayer(mLayer);
   
   // do we need to transpose the note?
   transp = mOverrideTranspose.get(0) + note;
   if (transp > 127)
      note = 127;
   else if (transp < 0)
      note = 0;
   else
      note = (int8_t)transp;

   // see if we must combine with another playing note   
   // or stop that one before this one starts
   for (i = 0; i < SEQ_MAX_MIDI_EVENTS; i++) {
      cur = &mEvents[i];
      // see if the same note is already playing and will be playing
      // when we add our new note
      if (velo &&
         cur->mNote == note &&
         cur->mChan == chan &&
         cur->mVelo == 0 &&
         cur->mNumSamples > startSamples) {

         if (sd->isCombineMode()) {
            cur->mNumSamples = startSamples + numSamples;
            return true; // nothing further to do
         }
         else { // truncate that note
            cur->mNumSamples = startSamples ? startSamples - 1 : 0;
            break; // continue to add our new note
         }
      }
   }   
   

   // we need two available slots, one for note on and one for note off
   for (i = 0; i < SEQ_MAX_MIDI_EVENTS; i++) {
      cur = &mEvents[i];
      if (cur->mNumSamples == -1) { // slot is available
         if (a == nullptr)
            a = cur;
         else if (b == nullptr) {
            b = cur;
            break; // we have both slots we need
         }
      }      
   }
   if(a==nullptr || (velo && b==nullptr)) 
      return false; // no slots available now

   

   // note on (or off in the case of velo=0)
   a->mNumSamples = startSamples;
   a->mNote = note;
   a->mVelo = velo;
   a->mChan = chan;
   if (velo) { // if it's a note on
      mNumActiveNoteOnEvents++;

      // add a corresponding note off
      a->mCorrespondingNoteOff = b;

      // note off
      b->mNumSamples = startSamples + numSamples;
      b->mNote = note;
      b->mVelo = 0;      
      b->mChan = chan;
   } else {
      a->mCorrespondingNoteOff = nullptr;      
   }

   mNumActiveNoteOffEvents++;
   
   return true;
}

StochaEngine::StochaEngine() :
   mNumActiveNoteOnEvents(0),
   mNumActiveNoteOffEvents(0),
   mCurrentStepPosition(INVALIDPOS),
   mRealStepPosition(INVALIDREALPOS),
   mSeq(nullptr),
   mLayer(-1),
   mOldSeed(0),
   mOldSeq(0),
   mOldStepPosInTrack(0.0),
   mPlayStartPosition(0.0)

{
   memset(mMapping, 0, sizeof(mMapping));
   resetMappingSchema();
}

StochaEngine::~StochaEngine()
{
   resetMappingSchema();
}

void StochaEngine::init(SeqDataBuffer * s, int layer)
{
   mSeq = s;
   mLayer = layer;
   
}

void StochaEngine::setRandomSeed(uint64 seed, uint64 seqno)
{
   // only set it if it's changing
   if(seed != mOldSeed || seqno != mOldSeq)
      mRand.setRandomSeedAndSequence(seed, seqno);
   mOldSeed = seed;
   mOldSeq = seqno;
   
}

int StochaEngine::getCurrentStepPosition(int *fraction)
{
   int ret = getCurrentOverallPosition(fraction);
   if (ret != -1) {
      SequenceData *s = mSeq->getAudSeqData();
      SequenceLayer *sd = s->getLayer(mLayer);
      int numsteps = mOverrideNumSteps.get(sd->getNumSteps());
      ret %= numsteps;
   }
   return ret;
}

int StochaEngine::getCurrentOverallPosition(int * fraction)
{
   int ret;

   if (mRealStepPosition >= 0.0) {
      // round down for step position
      ret = (int)mRealStepPosition;
      // fractional portion if needed
      if (fraction) {
         *fraction = (int)((mRealStepPosition - (double)((int)mRealStepPosition)) * 100.0);
      }
   }
   else {
      ret = -1;
      if (fraction)
         *fraction = 0;
   }

   return ret;
}

int StochaEngine::getNumSteps()
{
   SequenceData *s = mSeq->getAudSeqData();
   SequenceLayer *sd = s->getLayer(mLayer);
   return mOverrideNumSteps.get(sd->getNumSteps());
}

int StochaEngine::getPlayingPattern()
{
   SequenceData *s = mSeq->getAudSeqData();
   return mOverridePattern.get(s->getLayer(mLayer)->getCurrentPattern());
}

bool StochaEngine::getMuteState()
{
   SequenceData *s = mSeq->getAudSeqData();
   int v;
   if (s->getLayer(mLayer)->getMuted())
      v = SEQMIDI_VALUE_MUTE_MUTE;
   else
      v = SEQMIDI_VALUE_MUTE_UNMUTE;
   return mOverrideMute.get(v) == SEQMIDI_VALUE_MUTE_MUTE;
}

bool
StochaEngine::playPositionChange(int samples_per_step, // samples per sequencer step
                                 int step_position, // which step position is next
                                 int samples_until)  // num samples between now and next step pos                                 
{
   SequenceLayer *sd = 0;
   SequenceData *s = 0;
   int playcount = 0;
   int position = 0; // position in our pattern 
   jassert(step_position >=0);
   jassert(samples_per_step > 0);
   jassert(samples_until >= 0);

   s = mSeq->getAudSeqData();
   sd=s->getLayer(mLayer);
   int pat = mOverridePattern.get(s->getLayer(mLayer)->getCurrentPattern());
   int numsteps = mOverrideNumSteps.get(sd->getNumSteps());
   bool muted;
   // how long to play a step for (assuming a length 0 step. gets added to non-zero ones)
   // so a value of 50 means play it for half of a step, but that will get added to length
   // if it's present
   double dds=(double)mOverrideDutyCycle.get(sd->getDutyCycle()) /100.0;
   
   // this is the length of a step in samples
   int step_size = (int)((double)samples_per_step*dds);

   // which step are we on in this sequence
   position = step_position % numsteps;

   // determine whether it's muted (either by midi control or on the patch)
   muted = getMuteState();
   
   // prepare to get random values for this position in the track
   mRand.prepareSeqPosition(step_position);
   
   // determine notes to play. in mono mode, we determine one note to play at the
   // position, and consume one element in the array
   if (sd->isMonoMode()) {
      int selrow;
      selrow = getRandomSingle(position); // returns -1 if no note
      if (selrow != -1) {
         mMulti[0].rowToPlay = selrow;
         playcount = 1;
      }
      else
         playcount = 0;
   }
   else { // poly mode
      int maxpoly =mOverrideMaxPoly.get(sd->getMaxPoly());
      // determine an array of notes to play
      playcount = getRandomMulti(position, maxpoly);      
   }

   // clear this position in our tracker of notes
   mDependencySource[position].reset();

   // assuming we are not muted we play. We still need to do the above if we are
   // muted, so that our deterministic random chain is not broken.
   if (!muted) {
      int swing = mOverrideSwing.get(s->getSwing());
      // loop through played notes and add them
      for (int i = 0; i < playcount; i++) {

         // add it to our tracker of notes that have played
         mDependencySource[position].set(mMulti[i].rowToPlay);

         int8_t whichNote = sd->getCurNote(mMulti[i].rowToPlay);
         if (whichNote != -1) { // -1 is the "off" note, which we ignore
            int gruuv;
            int8_t velocity = sd->getVel(mMulti[i].rowToPlay, position, pat);
            int humanpos, humanvel, humanlen;

            // get additional length to play, or get number of times to trigger (0 means play one step,
            // so 1 means play one additional step. anything negative is for multiple triggers which
            // the step will be divided into that many)
            int numtriggers=1; // will be >1 for multi trigger
            int lensize=0;
            int steplen=sd->getLength(mMulti[i].rowToPlay, position, pat);

            if (steplen < 0) { // multi trigger
              numtriggers = (-steplen)+1; // ie. -1 means two triggers
              // determine total length of each sub step using duty cycle
              double substeplen = (double)samples_per_step / (double)numtriggers;
              double d = dds;
              // to keep things simple, max out our duty cycle around 100
              if(d > 0.99) d=0.99;
              lensize = (int)(substeplen * d);

            } else { // single trigger with optional additional length to play
              // length here is in samples and represents whole steps.
              lensize = samples_per_step * steplen; // will be 0 in the case of no additional length
              // add the step size with duty cycle applied
              lensize += step_size;
            }


            
            ////////////////////////////
            // Apply swing/groove/offset

            // see if the cell has an offset value. This overrides swing and groove
            gruuv = sd->getOffset(mMulti[i].rowToPlay, position, pat);
            if (!gruuv) { // no override so check swing
               if (swing) { 
                  // swing is active so ignore groove and apply swing instead
                  if (position % 2 == 1)
                     gruuv = swing;
                  else
                     gruuv = 0;
               }
               else { // no swing active
                  // adjust samples based on groove value for that 16th position
                  gruuv = s->getGroove(position % SEQ_DEFAULT_NUM_STEPS);
               }
            }

            /////////////////////
            // Humanize position
            if ((humanpos = mOverridePosVariance.get(sd->getHumanPosition())) != 0) {
               // humanpos will be 1 to 50 in this case,
               // where 50 is allowing it to go all the way to halfway to next step
               // or halfway to previous step
               // generate -humanpos..humanpos inclusive

               // note that this random number is generated after our steps so should
               // not affect our steps if in stable mode
               int h = mRand.getNextRandom((humanpos*2) + 1) - humanpos;
               jassert(h >= -humanpos && h <= humanpos);

               // add this to gruuv, ensuring we don't exceed 50 or -50
               gruuv +=h;
               if(gruuv < -50)
                  gruuv = -50;
               else if(gruuv > 50)
                  gruuv = 50;
            }

            /////////////////////
            // Humanize velocity?
            // only do so if it's not 0 (which may have special meaning)
            if (velocity && ((humanvel = mOverrideVeloVariance.get(sd->getHumanVelocity())) != 0)) {
               // human vel is a value from 0 to 100
               // determine a value between -humanvel and humanvel inclusive
               int h = mRand.getNextRandom((humanvel * 2) + 1) - humanvel;
               // determine how much it alters the current velocity
               // this is a fixed value rather than a percentage of current velocity
               // (is that ok?)
               h = (h * 127) / 100;
               h += velocity;
               if (h < 1)
                  h = 1;
               else if (h > 127)
                  h = 127;
               velocity = (int8_t)h;
            }

            /////////////////////
            // humanize length (only for single triggering)
            if ((humanlen = mOverrideLengthVariance.get(sd->getHumanLength())) != 0 && numtriggers == 1) {
               // a value between 1..100
               // so get 1..humanlen  inclusive
               int h = mRand.getNextRandom(humanlen)+1;
               // amount to adjust as a fraction of the actual note length to be played
               h = (h * lensize) / 100;
               // subtract this from the length
               lensize -= h;
               // it might be 0 at this point
            }

            // determine where in time the note is
            int gruuvOffset = samples_until + (gruuv * samples_per_step / 100);

            if (gruuvOffset < 0)
               gruuvOffset = 0; // can't go back in time

            // determine offsets for multiple triggers
            int samples_per_trigger = samples_per_step / numtriggers;
            
            // and finally add it assuming it has a length
            if (lensize) {
               for (int j = 0; j < numtriggers; j++) {
                  if (!addMidiEvent(gruuvOffset,
                whichNote, // which note to play
                velocity,  // velocity
                (int8_t)mOverrideOutputChannel.get(
                sd->getMidiChannel()), // midi channel to play on
          lensize)) // size in steps + size of one step (which is adjusted by duty cycle)
                    return false;

                  // set position for next trigger (if applicable)
                  gruuvOffset += samples_per_trigger;
               } // for each trigger
            } // if we have a length
         } // note is not -1
      } // for 0 to playcount
   } // if not muted
   return true;
}

int StochaEngine::getRandomSingle(int position)
{
   SequenceData *s = mSeq->getAudSeqData();
   SequenceLayer *sd;
   int i, startrow;
   int randy=0;
   bool neg = false;
   bool useMand = false;
   bool allMandAreZero = true; // will be set true if all mandatory notes have prob 0
   int lastMandSet = -1;
   std::bitset<SEQ_MAX_ROWS> mandatoryOn;
   std::bitset<SEQ_MAX_ROWS> mandatoryOff;
   
   sd =s->getLayer(mLayer);
   int maxprob = 0;
   int pat = mOverridePattern.get(s->getLayer(mLayer)->getCurrentPattern());

   // notes go bottom to top so chop off higher notes
   startrow = SEQ_MAX_ROWS - sd->getMaxRows();


   // first see if there are any mandatory notes to play
   for (i = startrow; i < SEQ_MAX_ROWS; i++) {      
      if (isMandatory(position, i, &neg, pat)) {
         if (!neg) {
            mandatoryOn.set(i);
            lastMandSet = i; // save time
            // for a weird case (see below)
            if (sd->getProb(i, position, pat) > 0)
               allMandAreZero = false; // we have at least one mandatory note that is not prob-0 ("never")
         }
         else
            mandatoryOff.set(i);
      }
   }

   // if there are more than one, use those 2..n and randomly pick one based on it's prob
   // If all the mandatory notes have a "never" prob, then none of them will ever play
   // (maxprob will be 0 below and -1 is returned)
   // to fix this, we need to check for this condition, and treat them all equally so that one
   // is randomly selected to play
   if (mandatoryOn.count() > 1) {
      useMand = true;
   }
   else if (mandatoryOn.count() == 1) {
      // if there is one, we are done
      return lastMandSet;
   }

   // if there are none, but we have any mandatory negatives, those should be excluded when we do
   // our normal determination
   
   // Cycle 1 adds up probabilities for selected notes and then generates a random number based on that.
   // Cycle 2 does the same thing, but exits as soon as a note's prob exceeds the random number and
   // returns that note
   
   for (int cycle = 1; cycle < 3; cycle++) {
      // add up probs for all "on" notes
      for (i = startrow; i < SEQ_MAX_ROWS; i++) {
         // if we have >1 mandatory note, we will only consider those that are mandatory
         if ((useMand && !mandatoryOn.test(i)))
            continue;

         // add it assuming it's not mandatorily off, and has some value
         if (!mandatoryOff.test(i)) {
            int8_t prob = sd->getProb(i, position, pat);
            if (prob != SEQ_PROB_OFF) {
               if (prob > 0) {
                  // the normal case
                  maxprob += (int)prob;
               }
               // probability 0 special handling ("never")
               else if (useMand && allMandAreZero) {
                  // if useMand is true it means that only mand notes are considered, 
                  // so in this case, we had all mandatory notes are "never". so they all should get equal
                  // chance to play (as opposed to one or more of them having a probability in which case
                  // the normal probability selection is followed)
                  maxprob += SEQ_PROB_ON;
               }
            } // if prob != SEQ_PROB_OFF
         } // if !mandatoryOff.test...

         // 2nd cycle thru we select the note that met or exceeded our random number
         if (cycle==2 && randy <= maxprob) {
            // it falls in this bucket, so play this note
            return i;
         }

      } // for each row

      if (cycle == 1) {
         if (!maxprob)
            return -1; // no active notes 

         // generate a random number 1..maxprob incl
         randy = 1 + mRand.getNextRandom(maxprob);

         // start counting again next time thru
         maxprob = 0;
      } // if cycle is 1
   } // for cycle 1..2


   jassertfalse; // it needs to have been picked up above
   return -1;
}


int StochaEngine::getRandomMulti(int position, int maxpoly)
{
   SequenceData *s = mSeq->getAudSeqData();
   SequenceLayer *sd = s->getLayer(mLayer);
   int startrow=0;
   int i, randy;
   int prob;
   int used = 0;
   bool mandOff = false; // mandatorily off
   int nummand = 0;
   int pat = mOverridePattern.get(s->getLayer(mLayer)->getCurrentPattern());
   int bias = mOverridePolyBias.get(sd->getPolyBias());
   bool playNone=false; // don't play any (except mandatory)
   bool playNoneMand = false; // don't play any at all

   // notes go bottom to top so chop off higher notes
   startrow = SEQ_MAX_ROWS - sd->getMaxRows();
   for (i = startrow; i < SEQ_MAX_ROWS; i++) {
      int playRow=-1;
      bool playMand = false;
      // we always generate it so that if the user has selected "Stable" for random
      // values, they can still add in other cells, and the existing ones will play fine
      // 0..99 inclusive
      randy = mRand.getNextRandom(100);

      prob = (int)sd->getProb(i, position, pat);
      // adjust for bias
      if (prob != SEQ_PROB_OFF && prob != SEQ_PROB_ON) {
         prob += bias;
         if (prob > SEQ_PROB_ON)
            prob = SEQ_PROB_ON;
         else if (prob < SEQ_PROB_OFF)
            prob = SEQ_PROB_OFF;
      }


      // determine if it's mandatory (it must also be on. can't play a cell thats not on even if mand.)
      // so it may have a prob of 0 which would allow it to play in this case but not otherwise. 
      if (isMandatory(position, i, &mandOff, pat)) {
         // we need to only add it if it's not mand off
         if (!mandOff && prob != SEQ_PROB_OFF) {
            playRow = i;
            playMand = true;
         }
      } else  // not mandatory
      if (prob >= SEQ_PROB_ON || (prob != SEQ_PROB_OFF && prob >= randy)) {
         // always play, or was selected
         // (however not considered mandatory, so may be trimmed)
         playRow = i;
         playMand = false;
      }            

      if (playRow != -1) {
         // if a value was selected
         mMulti[used].rowToPlay = playRow;
         mMulti[used].mandatory = playMand;
         used++;
         if (playMand)
            nummand++;

         if (sd->getCurNote(playRow) == -1) {
            // playing an "off" is tantamount to not playing anything at all
            // if this one is mandatory as well, we will definitely not play anything
            // otherwise we'll only play mandatory (if any)
            playNone = true;
            if (playMand)
               playNoneMand = true;
         }

         
      }
   }

   // if we have a play none situation remove non mandatory items
   // possibly mandatory items. Leave the -1 item
   if (used && playNone) {
      int remove = 0;
      // winnow down to mandatory only
      for (i = 0; i < used; i++) {
         if (sd->getCurNote(mMulti[i].rowToPlay) == -1) {
            // leave this note in here, so that our tracker 
            // (that keeps track of which notes ended up playing, will track it)
         }
         else {
            if (playNoneMand || !mMulti[i].mandatory) {
               mMulti[i].rowToPlay = -1;
               remove++;
            }
         }
      }
      if (remove) {
         compactArray(used);
         used -= remove;
      }
   }

   // we might have a number of mandatory notes that exceeds max poly
   // since they are mandatory, we can't deny them from playing.
   if (nummand > maxpoly)
      maxpoly = nummand;

   // randomly trim out some notes if maxpoly is exceeded 
   if (used > maxpoly) {
      used = trimPoly(maxpoly, used);      
   }

   

   return used;
}

void StochaEngine::setAutomationParameterValue(int paramId, int value)
{
   if (value == SEQ_AUT_DEFAULT_VALUE_DESIG) {
      // reset it
      MidiOverride *targ=0;
      switch (paramId) {
      case SEQ_AUT_CLOCKDIV: targ = &mOverrideSpeed; break;         
      case SEQ_AUT_NUMSTEPS: targ = &mOverrideNumSteps; break;
      case SEQ_AUT_STEPS_PER_MEASURE: targ = &mOverrideStepsPerMeasure; break;
      case SEQ_AUT_NOTE_LENGTH: targ=&mOverrideDutyCycle; break;
      case SEQ_AUT_POS_VARIANCE: targ=&mOverridePosVariance; break;
      case SEQ_AUT_VELO_VARIANCE: targ = &mOverrideVeloVariance; break;
      case SEQ_AUT_LENGTH_VARIANCE: targ = &mOverrideLengthVariance; break;
      case SEQ_AUT_MUTED: targ = &mOverrideMute; break;
      case SEQ_AUT_OUTPUT_CHANNEL: targ = &mOverrideOutputChannel; break;
      case SEQ_AUT_MAX_POLY: targ = &mOverrideMaxPoly; break;
      case SEQ_AUT_POLY_BIAS: targ = &mOverridePolyBias; break;
      case SEQ_AUT_CURRENT_PATTERN: targ = &mOverridePattern; break;
      case SEQ_AUT_TRANSPOSE: targ = &mOverrideTranspose; break;
      case SEQ_AUT_GLOBAL_SWING: targ = &mOverrideSwing; break;
      default: break;
      }
      if (targ)
         targ->clear();
      return;
   }
   
   // pass these on to performMidiMapAction where possible to keep things consolidated
   // todo really it should be performMidiMapAction that is calling into here rather
   switch (paramId) {
   case SEQ_AUT_CLOCKDIV:  // tested    
   {
      int id;
      // the value expected here is numerator, and we have the index
      SeqScale::getClockDividerTextAndId(value, &id);
      performMidiMapAction(SEQMIDI_ACTION_SPEED, id);
      break;
   }
   case SEQ_AUT_NUMSTEPS: // tested
      performMidiMapAction(SEQMIDI_ACTION_STEPS, value);
      break;
   case SEQ_AUT_STEPS_PER_MEASURE: // tested
      mOverrideStepsPerMeasure.override(value);
      break;
   case SEQ_AUT_NOTE_LENGTH: // tested
      mOverrideDutyCycle.override(value);
      break;
   case SEQ_AUT_POS_VARIANCE: // tested
      mOverridePosVariance.override(value);
      break;
   case SEQ_AUT_VELO_VARIANCE: // tested
      mOverrideVeloVariance.override(value);
      break;
   case SEQ_AUT_LENGTH_VARIANCE: // tested
      mOverrideLengthVariance.override(value);
      break;
   case SEQ_AUT_MUTED: // tested
      performMidiMapAction(SEQMIDI_ACTION_MUTE, value ? SEQMIDI_VALUE_MUTE_MUTE : SEQMIDI_VALUE_MUTE_UNMUTE);
      break;
   case SEQ_AUT_OUTPUT_CHANNEL: // tested
      mOverrideOutputChannel.override(value);
      break;
   case SEQ_AUT_MAX_POLY: // tested
      mOverrideMaxPoly.override(value);
      break;
   case SEQ_AUT_POLY_BIAS: // tested
      // value expected is -99 to 99 which is what we have here (differs from midi)
      mOverridePolyBias.override(value);
      break;
   case SEQ_AUT_CURRENT_PATTERN: // tested
      mOverridePattern.override(value); // 0-based pattern - differs from midi impl
      break;
   case SEQ_AUT_TRANSPOSE: // tested
      performMidiMapAction(SEQMIDI_ACTION_TRANS, value);      
      break;
   case SEQ_AUT_GLOBAL_SWING: // tested
      mOverrideSwing.override(value);
      break;
   default:
      break;
   }
}


void StochaEngine::performMidiMapAction(int action, int value)
{
   SequenceLayer *sd = mSeq->getAudSeqData()->getLayer(mLayer);

   switch (action) {
   case SEQMIDI_ACTION_CHGPAT:
      // real pattern number is 0 based!
      if (value == SEQMIDI_VALUE_PAT_NEXT) {
         if (mOverridePattern.mValue < SEQ_MAX_PATTERNS - 1)
            mOverridePattern.override(mOverridePattern.mValue + 1);
      } 
      else if (value == SEQMIDI_VALUE_PAT_PREV) {
         if (mOverridePattern.mValue > 0)
            mOverridePattern.override(mOverridePattern.mValue - 1);
      }
      else { // 1 based pattern number incoming
         mOverridePattern.override(value-1);
      }
      break;
   case SEQMIDI_ACTION_MUTE:
      // we need to set it to mute or unmute only
      if (value == SEQMIDI_VALUE_MUTE_TOGGLE) {
         int v; // determine new value
         if(sd->getMuted())
            v = SEQMIDI_VALUE_MUTE_MUTE;
         else
            v= SEQMIDI_VALUE_MUTE_UNMUTE;

         v = mOverrideMute.get(v);

         if (v == SEQMIDI_VALUE_MUTE_MUTE)
            v = SEQMIDI_VALUE_MUTE_UNMUTE;
         else
             v = SEQMIDI_VALUE_MUTE_MUTE;

         mOverrideMute.override(v);

      } else{ // mute or unmute
         mOverrideMute.override(value);
      }
      break;
   case SEQMIDI_ACTION_SPEED: {
      int cs = mOverrideSpeed.get(sd->getClockDivider());

      if (value == SEQMIDI_VALUE_SPD_DBL) { // mult current by 2 if possible
         if (cs < SEQ_MAX_CLOCK_DIV)
            mOverrideSpeed.override(cs * 2);
      }
      else if (value == SEQMIDI_VALUE_SPD_HALF) { // div current by 2 if possible
         if (cs > SEQ_MIN_CLOCK_DIV)
            mOverrideSpeed.override(cs / 2);
      }
      else // set to actual incoming value
         mOverrideSpeed.override(value);
      break;
   }
   case SEQMIDI_ACTION_PBIAS: {      
      // value will be variable in range 0 to 127
      // convert to -99 to 99
      int range = (SEQ_POLY_BIAS_MAX - SEQ_POLY_BIAS_MIN) + 1;      
      mOverridePolyBias.override(((range*value) / 127) + (SEQ_POLY_BIAS_MIN-1));
      break;
   }
   case SEQMIDI_ACTION_TRANS:
   {
      // transpose
      int t;
      if (SeqScale::getTransposeSemitones(value, &t)) {
         // value is absolute
         mOverrideTranspose.override(t);
      }
      else { // value is relative to current transpose and in semitones
         // no real way to validate this properly. we need to validate when generating the note
         int q = mOverrideTranspose.get(0) + t;
         if(q >=-127 && q <= 127)
            mOverrideTranspose.override(q);
      }
      break;
   }
   case SEQMIDI_ACTION_STEPS:
      // set number of steps.  
      mOverrideNumSteps.override(value);
      break;
   case SEQMIDI_ACTION_RESET: {
      // reset one of the items
      switch (value) {
      case SEQMIDI_ACTION_CHGPAT:
         mOverridePattern.clear();
         break;
      case SEQMIDI_ACTION_MUTE:
         mOverrideMute.clear();
         break;
      case SEQMIDI_ACTION_SPEED:
         mOverrideSpeed.clear();
         break;
      case SEQMIDI_ACTION_TRANS:
         mOverrideTranspose.clear();
         break;
      case SEQMIDI_ACTION_STEPS:
         mOverrideNumSteps.clear();
         break;
      case SEQMIDI_ACTION_PBIAS:
         mOverridePolyBias.clear();
         break;
      default:
         jassertfalse;
      }

      break;
   }
   default:
      // might be handled in PluginProcessor
      break;
   }
}

void StochaEngine::rebuildMappingSchema()
{
   // this rebuilds our linked list hash mechanism to optimize for
   // incoming data
   SequenceData *sd= mSeq->getAudSeqData();
   MidiMappingItem *cur;
   int len = sd->getMappingCount();
   for (int i = 0; i < len; i++) {
      SeqMidiMapItem *mm = sd->getMappingItem(i);
      if (mm->mAction != SEQMIDI_ACTION_INVALID && 
         mm->mTarget != 0 &&
         mm->mValue != 0 && 
         mm->mChannel != 0 && 
         mm->mNote >= 0 && 
         (mm->mTarget== SEQMIDI_TARGET_ALL || mm->mTarget == (mLayer+1))
         ) {
         if (!mMapping[mm->mNote]) {
            cur = mMapping[mm->mNote] = new MidiMappingItem();
         } else {
            cur = mMapping[mm->mNote];
            while (cur->mNext) {
               cur = cur->mNext;
            }
            cur->mNext = new MidiMappingItem();
            cur = cur->mNext;
         }
         cur->mChannel = mm->mChannel;
         cur->mAction = mm->mAction;
         cur->mValue = mm->mValue;
         cur->mType = mm->mType;
      }
   }

   mMappingIsValid = true;
}

int StochaEngine::trimPoly(int maxpoly, int used)
{
   /* The most random numbers we should ever generate would not exceed used - maxpoly         
   */
   int idx=-1;
   int left = used;
   int state = 0; // 0=rand, 1=up, 2=down
   int bailout = 1000; // prevent infinite loop
   while (left > maxpoly && bailout) {
      bailout--;
      if (state == 0) {
         // pick one slot to throw away. don't throw away mandatory slots
         idx = mRand.getNextRandom(used);
         if (mMulti[idx].rowToPlay != -1 && mMulti[idx].mandatory==false) {
            mMulti[idx].rowToPlay = -1;
            left--;
            goto reloop; // do random again
         }

         // failed to get a non empty slot
         state = 1 + mRand.getNextRandom(2); // up or down
      }

      if(state==1) { // up
         jassert(idx != -1);
         while (idx < used - 1) {
            idx++;
            if (mMulti[idx].rowToPlay != -1 && mMulti[idx].mandatory == false) {
               mMulti[idx].rowToPlay = -1;
               left--;
               state = 0; // do random next time
               goto reloop;
            }
         }
         state = 2; // try going down
      }

      if (state == 2) {
         while (idx > 0) {
            idx--;
            if (mMulti[idx].rowToPlay != -1 && mMulti[idx].mandatory == false) {
               mMulti[idx].rowToPlay = -1;
               left--;
               state = 0; // do random next time
               goto reloop;
            }
         }
         state = 1; // try going up
      }
      
   reloop:;
   } // while we are above max poly

   // you got an infinite loop problem
   jassert(bailout);

   // remove gaps in array
   compactArray(used);

   jassert(left == maxpoly);
   return left;
}

void StochaEngine::compactArray(int used)
{
   // shift all down to eliminate -1 items
   int p = 0;
   for (int i = 0; i < used; i++) {
      mMulti[p] = mMulti[i];
      if (mMulti[p].rowToPlay != -1)
         p++;

   }
}

bool StochaEngine::isMandatory(int col, int row, bool * off, int pat)
{
   bool ret = false;
   int srow, scol;
   bool negtgt;
   bool negsrc;
   int it;
   *off = false;
   

   // find a list of dependent cells
   SequenceLayer *lay = mSeq->getAudSeqData()->getLayer(mLayer);
   it = -1;
   while (lay->getChainSource(row, col, &it, &srow, &scol, &negtgt, &negsrc, pat)) {
      // did it play (or not play)?
      bool played = mDependencySource[scol].test(srow);
      if ((played && !negsrc) || (!played && negsrc)) {
         // yes, source triggered. Do we need to force it not to play?
         if (negtgt)
            *off = true;
         // we still need to loop thru all, because we might have more than one, and if a later
         // one is negative, it overrides a non-negative one we might have had earlier
         ret = true;
      }       
   }
   return ret;
}

void 
StochaEngine::quiesceMidi(bool moveNoteOffs)
{
   int i;
   StochaEvent *cur = nullptr;
   for (i = 0; i < SEQ_MAX_MIDI_EVENTS; i++) {
      cur = &mEvents[i];
      if (cur->mNumSamples != -1) { // is in use?
         if (cur->mVelo > 0) { // is note on? delete all note on events that exist
            // we will really have a note off unless you are an idiot
            jassert(cur->mCorrespondingNoteOff != nullptr);
            cur->mCorrespondingNoteOff->clear(); // remove it's corresponding note off event
            mNumActiveNoteOffEvents--;
            cur->clear(); // and remove itself
         }
      }
   }
   mNumActiveNoteOnEvents=0;

   if (moveNoteOffs) { // find any remaining note off events and move them forward
      for (i = 0; i < SEQ_MAX_MIDI_EVENTS; i++) {
         cur = &mEvents[i];
         if (cur->mNumSamples != -1) { // is in use?
            jassert(cur->mVelo == 0); // it better be a note off, because we removed the note on's above
            cur->mNumSamples = 0; // move it to occur right away
         }
      }
   }
}

bool StochaEngine::processBlock(double beatPosition,    // which quarter measure we are on
   double sampleRate,      // current sample rate
   int numSamplesInBlock,  // number of samples in the block
   double BPM,             // current bpm
   double bpb             // beats per bar
#ifdef CUBASE_HACKS
   ,double beatPositionStart // if this is non-zero, it indicates that our start
                             //position is after our current position
#endif
   )
{
   bool rc = true;
   jassert(mSeq != nullptr); // make sure initialized
   SequenceLayer *sd = mSeq->getAudSeqData()->getLayer(mLayer);
   int steps_per_measure = mOverrideStepsPerMeasure.get(sd->getStepsPerMeasure());
   // we will effectively divide the clock speed by this amount (ie smaller is slower clock)
   double clock_div = (double)mOverrideSpeed.get(sd->getClockDivider()) / SEQ_CLOCK_DENOM;

   // beatPosition - (could be fractional, indicates where in the track we are in quarters of a measure )
   //   each whole number is a beat (1/4 of a measure)
   //   note that the doc for this ppqPosition seems wrong, it seems to indicate "pulses per quarter note"

   // mPlayStartPosition - normally 0, but might represent a position where PLAY was manually started.
   // We might be positioned before this (eg if the DAW looped back around after play was nmanually started)
   // in which case we need to offset it so we don't get a negative value
   // and also so that we continue to line up when the daw reaches the manual play start position again
   double adjBeatPos;

   if(beatPosition < mPlayStartPosition) {
      // convert play start pos to steps
      double q = (mPlayStartPosition*steps_per_measure)/bpb;
      // next pattern start after that in steps
      double r=((int)(q/steps_per_measure)+1)*steps_per_measure;
      // number of steps before the next pattern start that mPlayStartPosition is
      double bs = r-q;
      // convert back to beats
      double s=(bs * bpb)/steps_per_measure;
      // add that to our beat pos. this is where we are in the pattern's cycle
      // this should always be a positive value
      adjBeatPos = beatPosition+s;
   } else
   {
       // in normal cases we are relative to start position (ie start position is beat 0)
      adjBeatPos = beatPosition - mPlayStartPosition;

   }
   // step position in the track where one step equals some fraction of a measure
   // as determined by steps per measure in the sequence data
   // *note that this may be less than the last one we saw if play position jumped back
   double steppos_in_track = (adjBeatPos / bpb) * steps_per_measure * clock_div;


   
#ifdef CUBASE_HACKS
   // this might be less than the above on cubase due to lead-in.
   // we don't want to insert any midi notes before this position
   double start_steppos_in_track;
   if (beatPositionStart)
      start_steppos_in_track = (beatPositionStart / bpb) * steps_per_measure * clock_div;
   else
      start_steppos_in_track = 0.0;
#endif

   
   // this is only used for informing the user where we are
   // as well as recording of midi notes
   mRealStepPosition = steppos_in_track;
   

   // number of samples in each measure
   double samples_per_measure = (60.0 *  sampleRate) / (BPM / bpb);

   // num samples per sequence step
   double spss = samples_per_measure/steps_per_measure; 
   spss /= clock_div;

   // number of sequence steps per sample
   double steps_per_sample = 1 / spss; 

   // which step we need to look at next (in step count) relative to track
   int next_step_position = 0;

   // If the play position jumped back, or is jumping forward more than one
   // step, we need to make sure we reposition properly
   if ((steppos_in_track < mOldStepPosInTrack) ||
      (mCurrentStepPosition != INVALIDPOS && steppos_in_track >= (double)(mCurrentStepPosition + 1))) {

      // force to look at next upcoming
      mCurrentStepPosition = INVALIDPOS;
      quiesceMidi(false);   // there may be events that were already added for step which
                                        // have not occurred yet. remove them
   }

   mOldStepPosInTrack = steppos_in_track;
   
   // 1. determine the next step position that we need to deal with (mCurrentStepPosition is the last one we played)
   if (mCurrentStepPosition == INVALIDPOS) {
       // if we are just starting playback (or have looped around)
       if (steppos_in_track < 0) {
           // in cubase (and maybe others), the step position can be negative when playing from 0 position
           next_step_position = 0;
       } else {
           // Since we looped or are starting playback:
           // If we are less than half-way between one step and another step, we will play the first step
           // (eg for step 0, if we are at 0.4 we will still play step 0 even though it's actually in the past).
           // this is to solve the issue of looping when a sample block does not start at the beginning of the loop.
           // if this becomes problematic for people (cant see how it would) then we could always round down at eg 0.1
           // and round up otherwise. i'm just trying to handle the case of eg combination of a very large block
           // size and a very high tempo
           next_step_position = roundToInt(steppos_in_track);
       }
   }
   else {
      // we played one already, so we want the next one
      next_step_position = mCurrentStepPosition + 1;
   }

   // 2. determine the step position (frac) of either the halfway point before that position, or if we
   //    are past the halfway point to that position, then our position is the current step (frac) position
   // NOTE this is also calculated down below the same way
   double next_half_step_pos = (double)next_step_position - 0.5;
   if (steppos_in_track > next_half_step_pos)
      next_half_step_pos = steppos_in_track;

   // 3. is the determined position occurring in our current block?

   // last position that would occur in the current block
   double last_step_pos_in_block = steppos_in_track + (numSamplesInBlock *steps_per_sample);
   while (next_half_step_pos < last_step_pos_in_block) {
      // yes, it would occur in this block
      // 4. if so, determine how many samples are occurring between now and the next _step_ position
      int samples_until = (int)(((double)next_step_position - steppos_in_track)*spss);

      if(samples_until < 0)
          samples_until=0; // just play it now (this would happen if we rounded down above)

      // 6. insert event with that many samples and that step position
#ifdef CUBASE_HACKS
      // for cubase, start_steppos_in_track might be after our REAL start
      if (!start_steppos_in_track || start_steppos_in_track <= next_step_position) {         
#endif
         if (!playPositionChange((int)spss, next_step_position, samples_until))
            rc = false;
#ifdef CUBASE_HACKS
      }
#endif
      // 7. update mCurrentStepPosition to say that we are done dealing with that position
      mCurrentStepPosition = next_step_position;
      // see if there are more in the block
      next_step_position++;
      // NOTE same calc as above
      next_half_step_pos = (double)next_step_position - 0.5;

   }
   // 5. if not, we are done
   return rc;
}

void StochaEngine::playbackStopped()
{
   mCurrentStepPosition = INVALIDPOS;
   mRealStepPosition = INVALIDREALPOS;
   mPlayStartPosition = 0.0;
   // reset our midi controller values that may have been set from incoming midi
   // or from vst automation
   resetMidiControl();
   quiesceMidi();

   // clear out played note tracker
   for (int i = 0; i < SEQ_MAX_STEPS; i++)
      mDependencySource[i].reset();

   
}

void StochaEngine::setPlaybackStartPosition(double pos)
{
   mPlayStartPosition = pos;
}

bool
StochaEngine::getMidiEvent(int numSamplesInBlock, int * pos, int8_t * note, int8_t * velo, int8_t *chan)
{
   StochaEvent *cur = nullptr;
   bool rc = true;

   // first see if there are any note-off events that need to come out
   // we need to put these out before any note on events in case we get a note-on and note-off at same sample
   // position. the note off needs to come out first
   if (mNumActiveNoteOffEvents) {
      for (int i = 0; i < SEQ_MAX_MIDI_EVENTS; i++) {
         cur = &mEvents[i];
         if (cur->mVelo == 0 && cur->mNumSamples != -1) { // it's occupied
            if (cur->mNumSamples < numSamplesInBlock) { // it needs to be sent out with this block
               *pos = cur->mNumSamples;
               *note = cur->mNote;
               *velo = cur->mVelo;
               *chan = cur->mChan;
               cur->clear(); // make the slot available 
               mNumActiveNoteOffEvents--;
               goto end;
            }
         }
      }
   }

   // now output the rest of them (note on's)
   if (mNumActiveNoteOnEvents) {
      for (int i = 0; i < SEQ_MAX_MIDI_EVENTS; i++) {
         cur = &mEvents[i];
         if (cur->mNumSamples != -1) { // it's occupied
             // regarding the -1 here: this is to try to resolve the issue of looped playback
             // when it loops back around and we get an extra note. I'm guessing that its because we
             // are putting out a note in a block prior to where it actually should be put out.
             // This may be due to rounding(?)
             // note that this issue only crops up with certain block sizes and tempos obviously because
             // it happens (i think) when the event is at the very end of the block and should actually
             // be fired in the following block.
            if (cur->mNumSamples < numSamplesInBlock-1) { // it needs to be sent out with this block
               *pos = cur->mNumSamples;
               *note = cur->mNote;
               *velo = cur->mVelo;
               *chan = cur->mChan;
               cur->clear(); // make the slot available 
               mNumActiveNoteOnEvents--;
               goto end;
            }
         }
      }
   }
   rc = false; // no events are ready
end:
   return rc;
}


void
StochaEngine::doneBlock(int numSamplesInBlock)
{
   int x;
   for (int i = 0; i < SEQ_MAX_MIDI_EVENTS; i++) {
      x = mEvents[i].mNumSamples;
      if (x != -1) { // if active
                     // advance it forward
         x -= numSamplesInBlock;
         if (x < 0)
            x = 0;
         mEvents[i].mNumSamples = x;
      }
   }

}

bool
StochaEngine::incomingMidiData(int type,     // note on, off or cc
                               int8_t number,  // note number or cc number                               
                               int8_t chan,    // channel
                               int8_t val)     // if cc number will be cc val
{
   bool ret = false;
   if (!mMappingIsValid)
      rebuildMappingSchema();
   MidiMappingItem *mi;
   if (mMapping[number]) {
      mi = mMapping[number];
      while (mi) {
         if (mi->mChannel == chan && mi->mType==type) {
            int myVal;
            if (mi->mValue == SEQMIDI_VALUE_VARIABLE)
               myVal = val;
            else
               myVal = mi->mValue;

            performMidiMapAction(mi->mAction, myVal);
            ret = true;
         }
         mi = mi->mNext;
      }
   }

   return ret;
}

void StochaEngine::resetMappingSchema()
{
   if (mMappingIsValid) {
      // clear current
      for (int i = 0; i < 127; i++) {
         MidiMappingItem *tmp;
         MidiMappingItem *it = mMapping[i];
         while (it) {
            tmp = it;
            it = it->mNext;
            delete tmp;
         }
         mMapping[i] = nullptr;
      }
   }
   mMappingIsValid = false;
}

void StochaEngine::resetMidiControl()
{
   mOverridePattern.clear();
   mOverrideMute.clear();
   mOverrideSpeed.clear();
   mOverrideTranspose.clear();
   mOverrideNumSteps.clear();
   mOverridePolyBias.clear();
   mOverrideStepsPerMeasure.clear();
   mOverrideDutyCycle.clear();
   mOverridePosVariance.clear();
   mOverrideVeloVariance.clear();
   mOverrideLengthVariance.clear();
   mOverrideOutputChannel.clear();
   mOverrideMaxPoly.clear();
   mOverrideSwing.clear();
}

bool StochaEngine::getStepPlayedState(int position, int notenum)
{
   jassert(position >= 0 && position < SEQ_MAX_STEPS);
   jassert(notenum >= 0 && notenum < SEQ_MAX_ROWS);
   // might not be thread safe:
   return mDependencySource[position].test(notenum);   
}

