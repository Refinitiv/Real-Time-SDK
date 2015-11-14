// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global WWHFrame */
/*global WWHOutlineImagingSafe_Advance */
/*global WWHOutlineImagingSafe_CloseLevel */
/*global WWHOutlineImagingSafe_DisplayEntry */
/*global WWHOutlineImagingSafe_GenerateStyles */
/*global WWHOutlineImagingSafe_OpenLevel */
/*global WWHOutlineImagingSafe_Reset */
/*global WWHOutlineImagingSafe_RevealEntry */
/*global WWHOutlineImagingSafe_SameLevel */
/*global WWHOutlineImagingSafe_UpdateEntry */
/*global WWHOutlineImaging_GetEntryHTML */
/*global WWHOutlineImaging_GetIconURL */
/*global WWHOutlineImaging_GetLink */
/*global WWHOutlineImaging_GetPopupAction */
/*global WWHOutlineImaging_ImageSrcDir */
/*global WWHOutlineIterator_Object */
/*global WWHPopup_EventString */
/*global WWHStringBuffer_Object */

function  WWHOutlineImagingSafe_Object()
{
  this.mIterator    = new WWHOutlineIterator_Object(true);
  this.mImageSrcDir = WWHOutlineImaging_ImageSrcDir();
  this.mEventString = WWHPopup_EventString();
  this.mHTMLSegment = new WWHStringBuffer_Object();

  this.fGetIconURL     = WWHOutlineImaging_GetIconURL;
  this.fGetPopupAction = WWHOutlineImaging_GetPopupAction;
  this.fGetLink        = WWHOutlineImaging_GetLink;
  this.fGetEntryHTML   = WWHOutlineImaging_GetEntryHTML;

  this.fGenerateStyles = WWHOutlineImagingSafe_GenerateStyles;
  this.fReset          = WWHOutlineImagingSafe_Reset;
  this.fAdvance        = WWHOutlineImagingSafe_Advance;
  this.fOpenLevel      = WWHOutlineImagingSafe_OpenLevel;
  this.fCloseLevel     = WWHOutlineImagingSafe_CloseLevel;
  this.fSameLevel      = WWHOutlineImagingSafe_SameLevel;
  this.fDisplayEntry   = WWHOutlineImagingSafe_DisplayEntry;
  this.fUpdateEntry    = WWHOutlineImagingSafe_UpdateEntry;
  this.fRevealEntry    = WWHOutlineImagingSafe_RevealEntry;
}

function  WWHOutlineImagingSafe_GenerateStyles()
{
  var  StyleBuffer = new WWHStringBuffer_Object();


  StyleBuffer.fAppend("<style type=\"text/css\">\n");
  StyleBuffer.fAppend(" <!--\n");
  StyleBuffer.fAppend("  a:active\n");
  StyleBuffer.fAppend("  {\n");
  StyleBuffer.fAppend("    text-decoration: none;\n");
  StyleBuffer.fAppend("    background-color: " + WWHFrame.WWHJavaScript.mSettings.mTOC.mHighlightColor + ";\n");
  StyleBuffer.fAppend("    " + WWHFrame.WWHJavaScript.mSettings.mTOC.mFontStyle + ";\n");
  StyleBuffer.fAppend("  }\n");
  StyleBuffer.fAppend("  a:hover\n");
  StyleBuffer.fAppend("  {\n");
  StyleBuffer.fAppend("    text-decoration: underline;\n");
  StyleBuffer.fAppend("    color: " + WWHFrame.WWHJavaScript.mSettings.mTOC.mEnabledColor + ";\n");
  StyleBuffer.fAppend("    " + WWHFrame.WWHJavaScript.mSettings.mTOC.mFontStyle + ";\n");
  StyleBuffer.fAppend("  }\n");
  StyleBuffer.fAppend("  a\n");
  StyleBuffer.fAppend("  {\n");
  StyleBuffer.fAppend("    text-decoration: none;\n");
  StyleBuffer.fAppend("    color: " + WWHFrame.WWHJavaScript.mSettings.mTOC.mEnabledColor + ";\n");
  StyleBuffer.fAppend("    " + WWHFrame.WWHJavaScript.mSettings.mTOC.mFontStyle + ";\n");
  StyleBuffer.fAppend("  }\n");
  StyleBuffer.fAppend("  td\n");
  StyleBuffer.fAppend("  {\n");
  StyleBuffer.fAppend("    margin-top: 0pt;\n");
  StyleBuffer.fAppend("    margin-bottom: 0pt;\n");
  StyleBuffer.fAppend("    margin-left: 0pt;\n");
  StyleBuffer.fAppend("    margin-right: 0pt;\n");
  StyleBuffer.fAppend("    text-align: left;\n");
  StyleBuffer.fAppend("    vertical-align: middle;\n");
  StyleBuffer.fAppend("    " + WWHFrame.WWHJavaScript.mSettings.mTOC.mFontStyle + ";\n");
  StyleBuffer.fAppend("  }\n");
  StyleBuffer.fAppend(" -->\n");
  StyleBuffer.fAppend("</style>\n");

  return StyleBuffer.fGetBuffer();
}

function  WWHOutlineImagingSafe_Reset()
{
  this.mIterator.fReset(WWHFrame.WWHOutline.mTopEntry);
}

function  WWHOutlineImagingSafe_Advance(ParamMaxHTMLSegmentSize)
{
  var  Entry;


  this.mHTMLSegment.fReset();
  while (((ParamMaxHTMLSegmentSize === -1) ||
          (this.mHTMLSegment.fSize() < ParamMaxHTMLSegmentSize)) &&
         (this.mIterator.fAdvance(this)))
  {
    Entry = this.mIterator.mEntry;

    // Process current entry
    //
    if (Entry.mbShow)
    {
      this.mHTMLSegment.fAppend(this.fDisplayEntry(Entry));
    }
  }

  return (this.mHTMLSegment.fSize() > 0);  // Return true if segment created
}

function  WWHOutlineImagingSafe_OpenLevel()
{
}

function  WWHOutlineImagingSafe_CloseLevel(bParamScopeComplete)
{
}

function  WWHOutlineImagingSafe_SameLevel()
{
}

function  WWHOutlineImagingSafe_DisplayEntry(ParamEntry)
{
  var  VarEntryHTML = "";


  VarEntryHTML += this.fGetEntryHTML(ParamEntry);
  VarEntryHTML += "\n";

  return VarEntryHTML;
}

function  WWHOutlineImagingSafe_UpdateEntry(ParamEntry)
{
  // Reload page to display expanded/collapsed entry
  //
  WWHFrame.WWHJavaScript.mPanels.fReloadView();
}

function  WWHOutlineImagingSafe_RevealEntry(ParamEntry,
                                            bParamVisible)
{
  var  ParentEntry;
  var  LastClosedParentEntry = null;


  // Expand out enclosing entries
  //
  ParentEntry = ParamEntry.mParent;
  while (ParentEntry !== null)
  {
    if ( ! ParentEntry.mbExpanded)
    {
      ParentEntry.mbExpanded = true;
      LastClosedParentEntry = ParentEntry;
    }

    ParentEntry = ParentEntry.mParent;
  }

  // Set target entry
  //
  WWHFrame.WWHOutline.mPanelAnchor = "t" + ParamEntry.mID;

  // Update display
  //
  if (bParamVisible)
  {
    // Update display if entry not already visible
    //
    if (LastClosedParentEntry !== null)
    {
      this.fUpdateEntry(ParamEntry);
    }
    else
    {
      // Display target
      //
      WWHFrame.WWHJavaScript.mPanels.fJumpToAnchor();
    }
  }
}
