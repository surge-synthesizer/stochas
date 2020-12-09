/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "EditorState.h"
#define COLORS_H_INCLUDE
#include "Colors.h"
#include "Constants.h"

EditorState::EditorState() : 
   mEditMode(editingSteps),
   mCurrentLayer(0),
   mVisibleStep(0),
   mSelectedCells(0),
   mSelectedLowRow(0),
   mSelectedHighRow(0),
   mSelectedLowCol(0),
   mSelectedHighCol(0)

{
   configSerialization(true); // read settings from config
}

EditorState::~EditorState()
{
   configSerialization(false); // write settings to config
   
}

EditorState::EditMode EditorState::getEditMode()
{
   return mEditMode;
}

void EditorState::setEditMode(EditMode newMode)
{
   mEditMode = newMode;
}

void EditorState::setVisibleStep(int step)
{
   jassert(step >= -1 && step < SEQ_MAX_STEPS );
   mVisibleStep = step;
}

int EditorState::getVisibleStep()
{
   return mVisibleStep;
}

juce::Colour EditorState::getColorFor(coloredElements e)
{
   int r;
   jassert(e != Count);

   // in future we could pull this from a "current theme" sub array
   // add alpha channel
   if(mColorTheme==0)
      r = 0xFF000000 | gDarkColor[(int)e];
   else if(mColorTheme==1)
      r = 0xFF000000 | gLightColor[(int)e];
   else
      r = 0xFF000000 | mUserColors[(int)e];
   return juce::Colour(r);
}

char EditorState::velocityCycleNext(char curVel, bool next)
{
   
   if (curVel == 0) // use default
      curVel = (char)mDefaultVelocity;
   else if (next) { // cycle up
      if (curVel < SEQ_VELOCITY_STEP_1)
         curVel = SEQ_VELOCITY_STEP_1;
      else if (curVel < SEQ_VELOCITY_STEP_2)
         curVel = SEQ_VELOCITY_STEP_2;
      else if (curVel < SEQ_VELOCITY_STEP_3)
         curVel = SEQ_VELOCITY_STEP_3;
      else if (curVel < 127)
         curVel = 127;
      else
         curVel = SEQ_VELOCITY_STEP_1;
   } else { // cycle down
      if (curVel > SEQ_VELOCITY_STEP_3)
         curVel = SEQ_VELOCITY_STEP_3;
      else if (curVel > SEQ_VELOCITY_STEP_2)
         curVel = SEQ_VELOCITY_STEP_2;
      else if (curVel > SEQ_VELOCITY_STEP_1)
         curVel = SEQ_VELOCITY_STEP_1;
      else
         curVel = 0;         
   }
   return curVel;
}

char EditorState::probCycleNext(char curprob, bool mono, bool next)
{
   if (curprob == SEQ_PROB_OFF) {
      // if we are off, we want to explicitly set to our default
      if (mono)
         curprob = (char)mDefaultMono;
      else
         curprob = (char)mDefaultPoly;
   }
   else if (next) {// cycle to next
      if (curprob < SEQ_PROB_NEVER)
         curprob = SEQ_PROB_NEVER;
      else if (curprob < SEQ_PROB_LOW_VAL)
         curprob = SEQ_PROB_LOW_VAL;
      else if (curprob < SEQ_PROB_MED_VAL)
         curprob = SEQ_PROB_MED_VAL;
      else if (curprob < SEQ_PROB_HIGH_VAL)
         curprob = SEQ_PROB_HIGH_VAL;
      else if (mono) // prob=high val and we are in mono - cycle back to off in mono mode
         curprob = SEQ_PROB_OFF;
      else { // prob=high val and we are in poly mode 
         if (curprob < SEQ_PROB_ON)
            curprob = SEQ_PROB_ON;
         else // cycle to off in poly mode
            curprob = SEQ_PROB_OFF;
      }
   } else { // cycle down
      if (curprob > SEQ_PROB_MED_VAL)
         curprob = SEQ_PROB_MED_VAL;
      else if (curprob > SEQ_PROB_LOW_VAL)
         curprob = SEQ_PROB_LOW_VAL;
      else if (curprob > SEQ_PROB_NEVER)
         curprob = SEQ_PROB_NEVER;
      else
         curprob = SEQ_PROB_OFF;      
   }


   return curprob;
}


//===================================================================================
// Below here are settings that are persisted to the config file
//===================================================================================
void EditorState::configGetSetInt(juce::PropertiesFile *pf, 
   bool read, int &val, const char *key, int min, int max, int def)
{
   if (read) {
      val = pf->getIntValue(key, def);
      if (val < min || val >max)
         val = def;      
   }
   else {
      pf->setValue(key, var(val));
   }
}
void EditorState::configGetSetString(juce::PropertiesFile * pf, bool read, 
   String & val, const char * key, const String &def)
{
   if (read) {
      val = pf->getValue(key, def);
   }
   else {
      pf->setValue(key, var(val));
   }
}
EditorState::mouseRightClickAction 
EditorState::getMouseRightClickAction()
{
   return mRightClickAction;
}
void EditorState::setMouseRightClickAction(mouseRightClickAction action)
{
   mRightClickAction = action;
}
bool EditorState::getSelectedCells(int * lowCol, int * hiCol, int * lowRow, int * hiRow)
{
   if (lowCol) *lowCol = mSelectedLowCol;
   if (hiCol) *hiCol = mSelectedHighCol;
   if (lowRow) *lowRow = mSelectedLowRow;
   if (hiRow) *hiRow = mSelectedHighRow;


   return mSelectedCells;
}
void EditorState::setSelectedCells(int lowCol, int hiCol, int lowRow, int hiRow)
{
   mSelectedCells = true;
   mSelectedLowCol = lowCol;
   mSelectedHighCol = hiCol;
   mSelectedLowRow = lowRow;
   mSelectedHighRow = hiRow;
   
}
void EditorState::enlargeSelectedCells(int col, int row)
{
   // note that this may place it in a non-normalized state
   // eg. high row less than low row, etc
   jassert(mSelectedCells);
   mSelectedHighRow = row;
   mSelectedHighCol = col;
}
void EditorState::normalizeSelectedCells()
{
   jassert(mSelectedCells);
   int tmp = 0;
   if (mSelectedLowCol > mSelectedHighCol) {
      tmp = mSelectedLowCol; 
      mSelectedLowCol = mSelectedHighCol; 
      mSelectedHighCol = tmp; 
   }
   if (mSelectedLowRow > mSelectedHighRow) {
      tmp = mSelectedLowRow; 
      mSelectedLowRow = mSelectedHighRow; 
      mSelectedHighRow = tmp; 
   }
   
}
void EditorState::clearSelectedCells()
{
   mSelectedCells = false;
}

int EditorState::getNumSelectedCells()
{
   int tmp;
   if (!mSelectedCells)
      return 0;

   if (mSelectedLowCol < mSelectedHighCol)
      tmp = (1 + mSelectedHighCol - mSelectedLowCol);
   else
      tmp = (1 + mSelectedLowCol - mSelectedHighCol);

   if (mSelectedLowRow < mSelectedHighRow)
      tmp *= (1 + mSelectedHighRow - mSelectedLowRow);
   else
      tmp *= (1 + mSelectedLowRow - mSelectedHighRow);
   return tmp;
}

bool EditorState::isCellSelected(int col, int row)
{
   if (!mSelectedCells)
      return false;
   if (col >= mSelectedLowCol && col <= mSelectedHighCol &&
      row >= mSelectedLowRow && row <= mSelectedHighRow)
      return true;
   return false;
}

bool EditorState::isShiftReversed()
{
   return mShiftReversed==1;
}

void EditorState::setShiftReversed(bool isReversed)
{
   mShiftReversed = isReversed ? 1:0;
}

void EditorState::setFileDirectory(const String & dir)
{
   mDefaultFileDir = dir;
}

String EditorState::getFileDirectory()
{
   return mDefaultFileDir;
}

bool EditorState::getKeyboardDisabled()
{
   return mKeyboardDisabled==1;
}

void EditorState::setPPQOffset(int offset)
{
   mPPQOffset = offset;
}

int EditorState::getPPQOffset()
{
   return mPPQOffset;
}

void EditorState::setScaleFactor(int factor)
{
   mScaleFactor = factor;
}

int EditorState::getScaleFactor()
{
   return mScaleFactor;
}

void EditorState::configSerialization(bool read)
{
   int tmp;
   juce::PluginHostType hostType;

   ApplicationProperties appProperties;
   PropertiesFile::Options options;
   options.applicationName = SEQ_APP_NAME;
   options.commonToAllUsers = false;
   options.doNotSave = false;
   options.filenameSuffix = ".cfg";
   options.folderName = SEQ_CO_NAME;
   options.ignoreCaseOfKeyNames = true;
   options.osxLibrarySubFolder = "Application Support";
   appProperties.setStorageParameters(options);
   
   
   juce::PropertiesFile *pf = appProperties.getUserSettings();
   
   configGetSetInt(pf, read, mKeyboardDisabled, "disablehotkeys", 0, 1, 0);
   configGetSetInt(pf, read, mShiftReversed, "shiftBehaviorReversed", 0, 1, 0);
   configGetSetInt(pf, read, mMouseSense, "mouseSense", 1, SEQ_MOUSE_SENSE_MAX, SEQ_MOUSE_SENSE_DEFAULT);
   configGetSetInt(pf, read, mDefaultVelocity, "defaultVelocity", 0, 127, SEQ_VELOCITY_DEFAULT);
   configGetSetInt(pf, read, mLowestOctave, "lowestOctave", SEQ_BASE_OCT_LOW, SEQ_BASE_OCT_HIGH, SEQ_BASE_OCT_DEFAULT);
   configGetSetInt(pf, read, mDefaultMono, "defaultMono", SEQ_PROB_OFF, SEQ_PROB_ON, SEQ_PROB_DEFAULT_MONO);
   configGetSetInt(pf, read, mDefaultPoly, "defaultPoly", SEQ_PROB_OFF, SEQ_PROB_ON,SEQ_PROB_DEFAULT_POLY);

   // affects ppqPosition which appears buggy in protools
   if (hostType.isProTools())
      tmp = 22;
   else
      tmp = 0;
   configGetSetInt(pf, read, mPPQOffset, "positionOffset", SEQ_POS_OFFSET_MIN, SEQ_POS_OFFSET_MAX, tmp);

   configGetSetInt(pf, read, mScaleFactor, "uiScaleFactor", SEQ_UI_SCALE_MIN, SEQ_UI_SCALE_MAX, SEQ_UI_SCALE_DEFAULT);

   configGetSetString(pf, read, mDefaultFileDir, "fileDirectory", "");

   tmp = (int)mRightClickAction;
   configGetSetInt(pf, read, tmp, "rightClickAction", mouseRightClickAction::deleteCell, 
      mouseRightClickAction::cycleDown, mouseRightClickAction::deleteCell);
   mRightClickAction = (mouseRightClickAction)tmp;

   configGetSetInt(pf, read, mColorTheme, "colorTheme", 0,2,0);
   if (read)
      setColorTheme(mColorTheme); // force load of custom colors if applicable

   if (!read)
      appProperties.saveIfNeeded();
}

void EditorState::loadColorsFromFile()
{
   bool newFile = false;
   ApplicationProperties appProperties;
   PropertiesFile::Options options;
   options.applicationName = SEQ_APP_NAME;
   options.commonToAllUsers = false;
   options.doNotSave = false;
   options.filenameSuffix = ".skin";
   options.folderName = SEQ_CO_NAME;
   options.ignoreCaseOfKeyNames = true;
   options.osxLibrarySubFolder = "Application Support";
   appProperties.setStorageParameters(options);

   if (!options.getDefaultFile().existsAsFile())
      newFile = true;


   juce::PropertiesFile *pf = appProperties.getUserSettings();
   for (int i = 0; i < coloredElements::Count; i++) {
      String val;
      if (newFile)
         val = String::toHexString(gDarkColor[i]);
      configGetSetString(pf, !newFile, val, gColorNames[i],String::toHexString(gDarkColor[i]));
      mUserColors[i] = val.getHexValue32();      
   }
   
}



int EditorState::getMouseSense()
{
   return mMouseSense;
}

void EditorState::setMouseSense(int val)
{
   jassert(val > 0 && val <= SEQ_MOUSE_SENSE_MAX);
   mMouseSense = val;   
}


char
EditorState::getDefaultVelocity()
{
   return (char)mDefaultVelocity;   
}

void
EditorState::setDefaultVelocity(char val)
{
   jassert(val >= 0 && val <= 127);
   mDefaultVelocity = val;
   
}

int EditorState::getLowestOctave()
{
   return mLowestOctave;   
}

void EditorState::setLowestOctave(int val)
{
   jassert(val >= SEQ_BASE_OCT_LOW && val <= SEQ_BASE_OCT_HIGH);
   mLowestOctave = val;
}

char EditorState::getDefaultProbability(bool mono)
{
   if (mono)
      return (char)mDefaultMono;
   else
      return (char)mDefaultPoly;
   
}

void EditorState::setDefaultProbability(char val, bool mono)
{
   jassert(val >= SEQ_PROB_OFF && val <= SEQ_PROB_ON);
   if (mono)
      mDefaultMono = val;
   else
      mDefaultPoly = val;
}

int EditorState::getColorTheme()
{
   return mColorTheme;
}

void EditorState::setColorTheme(int val)
{
   jassert(val == 0 || val == 1 || val==2);
   mColorTheme = val;
   if (val == 2) { // user
      loadColorsFromFile(); // create if doesn't exist      
   }
}

int64 generateNewRootSeed()
{
   Random r;
   r.setSeedRandomly();
   return r.getSeed();   
}

void fixColors(EditorState *e, Component * cpt)
{
   /* Should have really tried to use the in-build color scheming stuff and would have saved
   all this trouble. but too late
   */
   int len = cpt->getNumChildComponents();
   juce::Colour bg = e->getColorFor(EditorState::background);
   juce::Colour fg = bg.contrasting(.5f);
   juce::Colour border =e->getColorFor(EditorState::border);
   juce::Colour btn = e->getColorFor(EditorState::button);



   for (int i = 0; i < len; i++) {
      Component *innercpt = cpt->getChildComponent(i);

      if (dynamic_cast<TextButton *>(innercpt) != nullptr) {
         TextButton *tb = dynamic_cast<TextButton *>(innercpt);
         tb->setColour(TextButton::ColourIds::buttonOnColourId, btn.contrasting(.5F));
         tb->setColour(TextButton::ColourIds::textColourOnId, btn);

         tb->setColour(TextButton::ColourIds::buttonColourId, btn);
         tb->setColour(TextButton::ColourIds::textColourOffId, btn.contrasting(.9F));
      }
      else if (dynamic_cast<TabbedComponent *>(innercpt) != nullptr) {
         TabbedComponent *tc = dynamic_cast<TabbedComponent *>(innercpt);
         int tl = tc->getNumTabs();
         for (int ti = 0; ti < tl; ti++) {
            tc->setTabBackgroundColour(ti, bg);
            fixColors(e,tc->getTabContentComponent(ti));
         }
      }
      else if (dynamic_cast<ListBox *>(innercpt) != nullptr) {
         ListBox *lb = dynamic_cast<ListBox *>(innercpt);
         lb->setColour(juce::TableListBox::ColourIds::backgroundColourId, bg);
         lb->setColour(ListBox::outlineColourId, border);
         lb->updateContent();
      }
      else if (dynamic_cast<ToggleButton *>(innercpt) != nullptr) {
         ToggleButton *tb = dynamic_cast<ToggleButton *>(innercpt);
         tb->setColour(ToggleButton::textColourId, fg);
         tb->setColour(ToggleButton::ColourIds::tickColourId, btn.contrasting(.5)/* Colours::black*/);

      }
      else if (dynamic_cast<ComboBox *>(innercpt) != nullptr) {
         ComboBox *cb = dynamic_cast<ComboBox *>(innercpt);
         cb->setColour(ComboBox::ColourIds::backgroundColourId, bg);
         cb->setColour(ComboBox::ColourIds::textColourId, fg);
         cb->setColour(ComboBox::ColourIds::arrowColourId, fg);
         cb->setColour(ComboBox::ColourIds::outlineColourId, border);
      }
      else if (dynamic_cast<Label *>(innercpt) != nullptr) {
         Label *lbl = dynamic_cast<Label *>(innercpt);
         lbl->setColour(Label::textColourId, fg);
         lbl->setColour(Label::ColourIds::textWhenEditingColourId, fg);
         lbl->setColour(Label::ColourIds::backgroundWhenEditingColourId, bg);
      }
      else if (dynamic_cast<TextEditor *>(innercpt) != nullptr) {
         TextEditor *te = dynamic_cast<TextEditor *>(innercpt);
         te->setColour(TextEditor::ColourIds::textColourId, fg);
         te->setColour(TextEditor::ColourIds::highlightColourId, fg);
         te->setColour(TextEditor::ColourIds::highlightedTextColourId, bg);
         te->setColour(TextEditor::ColourIds::backgroundColourId, bg);
         te->setColour(CaretComponent::caretColourId, btn);
         te->setColour(TextEditor::ColourIds::focusedOutlineColourId, btn);
         te->setColour(TextEditor::ColourIds::outlineColourId, btn.contrasting(.5));

      }
      else if (dynamic_cast<ScrollBar *>(innercpt) != nullptr) {
         ScrollBar *sb = dynamic_cast<ScrollBar *>(innercpt);
         sb->setColour(ScrollBar::ColourIds::thumbColourId, fg);
         sb->setColour(ScrollBar::ColourIds::trackColourId, bg);
      }
      else {
         fixColors(e,innercpt);
      }

   }

}

void fixDynamicTextEditBox(EditorState *e, TextEditor & te, int maxlen)
{
   juce::Colour bg = e->getColorFor(EditorState::background);
   juce::Colour fg = bg.contrasting(.5f);
   juce::Colour border = e->getColorFor(EditorState::border);
   juce::Colour btn = e->getColorFor(EditorState::button);

   te.setColour(TextEditor::ColourIds::textColourId, fg);
   te.setColour(TextEditor::ColourIds::highlightColourId, fg);
   te.setColour(TextEditor::ColourIds::highlightedTextColourId, bg);
   te.setColour(TextEditor::ColourIds::backgroundColourId, bg);
   te.setColour(CaretComponent::caretColourId, btn);
   
   te.setColour(TextEditor::ColourIds::focusedOutlineColourId, btn);
   te.setColour(TextEditor::ColourIds::outlineColourId, btn.contrasting(.5));
   te.setInputRestrictions(maxlen);
   //te.setInputFilter(new TextEditor::LengthAndCharacterRestriction(maxlen, ""), true);
}

void gradientFill(EditorState &e, Component * c, Graphics & g)
{
   Rectangle<int> me = c->getLocalBounds();
   Colour top, mid, bot;
   bot = e.getColorFor(EditorState::background);
   top = bot.brighter(.6f);
   mid = bot.brighter(.1f);
   ColourGradient grad(top, 0.f, 0.f, bot, 0.f, (float)me.getBottom(), false);
   grad.addColour(.08, mid);
   g.setGradientFill(grad);
   g.fillAll();// mGlob->mEditorState->getColorFor(EditorState::background));
}
