/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <JuceHeader.h>

// the version.h file is auto-generated by CMake (see version.h.in)
#include "version.h"

// current version of serialization
#define SEQ_SERIALIZE_CURRENT_VERSION  1

// used for properties file
#define SEQ_APP_NAME "stochas"
#define SEQ_CO_NAME  "AudioVitamins"

// for help button
#define SEQ_HELP_URL  "https://stochas.org/documentation/"
#define SEQ_WEBSITE_URL  "https://stochas.org"

#define SEQ_RESIZE_MSG "UI was resized. Depending on your host application, you might need to restart Stochas to properly adjust the window size"

// add some code to get around cubase behavior where it actually starts play position prior to
// where the cursor is when user hits play. if any issues arise, disable this to see if the workaround
// code is the cause. its a hack because it assumes that we will get process block called while we are
// not playing
#define CUBASE_HACKS 1

/*======CONSTANTS RELATED TO INTERACTING WITH PROB, VELOCITY AND NOTES==========*/
/* probabilities:
in poly mode are 0..100 where 0 is off, and 100 is always on
in mono mode are 0..high val where 0 is off, and the following applies
(each is double the preceding one)
*/
#define SEQ_PROB_LOW_VAL      25    
#define SEQ_PROB_MED_VAL      50
#define SEQ_PROB_HIGH_VAL     100
#define SEQ_PROB_NEVER        0     // never play unless chain
#define SEQ_PROB_OFF          -1    // off
#define SEQ_PROB_ON           100   // always on val (only valid in poly mode)
#define SEQ_NOTE_OFF          -1    // note value that represents "off"

#define SEQ_PROB_NEVER_TEXT   "--C--" // never
#define SEQ_PROB_OFF_TEXT     "Off"
#define SEQ_PROB_ON_TEXT      "On"
#define SEQ_PROB_LOW_TEXT     "Low"
#define SEQ_PROB_MED_TEXT     "Med"
#define SEQ_PROB_HIGH_TEXT    "High"

// default velocity when new steps are placed
#define SEQ_VELOCITY_DEFAULT  127
// default probability for mono
#define SEQ_PROB_DEFAULT_MONO 20
// default probability for poly
#define SEQ_PROB_DEFAULT_POLY 100

// when clicking velocity, it will toggle through these steps before hitting max (127)
#define SEQ_VELOCITY_STEP_1   35
#define SEQ_VELOCITY_STEP_2   70
#define SEQ_VELOCITY_STEP_3   95

// mouse sensitivity default. higher value means less sensitive
#define SEQ_MOUSE_SENSE_DEFAULT 4   
// max mouse sensitivity (ie least sensitive value)
#define SEQ_MOUSE_SENSE_MAX   10

#define SEQ_MOUSE_AXIS_SENSE  8     // how much the user must drag the mouse before an axis is guessed

#define SEQ_POS_OFFSET_MIN    -500
#define SEQ_POS_OFFSET_MAX    500

// ui scaling
#define SEQ_UI_SCALE_DEFAULT  100
#define SEQ_UI_SCALE_MIN      100
#define SEQ_UI_SCALE_MAX      200

// duty cycle in % (101 is legato)
#define SEQ_DUTY_MIN          5
#define SEQ_DUTY_MAX          200   // anything above 100% is legato
#define SEQ_DUTY_INTERVAL     5
#define SEQ_DUTY_DEFAULT      100

/*======CONSTANTS RELATED TO LIMITS IN THE ENGINE==============================*/
// min/max steps we'd ever have
#define SEQ_MIN_STEPS         1 // also min steps per measure
#define SEQ_MAX_STEPS         64
#define SEQ_DEFAULT_NUM_STEPS 16 // this is also max steps per measure and steps per visible page
// max rows we'd ever have is a chromatic progression of 0-127 (last row is off row)
#define SEQ_MAX_ROWS          129
#define SEQ_MAX_VISIBLE_ROWS  25
#define SEQ_MIN_ROWS          2  // we need one for the off row
// max patterns
#define SEQ_MAX_PATTERNS      8
// max layers
#define SEQ_MAX_LAYERS        4

// max number of source cells for a column of targets (each column shares the array)
#define SEQ_MAX_CHAIN_SOURCES 48

#define SEQ_CHAIN_FLAG_USED   1
// a chain is effective when the source cell triggered in the last cycle.
// if the NEGSRC is set, then it negates this, and it becomes effective when the source
// cell did not trigger. The effect of a chain is that the target cell will be triggered
// unless the NEGTGT is set in which case it will never trigger
#define SEQ_CHAIN_FLAG_NEGTGT 2     // do not trigger target cell when chain is effective
#define SEQ_CHAIN_FLAG_NEGSRC 4     // in effect when source cell was NOT triggered

#define SEQ_NUM_CLOCK_DIVS    7     // number of clock divider values
#define SEQ_MIN_CLOCK_DIV     1     // min numerator
#define SEQ_DEFAULT_CLOCK_DIV 16    // default numerator
#define SEQ_MAX_CLOCK_DIV     64    // max numerator
#define SEQ_CLOCK_DENOM       16    // clock divider denominator

// max num future midi events to hold (includes note on as well as note off events)
#define SEQ_MAX_MIDI_EVENTS   48
#define SEQ_DEFAULT_MIDI_CHAN 1
#define SEQ_DEFAULT_MAX_POLY  25

#define SEQ_POLY_BIAS_MIN     -99
#define SEQ_POLY_BIAS_MAX     99

#define SEQ_MAX_HUMAN_VELOCITY 100
#define SEQ_MAX_HUMAN_LENGTH   100
#define SEQ_MAX_HUMAN_POSITION 50

#define SEQ_MIN_SWING          -50
#define SEQ_MAX_SWING          50

// maximum notes in a chord (see scale.cpp)
#define SEQ_MAX_CHORD_COUNT 7

// number of note names that exist (C..B) and number of octaves
// note that for display purposes we will offset the octave by the user's preference
// which can be -2, -1 or 0
// internally, octaves start at 0 always. So we refer to them as internal octave and display octave
#define SEQ_NUM_NOTE_NAMES    12
#define SEQ_NUM_OCTAVES       11
#define SEQ_BASE_OCT_LOW      -2    // lowest selectable octave - low bound
#define SEQ_BASE_OCT_HIGH     0     // lowest selectable octave - high bound
#define SEQ_BASE_OCT_DEFAULT  -1    // default for the above two
#define SEQ_DEFAULT_OCTAVE    3     // default initial octave (internal octave number - 0 based)
#define SEQ_DEFAULT_SCALE     "Chromatic"
#define SEQ_DEFAULT_KEY       "C"
#define SEQ_DEFAULT_LAYER_NAME "<layer name>"
#define SEQ_DEFAULT_PAT_NAME   "<pattern name>"

#define SEQ_KEY_NAME_MAXLEN   3     // holds the text of a key name
#define SEQ_SCALE_NAME_MAXLEN 33    // holds the text of a scale name
#define SEQ_NOTE_NAME_MAXLEN  5     // holds a full note name eg C#-1, A# 5 or G# 10
#define SEQ_MAX_NOTELABEL_LEN 16    // maximum custom name (label, not actual note) for a note (excl terminator)
#define SEQ_LAYER_NAME_MAXLEN 15    // max len for layer name
#define SEQ_PATTERN_NAME_MAXLEN 15  // max len for layer name

// for midi passthru option and respond
#define SEQ_MIDI_PASSTHRU_NONE      1
#define SEQ_MIDI_PASSTHRU_UNHANDLED 2
#define SEQ_MIDI_PASSTHRU_ALL       3
#define SEQ_MIDI_RESPOND_NO         0
#define SEQ_MIDI_RESPOND_YES        1

// for play mode
#define SEQ_PLAYMODE_AUTO           0 // autoplay. follows daw playback
#define SEQ_PLAYMODE_INSTANT        1 // play starts instantly when hit
#define SEQ_PLAYMODE_STEP           2 // play starts on next step
#define SEQ_PLAYMODE_BEAT           3 // play starts on next beat
#define SEQ_PLAYMODE_MEASURE        4 // play starts on next measure

#define SEQ_DEFAULT_STANDALONE_BPM  120 // default bpm in standalone mode

/*====UI RELATED CONSTANTS====================================================*/

// sizing constants
#define SEQ_SIZE_MAIN_BORDER  4
#define SEQ_SIZE_VSCROLL      20
#define SEQ_SIZE_HSCROLL      20
#define SEQ_SIZE_NOTE_W       140   // total width of note area (including editable)
#define SEQ_SIZE_NOTE_SUB_W   35    // width of just the note text
#define SEQ_SIZE_CELL_H       16    // for note and cell
#define SEQ_SIZE_PP_H         12    // flashing play cursor panel
#define SEQ_SIZE_CELL_W       44    // sequencer cell width
#define SEQ_SIZE_MAIN_W       ((SEQ_DEFAULT_NUM_STEPS*SEQ_SIZE_CELL_W)+SEQ_SIZE_NOTE_W+(SEQ_SIZE_MAIN_BORDER*2)+SEQ_SIZE_VSCROLL)
#define SEQ_SIZE_MAIN_H       (690+SEQ_SIZE_HSCROLL) // test hscroll

#define SEQ_SIZE_LENCURSOR    10    // how wide is the length adjusting cursor

#define SEQ_SIZE_PROP_HEIGHT  20    // how high properties (eg in tabs at bottom) are
#define SEQ_SIZE_PROP_VSPACE  4     // spacing between these property items

///////////////////////
//MAIN UI CONTROL IDS
///////////////////////
// these are the id's for each toggle on the main UI and possible values for each
// Items designated with * should be unique for that control
#define SEQCTL_LAYER_TOGGLE         1  //*

#define SEQCTL_EDIT_TOGGLE          2  //*
#define SEQCTL_EDIT_TOGGLE_PROB     1
#define SEQCTL_EDIT_TOGGLE_VELO     2
#define SEQCTL_EDIT_TOGGLE_CHAIN    3
#define SEQCTL_EDIT_TOGGLE_OFFSET   4

#define SEQCTL_HELP_BUTTON          3  //*

#define SEQCTL_PATSEL_TOGGLE        4  //*
#define SEQCTL_SECTION_TOGGLE       5  //* pan around the grid


#define SEQCTL_OPTION_PANEL         6  //* id of the option panel for the purpose of notifying main ui to repaint
#define SEQCTL_OPTION_PANEL_REPAINT 0
#define SEQCTL_OPTION_PANEL_LOAD    1 // load custom notes
#define SEQCTL_OPTION_PANEL_SAVE    2 // save custom notes

#define SEQCTL_EDIT_BUTTON          7  //* edit dialog button

#define SEQCTL_EDITDIALOG           8  // * edit dialog itself (will notify when its done)
#define SEQCTL_GROOVE               9  //* groove control
#define SEQCTL_SWING_NUMBER         10 //* swing control
#define SEQCTL_GRV_CLR_BUTTON       11 //* clear groove
#define SEQCTL_TABS                 12 //* tabs at bottom
#define SEQCTL_SETTINGS_TAB         13 //* for notification when one of the settings changes
#define SEQCTL_MIDI_PASSTHRU        14 //* how to pass thru midi data
#define SEQCTL_MIDI_PASSTHRU_ALL    1
#define SEQCTL_MIDI_PASSTHRU_UNHND  2
#define SEQCTL_MIDI_PASSTHRU_NONE   3
#define SEQCTL_MIDI_RESPOND         15 //* respond to midi (yes no)
#define SEQCTL_MIDI_RESPOND_YES     1         
#define SEQCTL_MIDI_RESPOND_NO      2
#define SEQCTL_MIDI_MAP_BUTTON      16 //* midi mapping button
#define SEQCTL_RANDOM_TOGGLE        17 //* randomization options
#define SEQCTL_STEP_PANEL           18 //* to receive notifications from step panel (right now just del key)
#define SEQCTL_RANDOM_TOGGLE_VARYING 1
#define SEQCTL_RANDOM_TOGGLE_STABLE 2
#define SEQCTL_CHORD_TOGGLE         20 //* chord selection
#define SEQCTL_FILEDIALOG           21 //* file chooser completed
#define SEQCTL_GRV_LOAD_BUTTON      22 //* load groove from midi file
#define SEQCTL_LOAD_PATCH           23 //* load a new patch xml
#define SEQCTL_SAVE_PATCH           24 //* save a patch
#define SEQCTL_GRV_SAVE_BUTTON      25 //* save groove to midi file
#define SEQCTL_ADDCHAINDIALOG       28 //* add chain dialog
#define SEQCTL_UNDO_BUTTON          29 //* undo button
#define SEQCTL_RECORD_BUTTON        30 //* record button
#define SEQCTL_PLAYBACK_MODE        31 //* playback mode
#define SEQCTL_PLAYBACK_MODE_AUTO    1   
#define SEQCTL_PLAYBACK_MODE_INSTANT 2   
#define SEQCTL_PLAYBACK_MODE_Q_STEP  3
#define SEQCTL_PLAYBACK_MODE_Q_BEAT  4
#define SEQCTL_PLAYBACK_MODE_Q_MEAS  5
#define SEQCTL_PLAY_BUTTON          32 //* play button
#define SEQCTL_INFODIALOG           33 // * info dialog itself (will notify when its done)
#define SEQCTL_STANDALONE_BPM_BUTTON 34 // for standalone mode only
#define SEQCTL_SIZE_PANIC           35 // * press to restore UI size to default

///////////////////////
// OPTION TAB PANEL IDS
///////////////////////

#define SEQCTL_OPT_MP_TOGGLE        1  //*
#define SEQCTL_OPT_MP_TOGGLE_MONO   1
#define SEQCTL_OPT_MP_TOGGLE_POLY   2

#define SEQCTL_OPT_SCALE_TOGGLE     2  //*
#define SEQCTL_OPT_SCALE_TOGGLE_STD 1
#define SEQCTL_OPT_SCALE_TOGGLE_CST 2

#define SEQCTL_OPT_CLOCKDIV_TOGGLE  3  //* values are calculated

// pattern length on options panel
#define SEQCTL_OPT_PATLEN_NUMCTL    4  //*

#define SEQCTL_OPT_DUTYCY_NUMCTL    5  //* step duty cycle
#define SEQCTL_OPT_MIDICH_NUMCTL    6  //* midi channel
#define SEQCTL_OPT_STEPSPERM_NUMCTL 7  //* steps per measure
#define SEQCTL_OPT_NUMROWS_NUMCTL   8  //* number of visible rows
#define SEQCTL_OPT_LOAD_CUSTOM      9  //* load custom drum map
#define SEQCTL_OPT_SAVE_CUSTOM      10 //* save custom drum map
#define SEQCTL_OPT_STD_TO_CUSTOM    11 //* import drum map
#define SEQCTL_OPT_MAX_POLY         12 //* max polyphony in poly mode
#define SEQCTL_OPT_MUTE_TOGGLE      13 //* mute layer
#define SEQCTL_OPT_HUMAN_VELO       14 //* humanize velocity
#define SEQCTL_OPT_HUMAN_POS        15 //* humanize step position
#define SEQCTL_OPT_HUMAN_LENGTH     16 //* humanize step length
#define SEQCTL_OPT_POLYBIAS         17 //* poly bias

#define SEQCTL_COMBINE_TOGGLE       18 //* combine overlaps
#define SEQCTL_COMBINE_TOGGLE_JOIN  1
#define SEQCTL_COMBINE_TOGGLE_TRUNC 2

#define SEQCTL_OPT_MUTE_ALL         19 //* mute or unmute all

////////////////////////
// Settings panel id's
////////////////////////
#define SEQCTL_SET_MOUSESENSE       1
#define SEQCTL_SET_RTMOUSE          2
#define SEQCTL_SET_OCTAVE           3
#define SEQCTL_SET_DEFMONO          4
#define SEQCTL_SET_DEFPOLY          5
#define SEQCTL_SET_DEFVELO          6
#define SEQCTL_SET_COLOR            7
#define SEQCTL_SET_SHIFTREV         9
#define SEQCTL_SET_POSOFFSET        10
#define SEQCTL_SET_UISCALE          11



///////////////////////////
// MIDI panel IDs
//////////////////////////
#define SEQCTL_MIDI_OK              1
#define SEQCTL_MIDI_CLEAR           2
#define SEQCTL_MIDI_RESET           3
#define SEQCTL_MIDI_ADD             4


// IDs that the file dialog knows about (FileDialog.h)
#define SEQ_FILE_SAVE_NOTES         1
#define SEQ_FILE_LOAD_NOTES         2
#define SEQ_FILE_LOAD_MIDI          3
#define SEQ_FILE_LOAD_PATCH         4
#define SEQ_FILE_SAVE_PATCH         5
#define SEQ_FILE_SAVE_MIDI          6

//==========================================MIDI MAPPING CONSTANTS
#define SEQMIDI_MAX_ITEMS     100   // max number of items we can have
#define SEQMIDI_NUM_DEFAULT_ITEMS 12

// MIDI mapping actions
#define SEQMIDI_ACTION_INVALID 0
#define SEQMIDI_ACTION_CHGPAT 1     // pattern change
#define SEQMIDI_ACTION_MUTE   2     // mute/unmute layer
#define SEQMIDI_ACTION_SPEED  3     // change speed
#define SEQMIDI_ACTION_TRANS  4     // transpose 
#define SEQMIDI_ACTION_STEPS  5     // set number of steps
#define SEQMIDI_ACTION_RESET  6     // reset one of the above to default (use id above to signify which)
#define SEQMIDI_ACTION_PBIAS  7     // poly bias (variable. value passed will be the cc value) 
#define SEQMIDI_ACTION_PLAYBACK 8   // start/stop playback (not layer specific)

// midi mapping
// targets (1-4 are layers 1-4)
#define SEQMIDI_TARGET_ALL    SEQ_MAX_LAYERS+1
// midi mapping
// values for CC
#define SEQMIDI_VALUE_VARIABLE   -1 // valid for CC type actions (pbias right now)

// values for pattern (1-8 are pattern nums)
#define SEQMIDI_VALUE_PAT_NEXT (SEQ_MAX_PATTERNS+1)
#define SEQMIDI_VALUE_PAT_PREV (SEQ_MAX_PATTERNS+2)
// midi mapping
// values for mute
#define SEQMIDI_VALUE_MUTE_MUTE 1
#define SEQMIDI_VALUE_MUTE_UNMUTE 2
#define SEQMIDI_VALUE_MUTE_TOGGLE 3

// midi mapping
// values for PLAYBACK
#define SEQMIDI_VALUE_PLAYBACK_START  1
#define SEQMIDI_VALUE_PLAYBACK_STOP   2
#define SEQMIDI_VALUE_PLAYBACK_TOGGLE 3

// midi mapping
// values for speed (use options panel clock divider code)
// these are the first two and then clock divider values are next
#define SEQMIDI_VALUE_SPD_DBL  -1 // mult by 2 (thereby making faster)
#define SEQMIDI_VALUE_SPD_HALF -2 // div by 2 (making slower)
// midi mapping
// transpose. the ordering is: -11..-1 semi, +1..+11 semi, -2..-1 oct, 1..2 oct, next semi, prev semi
#define SEQMIDI_VALUE_TRANSPOSE_MIN   1
// total number we need. the 2 are for next/prev
#define SEQMIDI_VALUE_TRANSPOSE_MAX   28
// for automation we don't expose the last two
#define SEQMIDI_VALUE_TRANSPOSE_MAX_AUT   26
// last two are next/prev
#define SEQMIDI_VALUE_TRANS_NEXT (SEQMIDI_VALUE_TRANSPOSE_MAX-1) // +1 semitone
#define SEQMIDI_VALUE_TRANS_PREV (SEQMIDI_VALUE_TRANSPOSE_MAX) // -1 semitone

// midi mapping
// num steps (we will have 1-16, 24, 32, 48, 64)
#define SEQMIDI_VALUE_NS_MIN  1 // minimum
#define SEQMIDI_VALUE_NS_MAX  SEQ_DEFAULT_NUM_STEPS // max before 32
#define SEQMIDI_VALUE_NS_TWO  (SEQ_DEFAULT_NUM_STEPS*2)
#define SEQMIDI_VALUE_NS_TWENTYFOUR  ((SEQ_DEFAULT_NUM_STEPS*3)/2)
#define SEQMIDI_VALUE_NS_THREE (SEQ_DEFAULT_NUM_STEPS*3)
#define SEQMIDI_VALUE_NS_FOUR (SEQ_DEFAULT_NUM_STEPS*4)


// midi message constants (internal)
#define SEQ_MIDI_NOTEON    1
#define SEQ_MIDI_NOTEOFF   2
#define SEQ_MIDI_CC        3
#define SEQ_MIDI_OTHER     4    // some other message that we don't care about now (indicator light only)
#define SEQ_REFRESH_MAP_MSG 100 // not a midi message, but sent by ui to processor
#define SEQ_NOTIFY_HOST    101  // notify host that a change has occurred so that it knows the state is dirty
#define SEQ_SET_RECORD_MODE 102 // notify processor thread that recording is on/off (toggle)
#define SEQ_SET_PLAY_START_STOP 103 // notify processor that manual playback is started (1) or stopped (0)
#define SEQ_STANDALONE_SET_TEMPO 104 // standalone tempo is changed by the user

// automation target constants

// Per layer(ie. 4 of each)
#define SEQ_AUT_CLOCKDIV            1 // range from 1/16 up to 4/1 
#define SEQ_AUT_NUMSTEPS            2
#define SEQ_AUT_STEPS_PER_MEASURE   3
#define SEQ_AUT_NOTE_LENGTH         4
#define SEQ_AUT_POS_VARIANCE        5
#define SEQ_AUT_VELO_VARIANCE       6
#define SEQ_AUT_LENGTH_VARIANCE     7
#define SEQ_AUT_MUTED               8 // 1=mute, 0=unmute
#define SEQ_AUT_OUTPUT_CHANNEL      9
#define SEQ_AUT_MAX_POLY            10
#define SEQ_AUT_POLY_BIAS           11
#define SEQ_AUT_CURRENT_PATTERN     12
#define SEQ_AUT_TRANSPOSE           13
// global
#define SEQ_AUT_GLOBAL_SWING        14

// special value to indicate automation value is set to default
#define SEQ_AUT_DEFAULT_VALUE_DESIG 0xFFFF

#endif
