/* Copyright (C) 2016 Andrew Shakinovsky
*/
#include "InfoDialog.h"


// called when the dialog is ending
void SeqInfoDialog::endDialog(bool hitOk)
{
   
}


#define S_GAP     4 // gap between items
#define S_HEIGHT  22
#define S_BTNWIDTH  120
#define S_BORDER  5  // size of border gap

// this is called when the inner box is resized (ie we need to do our resize here)
void SeqInfoDialog::resizedInner(Component * inner)
{
   Rectangle<int> r;
   Rectangle<int> bottom, top, btns, tmp, tmp2;

   // inner panel
   r = inner->getLocalBounds();   
   r.reduce(S_BORDER*2, S_BORDER*2); // have a bit of a space all around

   // space for the label at the top
   top = r.removeFromTop(S_HEIGHT);
   mLblMain->setBounds(top);

   btns=r.removeFromBottom(S_HEIGHT);
   tmp=btns.removeFromRight(S_BTNWIDTH);
   mBtnOk->setBounds(tmp);
   tmp.removeFromRight(S_GAP);
   tmp=btns.removeFromRight(S_BTNWIDTH);
   mBtnWebsite->setBounds(tmp);
   tmp.removeFromRight(S_GAP);
   tmp=btns.removeFromRight(S_BTNWIDTH);
   mBtnHelp->setBounds(tmp);
   
   
   // leave some space between tabs and other
   r.removeFromTop(S_GAP);
   r.removeFromBottom(S_GAP);
   mLblDescription->setBounds(r);

   

   

}

SeqInfoDialog::SeqInfoDialog(SeqGlob * glob, CptNotify *parent) :
   SeqModalDialog(glob, SEQCTL_INFODIALOG,parent, 640,200)
{
   Colour txtColor, bgColor, hlColor;
   
   bgColor = mGlob->mEditorState->getColorFor(EditorState::background);   
   txtColor = bgColor.contrasting(0.5f);

   // big label at top
   mLblMain = std::unique_ptr<Label>(addStdLabel( "Stochas Information")); 
   mLblMain->setFont(Font(26.0f, Font::bold));
   mLblMain->setJustificationType(Justification::centred);
   
   // big descr
   mLblDescription = std::unique_ptr<Label>(addStdLabel(
      
      "Stochas is an open-source plugin available on Windows and Mac, which was originally developed as commercial software in 2016 by Andrew Shakinovsky. "
      "It was converted over to OSS in July of 2020. For support, "
      "please email me at " SEQ_HELP_EMAIL " (no guarantee on my ability to respond in a timely manner.) "
      "Please use the below links to access web resources."
   
   ));
   mLblDescription->setFont(Font(18.0f, Font::plain));
   mLblDescription->setJustificationType(Justification::horizontallyJustified);
   mLblDescription->setMinimumHorizontalScale(1.0f);

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
