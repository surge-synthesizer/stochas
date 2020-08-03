/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "PlayPanel.h"

void PlayPanel::resized()
{
   int stepX = getWidth() / SEQ_MAX_STEPS;
   int stepY = getHeight();

   for (int x = 0; x < SEQ_MAX_STEPS; x++) {
      Rectangle<int> elementBounds(x * stepX, 0, stepX, stepY);
      mGrid[x].setBounds(elementBounds);
   }

}

PlayPanel::PlayPanel(SeqGlob *glob) :
   Component("playPanel"),
   mGlob(glob),
   mCurPosition(-1)
{
   
   for (int i = 0; i < SEQ_MAX_STEPS; i++) {
      addAndMakeVisible(mGrid[i]);
      mGrid[i].mGlob = mGlob;      
      mGrid[i].setText(String::formatted("%d", i + 1));
      
   }
}

void PlayPanel::refreshAll()
{
   SeqDataBuffer *s = mGlob->mSeqBuf;
   SequenceLayer *d = s->getUISeqData()->getLayer(mGlob->mEditorState->getCurrentLayer());
   
   // make steps invisible if they are past the end of our number of steps in pattern value
   int numvis = d->getNumSteps();
   for (int i = 0; i < SEQ_MAX_STEPS; i++) {
      mGrid[i].setVisible(i < numvis);
   }
}

void PlayPanel::check()
{
   int pp;
   pp = mGlob->mAudNotify->getPlayPosition(mGlob->mEditorState->getCurrentLayer());
   if (pp != mCurPosition) {
      mCurPosition = pp;
      for (int i = 0; i < SEQ_MAX_STEPS; i++) {
         mGrid[i].setOn(i == pp);         
      }      
      repaint();
   }
}

PlayLightCpt::PlayLightCpt(const String &name) :
   Component(name),
   mOn(false)
{
   
}

void PlayLightCpt::paint(Graphics & g)
{
   EditorState *e = mGlob->mEditorState;
   
   juce::Colour c;
   if(!mOn)
      c = e->getColorFor(EditorState::coloredElements::playIndicatorOff);
   else
      c = e->getColorFor(EditorState::coloredElements::playIndicatorOn);
   
   // background
   g.fillAll(e->getColorFor(EditorState::background));
   // current color
   g.setColour(c);
   Rectangle<int> r = getLocalBounds();
   r.removeFromRight(1);
   r.removeFromBottom(1);
   g.fillRect(r.toFloat());
   if (getWidth() > 20) { // todo: this is approximation. figure out better way. is there a function to determine?
      // only draw text if it fits
      g.setColour(c.contrasting(.5));
      g.drawText(mTxt, r.toFloat(), Justification::centred);
   }

}


void PatternPlayPanel::resized()
{
   // This was done to match up with ToggleCpt so that the pattern select
   // matches the pattern play lights
   Rectangle<int> r = getLocalBounds();
   float width = (float)getWidth() / SEQ_MAX_PATTERNS;
   int intwidth = (int)width;   
   Rectangle<int> one;
   float rem = 0.0f;
   r.reduce(1, 1); // pull 1 pixel of all sides so we can see background a little
   for (int x = 0; x < SEQ_MAX_PATTERNS; x++) {
      one = r.removeFromLeft(intwidth + (int)rem);
      mGrid[x].setBounds(one);
      // this nonsense is so that we take up the whole width
      if (rem >= 1.0)
         rem = 0;
      rem += (width - intwidth);
   }
}


PatternPlayPanel::PatternPlayPanel(SeqGlob * glob) :
   Component("patPanel"),
   mGlob(glob),
   mCurPosition(-1)
{
   
   for (int i = 0; i < SEQ_MAX_PATTERNS; i++) {
      addAndMakeVisible(mGrid[i]);
      mGrid[i].mGlob = mGlob;
      //mGrid[i].setText(String::formatted("%d", i + 1));

   }

}


void PatternPlayPanel::check()
{
   int pp;
   pp = mGlob->mAudNotify->getCurrentPattern(mGlob->mEditorState->getCurrentLayer());
   if (pp != mCurPosition) {
      mCurPosition = pp;
      for (int i = 0; i < SEQ_MAX_PATTERNS; i++) {
         mGrid[i].setOn(i == pp);
      }
      repaint();
   }

}
