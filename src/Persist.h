/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef PERSIST_H_
#define PERSIST_H_
#include "SequenceData.h"
// persist or unpersist sequence data to/from xml

class SeqPersist {
   XmlElement mRoot;
   // add simple key value eg <name val="value"/>
   XmlElement *addKeyVal(const char *name, const String &value);   
   XmlElement *addKeyVal(const char *name, int64 value);
   XmlElement *addKeyVal(const char *name, double value);
   bool getKeyVal(XmlElement *e, int64 *val);
   bool getKeyVal(XmlElement *e, double *val);
   bool getKeyVal(XmlElement *e, String *val);

   // add an indexed item eg <i idx="1" val="value"/>
   void retrieveLayer(XmlElement *e, SequenceLayer *lay);
   void retrievePattern(XmlElement *e, SequenceLayer *lay);
   void storeMidiMap(int idx, SeqMidiMapItem *item, XmlElement *parent);
   void storeLayer(int idx, SequenceLayer *item, XmlElement *parent);
   void storeNote(int idx, SequenceLayer *item, XmlElement *parent);
   void storePattern(int idx, SequenceLayer *item, XmlElement *parent);
   bool storeRow(int idx, int pat, SequenceLayer *item, XmlElement *parent);
   bool storeCell(int idx, int pat, int row, SequenceLayer *item, XmlElement *parent);
public:
   SeqPersist() : mRoot("stochas") {}
   // save the sequence data to XML and returns a ref to it.
   const XmlElement &store(SequenceData *sourceData);

   // read xml into sequence data
   // returns false if incompatible version etc
   bool retrieve(SequenceData *targetData,const XmlElement *sourceData);
};
#endif
