/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef HELPBANNER_H_
#define HELPBANNER_H_

#include "EditorState.h"
#include "Constants.h"
#include "CommonComponents.h"

struct HelpPair {
   const char *key;
   const char *value;
   HelpPair() : key(0), value(0) {}
   HelpPair(const char *k, const char *v) : key(k), value(v) {}
};

/* This facilitates the help banner at the bottom of the main ui
*/
class SeqHelpBanner : public Component, public juce::Button::Listener {
   SeqGlob *mGlob;
   Label mText;
   ImageButton mButtonX;
   ImageButton mIcon;
   String mHelpText;
   bool mAlertMode; // will be true if in alert mode. when x is pressed, reverts to regular mode
   HashMap<String, HelpPair> mHelpTexts;
   juce::Colour mBgColor;
   void setColors();
public:
   // sets alert text which must be cleared by hitting the x button
   void setAlert(String alertText);

   // sets the help text to a specific string
   void setText(const String &text);
   // sets help text based on a component. get's it's name, looks at it's parent, etc
   void lookupAndSetText(const Component * id);
   SeqHelpBanner(SeqGlob *glob);
   void resized() override;
   void paint(Graphics &g) override;
   void buttonClicked(Button*) override;

};

#endif
