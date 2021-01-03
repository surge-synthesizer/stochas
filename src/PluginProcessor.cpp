/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Persist.h"

void SeqAudioProcessor::addAutParam(SeqAudioProcessorParameter * p)
{
   addParameter(p);
   mAutomationParameters.add(p);
}

SeqAudioProcessor::SeqAudioProcessor()  :
#ifndef JucePlugin_PreferredChannelConfigurations
   AudioProcessor(BusesProperties().withOutput("Output", AudioChannelSet::stereo(),true)),
#endif
   mPlaying(false), mEditorState(0), mPPQAdjust(0.0f), 
   mNotifier(this),mRecordingMode(false),
   mMPBrequestArmed(false), mMPBstate(MPBstopped), mMPBStartPosition(0.0)
{
   //_CrtSetBreakAlloc(18151);

   // are we in standalone mode?
   // if so, determine our reference point, etc
   if(wrapperType == wrapperType_Standalone) {
      mStandaloneTempo=(double)SEQ_DEFAULT_STANDALONE_BPM;
      mStandaloneStartTime=Time::getMillisecondCounterHiRes();
      // default play mode is set to manual so that play button shows
      SequenceData *d=mData.getUISeqData();
      d->setAutoPlayMode(SEQ_PLAYMODE_INSTANT);
      mData.swap();
   }

   memset(mMiniMidiMap, 0, sizeof(MiniMidiMapItem*) * 128);

   // init all the layers
   for (int i = 0; i < SEQ_MAX_LAYERS; i++) {
      mStocha[i].init(&mData, i);
   }

   // audioprocessor becomes the owner of these and will delete them
   for (int i = 0; i < SEQ_MAX_LAYERS; i++) {
      // 0-based!
      addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_CLOCKDIV, 0, SEQ_NUM_CLOCK_DIVS-1, i,"speed"));
      addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_NUMSTEPS, SEQ_MIN_STEPS, SEQ_MAX_STEPS, i,"steps"));
      addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_STEPS_PER_MEASURE, SEQ_MIN_STEPS, SEQ_DEFAULT_NUM_STEPS, i,"steps/measure"));
      addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_NOTE_LENGTH, SEQ_DUTY_MIN, SEQ_DUTY_MAX, i,"note length"));
      addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_POS_VARIANCE, 0, SEQ_MAX_HUMAN_POSITION, i,"pos variance"));
      addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_VELO_VARIANCE, 0, SEQ_MAX_HUMAN_VELOCITY, i,"velo variance"));
      addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_LENGTH_VARIANCE, 0, SEQ_MAX_HUMAN_LENGTH, i,"len variance"));
      addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_MUTED, 0, 1, i,"muted"));
      addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_OUTPUT_CHANNEL, 1, 16, i,"midi channel"));
      addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_MAX_POLY, 1, SEQ_MAX_ROWS, i,"max poly"));
      addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_POLY_BIAS, SEQ_POLY_BIAS_MIN, SEQ_POLY_BIAS_MAX, i,"poly bias"));
      addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_TRANSPOSE, SEQMIDI_VALUE_TRANSPOSE_MIN, SEQMIDI_VALUE_TRANSPOSE_MAX_AUT, i, "transpose"));
      // 0-based!
      addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_CURRENT_PATTERN, 0, SEQ_MAX_PATTERNS-1, i,"pattern"));
   }
   addAutParam(new SeqAudioProcessorParameter(this, SEQ_AUT_GLOBAL_SWING, SEQ_MIN_SWING, SEQ_MAX_SWING, -1,"swing"));
   // special case one handled internally
   addParameter(new SeqPlaybackParameter(this));

   // this dummy parameter just used to tell the host that we need a "save" when the user
   // modifies some parameter
   addParameter(mDummyParam=new AudioParameterFloat("_rsvd","reserved",NormalisableRange<float>(0.0f, 1.0f),0.0f));

   // note that this is accessed from the UI thread as well!!
   // avoid non-atomic operations
   mEditorState = new EditorState();
}

SeqAudioProcessor::~SeqAudioProcessor()
{

   resetMiniMidiMap();

   if (mEditorState)
      delete mEditorState;
}

bool SeqAudioProcessor::getStepPlayedState(int layer, int position, int notenum)
{
   jassert(layer >= 0 && layer < SEQ_MAX_LAYERS);
   return mStocha[layer].getStepPlayedState(position,notenum);
}

const String SeqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SeqAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SeqAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SeqAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
   return true;
   #else
   return false;
#endif
}

double SeqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SeqAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SeqAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SeqAudioProcessor::setCurrentProgram (int )
{
}

const String SeqAudioProcessor::getProgramName (int )
{
    return String();
}

void SeqAudioProcessor::changeProgramName (int , const String& )
{
}

void SeqAudioProcessor::prepareToPlay (double , int )
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SeqAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SeqAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
 #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SeqAudioProcessor::resetMiniMidiMap()
{
   for (int i = 0; i < 127; i++) {
      MiniMidiMapItem *tmp;
      MiniMidiMapItem *it = mMiniMidiMap[i];
      while (it) {
         tmp = it;
         it = it->mNext;
         delete tmp;
      }
      mMiniMidiMap[i] = nullptr;
   }
}

// check the midi mapping. if any mapping for playback start/stop or record start/stop, grab it
void SeqAudioProcessor::rebuildMiniMidiMap()
{
   // this rebuilds our linked list hash mechanism to optimize for
   // incoming data
   SequenceData *sd = mData.getAudSeqData();
   int len = sd->getMappingCount();
   int i;
   MiniMidiMapItem *cur;

   // free old
   resetMiniMidiMap();
   // build new
   for (i = 0; i < len; i++) {
      SeqMidiMapItem *mm = sd->getMappingItem(i);
      if ((mm->mAction == SEQMIDI_ACTION_PLAYBACK || mm->mAction == SEQMIDI_ACTION_RECORD) && mm->mValue != 0 &&
         mm->mChannel != 0 && mm->mNote >= 0) {

         if (!mMiniMidiMap[mm->mNote]) {
            cur = mMiniMidiMap[mm->mNote] = new MiniMidiMapItem();
         }
         else {
            cur = mMiniMidiMap[mm->mNote];
            while (cur->mNext) {
               cur = cur->mNext;
            }
            cur->mNext = new MiniMidiMapItem();
            cur = cur->mNext;
         }

         cur->mChannel = mm->mChannel;
         cur->mValue = mm->mValue; // start/stop/toggle
         cur->mType = mm->mType;
      }
   }   
}

// request to either start or stop playback
// send notification if applicable
void SeqAudioProcessor::requestManualPlayback(bool start)
{
   
   if (start) {
      if (mMPBstate == MPBstopped) {
         mMPBstate = MPBrequested;
         mNotifier.setPlaybackState(SeqProcessorNotifier::standby);         
      }
   }
   else { // stop
      if (mMPBstate != MPBstopped) {
         mMPBstate = MPBstopped;
         mNotifier.setPlaybackState(SeqProcessorNotifier::off);
      }
   }
}

// the only midi messages we handle here right now are ones related to manual playback
// and recording which can't be handled by stocha engines as they assume they are already 
// playing, and don't know anything about recording
void
SeqAudioProcessor::handleMiniMidiMap(int type, char number, char chan, char)
{
   MiniMidiMapItem *mi;
   // this is an array sized to number of midi notes
   jassert(number >= 0);

   SequenceData *seq = mData.getAudSeqData();

   mi = mMiniMidiMap[number];
   while (mi) {
      if (mi->mChannel == chan && mi->mType == type) {
         switch (mi->mValue) {
         case SEQMIDI_VALUE_PLAYBACK_START:            
            // only react if we are not in auto play mode
            if(seq->getAutoPlayMode() != SEQ_PLAYMODE_AUTO) {
               requestManualPlayback(true);
            }
            break;
         case SEQMIDI_VALUE_PLAYBACK_STOP:
            // only react if we are not in auto play mode
            if(seq->getAutoPlayMode() != SEQ_PLAYMODE_AUTO) {
               requestManualPlayback(false);
            }
            break;
         case SEQMIDI_VALUE_PLAYBACK_TOGGLE:
            // only react if we are not in auto play mode
            if(seq->getAutoPlayMode() != SEQ_PLAYMODE_AUTO) {
               if (mMPBstate == MPBstopped)
                  requestManualPlayback(true);
               else // started or standby
                  requestManualPlayback(false);
            }
            break;
         case SEQMIDI_VALUE_RECORD_START:
            if(!mRecordingMode) {
               mRecordingMode=true;
               recordingModeChanged();
            }
            break;
         case SEQMIDI_VALUE_RECORD_STOP:
            if(mRecordingMode) {
               mRecordingMode=false;
               recordingModeChanged();
            }            
            break;
         case SEQMIDI_VALUE_RECORD_TOGGLE:
            mRecordingMode=!mRecordingMode;
            recordingModeChanged();
            break;
         default:
            jassertfalse;            
         }
      }
      mi = mi->mNext;
   }
   
}

// this handles incoming midi messages and outputs messages that need to be
// passed on if applicable.
void SeqAudioProcessor::handleIncomingMidi(bool currentlyPlaying,
   bool startingPlayback, MidiBuffer & midiMessages, 
   MidiBuffer &processedMidi, MidiBuffer &recordedNotes)
{
   if (midiMessages.isEmpty())
      return;
   else {
      SequenceData *sd = mData.getAudSeqData();
      int respond = sd->getMidiRespond();
      int passthru = sd->getMidiPassthru();

      for (const MidiMessageMetadata it : midiMessages) {
         MidiMessage msg = it.getMessage();
         int position = it.samplePosition;
         bool remove = (passthru == SEQ_MIDI_PASSTHRU_NONE);
         char midiNumber = 0; // note number or cc number
         char midiChan = 0;
         char midiType = 0; // noteon or cc
         char midiVal = 0; // value for cc

         if (msg.isNoteOn()) {
            midiType = SEQ_MIDI_NOTEON;
            midiNumber = (char)msg.getNoteNumber();
         }
         else if (msg.isNoteOff()) {
            midiType = SEQ_MIDI_NOTEOFF;
            midiNumber = (char)msg.getNoteNumber();
         }
         else if (msg.isController()) {
            midiType = SEQ_MIDI_CC;
            midiNumber = (char)msg.getControllerNumber();
            midiVal = (char)msg.getControllerValue();
         }
         else
            midiType = SEQ_MIDI_OTHER;
         
         midiChan = (char)msg.getChannel();

         // let the UI know about the event
         mNotifier.setMidiEventOccurred(midiType, midiChan, midiNumber, midiVal);
         
         if (currentlyPlaying) { // currently playing recording or looping
               // if we are playing, we can handle incoming midi data
            bool handled = false;

            // we just started playing, so clear out all our mapping
            // schemas so they are rebuilt the first time incomingMidiData is called
            if (startingPlayback) {
               for (int i = 0; i < SEQ_MAX_LAYERS; i++)
                  mStocha[i].resetMappingSchema();
            }

            // if we are responding to incoming messages, then do so
            if (respond == SEQ_MIDI_RESPOND_YES) {
               bool h;
               // see if it's handled by our midi map
               // if so, set data in stochaEngine
               // also notify ui in case it needs repaint (due to eg pattern change)
               for (int i = 0; i < SEQ_MAX_LAYERS; i++) {
                  h = mStocha[i].incomingMidiData(midiType, midiNumber, midiChan, midiVal);
                  if (h)
                     handled = true;
               }
            }

            // if recording (which doesn't care whether we are responding or not, if the user hits
            // record, it's assumed they want to respond to midi) then build up our buffer with
            // those notes
            if (mRecordingMode && !handled && (msg.isNoteOn() || msg.isNoteOff())) {
               recordedNotes.addEvent(msg, position);
            }

            // if we handled it, and in this mode, we remove it
            if (passthru == SEQ_MIDI_PASSTHRU_UNHANDLED && handled) {
               // we need to remove this message from the queueue
               remove = true;
            }
         } // if playing
         

         // add the event to our output buffer
         if (!remove) {
            processedMidi.addEvent(msg, position);
         }
      } // while get event
   } // if incoming midi

}

// we need to check to see if there are midi messages that cause play/record start/stop
// so we can react to them before we determine playback state
void 
SeqAudioProcessor::checkIncomingMidiForStartStop(MidiBuffer &midiMessages)
{
   if (midiMessages.isEmpty()) // nothing to do
      return;

   // scan thru the buffer and react to note on/off
   for (const MidiMessageMetadata it : midiMessages) {
      MidiMessage msg = it.getMessage();
     
      char midiNumber = 0; // note number or cc number
      char midiChan = 0;
      char midiType = 0; // noteon or cc
      char midiVal = 0; // value for cc

      if (msg.isNoteOn()) {
         midiType = SEQ_MIDI_NOTEON;
         midiNumber = (char)msg.getNoteNumber();
      }
      else if (msg.isNoteOff()) {
         midiType = SEQ_MIDI_NOTEOFF;
         midiNumber = (char)msg.getNoteNumber();
      }
      else
         continue; // don't care about it

      midiChan = (char)msg.getChannel();
      handleMiniMidiMap(midiType, midiNumber, midiChan, midiVal);
   } // while get event

}

/* Send completed midi notes (have on and off) to the UI so that it 
   can record them
*/
void SeqAudioProcessor::dispatchRecordedMidiNotes(MidiBuffer &midiNoteData)
{
   
   int curLyr = mEditorState->getCurrentLayer(); // needed so we can access the right mStocha
   int frac;
   int noteNum;
   // number of steps in current layer
   int numSteps = mStocha[curLyr].getNumSteps();
   // current position in current layer
   int stepPos = mStocha[curLyr].getCurrentOverallPosition(&frac);

   for (const MidiMessageMetadata it : midiNoteData) {
      MidiMessage msg = it.getMessage();

      // we will only have note on or note off data   
      noteNum = msg.getNoteNumber();

      if (msg.isNoteOn()) { // start rec
         
         // indicate that we have a start note at the correct time
         mMidiRecord[noteNum].vel = msg.getVelocity();
         mMidiRecord[noteNum].stepPos = stepPos; 
         mMidiRecord[noteNum].stepPosFrac = (char)frac;
         if (frac > 49) // range is 0-99
            mMidiRecord[noteNum].stepPos++; // quantize to next position

         // wrap around possibly
         //if (stepPos >= numSteps)
         //   mMidiRecord[noteNum].stepPos -= numSteps;
      }
      else if (msg.isNoteOff()) { // stop rec and update sequence data         
         int gridPos; // position in grid
         int length;

         // if we have an ON note, we now have the OFF note, so send it
         if (mMidiRecord[noteNum].vel) {

            gridPos = mMidiRecord[noteNum].stepPos % numSteps;
            // calculate total length (stepPos will always be > what we stored as it's position in daw)
            // (should always result in a length of at least 1)
            length = stepPos - mMidiRecord[noteNum].stepPos + 1;

            // determine whether the two fractions on either end would cause the length to quantize upward
            // or downward
            if ((99 - mMidiRecord[noteNum].stepPosFrac) + frac < 50)
               length--;

            // if we exceeded width, set it to max
            if (length > numSteps)
               length = numSteps;

            // length is 0 based in SequenceData, so keep with this tradition
            length--;

            // finally, if any of the above cause length to go below 0, we will set it to 0 ensuring
            // we actually have a step that gets placed
            if (length < 0)
               length = 0;

            mNotifier.addCompletedMidiNote(noteNum, mMidiRecord[noteNum].vel, length, gridPos);
            mMidiRecord[noteNum].vel = 0; // clear it for next time            
         }
      }
   } // while get event   
}


void SeqAudioProcessor::checkforUIIncomingData(MidiBuffer & processedMidi)
{
   SequenceData *sd = mData.getAudSeqData();
   int fifoData1, fifoData2, fifoData3;
   /*data is received from the UI thread. could be a few things:
    *  is there a manual audition to play?
    *      (see NoteCpt mousedown on the play button)
    *      1 - SEQ_MIDI_NOTEON, or SEQ_MIDI_NOTEOFF
    *      2 - layer number
    *      3 - note index
    *  new mapping has been created
    *      1 - SEQ_REFRESH_MAP_MSG
    *      2 and 3 are meaningless
    *  in standalone mode, tempo has been changed by the user
    *      1 - SEQ_STANDALONE_SET_TEMPO
    */

   while (mIncomingData.readFromFifo(&fifoData1, &fifoData2, &fifoData3)) {
      switch (fifoData1) {
      case SEQ_MIDI_NOTEON:
      case SEQ_MIDI_NOTEOFF: {
         SequenceLayer *dl = sd->getLayer(fifoData2);
         char chan = dl->getMidiChannel();
         char note = dl->getCurNote(fifoData3);
         MidiMessage m;
         // note that this currently ignores automation override of midi channel (which I think is fine)
         if (fifoData1 == SEQ_MIDI_NOTEOFF) { // note off
            m = MidiMessage::noteOff(chan, (int)note, (uint8)0);
            processedMidi.addEvent(m, 0);
         }
         else if (fifoData1 == SEQ_MIDI_NOTEON) { // note on         
            m = MidiMessage::noteOn(chan, (int)note, (uint8)127);
            processedMidi.addEvent(m, 0);
         }
         break;
      }
      case SEQ_REFRESH_MAP_MSG:
         // ui has changed the midi mapping, so we need to tell our stocha's to rebuild their schema
         for (int i = 0; i < SEQ_MAX_LAYERS; i++) {
            mStocha[i].resetMappingSchema();
         }

         // also, we fix our mini map (for manual playback start/stop)
         rebuildMiniMidiMap();         
         break;
      case SEQ_NOTIFY_HOST: {
         // just effect a change so the host thinks somethings changed.
         float f = mDummyParam->get();
         f = f == 0 ? .5f : 0.0f;
         mDummyParam->setValueNotifyingHost(f);
         break;
      }
      case SEQ_SET_RECORD_MODE:
         // ui thread can turn on/off record mode with a toggle
         mRecordingMode = !mRecordingMode;
         recordingModeChanged();
         break;

      case SEQ_SET_PLAY_START_STOP:
         // manual playback start/stop
         // this is only used when manual playback is on (otherwise daw controls playback)
         if (sd->getAutoPlayMode() != SEQ_PLAYMODE_AUTO) {
            if (mMPBstate == MPBstopped)
               requestManualPlayback(true);            
            else // started or requested
               requestManualPlayback(false);            
         } // if playmode is not auto
         break;
      
      case SEQ_STANDALONE_SET_TEMPO:
         // tempo changed in standalone mode by user
         changeStandaloneTempo();
         break;
      default:
         jassertfalse;
      }
   }
}

// determine whether manual playback is started
// based on quantize, etc
// This will only be called if autoplay is off (manual play is on)
inline bool
SeqAudioProcessor::determinePlaybackState(int apm, bool playingInDaw,
                                          double beatsperbar, 
                                          double samplerate, double bpm,
                                          double beatPosition, 
                                          int samplesperblock)
{
   bool areWePlaying = false;
      // autoplay is off, determine playing mode by whether user hit play btn, etc
   switch (mMPBstate) {
   case MPBrequested: // user requested start
      if (playingInDaw) { // only honor this request if we are playing in the daw 

         // signify that we were in request mode while playing in the daw at some point
         if (!mMPBrequestArmed)
            mMPBrequestArmed = true;

         // quantization division could be measure, beat or step (quarter beat)
         // determine the division boundary multiplier
         if (apm != SEQ_PLAYMODE_INSTANT) {
            double multiplier=0;
            // if we are right after a division then allow start
            double fudge = 0;
            switch (apm) {
            case SEQ_PLAYMODE_STEP: // right now this is in daw 16th notes always. (stocha engines may have different step sizes)
               multiplier = 4; // 4 16th notes per beat
               fudge = 1.0 / 4.0; // up to 1/4 of a step after division
               break;
            case SEQ_PLAYMODE_BEAT:
               multiplier = 1; // 1 beat per beat
               fudge = 1.0 / 16.0; // up to 1/16 of a beat after division
               break;
            case SEQ_PLAYMODE_MEASURE:
               multiplier = 1 / beatsperbar; // 1/4 (or whatever) measures per beat
               fudge = 1.0 / 64.0; // up to 1/64th of a measure after division
               break;
            default:
               jassertfalse;
            }
            
            // determine whether the start of our quantization division occurs in this block
            // broken out so that everything is clear

            // samples per division
            double samples_per_div = (60.0 *  samplerate) / (bpm * multiplier);
            // current division position in track (measured in divisions)
            double divpos = beatPosition * multiplier;
            int roundedDiv = (int)divpos; // in divisions
            int nextDiv = roundedDiv + 1; // in divisions
            // division position in samples
            double divposInSamples = divpos * samples_per_div;
            // next div pos in samples
            double nextdivposInSamples = nextDiv * samples_per_div;

            // if we add blocksize to our current position, would we be on the next division?
            // (ie we are before the next division but close enough that the next block would
            // be on it)
            if (divposInSamples + samplesperblock >= nextdivposInSamples) {
               mMPBstate = MPBstarted;
               // round this to the next division
               mMPBStartPosition = beatPosition + (((double)nextDiv - divpos) / multiplier);               
            }
            // otherwise, check to see if we are right near the boundary
            else if (divpos < ((double)roundedDiv)+fudge){

               // this should handle the case where our position is very soon after start of bar
               mMPBstate = MPBstarted;
               // round this to the division boundary. if there are any stocha notes at this position
               // they will be skipped, as they occurred in the past
               mMPBStartPosition = (double)roundedDiv / multiplier;
            }
         }
         else {// apm = SEQ_PLAYMODE_INSTANT
            mMPBstate = MPBstarted;
            mMPBStartPosition = beatPosition;
         }

         // if we are started, send notification
         if (mMPBstate == MPBstarted) {
            mNotifier.setPlaybackState(SeqProcessorNotifier::on);
            areWePlaying = true;            
         }
      } // if playing in daw
      else {
         // at some point in time were we armed while daw was playing? if so cancel that
         if (mMPBrequestArmed) {
            mMPBrequestArmed = false;
            mNotifier.setPlaybackState(SeqProcessorNotifier::off);
            mMPBstate = MPBstopped;
            mMPBStartPosition = 0.0;
         }
      }
            
      
      
      break;
   case MPBstarted: // already started
      if (!playingInDaw) {// but we are not playing anymore in the daw
         mNotifier.setPlaybackState(SeqProcessorNotifier::off);
         mMPBstate = MPBstopped;
         mMPBStartPosition = 0.0;
      }
      else {
         // already playing, continue to do so
         areWePlaying = true;
      }
      break;
   case MPBstopped: // nothing doing
      break;
   default:
      jassertfalse;
   }
   
   return areWePlaying;
}

// For standalone mode, playback is considered to have started as soon as the app loads
// (this refers to 'daw' playback. in standalone mode we default to playback type 'instant'
// which means that stochas only starts playing when play button is hit),
// so we just need to calculate our beat position based on that
double SeqAudioProcessor::getStandaloneBeatPosition()
{
   double currentTime=Time::getMillisecondCounterHiRes();
   
   // elapsed m/s since start
   double elapsed=currentTime-mStandaloneStartTime;

   // tempo in beats per ms
   double bpms=mStandaloneTempo / (60*1000);

   // how many beats have elapsed since play start
   return elapsed*bpms;
}

// called when the recording mode has toggled, either via the ui or via
// midi mapping
void SeqAudioProcessor::recordingModeChanged()
{
   if (!mRecordingMode) // if turning off, notify. 
      mNotifier.setRecordingState(SeqProcessorNotifier::off);
   else { // recording is turning on
      if (!mPlaying) // if toggling it on and we are not playing, 
         mNotifier.setRecordingState(SeqProcessorNotifier::standby);
      else {
         mNotifier.setRecordingState(SeqProcessorNotifier::on);
         // get everything ready for recording
         mNotifier.clearCompletedMidiNotes();
         // and clear out the collector buffer
         memset(mMidiRecord, 0, sizeof(mMidiRecord));
      }
   }
}

// fill in position info in standalone mode.
// we will need all the same fields that we use in processBlock.
void SeqAudioProcessor::positionInfoStandalone(AudioPlayHead::CurrentPositionInfo *posinfo)
{
   memset(posinfo, 0, sizeof(AudioPlayHead::CurrentPositionInfo));
   posinfo->bpm=mStandaloneTempo;
   posinfo->timeSigNumerator=4;
   posinfo->timeSigDenominator=4;
   posinfo->isPlaying=true;
   posinfo->ppqPosition = getStandaloneBeatPosition();

}

// set new tempo for standalone.
// we need to reset the start time so that there is not a big
// jump in position if the tempo change is gradual.
// To do this, just calculate when the start time would have been
// if the new tempo were in effect, and set it to that
void SeqAudioProcessor::changeStandaloneTempo() {

   double currentTime=Time::getMillisecondCounterHiRes();
   SequenceData *seq = mData.getAudSeqData();
   double newTempo=seq->getStandaloneBPM();
   double beatPos=getStandaloneBeatPosition();

   // new tempo in ms per beat
   double new_mspb = (60*1000)/newTempo;

   // set the start time to reflect new tempo
   mStandaloneStartTime = currentTime - (new_mspb * beatPos);

   // set the new tempo from our sequence data
   mStandaloneTempo = newTempo;
}

void SeqAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
   
   MidiBuffer processedMidi; // buffer that holds output midi (initially empty)
   MidiBuffer recordedNotes; // buffer that holds recorded notes (if recording)
   AudioPlayHead *ph = getPlayHead();
   AudioPlayHead::CurrentPositionInfo posinfo;
   bool stoppingPlayback = false;
   bool startingPlayback = false;
   bool areWePlaying = false;
   int samplesperblock = 0; // number of samples per block
   double samplerate = getSampleRate();
   double beatsperbar;
   int i = 0;
   // this is used to determine whether to signal a play position change.
   int oldpos[SEQ_MAX_LAYERS];
   SequenceData *seq = mData.getAudSeqData();

   // clear all buffers. We don't produce audio nor do we filter incoming audio.
   buffer.clear();

   // retrieve some important info (ppqposition might be negative <ahem>cubase)
   if(wrapperType == wrapperType_Standalone) { 
      // in standalone mode, fake it out
      positionInfoStandalone(&posinfo);
   } else {
      ph->getCurrentPosition(posinfo);
   }

   // BITWIG FIX
   // I'm not entirely happy with this fix as it seems there must be a better way...
   // In Bitwig it's possible for the ppqPosition to be negative if
   // the pre-roll is turned on. This will bail if ppqposition is negative
   // and is not going to be positive in this block. If it is going to be
   // positive in this block, then just set it to 0 so it's not negative
   // (otherwise we possibly miss notes at the first position in the grid).
   if(posinfo.ppqPosition < 0 ) {
      // samples per beat
      double s = (60.0 *  getSampleRate()) / (posinfo.bpm);
      // ppq pos is in beats
      if ((posinfo.ppqPosition*s)+ buffer.getNumSamples() >=0)      
         posinfo.ppqPosition = 0;
      else
         return;
   }
   // END BITWIG FIX

   // position adjustment (which is a problem with protools and nothing else)
   // getPPQOffset should be an atomic operation.
   mPPQAdjust = (float)mEditorState->getPPQOffset() / 1000.0f;      
   if (mPPQAdjust)
      posinfo.ppqPosition += mPPQAdjust;

   beatsperbar = (double)posinfo.timeSigNumerator*4.0 / (double)posinfo.timeSigDenominator;
   
   // samplesblock is how many samples will come before the next processBlock call   
   samplesperblock = buffer.getNumSamples();

   // keep track of current positions so we can determine whether they are changing later
   for (i = 0; i<SEQ_MAX_LAYERS; i++)
      oldpos[i] = mStocha[i].getCurrentStepPosition();

#ifdef CUBASE_HACKS
   if (!mHostType.isCubase()) // just to be safe. Want to limit the scope of this
      mCubaseAtRestPos = 0.0;
#endif

   // check incoming midi to see if manual play start/stop, or a record start/stop is received.
   // needs to be done before determining playback below, so that we can capture
   // a message that might tell us to start playback. Also, for recording, since the engines 
   // don't handle record start/stop, we need to check for those messages as well
   checkIncomingMidiForStartStop(midiMessages);

   // determine whether we are currently playing
   if (seq->getAutoPlayMode() == SEQ_PLAYMODE_AUTO) // if autoplay is on
      // determined by daw alone
      areWePlaying = posinfo.isPlaying;
   else {
      // determined by manual playback, quantize, etc.
      // this will also transition state between standby/play/stop
      // and also send notification and setup mMPBStartPosition
      areWePlaying = determinePlaybackState(seq->getAutoPlayMode(), posinfo.isPlaying,
         beatsperbar, samplerate, posinfo.bpm, posinfo.ppqPosition, samplesperblock);
   }

   // determine whether we are transitioning from playing to not playing
   // or vice versa
   if (!areWePlaying) { // if we are stopped playback

#ifdef CUBASE_HACKS
         if (mHostType.isCubase())
            mCubaseAtRestPos = posinfo.ppqPosition;
#endif
         if (mPlaying) {        // but was playing before this, then
            mPlaying = false;   // we are stopping
            stoppingPlayback = true;

         }
      }
   else { // if we are actually playing


      if (!mPlaying) { // and was not playing before this         
         mPlaying = true;
         startingPlayback = true;
         // give this an initial value
         mLastPosition = posinfo.ppqPosition;

         // set recording state if applicable
         if (mRecordingMode) {
            // notify the ui that we are now recording (since we were in standby before)
            mNotifier.setRecordingState(SeqProcessorNotifier::on);
         }
      }
#ifdef CUBASE_HACKS
      else
      {
         if (mCubaseAtRestPos && posinfo.ppqPosition >= mCubaseAtRestPos)
            mCubaseAtRestPos = 0.0; // once we've past that play position, turn it off
                                    // so that we don't run into issues if looping back
      }
#endif

   } // if posinfo.isPlaying

   // determine whether we need to generate a new random number.
   // if we are first starting to play or if we have looped back,
   // we will need a new one
   if (startingPlayback || (mPlaying && posinfo.ppqPosition < mLastPosition)) {        
      //SequenceData *seq = mData.getAudSeqData();
      int64 seed = seq->getRandomSeed();
      if (seed == 0) {
         // we need to generate a new random seed         
         seed = generateNewRootSeed();         
         // we also need to make the ui aware of the seed in case the
         // user clicks "stable" so that it pulls that new seed to save with
         // the patch
         mNotifier.setRandomSeed(seed);      
      }

      // we use the seed on each of the engines
      // note that these are not seeds, but are
      // some random prime numbers to use as sequence id's.
      // the same seed is used for each engine, but each has it's
      // own sequence id
      for (i = 0; i < SEQ_MAX_LAYERS; i++) {
         static const uint64 seqid[SEQ_MAX_LAYERS] = { 
            999999587
#if(SEQ_MAX_LAYERS > 1)
            ,
            2000037797,
            300045709,
            40044757
#if(SEQ_MAX_LAYERS>4)
#error "Need to add some prime numbers to the array now"
#endif
#endif
         };
         // now set the seed for each of the engines
         mStocha[i].setRandomSeed((uint64)seed, seqid[i]);
      }
   }

   // set this so that if it's checked again we can know whether we've
   // moved back in time and need to generate again
   if(mPlaying)
      mLastPosition = posinfo.ppqPosition;


   // any incoming midi data that needs handling?
   // handle mapped midi, learn midi, midi light
   // pass the data on to processedMidi if we are passing through data
   // pass data to recordedNotes if recording is active

   handleIncomingMidi(mPlaying, startingPlayback, midiMessages, processedMidi,recordedNotes);
   if (stoppingPlayback) {
      // quiesce all playing notes if we are stopping
      for (i = 0; i < SEQ_MAX_LAYERS; i++)
         mStocha[i].playbackStopped();

      // also reset automation values      
      for (i = 0; i < mAutomationParameters.size(); i++)
         mAutomationParameters[i]->reset();

      // also turn off recording mode
      mRecordingMode = false;
      mNotifier.setRecordingState(SeqProcessorNotifier::off);
   }

   // main processing is right here. figure out what notes to play
   // now and in the near future
   if(mPlaying) {
      // this is the main processing to determine what notes to play
      for (i = 0; i < SEQ_MAX_LAYERS; i++) {

         // if we are just starting playback and we are in manual playback mode
         // (ie play starts at 0 position wherever the user hit play btn)
         if (startingPlayback && mMPBstate==MPBstarted) {
            mStocha[i].setPlaybackStartPosition(mMPBStartPosition);
         }
         // todo get retval and blink some overflow light if false
         mStocha[i].processBlock(posinfo.ppqPosition, //which quarter measure we are on
            samplerate,
            samplesperblock,
            posinfo.bpm,
            beatsperbar
#ifdef CUBASE_HACKS
            ,mCubaseAtRestPos
#endif
         );
         
      }

      // now that we have positional info calculated from StochaEngine, we can
      // deal with recorded midi notes (if any. there will only be some if we are
      // recording and we've received note on/off data)
      if(!recordedNotes.isEmpty())
         dispatchRecordedMidiNotes(recordedNotes);

   }

   // now see whether the engine has any midi notes that need playing
   // right now. only while playing or quiescing
   if (mPlaying || stoppingPlayback) { 
      char midi_note = 0, midi_velo = 0, midi_chan = 0;
      int midi_pos = 0;
      for (i = 0; i < SEQ_MAX_LAYERS; i++) {
         while (mStocha[i].getMidiEvent(samplesperblock, &midi_pos, &midi_note, &midi_velo, &midi_chan)) {
            MidiMessage m;
            if (midi_velo) {
               m = MidiMessage::noteOn(midi_chan, (int)midi_note, (uint8)midi_velo);
            }
            else { // note off
               m = MidiMessage::noteOff(midi_chan, (int)midi_note);
            }
            processedMidi.addEvent(m, midi_pos);
         } // get upcoming midi events and add them
      }
   }

   // tell the engine to advance midi in the queue for stuff that does not need to play
   // right away
   if (mPlaying) {
      for (i = 0; i < SEQ_MAX_LAYERS; i++)
         mStocha[i].doneBlock(samplesperblock);
   }

   // see if the UI has any data for us that we need to process
   checkforUIIncomingData(processedMidi);
   
   // flip from incoming midi data to outgoing data
   midiMessages.swapWith(processedMidi);


   // notify the UI of updates that need to be reflected
   for (i = 0; i < SEQ_MAX_LAYERS; i++) {
      int newpos;
      // currently playing step
      newpos = mStocha[i].getCurrentStepPosition();
      if (oldpos[i] != newpos)
         mNotifier.setPlayPosition(i,newpos);

      // currently playing pattern or -1
      mNotifier.setCurrentPattern(i, mPlaying ? mStocha[i].getPlayingPattern() : -1);

      // whether layer is muted or not
      mNotifier.setMuteState(i, mStocha[i].getMuteState());
   }
}


bool SeqAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SeqAudioProcessor::createEditor()
{   
    return new SeqAudioProcessorEditor (*this);
}

// not used anymore as we save to xml now
#if 0
#ifdef JUCE_MAC
static const int macwin = 1;
#elif JUCE_WINDOWS
static const int macwin = 2;
#else
static const int macwin = 0;
#endif
static const int curver = SEQ_SERIALIZE_CURRENT_VERSION;
#endif
/* 
Code to save the plugin state to the DAW project
*/
void
SeqAudioProcessor::getStateInformation (MemoryBlock& block)
{
   SeqPersist persist;
   const XmlElement &xml=persist.store(mData.getAudSeqData());
   //xml.writeToFile(juce::File("C:\\crap\\test.xml"), "");
   copyXmlToBinary(xml, block);

/* This is the code that just saves it as a block of memory
   int seqsize = sizeof(SequenceData);
   int *idata;

   SequenceData *seq = mData.getAudSeqData();

   // write header - version number and platform
   block.setSize((sizeof(int)*2)+seqsize,true);
   idata=(int *)block.getData();   
   *idata = curver;
   idata++;
   *idata = macwin;
   idata++;
   memcpy((void*)idata, seq, seqsize);
  */ 
}

/*
Code to load the plugin state from the daw project
*/
void SeqAudioProcessor::setStateInformation (const void* in , int size)
{
   
   std::unique_ptr<XmlElement> xml=getXmlFromBinary(in, size);
   if (xml) {
      SeqPersist persist;
      // TODO - we are using the UI side since we are setting data. 
      // we need to put a lock here or something
      if (persist.retrieve(mData.getUISeqData(), xml.get()))
         mData.swap();
   }

   // tempo in saved state might differ, we need to recalculate some stuff here
   changeStandaloneTempo();

   // if we happen to be playing back at the time, we need to have the stocha's
   // rebuild their midi mapping schemas
   for (int i = 0; i < SEQ_MAX_LAYERS; i++) {
      mStocha[i].resetMappingSchema();
   }

   // we also need to rebuild our mini midi map
   rebuildMiniMidiMap();

   // notify ui that state info is updated
   mNotifier.uiNeedsUpdate();


   /* This is the code that just saves it as a block of memory.
   int ver, plat;
   SequenceData *seq = mData.getUISeqData();
   const int *pos=(const int*)in;

   // sanity check
   if (size != sizeof(int) + sizeof(int) + sizeof(SequenceData))
      return;

   ver = *(pos);
   pos++;
   plat = *(pos);

   if (ver != curver && plat != macwin)
      return; // can't deserialize right now

   pos++;
   // next block is actual data
   memcpy((void *)seq, (const void *)pos, sizeof(SequenceData));

   // send data to aud side
   mData.swap();
   */
}


AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
   AudioProcessor *ret= new SeqAudioProcessor();
   
   return ret;
}

//==================================SeqAudioProcessorParameter===========================

float SeqAudioProcessorParameter::getValue() const
{
   // host is getting value
   float ret;
   // convert mValue to a 0.0 to 1.0 range
   ret = jmap<float>((float)mValue, (float)mRangeLo - 1.0f, (float)mRangeHi, 0.0f, 1.0f);   
   return ret;
}


void SeqAudioProcessorParameter::setValue(float newValue)
{
   // host is setting value
   mValue = roundToInt(jmap<float>(newValue, (float)mRangeLo - 1.0f, (float)mRangeHi));
   // if mValue is rangeLo-1 it means default
   mNotify->automationParameterHasChanged(mParamId, mValue == mRangeLo - 1 ? 
      SEQ_AUT_DEFAULT_VALUE_DESIG : mValue, mLayerNumber);
}

float SeqAudioProcessorParameter::getDefaultValue() const
{
   // called by host to determine default
   return 0.0f;
}

String SeqAudioProcessorParameter::getName(int maximumStringLength) const
{
   String ret;
   if (mLayerNumber != -1) {
      ret=String::formatted("L%d ", mLayerNumber+1);
   }
   else {
      ret = "";
   }
   ret.append(mName,maximumStringLength - ret.length());
   return ret;
}

String SeqAudioProcessorParameter::getLabel() const
{
   return String(); // not used
}

String SeqAudioProcessorParameter::getText(float value, int) const
{
   int v = roundToInt(jmap<float>(value, (float)mRangeLo - 1.0f, (float)mRangeHi));
   if (v == mRangeLo - 1)
      return "as assigned";
   else
      return mNotify->getTextForAutomationParameterValue(mParamId, v);
}

float SeqAudioProcessorParameter::getValueForText(const String & text) const
{
   int r;
   if (text.compare("as assigned") == 0)
      return 0.0f;

   r=mNotify->parseTextForAutomationParameterValue(mParamId, text);
   if (r == SEQ_AUT_DEFAULT_VALUE_DESIG)
      return 0.0f;
   return jmap<float>((float)r, (float)mRangeLo - 1.0f, (float)mRangeHi, 0.0f, 1.0f);
}

SeqAudioProcessorParameter::SeqAudioProcessorParameter(AutParamNotify * notify, 
   int paramId, int rangeLo, int rangeHi, int layerNum, const String &name) :
   mNotify(notify), mLayerNumber(layerNum), mParamId(paramId), mRangeLo(rangeLo), 
   mRangeHi(rangeHi),mValue(rangeLo-1),mName(name)
{
}

void SeqAudioProcessorParameter::reset()
{
   beginChangeGesture();
   // this should implicitly call setValue with 1.0
   setValueNotifyingHost(getDefaultValue());   
   endChangeGesture();
}

//==========implement AutParamNotify ============================
void SeqAudioProcessor::automationParameterHasChanged(int paramId, int paramValue, int layer)
{
   // talk to StochaEngine to tell it to override some value
   if(layer != -1)
      mStocha[layer].setAutomationParameterValue(paramId, paramValue);
   else {
      // apply it to all layers
      for(int i=0;i<SEQ_MAX_LAYERS;i++)
         mStocha[i].setAutomationParameterValue(paramId, paramValue);
   }
}

String SeqAudioProcessor::getTextForAutomationParameterValue(int paramId, int paramValue)
{
   String ret;
   // return a translation of the param's value to something displayable
   switch (paramId) {
   case SEQ_AUT_CLOCKDIV: // translate
      ret = SeqScale::getClockDividerTextAndId(paramValue);
      break;
   case SEQ_AUT_TRANSPOSE:
      ret = SeqScale::getTransposeText(paramValue);
      break;
   case SEQ_AUT_NOTE_LENGTH: // percentage
   case SEQ_AUT_POS_VARIANCE:
   case SEQ_AUT_VELO_VARIANCE: 
   case SEQ_AUT_LENGTH_VARIANCE:
   case SEQ_AUT_GLOBAL_SWING:
      ret = String::formatted("%d%%", paramValue);
      break;

   case SEQ_AUT_MUTED: // no/yes
      if (paramValue)
         ret = "yes";
      else
         ret = "no";
      break;
   case SEQ_AUT_CURRENT_PATTERN: // convert to 1-based
      ret=String::formatted("%d", paramValue+1);
      break;   
   default: // number is fine for the rest
      ret=String::formatted("%d", paramValue);
      break;

   }


   return ret; 
}

int SeqAudioProcessor::parseTextForAutomationParameterValue(int paramId, String text)
{
   int i;
   int res = SEQ_AUT_DEFAULT_VALUE_DESIG; // default if no match
   // parse the text and return the numeric representation
   // no need to look for "as assigned" as it's handled by the param thing
   switch (paramId) {
   case SEQ_AUT_CLOCKDIV: // translate      
      for (i = 0; i < SEQ_NUM_CLOCK_DIVS; i++) {
         if (text.compare(SeqScale::getClockDividerTextAndId(i)) == 0) {
            res = i;
            break;
         }
      }
      break;
   case SEQ_AUT_TRANSPOSE:
      for (i = 0; i < SEQMIDI_VALUE_TRANSPOSE_MAX_AUT; i++) {
         if (text.compare(SeqScale::getTransposeText(i)) == 0) {
            res = i;
            break;
         }
      }      
      break;
   case SEQ_AUT_NOTE_LENGTH: // percentage
   case SEQ_AUT_POS_VARIANCE:
   case SEQ_AUT_VELO_VARIANCE:
   case SEQ_AUT_LENGTH_VARIANCE:
   case SEQ_AUT_GLOBAL_SWING:
      res=text.dropLastCharacters(1).getIntValue();
      break;
   case SEQ_AUT_MUTED: // no/yes
      if (text.compare("yes") == 0)
         res = 1;
      else if (text.compare("no") == 0)
         res = 0;
      break;
   case SEQ_AUT_CURRENT_PATTERN: // convert back to 0-based
      res=text.getIntValue() - 1;
      break;   
   default: // number is fine for the rest
      res = text.getIntValue();
      break;

   }
   return res;
}

float SeqPlaybackParameter::getValue() const
{
   if (mValue == 0)
      return 0.0f;
   else if (mValue == 1)
      return 0.5f;
   else
      return 1.0f;
   /*
   SequenceData *sd = mParent->mData.getAudSeqData();
   if (sd->getAutoPlayMode() == SEQ_PLAYMODE_AUTO)
      return 0.0f;

   if (mParent->mMPBstate == SeqAudioProcessor::MPBstopped)
      return 0.0f;
   else
      return 1.0f;
      */
}
void SeqPlaybackParameter::setValue(float newValue)
{
   // host is setting value
   mValue = roundToInt(jmap<float>(newValue, 0.0f, 2.0f));
   
   SequenceData *sd = mParent->mData.getAudSeqData();
   if (sd->getAutoPlayMode() == SEQ_PLAYMODE_AUTO)
      return;
   
   if (mValue == 2)
      mParent->requestManualPlayback(true);
   else if (mValue == 1)
      mParent->requestManualPlayback(false);
   else // as assigned
      ;
}

String SeqPlaybackParameter::getText(float value, int) const
{
   int v = roundToInt(jmap<float>(value, 0.0f, 2.0f));
   if (!v)
      return "manual";
   else if(v==1)
      return "stop";
   else
      return "play";
}

float SeqPlaybackParameter::getValueForText(const String & text) const
{   
   if (text.compare("play") == 0)
      return 1.0f;
   else if (text.compare("stop"))
      return 0.5f;
   else // manual
      return 0.0f;
}

