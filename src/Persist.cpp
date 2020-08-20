/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "Persist.h"
#include "Constants.h"

XmlElement * SeqPersist::addKeyVal(const char *name, const String & value)
{
   XmlElement *ret = new XmlElement(name);
   ret->setAttribute("val", value);
   return ret;
}

XmlElement * SeqPersist::addKeyVal(const char * name, int64 value)
{
   String tmp;
   tmp << value;
   return addKeyVal(name,tmp);
}

XmlElement * SeqPersist::addKeyVal(const char * name, double value)
{
   String tmp;
   tmp << value;
   return addKeyVal(name,tmp);
}

bool SeqPersist::getKeyVal(XmlElement * e, int64 * val)
{
   String s;
   if (getKeyVal(e, &s)) {
      *val = s.getLargeIntValue();
      return true;
   }
   return false;
}

bool SeqPersist::getKeyVal(XmlElement * e, double * val)
{
   String s;
   if (getKeyVal(e, &s)) {
      *val = s.getDoubleValue();
      return true;
   }
   return false;
}

bool SeqPersist::getKeyVal(XmlElement * e, String * val)
{
   *val = e->getStringAttribute("val",String());
   if (val->length())
      return true;
   return false;
}

void SeqPersist::storeMidiMap(int idx, SeqMidiMapItem * item, XmlElement * parent)
{
   if (item->mAction != SEQMIDI_ACTION_INVALID) {
      XmlElement *mm = new XmlElement("mm");
      mm->setAttribute("idx", idx);
      mm->setAttribute("act", item->mAction);
      mm->setAttribute("chn", item->mChannel);
      mm->setAttribute("not", item->mNote);
      mm->setAttribute("tgt", item->mTarget);
      mm->setAttribute("typ", item->mType);
      mm->setAttribute("val", item->mValue);
      parent->prependChildElement(mm);
   }
}

void SeqPersist::storeLayer(int idx, SequenceLayer * item, XmlElement * parent)
{
   const char *scale;
   const char *key;
   int oct;
   XmlElement *lyr = new XmlElement("l");
   XmlElement *k;
   parent->prependChildElement(lyr);

   lyr->setAttribute("idx", idx);
   lyr->setAttribute("name", item->getLayerName());
   lyr->prependChildElement(addKeyVal("combine", item->isCombineMode() ? 1ll:0ll));
   lyr->prependChildElement(addKeyVal("humlen", (int64)item->getHumanLength()));
   lyr->prependChildElement(addKeyVal("humvel", (int64)item->getHumanVelocity()));
   lyr->prependChildElement(addKeyVal("humpos", (int64)item->getHumanPosition()));
   lyr->prependChildElement(addKeyVal("mute", item->getMuted() ? 1ll : 0ll));
   lyr->prependChildElement(addKeyVal("stppm", (int64)item->getStepsPerMeasure()));
   lyr->prependChildElement(addKeyVal("dcycle", (int64)item->getDutyCycle()));
   lyr->prependChildElement(addKeyVal("mchan", (int64)item->getMidiChannel()));
   lyr->prependChildElement(addKeyVal("clkdiv", (int64)item->getClockDivider()));
   lyr->prependChildElement(addKeyVal("notecust", item->noteSourceIsCustom() ? 1ll : 0ll));
   lyr->prependChildElement(addKeyVal("bias", (int64)item->getPolyBias()));
   lyr->prependChildElement(addKeyVal("maxpoly", (int64)item->getMaxPoly()));
   lyr->prependChildElement(addKeyVal("mono", item->isMonoMode() ? 1ll : 0ll));
   lyr->prependChildElement(addKeyVal("numsteps", (int64)item->getNumSteps()));
   lyr->prependChildElement(addKeyVal("numrows", (int64)item->getMaxRows()));

   item->getKeyScaleOct(&scale, &key, &oct);
   k = new XmlElement("sko");
   lyr->prependChildElement(k);
   k->setAttribute("stdscale", scale);
   k->setAttribute("stdkey", key);
   k->setAttribute("stdoct", oct);


   XmlElement *elem = new XmlElement("notes");
   lyr->prependChildElement(elem);
   for (int i = SEQ_MAX_ROWS-1; i >=0 ; i--) {
      storeNote(i, item, elem);
   }
   elem = new XmlElement("pats");
   lyr->prependChildElement(elem);
   for (int i = SEQ_MAX_PATTERNS-1; i >=0 ; i--) {
      storePattern(i, item, elem);
   }

}


void SeqPersist::retrieveLayer(XmlElement * e, SequenceLayer * lay)
{
   int64 ival;
   int val;
   String sval;
   int idx;
   sval = e->getStringAttribute("name");
   lay->setLayerName(sval.getCharPointer());
   forEachXmlChildElement(*e, ch) {
      if (ch->hasTagName("humlen")) {
         if (getKeyVal(ch, &ival))
            lay->setHumanLength((int)ival);
      }
      else if (ch->hasTagName("humvel")) {
         if (getKeyVal(ch, &ival))
            lay->setHumanVelocity((int)ival);
      }
      else if (ch->hasTagName("humpos")) {
         if (getKeyVal(ch, &ival))
            lay->setHumanPosition((int)ival);
      }
      else if (ch->hasTagName("combine")) {
         if (getKeyVal(ch, &ival))
            lay->setCombineMode(0 != ival);
      }
      else if (ch->hasTagName("mute")) {
         if (getKeyVal(ch, &ival))
            lay->setMuted(0 != ival);
      }
      else if (ch->hasTagName("stppm")) {
         if (getKeyVal(ch, &ival))
            lay->setStepsPerMeasure((int)ival);
      }
      else if (ch->hasTagName("dcycle")) {
         if (getKeyVal(ch, &ival))
            lay->setDutyCycle((int)ival);
      }
      else if (ch->hasTagName("mchan")) {
         if (getKeyVal(ch, &ival))
            lay->setMidiChannel((char)ival);
      }
      else if (ch->hasTagName("clkdiv")) {
         if (getKeyVal(ch, &ival))
            lay->setClockDivider((int)ival);
      }
      else if (ch->hasTagName("notecust")) {
         if (getKeyVal(ch, &ival))
            lay->setNoteSource(ival!=0);
      }
      else if (ch->hasTagName("bias")) {
         if (getKeyVal(ch, &ival))
            lay->setPolyBias((int)ival);
      }
      else if (ch->hasTagName("maxpoly")) {
         if (getKeyVal(ch, &ival))
            lay->setMaxPoly((int)ival);
      }
      else if (ch->hasTagName("mono")) {
         if (getKeyVal(ch, &ival))
            lay->setMonoMode(ival !=0);
      }
      else if (ch->hasTagName("numsteps")) {
         if (getKeyVal(ch, &ival))
            lay->setNumSteps((int)ival);
      }
      else if (ch->hasTagName("numrows")) {
         if (getKeyVal(ch, &ival))
            lay->setMaxRows((int)ival);
      }
      else if (ch->hasTagName("notes")) {
         forEachXmlChildElementWithTagName(*ch, note,"n") {            
            idx = note->getIntAttribute("idx");
            if (idx >= 0 && idx < SEQ_MAX_ROWS) {
               val = note->getIntAttribute("std");
               lay->setNote(idx, (char)val, false);
               val = note->getIntAttribute("cust");
               lay->setNote(idx, (char)val, true);
               sval = note->getStringAttribute("name");
               lay->setNoteName(idx, sval.getCharPointer());
            }
         }
      }
      else if (ch->hasTagName("pats")) {
         forEachXmlChildElementWithTagName(*ch, pat,"p") {
            retrievePattern(pat, lay);
         }
      }
      else if (ch->hasTagName("sko")) {
         String scale = ch->getStringAttribute("stdscale");
         String key = ch->getStringAttribute("stdkey");
         int oct = ch->getIntAttribute("stdoct");
         lay->setKeyScaleOct(scale.getCharPointer(), key.getCharPointer(), oct);
      }
   }
}

void SeqPersist::storeNote(int idx, SequenceLayer * item, XmlElement * parent)
{
   // note that although sequence data holds a note label for both std and cust, we only
   // store cust here
   char nbuf[SEQ_MAX_NOTELABEL_LEN + 1];
   nbuf[SEQ_MAX_NOTELABEL_LEN] = 0;
   XmlElement *n = new XmlElement("n");
   parent->prependChildElement(n);
   n->setAttribute("idx", idx);
   n->setAttribute("std", item->getNote(idx, false));
   n->setAttribute("cust", item->getNote(idx, true));
   strncpy(nbuf, item->getNoteName(idx), SEQ_MAX_NOTELABEL_LEN);
   if(strlen(nbuf))
      n->setAttribute("name", nbuf);

}


void SeqPersist::retrievePattern(XmlElement * e, SequenceLayer * lay)
{
   
   int pat = e->getIntAttribute("idx");
   if (pat < 0 || pat >= SEQ_MAX_PATTERNS)
      return;
   String sval;
   sval = e->getStringAttribute("name");   
   if(sval.length()) // if differs from default it will be saved
      lay->setPatternName(sval.getCharPointer(), pat);

   forEachXmlChildElementWithTagName(*e, ch,"rows") {
      forEachXmlChildElementWithTagName(*ch, row, "r") {
         int rowidx=row->getIntAttribute("idx");
         if (rowidx >= 0 && rowidx < SEQ_MAX_ROWS) {
            forEachXmlChildElementWithTagName(*row, cells, "cells") {
               forEachXmlChildElementWithTagName(*cells, cell, "c") {
                  int cellidx = cell->getIntAttribute("idx");
                  if (cellidx >= 0 && cellidx < SEQ_MAX_STEPS) {
                     lay->setProb(rowidx, cellidx, (char)cell->getIntAttribute("prob", SEQ_PROB_OFF), pat);
                     lay->setVel(rowidx, cellidx, (char)cell->getIntAttribute("velo"), pat);
                     lay->setLength(rowidx, cellidx, (char)cell->getIntAttribute("len"), pat);
                     lay->setOffset(rowidx, cellidx, (char)cell->getIntAttribute("offs"), pat);

                     forEachXmlChildElementWithTagName(*cell, cs, "cs") {
                        int srcRow=cs->getIntAttribute("row");
                        int srcStep=cs->getIntAttribute("col");
                        bool negtgt=(cs->getIntAttribute("neg") == 1);
                        bool negsrc = (cs->getIntAttribute("negsrc") == 1);
                        lay->addChainSource(rowidx, cellidx, srcRow, srcStep, negtgt,negsrc, pat);
                     }
                  }
               }
            }
         }
      }     
   }
}


void SeqPersist::storePattern(int idx, SequenceLayer * item, XmlElement * parent)
{
   bool haveRows = false;
   // only store pattern if it has chains and rows
   XmlElement *p = new XmlElement("p");
   const char *pname = item->getPatternName(idx);
   if (strcmp(pname, SEQ_DEFAULT_PAT_NAME) != 0) {
      // pattern name was edited
      p->setAttribute("name", pname);
   }
   else
      pname = 0; // indicate not edited

   p->setAttribute("idx", idx);
   XmlElement *elem = new XmlElement("rows");
   p->prependChildElement(elem);
   for (int i = SEQ_MAX_ROWS-1; i>=0 ; i--) {
      if (storeRow(i, idx, item, elem))
         haveRows = true;
   }   

   if (haveRows || pname != 0)
      parent->prependChildElement(p);
   else // no rows
      delete p;
}

bool SeqPersist::storeRow(int idx, int pat, SequenceLayer * item, XmlElement * parent)
{
   XmlElement *r = new XmlElement("r");
   bool haveRow = false;
   r->setAttribute("idx", idx);
   XmlElement *elem = new XmlElement("cells");
   r->prependChildElement(elem);
   for (int i = SEQ_MAX_STEPS-1; i >=0; i--) {
      if (storeCell(i,pat,idx, item, elem))
         haveRow = true;
   }

   // only do this if we had at least one cell
   if (haveRow) {
      parent->prependChildElement(r);
      return true;
   }
   else {
      delete r;
      return false;
   }
}

bool SeqPersist::storeCell(int idx, int pat, int row,  SequenceLayer * item, XmlElement * parent)
{
   int it, srcRow, srcCol;
   bool negtgt, negsrc;
   XmlElement *r;
   if (item->getProb(row, idx, pat) != SEQ_PROB_OFF) {
      r = new XmlElement("c");
      parent->prependChildElement(r);
      r->setAttribute("idx", idx);
      r->setAttribute("prob", item->getProb(row, idx, pat));
      r->setAttribute("velo", item->getVel(row, idx, pat));
      r->setAttribute("len", item->getLength(row, idx, pat));
      r->setAttribute("offs", item->getOffset(row, idx, pat));

      // store chain for this cell
      it = -1;
      while (item->getChainSource(row, idx, &it, &srcRow, &srcCol, &negtgt, &negsrc, pat)) {
         XmlElement *cs = new XmlElement("cs");
         r->prependChildElement(cs);
         cs->setAttribute("row", srcRow);
         cs->setAttribute("col", srcCol);
         cs->setAttribute("neg", negtgt ? 1 : 0);
         cs->setAttribute("negsrc", negsrc ? 1 : 0);
      }

      return true;
   }
   
   return false;
}

const XmlElement & SeqPersist::store(SequenceData * sourceData)
{
   int i,len;
   XmlElement *elem;
   mRoot.deleteAllChildElements();
   mRoot.setAttribute("version", SEQ_SERIALIZE_CURRENT_VERSION);
   /*
   data
     layers
       note sets
        notes
       patterns
         rows
           cells
         chain sources
           source cells
     groove
     midi map items
   */
   mRoot.prependChildElement(addKeyVal("bpm", sourceData->getStandaloneBPM()));
   mRoot.prependChildElement(addKeyVal("autoplay", (int64)sourceData->getAutoPlayMode())); 
   mRoot.prependChildElement(addKeyVal("offtime", (int64)sourceData->getOffsetTime())); // global offset time
   mRoot.prependChildElement(addKeyVal("curpat",(int64)sourceData->getCurrentPattern()));
   mRoot.prependChildElement(addKeyVal("swing", (int64)sourceData->getSwing()));
   mRoot.prependChildElement(addKeyVal("seed", sourceData->getRandomSeed()));
   mRoot.prependChildElement(addKeyVal("midipass", (int64)sourceData->getMidiPassthru()));
   mRoot.prependChildElement(addKeyVal("midiresp", (int64)sourceData->getMidiRespond()));
   elem = new XmlElement("groove");
   mRoot.prependChildElement(elem);
   for (i = SEQ_DEFAULT_NUM_STEPS-1; i >=0 ; i--) {
      int grv;
      // only store non-zero ones
      if ((grv = sourceData->getGroove(i)) != 0) {
         XmlElement *g = new XmlElement("i");
         g->setAttribute("idx", i);
         g->setAttribute("val", grv);
         elem->prependChildElement(g);
      }         
   }

   elem = new XmlElement("midimap");
   mRoot.prependChildElement(elem);
   len = sourceData->getMappingCount();
   if (len) {
      for (i = len - 1; i >= 0; i--)
         storeMidiMap(i, sourceData->getMappingItem(i), elem);
   }

   elem = new XmlElement("layer");
   mRoot.prependChildElement(elem);
   for (i = SEQ_MAX_LAYERS-1; i >=0 ; i--)
      storeLayer(i, sourceData->getLayer(i), elem);

   return mRoot;
}

bool SeqPersist::retrieve(SequenceData * targetData, const XmlElement * sourceData)
{
   int idx;
   int64 val;
   double dval;
   /// read source data into target data
   if (sourceData->getTagName().compare("stochas") != 0) 
      return false;
   
   if(sourceData->getIntAttribute("version",-1) != SEQ_SERIALIZE_CURRENT_VERSION)
      return false;

   // start fresh (dont put sequencedata on stack! it's too big!)
   std::unique_ptr<SequenceData> dummy(new SequenceData());
   *targetData = *dummy;

   forEachXmlChildElement(*sourceData, e) {
      if (e->hasTagName("groove")) {
         forEachXmlChildElement(*e, g) {
            if (g->hasTagName("i")) {
               idx = g->getIntAttribute("idx");
               val = g->getIntAttribute("val");
               if (idx < SEQ_DEFAULT_NUM_STEPS && idx >= 0)
                  targetData->setGroove(idx, (int)val);
            }
         }         
      } // end groove

      else if (e->hasTagName("midimap")) {
         int mapcount = 0;
         forEachXmlChildElement(*e, m) {
            if (m->hasTagName("mm")) {
               idx = m->getIntAttribute("idx");
               if (idx >= 0 && idx < SEQMIDI_MAX_ITEMS) {
                  SeqMidiMapItem *mm = targetData->getMappingItem(idx);
                  mm->mAction = (char)m->getIntAttribute("act");
                  mm->mChannel = (char)m->getIntAttribute("chn");
                  mm->mNote = (char)m->getIntAttribute("not");
                  mm->mTarget = (char)m->getIntAttribute("tgt");
                  mm->mType = (char)m->getIntAttribute("typ");
                  mm->mValue = (char)m->getIntAttribute("val");
                  mapcount++;
               }
            }
         }
         targetData->setMappingCount(mapcount);
      } // end midimap

      else if (e->hasTagName("layer")) {
         forEachXmlChildElementWithTagName(*e, l,"l") {            
            idx=l->getIntAttribute("idx");
            if (idx >= 0 && idx < SEQ_MAX_LAYERS)          
               retrieveLayer(l, targetData->getLayer(idx));
         }
      } // end layer

      else if (e->hasTagName("midiresp")) {
         if (getKeyVal(e, &val))
            targetData->setMidiRespond((int)val);
      }
      else if (e->hasTagName("midipass")) {
         if (getKeyVal(e, &val))
            targetData->setMidiPassthru((int)val);

      }
      else if (e->hasTagName("seed")) {
         if (getKeyVal(e, &val))
            targetData->setRandomSeed(val);

      }
      else if (e->hasTagName("swing")) {
         if (getKeyVal(e, &val))
            targetData->setSwing((int)val);

      }
      else if (e->hasTagName("curpat")) {
         if (getKeyVal(e, &val))
            targetData->setCurrentPattern((int)val);

      }
      else if (e->hasTagName("offtime")) {
         if (getKeyVal(e, &val))
            targetData->setOffsetTime((int)val);

      }
      else if (e->hasTagName("autoplay")) {
         if (getKeyVal(e, &val))
            targetData->setAutoPlayMode((int)val);
      }
      else if (e->hasTagName("bpm")) {
         if (getKeyVal(e, &dval))
            targetData->setStandaloneBPM(dval);
      }
   } // for each root level element

   return true;
}
