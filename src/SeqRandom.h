/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef SEQRANDOM_H_
#define SEQRANDOM_H_

#include "Constants.h"
#include "pcg_basic.h"

// max number we'd want to generate per position
// this is the maximum number of random numbers we'd need per
// sequencer position. Right now for poly mode we have 25, plus
// a few for humanization. if we exceed this per position, it just means
// we are getting values into the next position. probably not a big deal
#define MAX_PER_SEQ_POSITION 61 


// number of random values between each key-frame
// higher value means more rng generation when jumping around
// but also means there is a bigger span of time per key frame
// thus lower memory used for higher number of keyframes
#define FRAME_SIZE 1000      

// number of key-frames  to store. If we exceed this, we wrap around
// higher value takes more storage (use a prime number)
#define NUM_KEY_FRAMES 6007     



class SeqRandom {

   // we store just the state, since the seqno doesn't change. so save some memory
   uint64_t mKeyFrames[NUM_KEY_FRAMES];
   // store last generated because we are moving sequentially most of the time, so no
   // need to regenerate number since beginning of keyframe
   uint64_t mSequenceNumber;

   int mHighestCalculatedFrame; //   int highest_calced;  // highest frame index we have calculated

   uint64_t mLastGenerated; // last
   int mLastGeneratedPosition; // int last_position;   // last position calculated within frame
   int mLastGeneratedFrame;  // int last_position_frame;
   
   int mSeqPosition;
   int mSeqPositionSub;
public:
   SeqRandom() : mSeqPosition(0) , mSeqPositionSub(0) {}
   // for debugging only
   struct Statistics {
      int actual_numbers;  // random numbers we actually needed
      int generations;  // how many times we generated a random number for an intermediate value (ie we didn't use it)
      int seq_saves;    // how many times we avoided having to generate intermediate random values because positions were in sequence
      int frame_saves;  // how many times we avoided having to generate intermediate values because we already had a frame
      int end_saves;    // generated last position on a frame, so saved value for next keyframe
      Statistics() : actual_numbers(0), generations(0), seq_saves(0), frame_saves(0), end_saves(0) {}
   };
   Statistics mStats;
   
   // set the seed used for this sequence of random numbers.
   // sequence number allows us to get a different sequence of rng values
   // when using the same seed from another instance
   void setRandomSeedAndSequence(uint64 seed, uint64 seqno);

   // main function to get a random number at a position.
   // this will wrap around when position exceeds FRAME_SIZE*NUM_KEY_FRAMES
   int getRandAtPosition(int pos, int size);

   // get ready to get random values for a sequence position (there are MAX_PER_SEQ_POSITION)
   // values for each seq position
   void prepareSeqPosition(int pos);
   // get next random number for the current seq position
   // returns a number from 0 to size-1 inclusive
   int getNextRandom(int size);

};

#endif
