/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "Scale.h"
#include "Constants.h"

// maximum intervals in scale
#define MAX_NUM_INT 12

static const char *gNoteNames[SEQ_NUM_NOTE_NAMES] = {
   "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
};
struct Scale {
   const char *name;
   char intervals[MAX_NUM_INT+1];
};
/*
*/
// these may be loaded from file at some point
#define NUM_SCALES 15

static Scale gScaleNames[NUM_SCALES] = {
   {"Major","102034050607"},
   {"Natural Minor","102304056070" },
   {"Melodic Minor","102304050607" },
   {"Harmonic Minor","102304056007" },
   {"Dorian","102304050670" },
   {"Phrygian","120304056070" },
   {"Lydian","102030450607" },
   {"Mixolydian","102034050670" },
   {"Locrian","120304506070" },
   {"Major Pentatonic","102030050600" },
   {"Minor Pentatonic","100304050070" },
   {"Phrygian Dominant","120034056070" },
   {"Hungarian Minor","102300456070" },
   {"Ukrainian Minor","102300450670" },
   {SEQ_DEFAULT_SCALE,"111111111111" }
};

static const char *gDefScale = "111111111111";

struct TransposeVal {
   const char *text;
   int semitones;
};
static TransposeVal gTranspose[SEQMIDI_VALUE_TRANSPOSE_MAX] = {
   { "-11 semitones", -11 },
   { "-10 semitones", -10 },
   { "-9 semitones", -9 },
   { "-8 semitones", -8 },
   { "-7 semitones", -7 },
   { "-6 semitones", -6 },
   { "-5 semitones", -5 },
   { "-4 semitones", -4 },
   { "-3 semitones", -3 },
   { "-2 semitones", -2 },
   { "-1 semitone", -1 },
   { "+1 semitone", 1 },
   { "+2 semitones", 2 },
   { "+3 semitones", 3 },
   { "+4 semitones", 4 },
   { "+5 semitones", 5 },
   { "+6 semitones", 6 },
   { "+7 semitones", 7 },
   { "+8 semitones", 8 },
   { "+9 semitones", 9 },
   { "+10 semitones", 10 },
   { "+11 semitones", 11 },
   { "-2 octaves", -24 },
   { "-1 octave", -12 },
   { "+1 octave", 12 },
   { "+2 octaves", 24 },
   { "Up 1 semitone (relative)", 1 },      // special handling SEQMIDI_VALUE_TRANS_NEXT
   { "Down 1 semitone (relative)", -1 } // special handling SEQMIDI_VALUE_TRANS_PREV
};

struct ClockDivVal {
   const char *text;
   int id; // which is also the numerator
};

static ClockDivVal gClockDivs[SEQ_NUM_CLOCK_DIVS] = {
   { "1/16",1 },
   { "1/8",2 },
   { "1/4",4 },
   { "1/2",8 },
   { "1",16 },
   { "2",32 },
   { "4",64 }

};

struct ChordInfo {
   // these represent number of semitones from the root note (which we don't store)
   // trailing values are 0
   int intervals[SEQ_MAX_CHORD_COUNT];
   const char *name;
};

// derived from http://www.piano-keyboard-guide.com/piano-chords.html
#define NUM_CHORDS 14
static ChordInfo gChords[NUM_CHORDS] = {
   { {  4, 7 }, "major"},
   { {  3, 7 }, "minor" },
   { { 3, 6 }, "dim" },
   { { 3, 6, 9 }, "dim7" },
   { { 4, 8 }, "aug" },
   { { 4, 7, 10 }, "dom7" },
   { { 3, 7, 10 }, "min7" },
   { { 4, 7 , 11}, "maj7" },
   { { 4, 7, 9 }, "maj6" },
   { { 3, 7, 9 }, "min6" },
   { { 4, 8, 10 }, "7#5" },
   { { 4, 6, 10 }, "7b5" },
   { { 3, 7, 11 }, "Maj7b3" },
   { { 5, 7, 10 }, "7sus4" },
   
};

int
SeqScale::getNumNoteNames()
{
   return SEQ_NUM_NOTE_NAMES;
}

const char *
SeqScale::getNoteName(int idx)
{
   jassert(idx >= 0 && idx < SEQ_NUM_NOTE_NAMES);
   return gNoteNames[idx];
}

int
SeqScale::getNumScales()
{
   return NUM_SCALES;
}

const char *
SeqScale::getScaleName(int idx)
{
   jassert(idx >= 0 && idx < NUM_SCALES);
   return gScaleNames[idx].name;
}

const char * SeqScale::getMidiNoteName(int8_t num, int lowOct, char * buf)
{
   int oct;
   if (num == SEQ_NOTE_OFF)
      return "Off";
   jassert(lowOct >= SEQ_BASE_OCT_LOW && lowOct <= SEQ_BASE_OCT_HIGH);

   // get the note number (0 is C, 9 is A, 11 is B)
   // get the note letter ascii A=65, C=67. All repeat twice except E and B.
   // B is at the end, so only E presents a hurdle
   unsigned char n = num % 12;
   buf[0] = (char)((n > 8 ? 60 : 67) + ((n + (n > 4)) / 2));

   // determine natural or sharp (# if the note is 1, 3, 6, 8, 10)
   buf[1] = (n < 5 && (n & 1)) || (n > 5 && !(n & 1)) ? '#' : ' ';

   // determine the octave 0=48
   oct = (num / 12) +lowOct;
   // is it negative?
   if (oct < 0) {
      buf[2] = '-';
      oct = -oct;
      buf[3] = (char)(48 + oct);
      buf[4] = 0;
   }
   else if (oct >= 10) {
      buf[2] = '1';
      buf[3] = (char)(48 + (oct-10));
      buf[4] = 0;
   } else {
      buf[2] = ' ';
      buf[3] = (char)(48 + oct);
      buf[4] = 0;
   }

   return buf;

}

const char * SeqScale::getTransposeText(int id)
{
   jassert(id >= SEQMIDI_VALUE_TRANSPOSE_MIN && id <= SEQMIDI_VALUE_TRANSPOSE_MAX);
   return gTranspose[id-1].text;
   
}

bool SeqScale::getTransposeSemitones(int id, int * val)
{
   jassert(id >= SEQMIDI_VALUE_TRANSPOSE_MIN && id <= SEQMIDI_VALUE_TRANSPOSE_MAX);
   *val = gTranspose[id - 1].semitones;
   
   // relative movement special handling. return false
   if (id == SEQMIDI_VALUE_TRANS_NEXT || id== SEQMIDI_VALUE_TRANS_PREV)      
      return false;
   return true;

}

const char *
SeqScale::getClockDividerTextAndId(int idx, int *id)
{
   jassert(idx >= 0 && idx < SEQ_NUM_CLOCK_DIVS);
   if(id)
      *id = gClockDivs[idx].id;
   return gClockDivs[idx].text;   
}

int SeqScale::getNumChords()
{
   return NUM_CHORDS;
}

const char * SeqScale::getChordName(int idx)
{
   jassert(idx >= 0 && idx < NUM_CHORDS);
   return gChords[idx].name;
   
}

int * SeqScale::getChordIntervals(int idx, int * count)
{
   jassert(idx >= 0 && idx < NUM_CHORDS);
   int *c = gChords[idx].intervals;
   *count = 0;
   while (*c && *count < SEQ_MAX_CHORD_COUNT) {
      (*count)++;
      c++;
   }

   return gChords[idx].intervals;
}

SeqScale::SeqScale() :
   mScaleInt(0),
   mScaleOffset(-1),
   mCurNote(-1)
{
}

// note that start_octave is 0 based
void SeqScale::startIterateNotesInScale(const char * scale, const char * key, int start_octave)
{
   int i;
   jassert(start_octave >= 0 && start_octave < SEQ_NUM_OCTAVES);
   mScaleInt = gDefScale; // default in case we don't find it
   // find scale intervals
   for (i = 0; i < NUM_SCALES; i++) {
      if (!strcmp(scale, gScaleNames[i].name)) {
         if (strcmp(gScaleNames[i].intervals, "000000000000")!=0) // unlikely infinite loop later - use default
            mScaleInt = gScaleNames[i].intervals;
         break;
      }
   }
   // we might be loading these from a file at some point in which case it may not be found (which is valid)
   jassert(i != NUM_SCALES);

   // find index of the key we want
   for (i = 0; i < SEQ_NUM_NOTE_NAMES; i++) {
      if (!strcmp(key, gNoteNames[i])) {
         mKeyIdx = i;
         break;
      }
   }
   jassert(i != SEQ_NUM_NOTE_NAMES); // we should have found it


   mScaleOffset = 0; // next position to read

   // current note
   mCurNote = (int8_t)((start_octave * 12) + mKeyIdx);



}

int8_t
SeqScale::getNextNote()
{
   int8_t ret = 0;
   int sanity = 0;
   jassert(mScaleOffset != -1); // forgot to call startIterateNotesInScale

   // find next position to read
   while (mScaleInt[mScaleOffset] == '0') {
      mScaleOffset++;
      if (mCurNote < 127)
         mCurNote++;
      else {
         mCurNote = (int8_t)(mKeyIdx); // wrap to desired key in lowest octave
         mScaleOffset = 0;
      }

      if (mScaleOffset >= MAX_NUM_INT) // wrap
         mScaleOffset = 0;
      
      // infinite loop prevention
      sanity++;
      if (sanity > 100)
         return 0;
   }

   ret = mCurNote;
   
   // prepare for next
   mScaleOffset++;
   if (mScaleOffset >= MAX_NUM_INT) // wrap
      mScaleOffset = 0;
   if (mCurNote < 127)
      mCurNote++;
   else {
      mCurNote = (int8_t)(mKeyIdx); // wrap to desired key in lowest octave;
      mScaleOffset = 0;
   }

   return ret;
   
}
