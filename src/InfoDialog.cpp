/***************************************************************
 ** Copyright (C) 2020 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "InfoDialog.h"


// called when the dialog is ending
void SeqInfoDialog::endDialog(bool hitOk)
{
   
}


#define S_GAP     4 // gap between items
#define S_HEIGHT  24
#define S_BORDER  5  // size of border gap

// this is called when the inner box is resized (ie we need to do our resize here)
void SeqInfoDialog::resizedInner(Component * inner)
{
   Rectangle<int> r;
   Rectangle<int> leftpanel, logo, tmp;

   // inner panel
   r = inner->getLocalBounds();   
   r.reduce(S_BORDER*2, S_BORDER*2); // have a bit of a space all around

   // left panel
   leftpanel = r.removeFromLeft(262); // width of logo
   logo = leftpanel.removeFromTop(290); // height of logo
   mLogo.setBounds(logo);
   leftpanel.removeFromTop(S_GAP);
   tmp=leftpanel.removeFromTop(S_HEIGHT);
   mBtnHelp->setBounds(tmp);
   leftpanel.removeFromTop(S_GAP);
   tmp=leftpanel.removeFromTop(S_HEIGHT);
   mBtnWebsite->setBounds(tmp);
   leftpanel.removeFromTop(S_GAP);
   tmp=leftpanel.removeFromTop(S_HEIGHT);
   mBtnOk->setBounds(tmp);

   r.removeFromTop(5);
   tmp=r.removeFromTop(S_HEIGHT);
   mLblMain->setBounds(tmp);
   
   r.reduce(S_BORDER, S_BORDER);
   mLblDescription->setBounds(r);
   

}

SeqInfoDialog::SeqInfoDialog(SeqGlob * glob, CptNotify *parent) :
   SeqModalDialog(glob, SEQCTL_INFODIALOG,parent, 750,400)
{
   Colour txtColor, bgColor, hlColor;
   
   bgColor = mGlob->mEditorState->getColorFor(EditorState::background);   
   txtColor = bgColor.contrasting(0.5f);

   // logo
   addToInner(0,mLogo);
   juce::Image img = ImageCache::getFromMemory(SeqImageX::logo_no_bg_png, SeqImageX::logo_no_bg_pngSize);
   mLogo.setImage(img);
   mLogo.setVisible(true);
   

   // big label at top
   String vs = String("Stochas v. ");
   vs += Stochas::Build::FullVersionStr;
   mLblMain = std::unique_ptr<Label>(addStdLabel(vs)); 
   mLblMain->setFont(Font(20.0f, Font::bold));
   mLblMain->setJustificationType(Justification::centred);
   
   // big descr
   

   mLblDescription = std::unique_ptr<TextEditor>(addStdTextEditor());    
   mLblDescription->setFont(Font(14.0f, Font::plain));
   mLblDescription->setMultiLine(true);
   mLblDescription->setReadOnly(true);
   mLblDescription->setText(SeqImageX::infobox_txt, false);
   //mLblDescription->setJustificationType(/*Justification::horizontallyJustified | */ Justification::top);
   //mLblDescription->setMinimumHorizontalScale(1.0f);

   // buttons
   mBtnOk = std::unique_ptr<TextButton>(addStdButton("Close",0,CPTID_OK));
   mBtnHelp = std::unique_ptr<TextButton>(addStdButton("Documentation",0,CPTID_HELP_LAUNCH));
   mBtnWebsite = std::unique_ptr<TextButton>(addStdButton("Website",0,CPTID_WESITE_LAUNCH));
   
}

// this is called when any of the added components have changed.
// value may or may not be applicable
void SeqInfoDialog::notify(int cptId, int )
{
   switch (cptId) {
   case CPTID_OK:
      break;
   case CPTID_HELP_LAUNCH: {
      URL url(SEQ_HELP_URL);
      url.launchInDefaultBrowser();
      break;
   }
   case CPTID_WESITE_LAUNCH: {
      URL url(SEQ_WEBSITE_URL);
      url.launchInDefaultBrowser();  
      break;   
   }
   default:
      break;
   }
   closeDialog(true);
 
}


SeqInfoDialog::~SeqInfoDialog()
{
}
