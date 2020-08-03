/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "HelpBanner.h"

#define HELP_TEXT_OK
#include "HelpText.h"
#undef HELP_TEXT_OK

void SeqHelpBanner::setColors()
{
   Colour fgColor;
   if (!mAlertMode)      
      mBgColor = mGlob->mEditorState->getColorFor(EditorState::coloredElements::background);   
   else      
      mBgColor = mGlob->mEditorState->getColorFor(EditorState::coloredElements::alertBackground);
   

   fgColor = mBgColor.contrasting(.5);
   
   mText.setColour(juce::Label::textColourId, fgColor);
   Image img;
   img = ImageCache::getFromMemory(SeqImageX::xmark464_png, SeqImageX::xmark464_pngSize);
   mButtonX.setImages(false, true, true, img, 1.0, fgColor, img, 1.0, fgColor, img, 1.0, fgColor);
   img = ImageCache::getFromMemory(SeqImageX::exclamation64_png, SeqImageX::exclamation64_pngSize);
   mIcon.setImages(false, true, true, img, 1.0, fgColor, img, 1.0, fgColor, img, 1.0, fgColor);

}

void SeqHelpBanner::setAlert(String alertText)
{
   
   mText.setText(alertText, juce::NotificationType::dontSendNotification);
   if (alertText.length())
      mAlertMode = true;
   else
      mAlertMode = false;
   setColors();
   // force resize   
   resized();
   
}

void SeqHelpBanner::setText(const String &text)
{
   if (text != mHelpText) { // save some CPU      
      setColors();
      mHelpText = text; // save for later in case not visible
      if (!mAlertMode) {
         mText.setText(text, juce::NotificationType::dontSendNotification);
      }
      // force resize
      resized();
      
   }

}

void SeqHelpBanner::lookupAndSetText(const Component * cpt)
{
   
   HelpPair hp;
   String orig = cpt->getName();
   String txt;
   // find a component in the chain that has an id and we have a lookup value for it
   while (cpt && !hp.key) { // use current component, if it has no name go up a level
      txt = cpt->getName();
      if (txt.length()) {
         hp = mHelpTexts[txt];
      }
      cpt = cpt->getParentComponent();
   }

   if (hp.value) {
      setText(hp.value);
   } 
   else {
      String tmp = String("No help found for ");
      tmp += orig;
      setText(tmp);      
   }
   
}

SeqHelpBanner::SeqHelpBanner(SeqGlob * glob) :
   mGlob(glob),
   mText(),
   mButtonX(),
   mIcon(),
   mAlertMode(false)
{
   addAndMakeVisible(mText);
   addAndMakeVisible(mButtonX);  // icons are loaded in setColors
   addAndMakeVisible(mIcon);     // icons are loaded in setColors
   mButtonX.addListener(this);
   mIcon.addListener(this);

   // create hash map for quick help text lookup
   for (HelpPair *cur = gHelpText; cur->key != 0; cur++)
      mHelpTexts.set(cur->key, *cur);
  

}

void SeqHelpBanner::resized()
{
   Rectangle<int> r = getLocalBounds();
   Rectangle<int> rI;
   if (mAlertMode) {
      rI = r.removeFromLeft(r.getHeight()); //square
      mIcon.setBounds(rI);
      rI = r.removeFromRight(20);
      rI.removeFromRight(2);
      rI.removeFromBottom(10);
      mButtonX.setBounds(rI);
   }
   else {
      mButtonX.setBounds(0, 0, 0, 0);
      mIcon.setBounds(0, 0, 0, 0);
   }
   mText.setBounds(r);


}

void SeqHelpBanner::paint(Graphics & g)
{
   g.setColour(mBgColor);
   g.fillAll();

}

void SeqHelpBanner::buttonClicked(Button *)
{
   // revert back to regular mode
   mAlertMode = false;
   setColors();
   mText.setText(mHelpText, juce::NotificationType::dontSendNotification);
   // force repaint
   resized();

}
