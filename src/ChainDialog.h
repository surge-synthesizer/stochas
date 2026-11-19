/***************************************************************
 ** Copyright (C) 2026 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#ifndef CHAINDIALOG_H_
#define CHAINDIALOG_H_

#include "EditorState.h"
#include "Constants.h"
#include "StepPanel.h"

// one chain relationship as seen from the cell the dialog is about
struct SeqChainItem {
   int row;       // the cell at the other end of the chain
   int col;
   bool negSrc;   // chain fires when the source did NOT trigger
   bool negTgt;   // chain prevents the target from triggering
   bool operator==(const SeqChainItem &o) const {
      return row == o.row && col == o.col && negSrc == o.negSrc && negTgt == o.negTgt;
   }
};

class SeqChainDialog;

class SeqChainToggle : public TextButton {
public:
   enum Kind { source, target };
   SeqChainToggle(SeqGlob *glob, Kind kind, bool sources);
   void setOn(bool on);
   bool isOn() const { return getToggleState(); }
   void paintButton(Graphics &g, bool highlighted, bool down) override;
private:
   SeqGlob *mGlob;
   Kind mKind;
   bool mSources;
};

// the ui for a single list item
class SeqChainRow : public Component {
public:
   SeqChainRow(SeqGlob *glob, SeqChainDialog *dlg, bool sources);
   void setItem(int index, const SeqChainItem &item, const String &text);
   void resized() override;
   void paint(Graphics &g) override;
private:
   SeqGlob *mGlob;
   SeqChainDialog *mDlg;
   bool mSources;
   int mIndex;
   String mText;
   ImageButton mBtnDelete;
   SeqChainToggle mTglSrc;
   SeqChainToggle mTglTgt;
};

// one of the two columns
class SeqChainList : public Component, public ListBoxModel {
public:
   SeqChainList(SeqGlob *glob, SeqChainDialog *dlg, bool sources);
   void setItems(const Array<SeqChainItem> &items);
   const Array<SeqChainItem> &getItems() const { return mItems; }
   void deselect() { mList.deselectAllRows(); }
   void selectItem(int idx) { mList.selectRow(idx, false, true); }
   int getSelectedItem() { return mList.getSelectedRow(); }
   void resized() override;
   void paint(Graphics &g) override;
   void refreshColors();

   int getNumRows() override;
   void paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override;
   Component *refreshComponentForRow(int rowNumber, bool isRowSelected,
      Component *existingComponentToUpdate) override;
   void listBoxItemClicked(int row, const MouseEvent &) override;
private:
   SeqGlob *mGlob;
   SeqChainDialog *mDlg;
   bool mSources;
   Label mHeader;
   ListBox mList;
   Array<SeqChainItem> mItems;
};

/*
Non-modal panel shown below the grid in chain mode. It shows the chains that lead to (sources)
and from (targets) a single cell and allows editing/deleting them
*/
class SeqChainDialog : public Component, public Button::Listener {
   friend class SeqChainRow;
   friend class SeqChainList;
   SeqGlob *mGlob;
   StepPanel *mStepPanel;
   int mLayer;
   int mPattern;
   int mRow;   // the cell we are about
   int mCol;
   Label mLblHeader;
   TextButton mBtnClose;
   SeqChainList mSources;
   SeqChainList mTargets;

   void buttonClicked(Button *) override;
   void resized() override;
   void paint(Graphics &g) override;

   // read chain data for our cell
   void gather(Array<SeqChainItem> &sources, Array<SeqChainItem> &targets);
   // is the dialog still relevant (mode, layer, cell, selection)?
   bool isStillValid();
   String describeCell(int row, int col);
   void updateHeader();

   // used by the rows/lists
   void itemChosen(bool sources, int idx);
   void itemDelete(bool sources, int idx);
   void itemToggle(bool sources, int idx, bool negSrc, bool negTgt);
   void showHighlight(bool sources, const SeqChainItem &item);
   // all lists should be rebuilt from data
   void refresh();

public:
   SeqChainDialog(SeqGlob *glob, StepPanel *stepPanel);
   ~SeqChainDialog();
   // show for a cell
   void openFor(int row, int col);
   // follow a grid click only when the panel is already open
   void followCell(int row, int col);
   void closeDialog();
   // call periodically; closes if no longer relevant and updates lists if the data changed
   void check();
};

#endif
