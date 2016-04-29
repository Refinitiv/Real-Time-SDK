// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global WWHFrame */
/*global WWHOutlineImagingFast_Advance */
/*global WWHOutlineImagingFast_CloseLevel */
/*global WWHOutlineImagingFast_DisplayEntry */
/*global WWHOutlineImagingFast_GenerateStyles */
/*global WWHOutlineImagingFast_OpenLevel */
/*global WWHOutlineImagingFast_Reset */
/*global WWHOutlineImagingFast_RevealEntry */
/*global WWHOutlineImagingFast_SameLevel */
/*global WWHOutlineImagingFast_UpdateEntry */
/*global WWHOutlineImaging_GetEntryHTML */
/*global WWHOutlineImaging_GetIconURL */
/*global WWHOutlineImaging_GetLink */
/*global WWHOutlineImaging_GetPopupAction */
/*global WWHOutlineImaging_ImageSrcDir */
/*global WWHOutlineIterator_Object */
/*global WWHPopup_EventString */
/*global WWHStringBuffer_Object */

function  WWHOutlineImagingFast_Object()
{
  this.mIterator    = new WWHOutlineIterator_Object(true);
  this.mImageSrcDir = WWHOutlineImaging_ImageSrcDir();
  this.mEventString = WWHPopup_EventString();
  this.mHTMLSegment = new WWHStringBuffer_Object();
  this.mbUseList    = true;

  this.fGetIconURL     = WWHOutlineImaging_GetIconURL;
  this.fGetPopupAction = WWHOutlineImaging_GetPopupAction;
  this.fGetLink        = WWHOutlineImaging_GetLink;
  this.fGetEntryHTML   = WWHOutlineImaging_GetEntryHTML;

  this.fGenerateStyles = WWHOutlineImagingFast_GenerateStyles;
  this.fReset          = WWHOutlineImagingFast_Reset;
  this.fAdvance        = WWHOutlineImagingFast_Advance;
  this.fOpenLevel      = WWHOutlineImagingFast_OpenLevel;
  this.fCloseLevel     = WWHOutlineImagingFast_CloseLevel;
  this.fSameLevel      = WWHOutlineImagingFast_SameLevel;
  this.fDisplayEntry   = WWHOutlineImagingFast_DisplayEntry;
  this.fUpdateEntry    = WWHOutlineImagingFast_UpdateEntry;
  this.fRevealEntry    = WWHOutlineImagingFast_RevealEntry;

  // Workaround for Windows IE
  //
  if ((WWHFrame.WWHBrowser.mBrowser === 2) &&  // Shorthand for IE
      (WWHFrame.WWHBrowser.mPlatform === 1))   // Shorthand for Windows
  {
    this.mbUseList = false;
  }
}

function  WWHOutlineImagingFast_GenerateStyles()
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
  if (this.mbUseList)
  {
    StyleBuffer.fAppend("  ul\n");
    StyleBuffer.fAppend("  {\n");
    StyleBuffer.fAppend("    list-style-type: none;\n");
    StyleBuffer.fAppend("    padding-left: 0pt;\n");
    StyleBuffer.fAppend("    padding-right: 0pt;\n");
    StyleBuffer.fAppend("    margin-top: 0pt;\n");
    StyleBuffer.fAppend("    margin-bottom: 0pt;\n");
    StyleBuffer.fAppend("    margin-left: 0pt;\n");
    StyleBuffer.fAppend("    margin-right: 0pt;\n");
    StyleBuffer.fAppend("  }\n");
    StyleBuffer.fAppend("  li\n");
    StyleBuffer.fAppend("  {\n");
    StyleBuffer.fAppend("    margin-top: 0pt;\n");
    StyleBuffer.fAppend("    margin-bottom: 0pt;\n");
    StyleBuffer.fAppend("    margin-left: 0pt;\n");
    StyleBuffer.fAppend("    margin-right: 0pt;\n");
    StyleBuffer.fAppend("  }\n");
  }
  else
  {
    StyleBuffer.fAppend("  div.list\n");
    StyleBuffer.fAppend("  {\n");
    StyleBuffer.fAppend("    margin-top: 0pt;\n");
    StyleBuffer.fAppend("    margin-bottom: 0pt;\n");
    StyleBuffer.fAppend("    margin-left: 0pt;\n");
    StyleBuffer.fAppend("    margin-right: 0pt;\n");
    StyleBuffer.fAppend("  }\n");
  }
  StyleBuffer.fAppend(" -->\n");
  StyleBuffer.fAppend("</style>\n");

  return StyleBuffer.fGetBuffer();
}

function  WWHOutlineImagingFast_Reset()
{
  this.mIterator.fReset(WWHFrame.WWHOutline.mTopEntry);
}

function  WWHOutlineImagingFast_Advance(ParamMaxHTMLSegmentSize)
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
      if (this.mbUseList)
      {
        this.mHTMLSegment.fAppend("<li id=i" + Entry.mID + ">");
        this.mHTMLSegment.fAppend(this.fDisplayEntry(Entry));
      }
      else
      {
        if (Entry.mChildren !== null)
        {
          this.mHTMLSegment.fAppend("<div class=\"list\" id=l" + Entry.mID + ">\n");
          this.mHTMLSegment.fAppend(this.fDisplayEntry(Entry));
          if ( ! Entry.mbExpanded)
          {
            this.mHTMLSegment.fAppend("</div>\n");
          }
        }
        else
        {
          this.mHTMLSegment.fAppend(this.fDisplayEntry(Entry));
        }
      }
    }
  }

  return (this.mHTMLSegment.fSize() > 0);  // Return true if segment created
}

function  WWHOutlineImagingFast_OpenLevel()
{
  if (this.mbUseList)
  {
    this.mHTMLSegment.fAppend("<ul>\n");
  }
}

function  WWHOutlineImagingFast_CloseLevel(bParamScopeComplete)
{
  if (this.mbUseList)
  {
    this.mHTMLSegment.fAppend("</li>\n");
    this.mHTMLSegment.fAppend("</ul>\n");
    if ( ! bParamScopeComplete)
    {
      this.mHTMLSegment.fAppend("</li>\n");
    }
  }
  else
  {
    if ( ! bParamScopeComplete)
    {
      this.mHTMLSegment.fAppend("</div>\n");
    }
  }
}

function  WWHOutlineImagingFast_SameLevel()
{
  if (this.mbUseList)
  {
    this.mHTMLSegment.fAppend("</li>\n");
  }
}

function  WWHOutlineImagingFast_DisplayEntry(ParamEntry)
{
  var  VarEntryHTML = "";


  if (this.mbUseList)
  {
    VarEntryHTML += this.fGetEntryHTML(ParamEntry) + "\n";
  }
  else
  {
    VarEntryHTML += "<div id=i" + ParamEntry.mID + ">" + this.fGetEntryHTML(ParamEntry) + "</div>\n";
  }

  return VarEntryHTML;
}

function  WWHOutlineImagingFast_UpdateEntry(ParamEntry)
{
  var  EntryHTML = "";
  var  ElementID;
  var  VarPanelViewFrame;


  // Get entry display
  //
  EntryHTML = this.fDisplayEntry(ParamEntry);

  // Reset iterator to process current entry's children
  //
  this.mIterator.fReset(ParamEntry);

  // Process display of children
  // Result already stored in this.mHTMLSegment
  //
  this.fAdvance(-1);

  // Close down any popups we had going to prevent JavaScript errors
  //
  WWHFrame.WWHJavaScript.mPanels.mPopup.fHide();

  // Update HTML
  //
  VarPanelViewFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHPanelViewFrame"));
  if (this.mbUseList)
  {
    ElementID = "i" + ParamEntry.mID;
  }
  else
  {
    ElementID = "l" + ParamEntry.mID;
  }
  if ((WWHFrame.WWHBrowser.mBrowser === 2) ||  // Shorthand for IE
      (WWHFrame.WWHBrowser.mBrowser === 3))    // Shorthand for iCab
  {
    VarPanelViewFrame.document.all[ElementID].innerHTML = EntryHTML + this.mHTMLSegment.fGetBuffer();
  }
  else if ((WWHFrame.WWHBrowser.mBrowser === 4) ||  // Shorthand for Netscape 6.0
           (WWHFrame.WWHBrowser.mBrowser === 5))    // Shorthand for Safari
  {
    VarPanelViewFrame.document.getElementById(ElementID).innerHTML = EntryHTML + this.mHTMLSegment.fGetBuffer();
  }
}

function  WWHOutlineImagingFast_RevealEntry(ParamEntry,
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
    // Expand parent entry to reveal target entry
    //
    if (LastClosedParentEntry !== null)
    {
      this.fUpdateEntry(LastClosedParentEntry);
    }

    // Display target
    //
    WWHFrame.WWHJavaScript.mPanels.fJumpToAnchor();
  }
}
