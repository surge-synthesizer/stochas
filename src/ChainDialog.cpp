/***************************************************************
 ** Copyright (C) 2026 by Andrew Shakinovsky
 **
 ** You may also use this code under the terms of the 
 ** GPL v3 (see www.gnu.org/licenses).
 ** STOCHAS IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL 
 ** WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING 
 ** MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE DISCLAIMED.
 ***************************************************************/

#include "ChainDialog.h"
#include "Scale.h"

#define SEQCHAIN_ROW_H      24
#define SEQCHAIN_SOURCE_W   145
#define SEQCHAIN_TARGET_W   120
#define SEQCHAIN_HDR_H      22

/*===========================================================================================
SeqChainToggle
============================================================================================*/
SeqChainToggle::SeqChainToggle(SeqGlob *glob, Kind kind, bool sources) :
   mGlob(glob), mKind(kind), mSources(sources)
{
   setClickingTogglesState(true);
   setMouseCursor(MouseCursor::PointingHandCursor);
   setName(kind == source ? "chainSourceToggle" : "chainTargetToggle");
   setOn(true);
}

void SeqChainToggle::setOn(bool on)
{
   setToggleState(on, dontSendNotification);
   if (mKind == source) {
      setButtonText(mSources ? (on ? "source plays" : "source does not play")
                             : (on ? "cell plays" : "cell does not play"));
   }
   else {
      setButtonText(mSources ? (on ? "cell triggered" : "cell blocked")
                             : (on ? "target triggered" : "target blocked"));
   }
   repaint();
}

void SeqChainToggle::paintButton(Graphics &g, bool highlighted, bool down)
{
   EditorState *e = mGlob->mEditorState;
   Colour c = e->getColorFor(isOn() ? EditorState::chainPositive : EditorState::chainNegative);
   Rectangle<float> r = getLocalBounds().toFloat().reduced(0.5f);
   g.setColour(c);
   g.fillRoundedRectangle(r, 2.0f);
   g.setColour(c.contrasting(highlighted || down || hasKeyboardFocus(true) ? 1.0f : 0.9f));
   g.drawRoundedRectangle(r, 2.0f, highlighted || down || hasKeyboardFocus(true) ? 1.5f : 0.5f);
   g.setFont(Font(FontOptions(12.0f)));
   g.drawFittedText(getButtonText(), getLocalBounds().reduced(3, 0), Justification::centred, 1);
}

/*===========================================================================================
SeqChainRow
============================================================================================*/
SeqChainRow::SeqChainRow(SeqGlob *glob, SeqChainDialog *dlg, bool sources) :
   mGlob(glob), mDlg(dlg), mSources(sources), mIndex(-1),
   mTglSrc(glob, SeqChainToggle::source, sources), mTglTgt(glob, SeqChainToggle::target, sources)
{
   // let clicks on empty areas fall through to the list box so it can select the row
   setInterceptsMouseClicks(false, true);

   addAndMakeVisible(mBtnDelete);
   mBtnDelete.setName("chainDelete");
   mBtnDelete.onClick = [this]() {
      // deleting rebuilds the list (and this component), so defer it
      Component::SafePointer<SeqChainDialog> d(mDlg);
      const bool src = mSources;
      const int idx = mIndex;
      const int layer = mDlg->mLayer, pattern = mDlg->mPattern;
      const int row = mDlg->mRow, col = mDlg->mCol;
      const auto &items = src ? mDlg->mSources.getItems() : mDlg->mTargets.getItems();
      if (idx < 0 || idx >= items.size())
         return;
      const SeqChainItem item = items[idx];
      MessageManager::callAsync([d, src, idx, layer, pattern, row, col, item]() {
         if (d == nullptr || !d->isVisible() || d->mLayer != layer || d->mPattern != pattern ||
            d->mRow != row || d->mCol != col ||
            d->mGlob->mEditorState->getCurrentLayer() != layer ||
            d->mGlob->mSeqBuf->getUISeqData()->getLayer(layer)->getCurrentPattern() != pattern)
            return;
         const auto &current = src ? d->mSources.getItems() : d->mTargets.getItems();
         if (idx < current.size() && current[idx].row == item.row && current[idx].col == item.col)
            d->itemDelete(src, idx);
      });
   };
   addAndMakeVisible(mTglSrc);
   addAndMakeVisible(mTglTgt);
   auto toggled = [this]() {
      mDlg->itemToggle(mSources, mIndex, !mTglSrc.isOn(), !mTglTgt.isOn());
   };
   mTglSrc.onClick = [this, toggled]() { mTglSrc.setOn(mTglSrc.isOn()); toggled(); };
   mTglTgt.onClick = [this, toggled]() { mTglTgt.setOn(mTglTgt.isOn()); toggled(); };
}

void SeqChainRow::setItem(int index, const SeqChainItem &item, const String &text)
{
   mIndex = index;
   mText = text;
   mTglSrc.setOn(!item.negSrc);
   mTglTgt.setOn(!item.negTgt);

   EditorState *e = mGlob->mEditorState;
   Colour c = e->getColorFor(EditorState::button);
   Colour cd = e->getColorFor(EditorState::toggleOn);
   Image img = ImageCache::getFromMemory(SeqImageX::xmark464_png, SeqImageX::xmark464_pngSize);
   mBtnDelete.setImages(false, true, true, img, 1.0, c, img, 1.0, c, img, 1.0, cd);
   repaint();
}

void SeqChainRow::resized()
{
   Rectangle<int> r = getLocalBounds().reduced(2, 2);
   mBtnDelete.setBounds(r.removeFromLeft(SEQCHAIN_ROW_H - 4));
   const int buttonWidth = jmin(r.getWidth(), SEQCHAIN_SOURCE_W + SEQCHAIN_TARGET_W);
   const int targetWidth = buttonWidth * SEQCHAIN_TARGET_W / (SEQCHAIN_SOURCE_W + SEQCHAIN_TARGET_W);
   mTglTgt.setBounds(r.removeFromRight(targetWidth).reduced(1, 0));
   mTglSrc.setBounds(r.removeFromRight(buttonWidth - targetWidth).reduced(1, 0));
}

void SeqChainRow::paint(Graphics &g)
{
   Colour bg = mGlob->mEditorState->getColorFor(EditorState::background);
   g.setColour(bg.contrasting(.8f));
   g.setFont(Font(15.0f, Font::plain));
   Rectangle<int> r = getLocalBounds();
   r.removeFromLeft(SEQCHAIN_ROW_H + 4);
   r.setRight(mTglSrc.getX() - 2);
   g.drawText(mText, r, Justification::centredLeft, true);
}

/*===========================================================================================
SeqChainList
============================================================================================*/
SeqChainList::SeqChainList(SeqGlob *glob, SeqChainDialog *dlg, bool sources) :
   mGlob(glob), mDlg(dlg), mSources(sources), mList("tblChain", this)
{
   setName(sources ? "chainSources" : "chainTargets");
   mHeader.setText(sources ? "Sources" : "Targets", dontSendNotification);
   mHeader.setFont(Font(16.0f, Font::bold));
   mHeader.setJustificationType(Justification::centredLeft);
   addAndMakeVisible(mHeader);
   addAndMakeVisible(mList);
   mList.setRowHeight(SEQCHAIN_ROW_H);
   mList.setOutlineThickness(1);
   mList.setMultipleSelectionEnabled(false);
   refreshColors();
}

void SeqChainList::refreshColors()
{
   EditorState *e = mGlob->mEditorState;
   Colour bg = e->getColorFor(EditorState::background);
   mList.setColour(ListBox::backgroundColourId, bg);
   mList.setColour(ListBox::outlineColourId, e->getColorFor(EditorState::border));
   mHeader.setColour(Label::textColourId, bg.contrasting(.8f));
}

void SeqChainList::setItems(const Array<SeqChainItem> &items)
{
   mItems = items;
   mList.updateContent();
   mList.repaint();
}

void SeqChainList::resized()
{
   Rectangle<int> r = getLocalBounds();
   mHeader.setBounds(r.removeFromTop(SEQCHAIN_HDR_H));
   mList.setBounds(r);
}

void SeqChainList::paint(Graphics &) {}

int SeqChainList::getNumRows()
{
   return mItems.size();
}

void SeqChainList::paintListBoxItem(int, Graphics &g, int, int, bool rowIsSelected)
{
   EditorState *e = mGlob->mEditorState;
   Colour bg = e->getColorFor(EditorState::background);
   g.fillAll(rowIsSelected ? e->getColorFor(EditorState::cellSelection).withAlpha(0.35f) : bg);
}

Component *SeqChainList::refreshComponentForRow(int rowNumber, bool, Component *existing)
{
   if (rowNumber >= mItems.size()) {
      delete existing;
      return nullptr;
   }
   SeqChainRow *row = static_cast<SeqChainRow *>(existing);
   if (row == nullptr)
      row = new SeqChainRow(mGlob, mDlg, mSources);
   const SeqChainItem &item = mItems.getReference(rowNumber);
   row->setItem(rowNumber, item, mDlg->describeCell(item.row, item.col));
   return row;
}

void SeqChainList::listBoxItemClicked(int row, const MouseEvent &)
{
   if (row >= 0 && row < mItems.size())
      mDlg->itemChosen(mSources, row);
}

/*===========================================================================================
SeqChainDialog
============================================================================================*/
SeqChainDialog::SeqChainDialog(SeqGlob *glob, StepPanel *stepPanel) :
   mGlob(glob), mStepPanel(stepPanel), mLayer(-1), mPattern(-1), mRow(-1), mCol(-1),
   mSources(glob, this, true), mTargets(glob, this, false)
{
   setName("chainDialog");
   mLblHeader.setFont(Font(16.0f, Font::bold));
   mLblHeader.setJustificationType(Justification::centredLeft);
   addAndMakeVisible(mLblHeader);
   mBtnClose.setButtonText("Close");
   mBtnClose.setWantsKeyboardFocus(false);
   mBtnClose.addListener(this);
   addAndMakeVisible(mBtnClose);
   addAndMakeVisible(mSources);
   addAndMakeVisible(mTargets);
   setVisible(false);
}

SeqChainDialog::~SeqChainDialog() {}

void SeqChainDialog::paint(Graphics &g)
{
   EditorState *e = mGlob->mEditorState;
   Colour bg = e->getColorFor(EditorState::background);
   g.fillAll(bg);
   g.setColour(e->getColorFor(EditorState::border));
   g.drawRect(getLocalBounds(), 1);
   mLblHeader.setColour(Label::textColourId, bg.contrasting(.8f));
   mSources.refreshColors();
   mTargets.refreshColors();
}

void SeqChainDialog::resized()
{
   Rectangle<int> r = getLocalBounds().reduced(SEQ_SIZE_MAIN_BORDER);
   Rectangle<int> top = r.removeFromTop(SEQCHAIN_HDR_H + 2);
   mBtnClose.setBounds(top.removeFromRight(70));
   mLblHeader.setBounds(top);
   r.removeFromTop(SEQ_SIZE_MAIN_BORDER);
   int half = r.getWidth() / 2;
   mSources.setBounds(r.removeFromLeft(half - SEQ_SIZE_MAIN_BORDER));
   r.removeFromLeft(SEQ_SIZE_MAIN_BORDER * 2);
   mTargets.setBounds(r);
}

void SeqChainDialog::buttonClicked(Button *)
{
   closeDialog();
}

String SeqChainDialog::describeCell(int row, int col)
{
   EditorState *e = mGlob->mEditorState;
   SequenceLayer *lay = mGlob->mSeqBuf->getUISeqData()->getLayer(mLayer);
   char buf[SEQ_NOTE_NAME_MAXLEN];
   int8_t note = lay->getCurNote(row);
   String name = (note < 0) ? String("Off") :
      String(SeqScale::getMidiNoteName(note, e->getLowestOctave(), buf, e->isShowMidiNumbers()));
   return name + String::formatted("  -  step %d", col + 1);
}

void SeqChainDialog::updateHeader()
{
   mLblHeader.setText("Chains for cell: " + describeCell(mRow, mCol), dontSendNotification);
}

void SeqChainDialog::gather(Array<SeqChainItem> &sources, Array<SeqChainItem> &targets)
{
   SequenceLayer *lay = mGlob->mSeqBuf->getUISeqData()->getLayer(mLayer);
   int r, c, it;
   bool negTgt, negSrc;
   it = -1;
   while (lay->getChainSource(mRow, mCol, &it, &r, &c, &negTgt, &negSrc))
      sources.add({ r, c, negSrc, negTgt });
   it = -1;
   while (lay->getChainTarget(mRow, mCol, &it, &r, &c, &negTgt, &negSrc))
      targets.add({ r, c, negSrc, negTgt });
}

void SeqChainDialog::openFor(int row, int col)
{
   mLayer = mGlob->mEditorState->getCurrentLayer();
   mPattern = mGlob->mSeqBuf->getUISeqData()->getLayer(mLayer)->getCurrentPattern();
   mRow = row;
   mCol = col;
   mStepPanel->setChainDialogCell(row, col);
   mStepPanel->clearChainHighlight();
   mSources.deselect();
   mTargets.deselect();
   updateHeader();
   refresh();
   setVisible(true);
   toFront(false);
   repaint();
}

void SeqChainDialog::followCell(int row, int col)
{
   if (isVisible() && mGlob->mEditorState->getEditMode() == EditorState::editingChain &&
      mLayer == mGlob->mEditorState->getCurrentLayer() &&
      mPattern == mGlob->mSeqBuf->getUISeqData()->getLayer(mLayer)->getCurrentPattern() &&
      (mRow != row || mCol != col))
      openFor(row, col);
}

void SeqChainDialog::closeDialog()
{
   mStepPanel->clearChainDialogCell();
   mStepPanel->clearChainHighlight();
   mRow = mCol = mLayer = mPattern = -1;
   setVisible(false);
}

bool SeqChainDialog::isStillValid()
{
   EditorState *e = mGlob->mEditorState;
   if (e->getEditMode() != EditorState::editingChain || e->getCurrentLayer() != mLayer)
      return false;
   int lc, hc, lr, hr;
   if (e->getNumSelectedCells() != 1 || !e->getSelectedCells(&lc, &hc, &lr, &hr) ||
      ((lc != mCol || lr != mRow) && !mStepPanel->isChainGestureInProgress()))
      return false;
   SequenceLayer *lay = mGlob->mSeqBuf->getUISeqData()->getLayer(mLayer);
   return lay->getCurrentPattern() == mPattern &&
      mCol < lay->getNumSteps() && lay->getProb(mRow, mCol) != SEQ_PROB_OFF;
}

void SeqChainDialog::refresh()
{
   Array<SeqChainItem> src, tgt;
   gather(src, tgt);
   // keep highlight on the same chain if it is still there
   int selSrc = mSources.getSelectedItem();
   int selTgt = mTargets.getSelectedItem();
   SeqChainItem keepSrc = selSrc >= 0 ? mSources.getItems()[selSrc] : SeqChainItem{ -1, -1, false, false };
   SeqChainItem keepTgt = selTgt >= 0 ? mTargets.getItems()[selTgt] : SeqChainItem{ -1, -1, false, false };

   mSources.setItems(src);
   mTargets.setItems(tgt);

   bool highlighted = false;
   for (int i = 0; i < src.size(); i++)
      if (selSrc >= 0 && src[i].row == keepSrc.row && src[i].col == keepSrc.col) {
         mSources.selectItem(i);
         highlighted = true;
      }
   for (int i = 0; i < tgt.size(); i++)
      if (selTgt >= 0 && tgt[i].row == keepTgt.row && tgt[i].col == keepTgt.col) {
         mTargets.selectItem(i);
         highlighted = true;
      }
   if (!highlighted)
      mStepPanel->clearChainHighlight();
}

void SeqChainDialog::check()
{
   if (!isVisible())
      return;
   if (!isStillValid()) {
      closeDialog();
      return;
   }
   Array<SeqChainItem> src, tgt;
   gather(src, tgt);
   if (src != mSources.getItems() || tgt != mTargets.getItems())
      refresh();
}

void SeqChainDialog::showHighlight(bool sources, const SeqChainItem &item)
{
   if (sources)
      mStepPanel->setChainHighlight(item.row, item.col, mRow, mCol, item.row, item.col);
   else
      mStepPanel->setChainHighlight(mRow, mCol, item.row, item.col, item.row, item.col);
}

void SeqChainDialog::itemChosen(bool sources, int idx)
{
   SeqChainList &me = sources ? mSources : mTargets;
   SeqChainList &other = sources ? mTargets : mSources;
   other.deselect();
   if (idx >= 0 && idx < me.getItems().size())
      showHighlight(sources, me.getItems()[idx]);
}

void SeqChainDialog::itemDelete(bool sources, int idx)
{
   if (!isVisible())
      return;
   SeqChainList &me = sources ? mSources : mTargets;
   if (idx < 0 || idx >= me.getItems().size())
      return;
   const SeqChainItem it = me.getItems()[idx];
   SequenceLayer *lay = mGlob->mSeqBuf->getUISeqData()->getLayer(mLayer);
   if (sources)
      lay->removeChainSource(mRow, mCol, it.row, it.col);
   else
      lay->removeChainSource(it.row, it.col, mRow, mCol);
   mGlob->mSeqBuf->swap();
   mStepPanel->clearChainHighlight();
   me.deselect();
   refresh();
   mStepPanel->repaint();
}

void SeqChainDialog::itemToggle(bool sources, int idx, bool negSrc, bool negTgt)
{
   SeqChainList &me = sources ? mSources : mTargets;
   if (idx < 0 || idx >= me.getItems().size())
      return;
   const SeqChainItem it = me.getItems()[idx];
   SequenceLayer *lay = mGlob->mSeqBuf->getUISeqData()->getLayer(mLayer);
   if (sources)
      lay->addChainSource(mRow, mCol, it.row, it.col, negTgt, negSrc);
   else
      lay->addChainSource(it.row, it.col, mRow, mCol, negTgt, negSrc);
   mGlob->mSeqBuf->swap();
   refresh();
   mStepPanel->repaint();
}
