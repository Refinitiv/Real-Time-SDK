// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global WWHFrame */
/*global WWHIndexEntryBookHash_Object */
/*global WWHIndexEntryHash_Object */
/*global WWHIndexEntry_AddEntry */
/*global WWHIndexEntry_Object */
/*global WWHIndexEntry_SortChildren */
/*global WWHIndexIterator_Advance */
/*global WWHIndexIterator_Object */
/*global WWHIndexIterator_Reset */
/*global WWHIndexOptions_Object */
/*global WWHIndexOptions_SetSeperator */
/*global WWHIndexOptions_SetThreshold */
/*global WWHIndex_AddSeeAlsoEntry */
/*global WWHIndex_AdvanceHTMLSegment */
/*global WWHIndex_ClickedEntry */
/*global WWHIndex_ClickedSeeAlsoEntry */
/*global WWHIndex_DisplayLink */
/*global WWHIndex_DisplaySection */
/*global WWHIndex_EndHTMLSegments */
/*global WWHIndex_GetEntry */
/*global WWHIndex_GetHTMLSegment */
/*global WWHIndex_GetPopupAction */
/*global WWHIndex_GetSectionNavigation */
/*global WWHIndex_HeadHTML */
/*global WWHIndex_HoverTextFormat */
/*global WWHIndex_HoverTextTranslate */
/*global WWHIndex_InitBodyHTML */
/*global WWHIndex_InitHeadHTML */
/*global WWHIndex_InitLoadBookIndex */
/*global WWHIndex_NavigationBodyHTML */
/*global WWHIndex_NavigationHeadHTML */
/*global WWHIndex_PanelNavigationLoaded */
/*global WWHIndex_PanelViewLoaded */
/*global WWHIndex_ProcessSeeAlsoEntries */
/*global WWHIndex_SelectionListBodyHTML */
/*global WWHIndex_SelectionListHeadHTML */
/*global WWHIndex_SelectionListLoaded */
/*global WWHIndex_StartHTMLSegments */
/*global WWHIndex_ThresholdExceeded */
/*global WWHJavaScriptSettings_Index_DisplayOptions */
/*global WWHPopup_EventString */
/*global WWHSectionCache_Object */
/*global WWHStringBuffer_Object */
/*global WWHStringUtilities_EscapeHTML */
/*global WWHStringUtilities_EscapeURLForJavaScriptAnchor */
/*global WWHStringUtilities_FormatMessage */
/*global WWHStringUtilities_SearchReplace */

function  WWHIndex_Object()
{
  this.mbPanelInitialized  = false;
  this.mPanelAnchor        = null;
  this.mPanelTabTitle      = WWHFrame.WWHJavaScript.mMessages.mTabsIndexLabel;
  this.mPanelTabIndex      = -1;
  this.mPanelFilename      = ((WWHFrame.WWHBrowser.mBrowser === 1) ? "panelfni.htm" : "panelfsi.htm");
  this.mInitIndex          = 0;
  this.mOptions            = new WWHIndexOptions_Object();
  this.mTopEntry           = new WWHIndexEntry_Object(false, -1, null);
  this.mMaxLevel           = 0;
  this.mEntryCount         = 0;
  this.mSeeAlsoArray       = [];
  this.mSectionIndex       = 0;
  this.mbThresholdExceeded = null;
  this.mSectionCache       = new WWHSectionCache_Object();
  this.mIterator           = new WWHIndexIterator_Object();
  this.mHTMLSegment        = new WWHStringBuffer_Object();
  this.mEventString        = WWHPopup_EventString();
  this.mClickedEntry       = null;

  this.fInitHeadHTML          = WWHIndex_InitHeadHTML;
  this.fInitBodyHTML          = WWHIndex_InitBodyHTML;
  this.fInitLoadBookIndex     = WWHIndex_InitLoadBookIndex;
  this.fAddSeeAlsoEntry       = WWHIndex_AddSeeAlsoEntry;
  this.fProcessSeeAlsoEntries = WWHIndex_ProcessSeeAlsoEntries;
  this.fNavigationHeadHTML    = WWHIndex_NavigationHeadHTML;
  this.fNavigationBodyHTML    = WWHIndex_NavigationBodyHTML;
  this.fHeadHTML              = WWHIndex_HeadHTML;
  this.fStartHTMLSegments     = WWHIndex_StartHTMLSegments;
  this.fAdvanceHTMLSegment    = WWHIndex_AdvanceHTMLSegment;
  this.fGetHTMLSegment        = WWHIndex_GetHTMLSegment;
  this.fEndHTMLSegments       = WWHIndex_EndHTMLSegments;
  this.fPanelNavigationLoaded = WWHIndex_PanelNavigationLoaded;
  this.fPanelViewLoaded       = WWHIndex_PanelViewLoaded;
  this.fHoverTextTranslate    = WWHIndex_HoverTextTranslate;
  this.fHoverTextFormat       = WWHIndex_HoverTextFormat;
  this.fGetPopupAction        = WWHIndex_GetPopupAction;
  this.fThresholdExceeded     = WWHIndex_ThresholdExceeded;
  this.fGetSectionNavigation  = WWHIndex_GetSectionNavigation;
  this.fDisplaySection        = WWHIndex_DisplaySection;
  this.fSelectionListHeadHTML = WWHIndex_SelectionListHeadHTML;
  this.fSelectionListBodyHTML = WWHIndex_SelectionListBodyHTML;
  this.fSelectionListLoaded   = WWHIndex_SelectionListLoaded;
  this.fDisplayLink           = WWHIndex_DisplayLink;
  this.fGetEntry              = WWHIndex_GetEntry;
  this.fClickedEntry          = WWHIndex_ClickedEntry;
  this.fClickedSeeAlsoEntry   = WWHIndex_ClickedSeeAlsoEntry;

  // Set options
  //
  WWHJavaScriptSettings_Index_DisplayOptions(this.mOptions);
}

function  WWHIndex_InitHeadHTML()
{
  var  InitHeadHTML = "";


  return InitHeadHTML;
}

function  WWHIndex_InitBodyHTML()
{
  var  VarHTML = new WWHStringBuffer_Object();
  var  VarBookList = WWHFrame.WWHHelp.mBooks.mBookList;
  var  VarParameters;
  var  MaxIndex, Index;


  // Workaround Safari reload bug
  //
  VarParameters = "";
  if (WWHFrame.WWHBrowser.mBrowser === 5)  // Shorthhand for Safari
  {
    VarParameters = "?" + (new Date() * 1);
  }

  // Display initializing message
  //
  VarHTML.fAppend("<h2>" + WWHFrame.WWHJavaScript.mMessages.mInitializingMessage + "</h2>\n");

  // Load index data
  //
  this.mInitIndex = 0;
  for (MaxIndex = VarBookList.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    // Reference Index data
    //
    VarHTML.fAppend("<script language=\"JavaScript1.2\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + VarBookList[Index].mDirectory + "wwhdata/js/index.js" + VarParameters + "\"></script>\n");

    // Load Index data for current book
    //
    VarHTML.fAppend("<script language=\"JavaScript1.2\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/scripts/index1s.js" + VarParameters + "\"></script>\n");
  }

  return VarHTML.fGetBuffer();
}

function  WWHIndex_InitLoadBookIndex(ParamAddIndexEntriesFunc)
{
  var  VarMaxIndex;
  var  VarIndex;


  // Load Index
  //
  ParamAddIndexEntriesFunc(this.mTopEntry);

  // Increment init book index
  //
  this.mInitIndex += 1;

  // Check if done
  //
  if (this.mInitIndex === WWHFrame.WWHHelp.mBooks.mBookList.length)
  {
    // Process see also entries to set up links between source and target
    // Do this before the top level hashes are cleared by the sort children call
    //
    this.fProcessSeeAlsoEntries();

    // Sort top level entries
    //
    if (this.mTopEntry.mChildrenSortArray === null)
    {
      WWHIndexEntry_SortChildren(this.mTopEntry);
    }

    // Assign section indices
    //
    for (VarMaxIndex = this.mTopEntry.mChildrenSortArray.length, VarIndex = 0 ; VarIndex < VarMaxIndex ; VarIndex += 1)
    {
      this.mTopEntry.mChildrenSortArray[VarIndex].mSectionIndex = VarIndex;
    }

    // Panel is initialized
    //
    this.mbPanelInitialized = true;
  }
}

function  WWHIndex_AddSeeAlsoEntry(ParamEntry)
{
  this.mSeeAlsoArray[this.mSeeAlsoArray.length] = ParamEntry;
}

function  WWHIndex_ProcessSeeAlsoEntries()
{
  var  VarMaxIndex;
  var  VarIndex;
  var  VarEntry;
  var  VarSeeAlsoGroupEntry;
  var  VarSeeAlsoEntry;


  // Set see also references
  //
  for (VarMaxIndex = this.mSeeAlsoArray.length, VarIndex = 0 ; VarIndex < VarMaxIndex ; VarIndex += 1)
  {
    // Access entry
    //
    VarEntry = this.mSeeAlsoArray[VarIndex];

    // Access group entry
    //
    VarSeeAlsoGroupEntry = this.mTopEntry.mChildren[VarEntry.mSeeAlsoGroupKey];
    if ((VarSeeAlsoGroupEntry !== undefined) &&
        (VarSeeAlsoGroupEntry !== null) &&
        (VarSeeAlsoGroupEntry.mChildren !== null))
    {
      // Access see also entry
      //
      VarSeeAlsoEntry = VarSeeAlsoGroupEntry.mChildren[VarEntry.mSeeAlsoKey];
      if ((VarSeeAlsoEntry !== undefined) &&
          (VarSeeAlsoEntry !== null))
      {
        // Setup links between source and destination
        //

        // See if target entry is already tagged
        //
        if (VarSeeAlsoEntry.mSeeAlsoTargetName === undefined)
        {
          // Update target entry
          //
          VarSeeAlsoEntry.mSeeAlsoTargetName = "s" + VarIndex;
        }

        // Update source entry
        //
        VarEntry.mSeeAlsoTargetName = VarSeeAlsoEntry.mSeeAlsoTargetName;
        VarEntry.mSeeAlsoTargetGroupID = VarSeeAlsoGroupEntry.mGroupID;
      }
    }
  }

  // Clear see also array
  //
  this.mSeeAlsoArray = null;
}

function  WWHIndex_NavigationHeadHTML()
{
  var  HTML = new WWHStringBuffer_Object();


  // Generate style section
  //
  HTML.fAppend("<style type=\"text/css\">\n");
  HTML.fAppend(" <!--\n");
  HTML.fAppend("  a.selected\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    text-decoration: none;\n");
  HTML.fAppend("    color: " + WWHFrame.WWHJavaScript.mSettings.mIndex.mNavigationCurrentColor + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("  a.enabled\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    text-decoration: none;\n");
  HTML.fAppend("    color: " + WWHFrame.WWHJavaScript.mSettings.mIndex.mNavigationEnabledColor + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("  p.navigation\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    margin-top: 1pt;\n");
  HTML.fAppend("    margin-bottom: 1pt;\n");
  HTML.fAppend("    color: " + WWHFrame.WWHJavaScript.mSettings.mIndex.mNavigationDisabledColor + ";\n");
  HTML.fAppend("    " + WWHFrame.WWHJavaScript.mSettings.mIndex.mNavigationFontStyle + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend(" -->\n");
  HTML.fAppend("</style>\n");

  return HTML.fGetBuffer();
}

function  WWHIndex_NavigationBodyHTML()
{
  var  HTML = new WWHStringBuffer_Object();
  var  VarCacheKey;


  // Define accessor functions to reduce file size
  //
  HTML.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\">\n");
  HTML.fAppend(" <!--\n");
  HTML.fAppend("  function  fD(ParamSectionIndex)\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    WWHFrame.WWHIndex.fDisplaySection(ParamSectionIndex);\n");
  HTML.fAppend("  }\n");
  HTML.fAppend(" // -->\n");
  HTML.fAppend("</script>\n");

  // Display navigation shortcuts
  //
  if (this.fThresholdExceeded())
  {
    VarCacheKey = this.mSectionIndex;
  }
  else
  {
    VarCacheKey = -1;
  }

  // Calculate section navigation if not already cached
  //
  if (this.mSectionCache[VarCacheKey] === undefined)
  {
    this.mSectionCache[VarCacheKey] = this.fGetSectionNavigation();
  }

  // Display section selection
  //
  HTML.fAppend(this.mSectionCache[VarCacheKey]);
  HTML.fAppend("<p>&nbsp;</p>\n");

  return HTML.fGetBuffer();
}

function  WWHIndex_HeadHTML()
{
  var  HTML = new WWHStringBuffer_Object();
  var  MaxLevel;
  var  Level;

  // Generate style section
  //
  HTML.fAppend("<style type=\"text/css\">\n");
  HTML.fAppend(" <!--\n");
  HTML.fAppend("  a.Section\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    text-decoration: none;\n");
  HTML.fAppend("    font-weight: bold;\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("  a:active\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    text-decoration: none;\n");
  HTML.fAppend("    background-color: " + WWHFrame.WWHJavaScript.mSettings.mIndex.mHighlightColor + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("  a:hover\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    text-decoration: underline;\n");
  HTML.fAppend("    color: " + WWHFrame.WWHJavaScript.mSettings.mIndex.mEnabledColor + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("  a\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    text-decoration: none;\n");
  HTML.fAppend("    color: " + WWHFrame.WWHJavaScript.mSettings.mIndex.mEnabledColor + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("  a.AnchorOnly\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    text-decoration: none;\n");
  HTML.fAppend("    color: " + WWHFrame.WWHJavaScript.mSettings.mIndex.mDisabledColor + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("  p\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    margin-top: 1pt;\n");
  HTML.fAppend("    margin-bottom: 1pt;\n");
  HTML.fAppend("    margin-right: 0pt;\n");
  HTML.fAppend("    color: " + WWHFrame.WWHJavaScript.mSettings.mIndex.mDisabledColor + ";\n");
  HTML.fAppend("    " + WWHFrame.WWHJavaScript.mSettings.mIndex.mFontStyle + ";\n");
  HTML.fAppend("  }\n");

  for (MaxLevel = this.mMaxLevel + 1, Level = 0 ; Level <= MaxLevel ; Level += 1)
  {
    HTML.fAppend("  p.l" + Level + "\n");
    HTML.fAppend("  {\n");
    HTML.fAppend("    margin-left: " + (WWHFrame.WWHJavaScript.mSettings.mIndex.mIndent * Level) + "pt;\n");
    HTML.fAppend("  }\n");
  }
  HTML.fAppend(" -->\n");
  HTML.fAppend("</style>\n");

  return HTML.fGetBuffer();
}

function  WWHIndex_StartHTMLSegments()
{
  var  HTML = new WWHStringBuffer_Object();


  // Setup iterator for display
  //
  if (this.fThresholdExceeded())
  {
    this.mIterator.fReset(this.mSectionIndex);
  }
  else
  {
    this.mIterator.fReset(-1);
  }

  // Define accessor functions to reduce file size
  //
  HTML.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\">\n");
  HTML.fAppend(" <!--\n");
  HTML.fAppend("  function  fC(ParamEntryInfo)\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    WWHFrame.WWHIndex.fClickedEntry(ParamEntryInfo);\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("\n");
  HTML.fAppend("  function  fA(ParamEntryInfo)\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    WWHFrame.WWHIndex.fClickedSeeAlsoEntry(ParamEntryInfo);\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("\n");
  HTML.fAppend("  function  fN()\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    return false;\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("\n");
  HTML.fAppend("  function  fS(ParamEntryID,\n");
  HTML.fAppend("               ParamEvent)\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    WWHFrame.WWHJavaScript.mPanels.mPopup.fShow(ParamEntryID, ParamEvent);\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("\n");
  HTML.fAppend("  function  fH()\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    WWHFrame.WWHJavaScript.mPanels.mPopup.fHide();\n");
  HTML.fAppend("  }\n");
  HTML.fAppend(" // -->\n");
  HTML.fAppend("</script>\n");

  return HTML.fGetBuffer();
}

function  WWHIndex_AdvanceHTMLSegment()
{
  var  MaxHTMLSegmentSize = WWHFrame.WWHJavaScript.mMaxHTMLSegmentSize;
  var  mbAccessible = WWHFrame.WWHHelp.mbAccessible;
  var  BaseEntryInfo = "";
  var  Entry;
  var  EntryAnchorName;
  var  VarAccessibilityTitle = "";
  var  MaxIndex;
  var  Index;
  var  VarParentEntry;
  var  EntryPrefix;
  var  EntrySuffix;
  var  EntryInfo;


  // Add index in top entry to entry info if IteratorScope !== TopEntry
  //
  if (this.fThresholdExceeded())
  {
    BaseEntryInfo += this.mSectionIndex;
  }

  this.mHTMLSegment.fReset();
  while ((this.mHTMLSegment.fSize() < MaxHTMLSegmentSize) &&
         (this.mIterator.fAdvance()))
  {
    Entry = this.mIterator.mEntry;

    // Insert breaks between sections
    //
    if (Entry.mbGroup)
    {
      // Emit spacing, if necessary
      //
      if (this.mHTMLSegment.fSize() > 0)
      {
        // Emit a space
        //
        this.mHTMLSegment.fAppend("<p>&nbsp;</p>\n");
      }
    }

    // Display the entry
    //

    // See if entry needs a named anchor target
    //
    if (typeof Entry.mSeeAlsoTargetName === "string")
    {
      EntryAnchorName = " name=\"sa" + Entry.mSeeAlsoTargetName + "\"";
    }
    else
    {
      EntryAnchorName = "";
    }

    // Determine accessibility title
    //
    if (mbAccessible)
    {
      VarAccessibilityTitle = "";
      for (MaxIndex = this.mIterator.mParentStack.length, Index = 0 ; Index < MaxIndex ; Index += 1)
      {
        VarParentEntry = this.mIterator.mParentStack[Index];

        if ((VarParentEntry !== this.mTopEntry) && ( ! VarParentEntry.mbGroup))
        {
          VarAccessibilityTitle += VarParentEntry.mText + WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " ";
        }
      }

      VarAccessibilityTitle += Entry.mText;

      VarAccessibilityTitle = WWHStringUtilities_EscapeHTML(VarAccessibilityTitle);

      VarAccessibilityTitle = " title=\"" + VarAccessibilityTitle + "\"";
    }

    // Determine entry type
    //
    if (Entry.mbGroup)
    {
      EntryPrefix = "<b><a class=\"Section\" name=\"section" + Entry.mSectionIndex + "\">";
      EntrySuffix = "</a></b>";
    }
    else if (typeof Entry.mSeeAlsoKey === "string")
    {
      if (typeof Entry.mSeeAlsoTargetName === "string")
      {
        // Use position stack for link info
        //
        EntryInfo = BaseEntryInfo;
        for (MaxIndex = this.mIterator.mPositionStack.length, Index = 0 ; Index < MaxIndex ; Index += 1)
        {
          if (EntryInfo.length > 0)
          {
            EntryInfo += ":";
          }
          EntryInfo += this.mIterator.mPositionStack[Index];
        }

        EntryPrefix = "<i><a href=\"javascript:fA('" + EntryInfo + "');\"" + this.fGetPopupAction(EntryInfo) + VarAccessibilityTitle + ">";
        EntrySuffix = "</a></i>";
      }
      else
      {
        EntryPrefix = "<i>";
        EntrySuffix = "</i>";
      }
    }
    else if (Entry.mBookLinks !== null)
    {
      // Use position stack for link info
      //
      EntryInfo = BaseEntryInfo;
      for (MaxIndex = this.mIterator.mPositionStack.length, Index = 0 ; Index < MaxIndex ; Index += 1)
      {
        if (EntryInfo.length > 0)
        {
          EntryInfo += ":";
        }
        EntryInfo += this.mIterator.mPositionStack[Index];
      }

      EntryPrefix = "<a" + EntryAnchorName + " href=\"javascript:fC('" + EntryInfo + "');\"" + this.fGetPopupAction(EntryInfo) + VarAccessibilityTitle + ">";
      EntrySuffix = "</a>";
    }
    else if (EntryAnchorName.length > 0)
    {
      EntryPrefix = "<a class=\"AnchorOnly\"" + EntryAnchorName + VarAccessibilityTitle + " href=\"javascript:fN();\">";
      EntrySuffix = "</a>";
    }
    else
    {
      EntryPrefix = "";
      EntrySuffix = "";
    }

    this.mHTMLSegment.fAppend("<p class=l" + (this.mIterator.mPositionStack.length - 1) + "><nobr>" + EntryPrefix + Entry.mText + EntrySuffix + "</nobr></p>\n");
  }

  return (this.mHTMLSegment.fSize() > 0);
}

function  WWHIndex_GetHTMLSegment()
{
  return this.mHTMLSegment.fGetBuffer();
}

function  WWHIndex_EndHTMLSegments()
{
  return "";
}

function  WWHIndex_PanelNavigationLoaded()
{
  // Restore focus
  //
  WWHFrame.WWHHelp.fFocus("WWHPanelNavigationFrame", "in" + this.mSectionIndex);

  // Set accessibility title
  //
  if (WWHFrame.WWHHelp.mbAccessible)
  {
    WWHFrame.WWHHelp.fSetFrameName("WWHPanelNavigationFrame");
  }
}

function  WWHIndex_PanelViewLoaded()
{
  // Set accessibility title
  //
  if (WWHFrame.WWHHelp.mbAccessible)
  {
    WWHFrame.WWHHelp.fSetFrameName("WWHPanelViewFrame");
  }
}

function  WWHIndex_HoverTextTranslate(ParamEntryInfo)
{
  var  Entry;


  // Locate specified entry
  //
  Entry = this.fGetEntry(ParamEntryInfo);

  return Entry.mText;
}

function  WWHIndex_HoverTextFormat(ParamWidth,
                                   ParamTextID,
                                   ParamText)
{
  var  StyleAttribute;
  var  FormattedText   = "";
  var  ForegroundColor = WWHFrame.WWHJavaScript.mSettings.mHoverText.mForegroundColor;
  var  BackgroundColor = WWHFrame.WWHJavaScript.mSettings.mHoverText.mBackgroundColor;
  var  BorderColor     = WWHFrame.WWHJavaScript.mSettings.mHoverText.mBorderColor;
  var  ImageDir        = WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/images";
  var  ReqSpacer1w2h   = "<img src=\"" + ImageDir + "/spc1w2h.gif\" width=1 height=2 alt=\"\">";
  var  ReqSpacer2w1h   = "<img src=\"" + ImageDir + "/spc2w1h.gif\" width=2 height=1 alt=\"\">";
  var  ReqSpacer1w7h   = "<img src=\"" + ImageDir + "/spc1w7h.gif\" width=1 height=7 alt=\"\">";
  var  ReqSpacer5w1h   = "<img src=\"" + ImageDir + "/spc5w1h.gif\" width=5 height=1 alt=\"\">";
  

  // Set style attribute to insure small image height
  //
  StyleAttribute = " style=\"font-size: 1px; line-height: 1px;\"";

  FormattedText += "<table width=\"" + ParamWidth + "\" border=0 cellspacing=0 cellpadding=0 bgcolor=\"" + BackgroundColor + "\">";
  FormattedText += " <tr>";
  FormattedText += "  <td" + StyleAttribute + " height=2 colspan=5 bgcolor=\"" + BorderColor + "\">" + ReqSpacer1w2h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td" + StyleAttribute + " height=7 bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += "  <td" + StyleAttribute + " height=7 colspan=3>" + ReqSpacer1w7h + "</td>";
  FormattedText += "  <td" + StyleAttribute + " height=7 bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += "  <td>" + ReqSpacer5w1h + "</td>";
  FormattedText += "  <td width=\"100%\" id=\"" + ParamTextID + "\" style=\"color: " + ForegroundColor + " ; " + WWHFrame.WWHJavaScript.mSettings.mHoverText.mFontStyle + "\">" + ParamText + "</td>";
  FormattedText += "  <td>" + ReqSpacer5w1h + "</td>";
  FormattedText += "  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td" + StyleAttribute + " height=7 bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += "  <td" + StyleAttribute + " height=7 colspan=3>" + ReqSpacer1w7h + "</td>";
  FormattedText += "  <td" + StyleAttribute + " height=7 bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td" + StyleAttribute + " height=2 colspan=5 bgcolor=\"" + BorderColor + "\">" + ReqSpacer1w2h + "</td>";
  FormattedText += " </tr>";
  FormattedText += "</table>";

  return FormattedText;
}

function  WWHIndex_GetPopupAction(ParamEntryInfo)
{
  var  PopupAction = "";


  if (WWHFrame.WWHJavaScript.mSettings.mHoverText.mbEnabled)
  {
    PopupAction += " onmouseover=\"fS('" + ParamEntryInfo + "', " + this.mEventString + ");\"";
    PopupAction += " onmouseout=\"fH();\"";
  }

  return PopupAction;
}

function  WWHIndex_ThresholdExceeded()
{
  if (this.mbThresholdExceeded === null)
  {
    if ((WWHFrame.WWHHelp.mbAccessible) ||
        ((this.mOptions.mThreshold > 0) &&
         (this.mEntryCount > this.mOptions.mThreshold)))
    {
      this.mbThresholdExceeded = true;
    }
    else
    {
      this.mbThresholdExceeded = false;
    }
  }

  return this.mbThresholdExceeded;
}

function  WWHIndex_GetSectionNavigation()
{
  var  SectionNavHTML = "";
  var  SectionArray;
  var  MaxIndex;
  var  Index;


  SectionNavHTML += "<p class=\"navigation\">";

  // Calculate section selection
  //
  SectionArray = this.mTopEntry.mChildrenSortArray;
  for (MaxIndex = SectionArray.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    // Add spacers if necessary
    //
    if (Index > 0)
    {
      SectionNavHTML += this.mOptions.mSeperator;
    }

    // Display section with or without link as necessary
    //
    if ((this.fThresholdExceeded()) &&
        (Index === this.mSectionIndex))  // Currently being displayed
    {
      SectionNavHTML += "<a class=\"selected\" name=\"in" + Index + "\" href=\"javascript:void(0);\">" + SectionArray[Index].mText + "</a>";
    }
    else if ((SectionArray[Index].mChildren === null) &&         // Always display group
             (SectionArray[Index].mChildrenSortArray === null))  // SortArray null before sort, hash null after
    {
      SectionNavHTML += SectionArray[Index].mText;
    }
    else
    {
      SectionNavHTML += "<a class=\"enabled\" name=\"in" + Index + "\" href=\"javascript:fD(" + Index + ");\">" + SectionArray[Index].mText + "</a>";
    }
  }

  SectionNavHTML += "</p>";

  return SectionNavHTML;
}

function  WWHIndex_DisplaySection(ParamSectionIndex)
{
  // Set section
  //
  this.mSectionIndex = ParamSectionIndex;

  if (this.fThresholdExceeded())
  {
    // Reload panel
    //
    WWHFrame.WWHJavaScript.mPanels.fClearScrollPosition();
    WWHFrame.WWHJavaScript.mPanels.fReloadPanel();
  }
  else
  {
    // Focus current section
    //
    WWHFrame.WWHHelp.fFocus("WWHPanelNavigationFrame", "in" + this.mSectionIndex);

    // Whole index already visible, just jump to the specified entry
    //
    this.mPanelAnchor = "section" + this.mSectionIndex;

    // Workaround for IE/FireFox problems
    //
    if (WWHFrame.WWHBrowser.mbSupportsFocus)
    {
      WWHFrame.WWHBrowser.mbSupportsFocus = false;

      WWHFrame.WWHJavaScript.mPanels.fJumpToAnchor();

      WWHFrame.WWHBrowser.mbSupportsFocus = true;
    }

    WWHFrame.WWHJavaScript.mPanels.fJumpToAnchor();
  }
}

function  WWHIndex_SelectionListHeadHTML()
{
  var  HTML = new WWHStringBuffer_Object();
  var  Level;


  HTML.fAppend("<style type=\"text/css\">\n");
  HTML.fAppend(" <!--\n");
  HTML.fAppend("  a\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    text-decoration: none;\n");
  HTML.fAppend("    color: " + WWHFrame.WWHJavaScript.mSettings.mIndex.mEnabledColor + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("  p\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    margin-top: 1pt;\n");
  HTML.fAppend("    margin-bottom: 1pt;\n");
  HTML.fAppend("    margin-right: 0pt;\n");
  HTML.fAppend("    color: " + WWHFrame.WWHJavaScript.mSettings.mIndex.mDisabledColor + ";\n");
  HTML.fAppend("    " + WWHFrame.WWHJavaScript.mSettings.mIndex.mFontStyle + ";\n");
  HTML.fAppend("  }\n");
  for (Level = 1 ; Level < 3 ; Level += 1)
  {
    HTML.fAppend("  p.l" + Level + "\n");
    HTML.fAppend("  {\n");
    HTML.fAppend("    margin-left: " + (WWHFrame.WWHJavaScript.mSettings.mIndex.mIndent * Level) + "pt;\n");
    HTML.fAppend("  }\n");
  }
  HTML.fAppend("  h2\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    " + WWHFrame.WWHJavaScript.mSettings.mIndex.mFontStyle + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend(" -->\n");
  HTML.fAppend("</style>\n");

  return HTML.fGetBuffer();
}

function  WWHIndex_SelectionListBodyHTML()
{
  var  HTML = new WWHStringBuffer_Object();
  var  BookList = WWHFrame.WWHHelp.mBooks.mBookList;
  var  EntryClass;
  var  MaxBookIndex;
  var  BookIndex;
  var  BookListEntry;
  var  LinkArray;
  var  MaxLinkIndex;
  var  LinkIndex;
  var  Parts;
  var  PrevLinkFileIndex;
  var  LinkFileIndex;
  var  LinkAnchor;
  var  VarAccessibilityTitle;
  var  NumberedLinkCounter;
  var  DocumentURL;


  if (this.mClickedEntry !== null)
  {
    // Display multiple entry message
    //
    HTML.fAppend("<h2>");
    HTML.fAppend(WWHFrame.WWHJavaScript.mMessages.mIndexSelectMessage1 + " ");
    HTML.fAppend(WWHFrame.WWHJavaScript.mMessages.mIndexSelectMessage2);
    HTML.fAppend("</h2>\n");

    // Display text of entry clicked
    //
    HTML.fAppend("<p><b>" + this.mClickedEntry.mText + "</b></p>\n");

    // Determine level at which to display entries
    //
    if (BookList.length === 1)
    {
      EntryClass = "l1";
    }
    else
    {
      EntryClass = "l2";
    }

    // Display each book's link for this entry
    //
    for (MaxBookIndex = BookList.length, BookIndex = 0 ; BookIndex < MaxBookIndex ; BookIndex += 1)
    {
      if (this.mClickedEntry.mBookLinks[BookIndex] !== undefined)
      {
        BookListEntry = BookList[BookIndex];

        // Write the book's title, if necessary
        //
        if (BookList.length > 1)
        {
          HTML.fAppend("<p>&nbsp;</p>\n");
          HTML.fAppend("<p class=\"l1\"><nobr><b>" + BookListEntry.mTitle + "</b>");
        }

        // Sort link array to group files with anchors
        //
        // Use for loop to copy entries to workaround bug/problem in IE 5.0 on Windows
        //
        LinkArray = [];
        for (MaxLinkIndex = this.mClickedEntry.mBookLinks[BookIndex].length, LinkIndex = 0 ; LinkIndex < MaxLinkIndex ; LinkIndex += 1)
        {
          LinkArray[LinkIndex] = this.mClickedEntry.mBookLinks[BookIndex][LinkIndex];
        }
        LinkArray = LinkArray.sort();

        // Now display file links
        //
        PrevLinkFileIndex = null;
        for (MaxLinkIndex = LinkArray.length, LinkIndex = 0 ; LinkIndex < MaxLinkIndex ; LinkIndex += 1)
        {
          // Determine link file index and anchor
          //
          Parts = LinkArray[LinkIndex].split("#");
          LinkFileIndex = parseInt(Parts[0], 10);
          LinkAnchor = null;
          if (Parts.length > 1)
          {
            if (Parts[1].length > 0)
            {
              LinkAnchor = Parts[1];
            }
          }

          // Determine if all links for a single document have been processed
          //
          if ((PrevLinkFileIndex === null) ||
              (LinkFileIndex !== PrevLinkFileIndex))
          {
            NumberedLinkCounter = 1;

            // Determine title for accessibility
            //
            if (WWHFrame.WWHHelp.mbAccessible)
            {
              VarAccessibilityTitle = WWHStringUtilities_FormatMessage(WWHFrame.WWHJavaScript.mMessages.mAccessibilityIndexEntry,
                                                                       BookListEntry.mFiles.fFileIndexToTitle(LinkFileIndex),
                                                                       BookListEntry.mTitle);
              VarAccessibilityTitle = WWHStringUtilities_EscapeHTML(VarAccessibilityTitle);
              VarAccessibilityTitle = " title=\"" + VarAccessibilityTitle + "\"";
            }

            HTML.fAppend("</nobr></p>\n");

            // Build up absolute link URL
            //
            DocumentURL = WWHFrame.WWHHelp.fGetBookIndexFileIndexURL(BookIndex, LinkFileIndex, LinkAnchor);
            DocumentURL = WWHStringUtilities_EscapeURLForJavaScriptAnchor(DocumentURL);

            // Handle "double-indirection" issue
            //
            DocumentURL = WWHStringUtilities_SearchReplace(DocumentURL, "%", "%25");

            HTML.fAppend("<p class=\"" + EntryClass + "\"><nobr>");
            HTML.fAppend("<a name=\"indexselect\" href=\"javascript:WWHFrame.WWHIndex.fDisplayLink('" + DocumentURL + "');\"" + VarAccessibilityTitle + ">");
            HTML.fAppend(BookListEntry.mFiles.fFileIndexToTitle(LinkFileIndex) + "</a>");
          }
          else
          {
            NumberedLinkCounter += 1;

            // Determine title for accessibility
            //
            if (WWHFrame.WWHHelp.mbAccessible)
            {
              VarAccessibilityTitle = WWHStringUtilities_FormatMessage(WWHFrame.WWHJavaScript.mMessages.mAccessibilityIndexSecondEntry,
                                                                       BookListEntry.mFiles.fFileIndexToTitle(LinkFileIndex),
                                                                       BookListEntry.mTitle,
                                                                       NumberedLinkCounter);
              VarAccessibilityTitle = WWHStringUtilities_EscapeHTML(VarAccessibilityTitle);
              VarAccessibilityTitle = " title=\"" + VarAccessibilityTitle + "\"";
            }

            // Build up absolute link URL
            //
            DocumentURL = WWHFrame.WWHHelp.fGetBookIndexFileIndexURL(BookIndex, LinkFileIndex, LinkAnchor);
            DocumentURL = WWHStringUtilities_EscapeURLForJavaScriptAnchor(DocumentURL);

            // Handle "double-indirection" issue
            //
            DocumentURL = WWHStringUtilities_SearchReplace(DocumentURL, "%", "%25");

            HTML.fAppend(",&nbsp;");
            HTML.fAppend("<a href=\"javascript:WWHFrame.WWHIndex.fDisplayLink('" + DocumentURL + "');\"" + VarAccessibilityTitle + ">");
            HTML.fAppend(NumberedLinkCounter + "</a>");
          }

          PrevLinkFileIndex = LinkFileIndex;
        }

        HTML.fAppend("</nobr></p>\n");
      }
    }
  }

  return HTML.fGetBuffer();
}

function  WWHIndex_SelectionListLoaded()
{
  // Move focus to document selection list
  //
  WWHFrame.WWHHelp.fFocus("WWHDocumentFrame", "indexselect");
}

function  WWHIndex_DisplayLink(ParamURL)
{
  WWHFrame.WWHHelp.fSetDocumentHREF(ParamURL, false);
}

function  WWHIndex_GetEntry(ParamEntryInfo)
{
  var  Entry = null;
  var  EntryInfoParts;
  var  MaxIndex;
  var  Index;


  // Locate specified entry
  //
  Entry = this.mTopEntry;
  EntryInfoParts = ParamEntryInfo.split(":");
  for (MaxIndex = EntryInfoParts.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    Entry = Entry.mChildrenSortArray[EntryInfoParts[Index]];
  }

  return Entry;
}

function  WWHIndex_ClickedEntry(ParamEntryInfo)
{
  var  Entry;
  var  BookCount;
  var  BookIndex;
  var  Parts;
  var  LinkFileIndex;
  var  LinkAnchor;
  var  DocumentURL;


  // Locate specified entry
  //
  Entry = this.fGetEntry(ParamEntryInfo);

  // Display target document or selection list
  //
  BookCount = 0;
  for (BookIndex in Entry.mBookLinks)
  {
    BookCount += 1;
  }

  // See if this is a single entry
  //
  if ((BookCount === 1) &&
      (Entry.mBookLinks[BookIndex].length === 1))
  {
    // Determine link file index and anchor
    //
    Parts = Entry.mBookLinks[BookIndex][0].split("#");
    LinkFileIndex = parseInt(Parts[0], 10);
    LinkAnchor = null;
    if (Parts.length > 1)
    {
      if (Parts[1].length > 0)
      {
        LinkAnchor = Parts[1];
      }
    }

    // Set Document
    //
    DocumentURL = WWHFrame.WWHHelp.fGetBookIndexFileIndexURL(BookIndex, LinkFileIndex, LinkAnchor);
  }
  else
  {
    // Display selection list
    //
    this.mClickedEntry = Entry;
    DocumentURL = WWHFrame.WWHHelp.mBaseURL + "wwhelp/wwhimpl/js/html/indexsel.htm";
  }

  this.fDisplayLink(DocumentURL);
}

function  WWHIndex_ClickedSeeAlsoEntry(ParamEntryInfo)
{
  var  Entry;
  var  TargetSectionIndex;
  var  MaxIndex;
  var  Index;


  // Locate specified entry
  //
  Entry = this.fGetEntry(ParamEntryInfo);

  // Confirm entry has target information
  //
  if ((typeof Entry.mSeeAlsoTargetName === "string") &&
      (typeof Entry.mSeeAlsoTargetGroupID === "number"))
  {
    // Determine if we need to jump to another page
    //
    TargetSectionIndex = -1;
    for (MaxIndex = this.mTopEntry.mChildrenSortArray.length, Index = 0 ; Index < MaxIndex ; Index += 1)
    {
      if (this.mTopEntry.mChildrenSortArray[Index].mGroupID === Entry.mSeeAlsoTargetGroupID)
      {
        TargetSectionIndex = Index;

        // Exit for loop
        //
        Index = MaxIndex;
      }
    }

    // Confirm the target entry was located
    //
    if (TargetSectionIndex !== -1)
    {
      // Set target entry
      //
      this.mPanelAnchor = "sa" + Entry.mSeeAlsoTargetName;

      // Change navigation bar?
      //
      if ((this.fThresholdExceeded()) &&
          (TargetSectionIndex !== this.mSectionIndex))
      {
        // Need to switch to proper section
        //
        this.fDisplaySection(TargetSectionIndex);
      }
      else
      {
        // Focus current section
        //
        WWHFrame.WWHHelp.fFocus("WWHPanelNavigationFrame", "in" + this.mSectionIndex);

        // We're on the right page, so just jump to the correct entry
        //
        WWHFrame.WWHJavaScript.mPanels.fJumpToAnchor();
      }
    }
  }
}

function  WWHIndexIterator_Object()
{
  this.mIteratorScope      = null;
  this.mEntry              = null;
  this.mParentStack        = [];
  this.mPositionStack      = [];

  this.fReset   = WWHIndexIterator_Reset;
  this.fAdvance = WWHIndexIterator_Advance;
}

function  WWHIndexIterator_Reset(ParamIndex)
{
  if (ParamIndex === -1)  // Iterate buckets as well!
  {
    this.mIteratorScope = WWHFrame.WWHIndex.mTopEntry;
  }
  else if ((WWHFrame.WWHIndex.mTopEntry.mChildrenSortArray !== null) &&
           (WWHFrame.WWHIndex.mTopEntry.mChildrenSortArray.length > 0))
  {
    this.mIteratorScope = WWHFrame.WWHIndex.mTopEntry.mChildrenSortArray[ParamIndex];
  }
  else
  {
    this.mIteratorScope = null;
  }
  this.mEntry                = this.mIteratorScope;
  this.mParentStack.length   = 0;
  this.mPositionStack.length = 0;
}

function  WWHIndexIterator_Advance()
{
  var  ParentEntry;
  var  StackTop;


  // Advance to the next visible entry
  //
  if (this.mEntry !== null)
  {
    // Check for children
    //
    if (this.mEntry.mChildren !== null)
    {
      // Determine sort order if necessary
      //
      if (this.mEntry.mChildrenSortArray === null)
      {
        WWHIndexEntry_SortChildren(this.mEntry);
      }
    }

    // Process children
    //
    if ((this.mEntry.mChildrenSortArray !== null) &&
        (this.mEntry.mChildrenSortArray.length > 0))
    {
      this.mParentStack[this.mParentStack.length] = this.mEntry;
      this.mPositionStack[this.mPositionStack.length] = 0;
      this.mEntry = this.mEntry.mChildrenSortArray[0];
    }
    // If we've reached the iterator scope, we're done
    //
    else if (this.mEntry === this.mIteratorScope)
    {
      this.mEntry = null;
    }
    else
    {
      ParentEntry = this.mParentStack[this.mParentStack.length - 1];
      this.mEntry = null;

      // Find next child of parent entry
      //
      while (ParentEntry !== null)
      {
        // Increment position
        //
        StackTop = this.mPositionStack.length - 1;
        this.mPositionStack[StackTop] += 1;

        // Confirm this is a valid entry
        //
        if (this.mPositionStack[StackTop] < ParentEntry.mChildrenSortArray.length)
        {
          // Return the parent's next child
          //
          this.mEntry = ParentEntry.mChildrenSortArray[this.mPositionStack[StackTop]];

          // Signal break from loop
          //
          ParentEntry = null;
        }
        else
        {
          // Last child of parent, try up a level
          //
          if (ParentEntry === this.mIteratorScope)
          {
            ParentEntry = null;
          }
          else
          {
            this.mParentStack.length -= 1;
            this.mPositionStack.length -= 1;

            ParentEntry = this.mParentStack[this.mParentStack.length - 1];
          }
        }
      }
    }
  }

  return (this.mEntry !== null);
}

function  WWHIndexEntry_Object(bParamGroupHeading,
                               ParamBookIndex,
                               ParamText,
                               ParamLinks,
                               ParamSeeAlsoKey,
                               ParamSeeAlsoGroupKey)
{
  if (bParamGroupHeading)
  {
    this.mbGroup  = true;
    this.mGroupID = WWHFrame.WWHIndex.mEntryCount;
  }
  else
  {
    this.mbGroup = false;
  }

  this.mText              = ParamText;
  this.mBookLinks         = null;
  this.mChildren          = null;
  this.mChildrenSortArray = null;

  if (typeof ParamSeeAlsoKey === "string")
  {
    this.mSeeAlsoKey = ParamSeeAlsoKey;
  }
  if (typeof ParamSeeAlsoGroupKey === "string")
  {
    this.mSeeAlsoGroupKey = ParamSeeAlsoGroupKey;
  }

  this.fAddEntry  = WWHIndexEntry_AddEntry;
  this.fA         = WWHIndexEntry_AddEntry;

  // Bump entry count if not the top level node
  //
  if (ParamBookIndex !== -1)
  {
    WWHFrame.WWHIndex.mEntryCount += 1;
  }

  // Add links
  //
  if ((ParamLinks !== undefined) &&
      (ParamLinks !== null))
  {
    this.mBookLinks = new WWHIndexEntryBookHash_Object();
    this.mBookLinks[ParamBookIndex] = ParamLinks;
  }
}

function  WWHIndexEntry_GetKey(ParamGroupTag,
                               ParamText,
                               ParamSort)
{
  var  VarKey = null;


  if ((ParamText !== undefined) &&
      (ParamText !== null) &&
      (ParamText.length > 0))
  {
    if ((ParamGroupTag !== undefined) &&
        (ParamGroupTag !== null) &&
        (ParamGroupTag.length > 0))
    {
      if (VarKey === null)
      {
        VarKey = "";
      }

      VarKey += ParamGroupTag;
    }

    if ((ParamSort !== undefined) &&
        (ParamSort !== null) &&
        (ParamSort.length > 0) &&
        (ParamSort !== ParamText))
    {
      if (VarKey === null)
      {
        VarKey = "";
      }

      VarKey += ":" + ParamSort;
    }

    if (VarKey === null)
    {
      VarKey = "";
    }

    VarKey += ":" + ParamText;
  }

  return VarKey;
}

function  WWHIndexEntry_AddEntry(ParamText,
                                 ParamLinks,
                                 ParamSort,
                                 ParamGroupTag,
                                 ParamSeeAlso,
                                 ParamSeeAlsoSort,
                                 ParamSeeAlsoGroup,
                                 ParamSeeAlsoGroupSort,
                                 ParamSeeAlsoGroupTag)
{
  var  bVarGroupHeading;
  var  Links;
  var  VarKey;
  var  VarSeeAlsoKey;
  var  VarSeeAlsoGroupKey;
  var  BookIndex;
  var  ChildEntry;
  var  BookLinks;
  var  MaxIndex;
  var  Index;


  // See if this is a group heading
  //
  if ((ParamGroupTag !== undefined) &&
      (ParamGroupTag !== null) &&
      (ParamGroupTag.length > 0))
  {
    bVarGroupHeading = true;
  }

  // Set links if entries exist
  //
  if ((ParamLinks !== undefined) &&
      (ParamLinks !== null) &&
      (ParamLinks.length > 0))
  {
    Links = ParamLinks;
  }
  else
  {
    Links = null;
  }

  // See if this object has any children
  //
  if (this.mChildren === null)
  {
    this.mChildren = new WWHIndexEntryHash_Object();
  }

  // Define keys
  //
  VarKey             = WWHIndexEntry_GetKey(ParamGroupTag, ParamText, ParamSort);
  VarSeeAlsoKey      = WWHIndexEntry_GetKey(null, ParamSeeAlso, ParamSeeAlsoSort);
  VarSeeAlsoGroupKey = WWHIndexEntry_GetKey(ParamSeeAlsoGroupTag, ParamSeeAlsoGroup, ParamSeeAlsoGroupSort);

  // Access entry, creating it if it doesn't exist
  //
  BookIndex = WWHFrame.WWHIndex.mInitIndex;
  ChildEntry = this.mChildren[VarKey];
  if (ChildEntry === undefined)
  {
    ChildEntry = new WWHIndexEntry_Object(bVarGroupHeading, BookIndex, ParamText,
                                          Links, VarSeeAlsoKey, VarSeeAlsoGroupKey);
    this.mChildren[VarKey] = ChildEntry;

    // Add entry to see also collection if it is a see also entry
    //
    if (typeof VarSeeAlsoKey === "string")
    {
      WWHFrame.WWHIndex.fAddSeeAlsoEntry(ChildEntry);
    }
  }
  else  // Child entry exists, update with new information
  {
    // Add book links
    //
    if (Links !== null)
    {
      if (ChildEntry.mBookLinks === null)
      {
        ChildEntry.mBookLinks = new WWHIndexEntryBookHash_Object();
      }

      if (ChildEntry.mBookLinks[BookIndex] === undefined)
      {
        ChildEntry.mBookLinks[BookIndex] = Links;
      }
      else
      {
        // Append new links
        //
        BookLinks = ChildEntry.mBookLinks[BookIndex];
        for (MaxIndex = Links.length, Index = 0 ; Index < MaxIndex ; Index += 1)
        {
          BookLinks[BookLinks.length] = Links[Index];
        }
      }
    }
  }

  return ChildEntry;
}

function  WWHIndexEntry_SortChildren(ParamEntry)
{
  var  UnsortedArray;
  var  KeyHash = {};
  var  SortedArray;
  var  VarKey;
  var  VarKeyTrimmed;
  var  VarKeyUpperCase;
  var  VarSortKey;
  var  MaxIndex;
  var  Index;


  // Accumulate hash keys
  //
  UnsortedArray = [];
  for (VarKey in ParamEntry.mChildren)
  {
    VarKeyTrimmed = VarKey.substring(1, VarKey.length - 1);
    VarKeyUpperCase = VarKeyTrimmed.toUpperCase();
    VarSortKey = VarKeyUpperCase + ":" + VarKey;

    UnsortedArray[UnsortedArray.length] = VarSortKey;
    KeyHash[VarSortKey] = VarKey;
  }

  // Insure array exists
  //
  if (UnsortedArray.length > 0)
  {
    // Sort array
    //
    SortedArray = UnsortedArray.sort();

    // Replace sort keys with entries
    //
    for (MaxIndex = SortedArray.length, Index = 0 ; Index < MaxIndex ; Index += 1)
    {
      VarSortKey = SortedArray[Index];
      if ((KeyHash[VarSortKey] !== undefined) &&
          (KeyHash[VarSortKey] !== null))
      {
        VarKey = KeyHash[VarSortKey];
        SortedArray[Index] = ParamEntry.mChildren[VarKey];
      }
    }
  }
  else
  {
    // No children, possible error occurred?
    //
    SortedArray = [];
  }

  // Set children sort array
  // Clear hash table as it is no longer needed
  //
  ParamEntry.mChildrenSortArray = SortedArray;
  ParamEntry.mChildren = null;
}

function  WWHIndexEntryHash_Object()
{
}

function  WWHIndexEntryBookHash_Object()
{
}

function  WWHSectionCache_Object()
{
}

function  WWHIndexOptions_Object()
{
  this.mThreshold = 0;

  this.fSetThreshold = WWHIndexOptions_SetThreshold;
  this.fSetSeperator = WWHIndexOptions_SetSeperator;
}

function  WWHIndexOptions_SetThreshold(ParamThreshold)
{
  this.mThreshold = ParamThreshold;
}

function  WWHIndexOptions_SetSeperator(ParamSeperator)
{
  this.mSeperator = ParamSeperator;
}
