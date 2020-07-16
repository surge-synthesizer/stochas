/* Copyright (C) 2016 Andrew Shakinovsky
*/
#ifndef PLAYPANEL_H_
#define PLAYPANEL_H_
#include <JuceHeader.h>
#include "SequenceData.h"
#include "EditorState.h"
#include "Constants.h"

/**
This component represents a single cell that is lit up when the sequencer is playing
*/
class PlayLightCpt : public Component {
   bool mOn;
   juce::String mTxt;
   
public:
   SeqGlob *mGlob;
   PlayLightCpt(const String &name= "playLight");
   
   void paint(Graphics &g) override;
   void setOn(bool on) { mOn = on; repaint(); }
   void setText(String txt) { mTxt = txt; }
   
};

/**
This is the panel that hosts the play lights
*/
class PlayPanel : public Component {
   SeqGlob *mGlob;
   int mCurPosition;
   PlayLightCpt mGrid[SEQ_MAX_STEPS];
   void resized() override;
public:
   PlayPanel(SeqGlob *glob);
   void refreshAll();
   // called to tell it to see whether it needs updating
   void check();
   
};

class PatternPlayPanel : public Component {
   SeqGlob *mGlob;
   int mCurPosition;
   PlayLightCpt mGrid[SEQ_MAX_PATTERNS];
   void resized() override;
public:
   PatternPlayPanel(SeqGlob *glob);
   // called to tell it to see whether it needs updating
   void check();
};

#endif