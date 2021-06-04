/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "ChainDialog.h"

SeqChainDialog::SeqChainDialog(SeqGlob * glob, CptNotify *parent) :
   SeqModalDialog(glob, SEQCTL_ADDCHAINDIALOG, parent, 320, 170)
{
   Colour txtColor, bgColor, hlColor;

   bgColor = mGlob->mEditorState->getColorFor(EditorState::background);
   txtColor = bgColor.contrasting(0.5f);

   mLblDescription = std::unique_ptr<Label>( addStdLabel("Specify Chain Details"));
   mLblDescription->setFont(Font(20.0, Font::plain));
   mLblSource = std::unique_ptr<Label>(addStdLabel(""));
   mLblTarget = std::unique_ptr<Label>(addStdLabel(""));

   mSource = std::unique_ptr<ToggleCpt>(new ToggleCpt(mGlob, CPTID_SRC, this, "source"));
   mSource->addItem(0, "Triggers", true);
   mSource->addItem(1, "Does not trigger", false);
   mTarget = std::unique_ptr<ToggleCpt>(new ToggleCpt(mGlob, CPTID_TARG, this, "targ"));
   mTarget->addItem(0, "Will trigger", true);
   mTarget->addItem(1, "Will not trigger", false);
   addToInner(CPTID_SRC, *mSource);
   addToInner(CPTID_TARG, *mTarget);
   mSource->setVisible(true);
   mTarget->setVisible(true);
   mBtnOk = std::unique_ptr<TextButton>(addStdButton("Ok", 0, CPTID_OK));

}


// called when the dialog is ending
void SeqChainDialog::endDialog(bool hitOk)
{
   bool negsrc, negtgt;
   SequenceData *db = mGlob->mSeqBuf->getUISeqData();
   int curlyr = mGlob->mEditorState->getCurrentLayer();
   SequenceLayer *dl = db->getLayer(curlyr);
   if (hitOk) {      
      // do it
      negsrc = mSource->isCurrent(1);
      negtgt = mTarget->isCurrent(1);
      // this will modify existing
      dl->addChainSource(mTgtRow, mTgtStep, mSrcRow, mSrcStep, negtgt, negsrc);
      mGlob->mSeqBuf->swap();
      notifyParent(0);      
   }   
   
}


#define S_GAP     4 // gap between items
#define S_HEIGHT  22
#define S_BORDER  2  // size of border gap
#define S_TAB     80 // tab size (where labels are involved)
#define S_LISTWIDTH 50
// this is called when the inner box is resized (ie we need to do our resize here)
void SeqChainDialog::resizedInner(Component * inner)
{
   Rectangle<int> r;
   Rectangle<int> bottom, top, btns, tmp, tmp2;

   // inner panel
   r = inner->getLocalBounds();   
   r.reduce(S_BORDER*2, S_BORDER*2); // have a bit of a space all around

   top = r.removeFromTop(S_HEIGHT);
   mLblDescription->setBounds(top);
   r.removeFromTop(S_GAP);
   
   top = r.removeFromTop(S_HEIGHT);
   mLblSource->setBounds(top);
   r.removeFromTop(S_GAP);

   top = r.removeFromTop(S_HEIGHT);
   mSource->setBounds(top);
   r.removeFromTop(S_GAP);

   top = r.removeFromTop(S_HEIGHT);
   mLblTarget->setBounds(top);
   r.removeFromTop(S_GAP);

   top = r.removeFromTop(S_HEIGHT);
   mTarget->setBounds(top);
   r.removeFromTop(S_GAP);

   mBtnOk->setBounds(r);
   

}

void SeqChainDialog::setDesc()
{
   EditorState *e = mGlob->mEditorState;   
   SequenceLayer *data = mGlob->mSeqBuf->getUISeqData()->getLayer(e->getCurrentLayer());
   String srcLabel, destLabel;
   const char *midiName;
   const char *label;
   int8_t n;
   char noteBuf[SEQ_NOTE_NAME_MAXLEN];
   
   n = data->getCurNote(mSrcRow);
   midiName = SeqScale::getMidiNoteName(n, e->getLowestOctave(), noteBuf);
   label = data->getNoteName(mSrcRow);

   srcLabel << "When " << midiName;
   if (label[0]) {
      srcLabel << " (" << label << ")";
   }
   srcLabel << " step " << (mSrcStep+1);

   n = data->getCurNote(mTgtRow);
   midiName = SeqScale::getMidiNoteName(n, e->getLowestOctave(), noteBuf);
   label = data->getNoteName(mTgtRow);

   destLabel << "then " << midiName;
   if (label[0]) {
      destLabel << " (" << label << ")";
   }
   destLabel << " step " << (mTgtStep+1);
   

   mLblSource->setText(srcLabel, juce::dontSendNotification);
   mLblTarget->setText(destLabel, juce::dontSendNotification);

}


// this is called when any of the added components have changed.
// value may or may not be applicable
void SeqChainDialog::notify(int cptId, int )
{
   switch (cptId) {
   case CPTID_OK:
      closeDialog(true);
      break;
   default:
      break;
   }

   
}

void SeqChainDialog::doSetup(int srcRow, int srcCol, int destRow, int destCol)
{
   // this does initial setup of the dialog whenever the user accesses it.
   SequenceData *sd = mGlob->mSeqBuf->getUISeqData();
   int curlyr = mGlob->mEditorState->getCurrentLayer();
   SequenceLayer *sl = sd->getLayer(curlyr);
   mSrcRow = srcRow;
   mSrcStep = srcCol;
   mTgtRow = destRow;
   mTgtStep = destCol;
   // determine cur value
   int it=-1;
   int sR, sC;
   bool nT, nS;
   while (sl->getChainSource(mTgtRow, mTgtStep, &it, &sR, &sC, &nT, &nS)) {
      if (sR == mSrcRow && sC == mSrcStep) {
         if (nT)
            mTarget->setCurrentItem(1, true, false);
         else
            mTarget->setCurrentItem(0, true, false);
         if (nS)
            mSource->setCurrentItem(1, true, false);
         else
            mSource->setCurrentItem(0, true, false);
         goto found;
      }
   }
   jassertfalse;
   endDialog(false); // wasn't found

found:
   setDesc();
   
}

SeqChainDialog::~SeqChainDialog()
{
}
