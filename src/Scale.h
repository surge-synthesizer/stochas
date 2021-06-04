/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef SCALE_H_
#define SCALE_H_

#include <stdint.h>
/**
Scales, Keys, octaves, etc
*/
class SeqScale {
   // these are for iterating
   const char *mScaleInt;  // points to current scale interval
   int mKeyIdx;            // points to a member of gNoteNames for the current key we are iterating
   int mScaleOffset;
   int8_t mCurNote;

public:
   /* Return the number of note names that exist in the chromatic progression (12) */
   static int getNumNoteNames();

   /* Return the note name at an index where C is the first one */
   static const char *getNoteName(int idx);

   /* Return the number of scale names that exist */
   static int getNumScales();

   /* Return a scale name*/
   static const char *getScaleName(int idx);

   /* Given a midi number, return the note name including the octave.
      lowOct - specify the display offset for the octave (eg: -2, -1 or 0)  
      buf cannot exceed SEQ_NOTE_NAME_MAXLEN
   */
   static const char *getMidiNoteName(int8_t num, int lowOct, char *buf);

   /* For transpose. Get text to display for a given id*/
   static const char *getTransposeText(int id);
   /*for transpose, get value in semitones for a given id.
   This returns true if the value is absolute and false if it's relative (ie add 1 semi, subtract 1 semi)
   */
   static bool getTransposeSemitones(int id, int *val);
   
   /* Given 0 based index, return text at that index in val. Returns false if idx is past end*/
   static const char *getClockDividerTextAndId(int idx, int *id=0);

   static int getNumChords();
   static const char *getChordName(int idx);
   static int *getChordIntervals(int idx, int *count);


   SeqScale();
   /* start iterating over notes in a scale. 
      After calling this, call getNextNote, and it will return a midi note number
      Will wrap around to low end of octave.
      start_octave is always 0 based
   */
   void startIterateNotesInScale(const char *scale, const char *key, int start_octave);
   int8_t getNextNote();
};

#endif
