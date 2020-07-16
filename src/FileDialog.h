/* Copyright (C) 2016 Andrew Shakinovsky
*/
#ifndef FILEDIALOG_H_
#define FILEDIALOG_H_

#include <JuceHeader.h>
#include "EditorState.h"
#include "Constants.h"
#include "CommonComponents.h"

#define CPTID_OK 10
#define CPTID_CANCEL 11


class SeqFileDialog : public SeqModalDialog, public FileBrowserListener {
   void endDialog(bool hitOk) override;   
   void notify(int id, int val) override;
   void resizedInner(Component *inner) override;
   std::unique_ptr<FileBrowserComponent> mBrowser;
   std::unique_ptr<TextButton> mBtnOk;
   std::unique_ptr<TextButton> mBtnCancel;
   std::unique_ptr<Label> mLblMain;
   std::unique_ptr<WildcardFileFilter> mFileFilter;
   LookAndFeel_V3 mLookAndFeel;

   virtual void selectionChanged() override {}
   virtual void fileClicked(const File& , const MouseEvent& ) override {}
   virtual void browserRootChanged(const File&) override {}
   void fileDoubleClicked(const File &) override;
   File mLastLocation;
public:
   int mMode; // see constants.h eg SEQ_FILE_SAVE_NOTES
   String mFileName; // will be set to the filename chosen
   void doSetup(int mode);
   SeqFileDialog(SeqGlob *glob, CptNotify *parent);
   ~SeqFileDialog();
};

#endif
