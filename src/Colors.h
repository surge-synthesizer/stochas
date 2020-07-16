/* Copyright (C) 2016 Andrew Shakinovsky
*/
#ifndef COLORS_H_INCLUDE
#error Should not be including this!
#endif

// These need to correspond with the enum in editorstate.h
int gDarkColor[] = {
     0x2D2E3A  //background
   , 0x2D2E3A  //stepDisabled (have less than 16 steps)
   , 0x3D3E4C  //stepOff
   , 0x99CC00  //stepNeverProb
   , 0xEDF933  //stepLowProb
   , 0xFF9933  //stepMediumProb
   , 0xFF5050  //stepHighProb
   , 0xEEF4ED  //stepLowVel
   , 0x8DA9C4  //stepMedVel
   , 0x134074  //stepHighVel   
   , 0x3D3E4C  //playIndicatorOff
   , 0xFFEEFF  //playIndicatorOn
   , 0x44A800  //noteEditable
   , 0x3D3E4C  //noteLocked   
   , 0x1B3E03  //toggleOff
   , 0x67F207  //toggleOn
   , 0x79F2A2  //button  
   , 0xADD8BC  // interacting with something
   , 0x000000  // border
   , 0xFFFF00  // start of measure
   , 0xFF9933  // start of beat
   , 0xFF00FF  // groove edit
   , 0x000000  // ghost color (alpha will be applied)
   , 0xFFFFFF  // cell selection (alpha will be applied)
   , 0xF0E532  // positive chain
   , 0xD72020  // negative chain
   , 0xFFFF00  // positive chain selected
   , 0xFF0000  // negative chain selected
   , 0x5E0622  // alert
   , 0xFF0000  // record active
   , 0x5E0622  // record passive
};

int gLightColor[] = {
     0x759797  //background
   , 0xE8E9F3  //stepDisabled (have less than 16 steps)
   , 0xD7D7DD  //stepOff
   , 0x99CC00  //stepNeverProb
   , 0xEDF933  //stepLowProb
   , 0xFF9933  //stepMediumProb
   , 0xFF5050  //stepHighProb
   , 0xEEF4ED  //stepLowVel
   , 0x8DA9C4  //stepMedVel
   , 0x134074  //stepHighVel   
   , 0xE8E9F3  //playIndicatorOff
   , 0x99CC00  //playIndicatorOn
   , 0xAADDDD  //noteEditable
   , 0xD7D7DD  //noteLocked   
   , 0xA2D5D5  //toggleOff
   , 0x118877  //toggleOn
   , 0x557777  //button  
   , 0x8888AA  // interacting with something
   , 0xBBDDDD  // border
   , 0xFFFF00  // start of measure
   , 0xFF9933  // start of beat
   , 0xFF00FF  // groove edit
   , 0x000000  // ghost color (alpha will be applied)
   , 0xFFFFFF  // cell selection (alpha will be applied)
   , 0xF0E532  // positive chain
   , 0xD72020  // negative chain
   , 0xFFFF00  // positive chain selected
   , 0xFF0000  // negative chain selected
   , 0x5E0622  // alert
   , 0xFF0000  // record active
   , 0x5E0622  // record passive

};

// for user definable skin file
const char *gColorNames[] = {
   "background"
   , "stepDisabled"
   , "stepOff"
   , "stepNeverProb"
   , "stepLowProb"
   , "stepMediumProb"
   , "stepHighProb"
   , "stepLowVel"
   , "stepMedVel"
   , "stepHighVel"
   , "playIndicatorOff"
   , "playIndicatorOn"
   , "noteEditable"
   , "noteLocked"
   , "toggleOff"
   , "toggleOn"
   , "button"
   , "interact"
   , "border"
   , "measureStart"
   , "beatStart"
   , "grooveEdit"
   , "ghostedNotes"
   , "cellSelection"
   , "chainPositive"
   , "chainNegative"
   , "chainPositiveSel"
   , "chainNegativeSel" 
   , "alertBackground"  
   , "recordButtonActive"
   , "recordButtonPassive"
};

static_assert(sizeof(gDarkColor) / sizeof(int) == EditorState::coloredElements::Count,
   "Color array size must match enum size for colors");
static_assert(sizeof(gLightColor) / sizeof(int) == EditorState::coloredElements::Count,
   "Color array size must match enum size for colors");
static_assert(sizeof(gColorNames) / sizeof(const char*) == EditorState::coloredElements::Count,
   "Color name array size must match enum size for colors");
