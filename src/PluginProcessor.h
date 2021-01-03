/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "SequenceData.h"
#include "StochaEngine.h"
class EditorState;
class SeqAudioProcessor;

// for mini midi map
#define NUM_MINI_MIDI 3

/* This is used to allow SeqAudioProcessorParameter to notify the audio processor
   when a value has changed in the host
 */
class AutParamNotify {
public:
   // called when a value has changed
   // paramValue will be SEQ_AUT_DEFAULT_VALUE_DESIG when it's being reset
   virtual void automationParameterHasChanged(int paramId, int paramValue, int layer) = 0;

   // return textual representation of a parameter given id and value
   virtual String getTextForAutomationParameterValue(int paramId, int paramValue) = 0;

   // parse string to value
   virtual int parseTextForAutomationParameterValue(int paramId, String text) = 0;
};

/** This is used to handle automation parameters

A value of 0.0f (from the host's perspective) indicates "assigned value" (default), 
so eg if we get a setValue with 0.0f it means
we are resetting the value to default. That along with the other values are mapped in a range of 
x to y where x is the rangeLo and y is the rangeHi. so rangeLo-1 (for mValue) represents 0.0f
for the host value. Furthermore for the sake of callback to PluginProcessor, this "default" 
value is sent as SEQ_AUT_DEFAULT_VALUE_DESIG.
So to sum up:
0.0f = host's idea of default
rangeLo-1 = SeqAudioProcessorParameter's idea of default (because we need a smooth range where it occurs at the bottom)
SEQ_AUT_DEFAULT_VALUE_DESIG = SeqPluginProcessor's idea of default (because we need something clearly outside of any valid range)

Possible automation targets:

__Per layer (ie. 4 of each):
Playback speed divider (need textual representation)
total steps (ranged integer)
steps per measure (ranged integer)
note on length (ranged integer with %)
position variance (ranged integer with %)
velo variance (ranged integer with %)
length variance (ranged integer with %)
mute/unmute (boolean)
output channel (ranged integer)
max poly (ranged integer)
bias (ranged integer)
current pattern (ranged integer)
transpose
__Global:
swing value (Ranged integer with %)
playback
*/
class SeqAudioProcessorParameter : public AudioProcessorParameter {
   // Inherited via AudioProcessorParameter

   // host calls this to get parameter value. Return range 0.0-1.0
   virtual float getValue() const override;

   // host calls this to set a new value. also called when setValueNotifyingHost is called
   // need to map the 0.0-1.0 range into our real value
   virtual void setValue(float newValue) override;

   // should return the default value for the parameter (0.0 always)
   virtual float getDefaultValue() const override;

   // should return the display name and should fit the given length
   virtual String getName(int maximumStringLength) const override;

   // label string for units ie "%"
   virtual String getLabel() const override;

   // should return a string representing the value
   virtual String getText(float value, int) const override;

   // should parse string and return value for it
   virtual float getValueForText(const String & text) const override;

   bool isAutomatable() const override { return true; }
   
   AutParamNotify *mNotify;
   int mLayerNumber; // if parameter is layer specific, otherwise unused
   int mParamId;
   int mRangeLo;
   int mRangeHi;
   int mValue;    // actual value. value of mRangeLo-1 indicates default here   
   String mName;
   
public:
   SeqAudioProcessorParameter(AutParamNotify *notify, int paramId, int rangeLo, int rangeHi, int layerNum, const String &name);

   // call this to reset the value to default (ie when playback stops)
   void reset();
};

// special case for playback parameter
// 3 values: manual, stop, play
// TODO playback automation: in reaper at least, if you add a knob for this parameter and set it to "stop",
// then go into the stochas ui and hit play button it ignores this parameter being in "stop"
// mode (treats it as "manual", meaning it allows you to start playback even though auto
// param is in 'stop'). This behavior is ok I think, but if we wanted it to be consistent we could
// check the value of mValue from pluginProcessor to see if play is allowed.
class SeqPlaybackParameter : public AudioProcessorParameter {
   SeqAudioProcessor *mParent;
   int mValue; // 0,1 or 2
   // Inherited via AudioProcessorParameter
   virtual int getNumSteps() const override { return 3; }
   virtual bool isDiscrete() const override { return true; }
   // host calls this to get parameter value. Return range 0.0-1.0
   virtual float getValue() const override;
   // host calls this to set a new value. also called when setValueNotifyingHost is called
   virtual void setValue(float newValue) override;
   virtual String getText(float value, int) const override;
   // should parse string and return value for it
   virtual float getValueForText(const String & text) const override;
   // should return the default value for the parameter (0.0 always)
   virtual float getDefaultValue() const override { return 0.0f; }
   // should return the display name and should fit the given length
   virtual String getName(int) const override { return "playback";}
   // label string for units ie "%"
   virtual String getLabel() const override {return String(); }
   // should return a string representing the value
   bool isAutomatable() const override { return true; }
public:
   SeqPlaybackParameter(SeqAudioProcessor *parent) : mParent(parent),mValue(0) {}

};

// for recording midi
struct MidiRecordEvent {
   int stepPos; // overall position in daw project (needed because we need to subtract for length)
   char vel;   
   char stepPosFrac;
   MidiRecordEvent() : vel(0) {}
};

/**
*/
class SeqAudioProcessor  : public AudioProcessor, public SeqProcessorNotifierHelper, public AutParamNotify
{
   friend class SeqPlaybackParameter;

   MidiRecordEvent mMidiRecord[128];
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
   #endif
    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    // true if we are playing
    // this will be set at some point after we start playing, and
    // cleared at some point after we stop. It's used to determine if we
    // are transitioning to playing or transitioning to stopping
    bool mPlaying;
    // this will be used to keep track of whether we have looped back to a
    // loop point (for randomization)
    double mLastPosition;

    StochaEngine mStocha[SEQ_MAX_LAYERS];     // the engine. vroom

    void checkforUIIncomingData(MidiBuffer &processedMidi);
    inline bool determinePlaybackState(int apm, bool playingInDaw, 
       double beatsperbar, double samplerate, double bpm, 
       double beatPosition, int samplesperblock);
    void requestManualPlayback(bool start);

    // most of the midi mapping handling is handled by the StochaEngine except
    // for playback start/stop (stochaengine midi stuff is only processed while playback
    // is active). so we will keep just what we need here for that
    struct MiniMidiMapItem {
       MiniMidiMapItem *mNext;
       int8_t mType;     // SEQ_MIDI_NOTEON or OFF
       int8_t mChannel;  // 1 based channel
       int8_t mValue;       // one of SEQMIDI_VALUE_PLAYBACK_*** or 0 if inactive
    };
    MiniMidiMapItem *mMiniMidiMap[128]; // can't have more than start/stop/toggle
    void resetMiniMidiMap();
    void rebuildMiniMidiMap();
    void handleMiniMidiMap(int type, char number, char chan, char val);

   // this will handle it and possibly pass it on to processedMidi
   // if recording is active, note on/off data will be passed to recordedNotes
   void handleIncomingMidi(bool currentlyPlaying, bool startingPlayback, 
      MidiBuffer &midiMessages, MidiBuffer &processedMidi, MidiBuffer &recordedNotes);
   // look in the incoming midi buffer for start/stop msgs that we may need to handle
   void checkIncomingMidiForStartStop(MidiBuffer &midiMessages);

   void dispatchRecordedMidiNotes(MidiBuffer &midiNoteData);

   // this is so that we can tell the host that something has changed, so that the host
   // can offer to save if the user hits exit.
   // audioprocessor becomes the owner of this and will delete it
   AudioParameterFloat *mDummyParam;
   
   juce::Array<SeqAudioProcessorParameter*> mAutomationParameters;
   void addAutParam(SeqAudioProcessorParameter *p);
   float mPPQAdjust;
#ifdef CUBASE_HACKS
   juce::PluginHostType mHostType;
   double mCubaseAtRestPos;
#endif
   // fill in position info in standalone mode
   void positionInfoStandalone(AudioPlayHead::CurrentPositionInfo *posinfo);
   // change tempo in standalone mode
   void changeStandaloneTempo();
   // given current tempo determine the current beat position
   double getStandaloneBeatPosition();
   // called when the recording mode has changed
   void recordingModeChanged();
   
   JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SeqAudioProcessor)
public:
   // persist this in the processor so that user state is preserved when a new editor is created
   EditorState *mEditorState;

   SeqAudioProcessor();
   ~SeqAudioProcessor();

   // this stores the actual data in the sequence. UI thread needs to access it
   SeqDataBuffer mData;

   // for the ui thread to send in arbitrary data integers
   SeqFifo mIncomingData;

   // for this to notify UI thread when position changes
   SeqProcessorNotifier mNotifier;

   // will be set to true if we are in "record" mode.
   // in this mode, completed midi notes are sent to the ui thread
   bool mRecordingMode;

   ///////// MANUAL PLAYBACK MODE //////////////////
   // These settings are related to manual playback
   // 
   // In standalone mode, we are always considered to be
   // playing, and the play button is the same as if we
   // were in daw mode
   /////////////////////////////////////////////////

   // set to true when daw is playing and playback is requested but not started yet.
   // needed so that we know that the daw was in playback mode and we were in request
   // mode at some time so that we can cancel the request if the daw stopped without us
   // actually starting stocha playback
   bool mMPBrequestArmed;   
   // current state of things for manual playback mode
   enum {MPBstopped, MPBrequested, MPBstarted} mMPBstate;
   // offset position in beats for manual playback
   double mMPBStartPosition;

   // For standalone mode only:
   double mStandaloneTempo; // current tempo in bpm
   // tick count when playback started or when tempo changed
   // playback in standalone mode starts as soon as we launch.
   // Current position is calculated based on the standalone tempo
   // and this start time
   double mStandaloneStartTime; 


   // Inherited via SeqProcessorNotifierHelper
   virtual bool getStepPlayedState(int layer, int position, int notenum) override;


   // Inherited via AutParamNotify
   virtual void automationParameterHasChanged(int paramId, int paramValue, int layer) override;
   virtual String getTextForAutomationParameterValue(int paramId, int paramValue) override;
   virtual int parseTextForAutomationParameterValue(int paramId, String text) override;

};

#endif  // PLUGINPROCESSOR_H_INCLUDED
