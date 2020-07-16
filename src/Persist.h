/* Copyright (C) 2016 Andrew Shakinovsky
*/
#ifndef PERSIST_H_
#define PERSIST_H_
#include "SequenceData.h"
// persist or unpersist sequence data to/from xml

class SeqPersist {
   XmlElement mRoot;
   // add simple key value eg <name val="value"/>
   XmlElement *addKeyVal(const char *name, const String &value);   
   XmlElement *addKeyVal(const char *name, int64 value);
   bool getKeyVal(XmlElement *e, int64 *val);
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
