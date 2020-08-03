/***************************************************************
 ** Copyright (C) 2016 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "FileDialog.h"
void SeqFileDialog::endDialog(bool hitOk)
{   
   if (hitOk) {
      if (mBrowser->getNumSelectedFiles() > 0) {
         mLastLocation = mBrowser->getSelectedFile(0).getParentDirectory();
         mGlob->mEditorState->setFileDirectory(mLastLocation.getFullPathName());
         mFileName = mBrowser->getSelectedFile(0).getFullPathName();
         notifyParent(0);
      }
   }
}


void SeqFileDialog::notify(int id, int /*val*/) {
   switch (id) {
   case CPTID_OK:
      closeDialog(true);
      break;
   case CPTID_CANCEL:
      closeDialog(false);
      break;
   default:
      jassertfalse;
   }
}

#define S_GAP     4 // gap between items
#define S_HEIGHT  22
#define S_BORDER  2  // size of border gap
#define S_TAB     80 // tab size (where labels are involved)
#define S_LISTWIDTH 50
void SeqFileDialog::resizedInner(Component * inner) {
   Rectangle<int> r = inner->getLocalBounds();
   Rectangle<int> btns, top;
   Rectangle<int> tmp;
   r.reduce(8, 8);
   btns = r.removeFromBottom(S_HEIGHT);
   top = r.removeFromTop(S_HEIGHT);
   mLblMain->setBounds(top);
   if(mBrowser)
      mBrowser->setBounds(r);

   // buttons
   tmp = btns.removeFromRight(60);
   mBtnCancel->setBounds(tmp);
   btns.removeFromRight(S_GAP);
   tmp = btns.removeFromRight(60);
   mBtnOk->setBounds(tmp);

}

void SeqFileDialog::fileDoubleClicked(const File &)
{
   closeDialog(true);
}

void SeqFileDialog::doSetup(int mode)
{
   juce::Colour bg = mGlob->mEditorState->getColorFor(EditorState::background);
   juce::Colour fg = bg.contrasting(.5f);
   juce::Colour hl = mGlob->mEditorState->getColorFor(EditorState::coloredElements::toggleOff);

   bool saveMode = false;
   String txt;
   
   if (mBrowser) {
      removeFromInner(mBrowser.get());
      // TODO should we do the proper thing here with unique_ptr?
      delete mBrowser.release();
   }

   // this needs to stick around since the browser object sticks around and its referenced by that
   if (mFileFilter) {
      // TODO i switched this to unique_ptr, is there a better way?
      delete mFileFilter.release();
   }

   mMode = mode; // this is public and used by plugineditor to get results
   switch (mode) {
   case SEQ_FILE_SAVE_NOTES:
      txt = "Save note information to file";
      mFileFilter = std::unique_ptr<WildcardFileFilter>( new WildcardFileFilter("*.stnote", "", "note files"));
      saveMode = true;
      break;
   case SEQ_FILE_LOAD_NOTES:
      mFileFilter = std::unique_ptr<WildcardFileFilter>(new WildcardFileFilter("*.stnote", "", "note files"));
      txt = "Load note information from file";
      saveMode = false;
      break;
   case SEQ_FILE_LOAD_MIDI:
      mFileFilter = std::unique_ptr<WildcardFileFilter>(new WildcardFileFilter("*.*", "", "midi files"));
      txt = "Import groove from MIDI file";
      saveMode = false;
      break;
   case SEQ_FILE_SAVE_MIDI:
      mFileFilter = std::unique_ptr<WildcardFileFilter>(new WildcardFileFilter("*.*", "", "midi files"));
      txt = "Export groove to MIDI file";
      saveMode = true;
      break;
   case SEQ_FILE_LOAD_PATCH:
      mFileFilter = std::unique_ptr<WildcardFileFilter>(new WildcardFileFilter("*.stochas", "", "note files"));
      txt = "Load Stochas patch";
      saveMode = false;
      break;
   case SEQ_FILE_SAVE_PATCH:
      mFileFilter = std::unique_ptr<WildcardFileFilter>(new WildcardFileFilter("*.stochas", "", "note files"));
      txt = "Save Stochas patch";
      saveMode = true;
      break;
   default:
      jassertfalse;
   }


   int fflags = 0;
   if (saveMode)
      fflags = FileBrowserComponent::FileChooserFlags::saveMode;
   else
      fflags = FileBrowserComponent::FileChooserFlags::openMode;
   fflags |= FileBrowserComponent::FileChooserFlags::canSelectFiles;

   mLblMain->setText(txt, juce::NotificationType::dontSendNotification);

   mBrowser = std::unique_ptr<FileBrowserComponent>(new FileBrowserComponent( fflags ,mLastLocation, mFileFilter.get(), 0)); 
   mBrowser->setLookAndFeel(&mLookAndFeel);
   mBrowser->getLookAndFeel().setColour(DirectoryContentsDisplayComponent::textColourId, fg);
   mBrowser->getLookAndFeel().setColour(DirectoryContentsDisplayComponent::highlightColourId, hl);
   fixColors(mGlob->mEditorState, mBrowser.get());
   addToInner(0, *mBrowser);
   mBrowser->setVisible(true);
   mBrowser->addListener(this);
   forceResize();
}

SeqFileDialog::SeqFileDialog(SeqGlob * glob, CptNotify * parent) :
   SeqModalDialog(glob, SEQCTL_FILEDIALOG, parent, 600, 500) 
{

   
   mBtnOk = std::unique_ptr<TextButton>(addStdButton("Ok", 0, CPTID_OK));
   mBtnCancel = std::unique_ptr<TextButton>(addStdButton("Cancel", 0, CPTID_CANCEL));

   mLblMain = std::unique_ptr<Label>(addStdLabel("Select a file")); // will be changed as appropriate
   mLblMain->setFont(Font(20.0f, Font::plain));
   mLblMain->setJustificationType(Justification::centred);
   // initial directory
   String dir = mGlob->mEditorState->getFileDirectory();
   if (dir.length())
      mLastLocation = File(dir);
   else
      mLastLocation = File::getSpecialLocation(File::userHomeDirectory);
}


SeqFileDialog::~SeqFileDialog() {
   // this avoids an assert related to look and feel (also probably a memory leak)
   // TODO is this still the case with new JUCE? also I switched this from ScopedPointer to unique_ptr
   if (mBrowser) {
      delete mBrowser.release();
   }
}
