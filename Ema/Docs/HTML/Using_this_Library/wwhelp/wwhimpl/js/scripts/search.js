// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global WWHFrame */
/*global MultiPhrase_Object */
/*global WWHBookSearchInfo_AddSkipWord */
/*global WWHBookSearchInfo_Object */
/*global WWHBookSearchInfo_SkipWords_Object */
/*global WWHBookSearchInfo_ValidSearchWord */
/*global WWHPopup_EventString */
/*global WWHSearchBookMatches_AddMatches */
/*global WWHSearchBookMatches_Clear */
/*global WWHSearchBookMatches_FileScores_Object */
/*global WWHSearchBookMatches_JoinFileScores */
/*global WWHSearchBookMatches_Object */
/*global WWHSearchBookMatches_SetMatchedWordIndex */
/*global WWHSearchResultsEntry_ByBookIndexByScoreByTitleFileIndexURL */
/*global WWHSearchResultsEntry_ByScoreByBookIndexByTitleFileIndexURL */
/*global WWHSearchResultsEntry_CompareByBookIndex */
/*global WWHSearchResultsEntry_CompareByScore */
/*global WWHSearchResultsEntry_CompareByTitleFileIndexURL */
/*global WWHSearchResultsEntry_Object */
/*global WWHSearchResults_AddEntry */
/*global WWHSearchResults_Clear */
/*global WWHSearchResults_DisplayAdvance */
/*global WWHSearchResults_DisplayReset */
/*global WWHSearchResults_GetPopupAction */
/*global WWHSearchResults_Object */
/*global WWHSearchResults_ShowEntry */
/*global WWHSearchResults_SortByBookIndex */
/*global WWHSearchResults_SortByScore */
/*global WWHSearchScope_AddScopeEntries */
/*global WWHSearchScope_Object */
/*global WWHSearch_AdvanceHTMLSegment */
/*global WWHSearch_ApplyWordBreaks */
/*global WWHSearch_CheckForMatch */
/*global WWHSearch_CheckForPhraseMatch */
/*global WWHSearch_CombineResults */
/*global WWHSearch_DisplaySearchForm */
/*global WWHSearch_EndHTMLSegments */
/*global WWHSearch_FocusSearchWords */
/*global WWHSearch_GetHTMLSegment */
/*global WWHSearch_HeadHTML */
/*global WWHSearch_HoverTextFormat */
/*global WWHSearch_HoverTextTranslate */
/*global WWHSearch_InitBodyHTML */
/*global WWHSearch_InitHeadHTML */
/*global WWHSearch_InitLoadBookSearchInfo */
/*global WWHSearch_Initialize */
/*global WWHSearch_NavigationBodyHTML */
/*global WWHSearch_NavigationHeadHTML */
/*global WWHSearch_OnFocus_SearchWords */
/*global WWHSearch_PanelNavigationLoaded */
/*global WWHSearch_PanelViewLoaded */
/*global WWHSearch_SearchPairsComplete */
/*global WWHSearch_SearchWordsComplete */
/*global WWHSearch_SetSearchWords */
/*global WWHSearch_ShowEntry */
/*global WWHSearch_StartHTMLSegments */
/*global WWHSearch_Submit */
/*global WWHStringBuffer_Object */
/*global WWHStringUtilities_EscapeHTML */
/*global WWHStringUtilities_ParseWordsAndPhrases */
/*global WWHStringUtilities_WordToRegExpPattern */
/*global WWHUnicode_CheckBreakAtIndex */

function  WWHSearch_Object()
{
  this.mbPanelInitialized     = false;
  this.mPanelAnchor           = null;
  this.mPanelTabTitle         = WWHFrame.WWHJavaScript.mMessages.mTabsSearchLabel;
  this.mPanelTabIndex         = -1;
  this.mPanelFilename         = ((WWHFrame.WWHBrowser.mBrowser === 1) ? "panelfns.htm" : "panelfss.htm");
  this.mInitIndex             = 0;
  this.mBookSearchInfoList    = [];
  this.mMultiPhraseList       = [];
  this.mSearchState           = null;
  this.mSearchScopeInfo       = null;
  this.mSavedSearchWords      = "";
  this.mSavedSearchScope      = 0;
  this.mSearchWordList        = [];
  this.mSearchWordRegExpList  = [];
  this.mBookIndex             = 0;
  this.mBookMatchesList       = [];
  this.mCombinedResults       = new WWHSearchResults_Object();
  this.mCombinedResultsIndex  = 0;
  this.mbChangingToSearch     = true;

  this.fInitialize             = WWHSearch_Initialize;
  this.fInitHeadHTML           = WWHSearch_InitHeadHTML;
  this.fInitBodyHTML           = WWHSearch_InitBodyHTML;
  this.fInitLoadBookSearchInfo = WWHSearch_InitLoadBookSearchInfo;
  this.fNavigationHeadHTML     = WWHSearch_NavigationHeadHTML;
  this.fNavigationBodyHTML     = WWHSearch_NavigationBodyHTML;
  this.fHeadHTML               = WWHSearch_HeadHTML;
  this.fStartHTMLSegments      = WWHSearch_StartHTMLSegments;
  this.fAdvanceHTMLSegment     = WWHSearch_AdvanceHTMLSegment;
  this.fGetHTMLSegment         = WWHSearch_GetHTMLSegment;
  this.fEndHTMLSegments        = WWHSearch_EndHTMLSegments;
  this.fFocusSearchWords       = WWHSearch_FocusSearchWords;
  this.fPanelNavigationLoaded  = WWHSearch_PanelNavigationLoaded;
  this.fPanelViewLoaded        = WWHSearch_PanelViewLoaded;
  this.fHoverTextTranslate     = WWHSearch_HoverTextTranslate;
  this.fHoverTextFormat        = WWHSearch_HoverTextFormat;
  this.fDisplaySearchForm      = WWHSearch_DisplaySearchForm;
  this.fOnFocus_SearchWords    = WWHSearch_OnFocus_SearchWords;
  this.fSubmit                 = WWHSearch_Submit;
  this.fApplyWordBreaks        = WWHSearch_ApplyWordBreaks;
  this.fSetSearchWords         = WWHSearch_SetSearchWords;
  this.fCheckForMatch          = WWHSearch_CheckForMatch;
  this.fSearchWordsComplete    = WWHSearch_SearchWordsComplete;
  this.fCheckForPhraseMatch    = WWHSearch_CheckForPhraseMatch;
  this.fSearchPairsComplete    = WWHSearch_SearchPairsComplete;
  this.fCombineResults         = WWHSearch_CombineResults;
  this.fShowEntry              = WWHSearch_ShowEntry;

  // Initialize
  //
  this.fInitialize();
}

function  WWHSearch_Initialize()
{
}

function  WWHSearch_InitHeadHTML()
{
  var  InitHeadHTML = "";


  // Create search scope info
  //
  this.mSearchScopeInfo = new WWHSearchScope_Object();

  return InitHeadHTML;
}

function  WWHSearch_InitBodyHTML()
{
  var  HTML = new WWHStringBuffer_Object();
  var  VarParameters;
  var  BookList;
  var  MaxIndex;
  var  Index;


  // Workaround Safari reload bug
  //
  VarParameters = "";
  if (WWHFrame.WWHBrowser.mBrowser === 5)  // Shorthhand for Safari
  {
    VarParameters = "?" + (new Date() * 1);
  }

  // Display initializing message
  //
  HTML.fAppend("<h2>" + WWHFrame.WWHJavaScript.mMessages.mInitializingMessage + "</h2>\n");

  // Load search info
  //
  this.mInitIndex = 0;
  BookList = WWHFrame.WWHHelp.mBooks.mBookList;
  for (MaxIndex = BookList.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    // Reference search info
    //
    HTML.fAppend("<script language=\"JavaScript1.2\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + BookList[Index].mDirectory + "wwhdata/js/search.js" + VarParameters + "\"></script>\n");

    // Load search info for current book
    //
    HTML.fAppend("<script language=\"JavaScript1.2\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/scripts/search1s.js" + VarParameters + "\"></script>\n");
  }

  return HTML.fGetBuffer();
}

function  WWHSearch_InitLoadBookSearchInfo(ParamSearchFileCount,
                                           ParamMinimumWordLength,
                                           ParamSearchSkipWordsFunc)
{
  // Load book search info
  //
  this.mBookSearchInfoList[this.mInitIndex] = new WWHBookSearchInfo_Object(ParamSearchFileCount, ParamMinimumWordLength);
  ParamSearchSkipWordsFunc(this.mBookSearchInfoList[this.mInitIndex]);

  // Create match objects for each book
  //
  this.mBookMatchesList[this.mBookMatchesList.length] = new WWHSearchBookMatches_Object();

  // Increment init book index
  //
  this.mInitIndex += 1;

  // Mark initialized if done
  //
  if (this.mInitIndex === WWHFrame.WWHHelp.mBooks.mBookList.length)
  {
    this.mbPanelInitialized = true;
  }
}

function  WWHSearch_NavigationHeadHTML()
{
  return "";
}

function  WWHSearch_NavigationBodyHTML()
{
  return this.fDisplaySearchForm();
}

function  WWHSearch_HeadHTML()
{
  var  HTML = new WWHStringBuffer_Object();
  var  Settings = WWHFrame.WWHJavaScript.mSettings.mSearch;


  // Generate style section
  //
  HTML.fAppend("<style type=\"text/css\">\n");
  HTML.fAppend(" <!--\n");
  HTML.fAppend("  a:active\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    text-decoration: none;\n");
  HTML.fAppend("    background-color: " + Settings.mHighlightColor + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("  a:hover\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    text-decoration: underline;\n");
  HTML.fAppend("    color: " + Settings.mEnabledColor + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("  a\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    text-decoration: none;\n");
  HTML.fAppend("    color: " + Settings.mEnabledColor + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("  p\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    margin-top: 1pt;\n");
  HTML.fAppend("    margin-bottom: 1pt;\n");
  HTML.fAppend("    " + Settings.mFontStyle + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("  p.BookTitle\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    margin-top: 1pt;\n");
  HTML.fAppend("    margin-bottom: 1pt;\n");
  HTML.fAppend("    font-weight: bold;\n");
  HTML.fAppend("    " + Settings.mFontStyle + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend("  ol\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    margin-top: 1pt;\n");
  HTML.fAppend("    margin-bottom: 1pt;\n");
  if (Settings.mbShowRank)
  {
    HTML.fAppend("    " + Settings.mFontStyle + ";\n");
  }
  else
  {
    HTML.fAppend("    list-style: none;\n");
  }
  HTML.fAppend("  }\n");
  HTML.fAppend("  li\n");
  HTML.fAppend("  {\n");
  HTML.fAppend("    margin-top: 2pt;\n");
  HTML.fAppend("    margin-bottom: 0pt;\n");
  HTML.fAppend("    " + Settings.mFontStyle + ";\n");
  HTML.fAppend("  }\n");
  HTML.fAppend(" -->\n");
  HTML.fAppend("</style>\n");

  return HTML.fGetBuffer();
}

function  WWHSearch_StartHTMLSegments()
{
  var  HTML = new WWHStringBuffer_Object();
  var  VarParameters;
  var  StartBookIndex;
  var  MaxBookIndex;
  var  BookIndex;
  var  BookList;
  var  MaxIndex;
  var  Index;
  var  BookDirectory;
  var  bDisplayBookTitles;
  var  Entry;
  var  MultiPhraseEntry;


  if (this.mbPanelInitialized)
  {
    // Workaround Safari reload bug
    //
    VarParameters = "";
    if (WWHFrame.WWHBrowser.mBrowser === 5)  // Shorthhand for Safari
    {
      VarParameters = "?" + (new Date() * 1);
    }

    // Perform search if required
    //
    if (this.mSearchState === "words")
    {
      // Display searching message
      //
      HTML.fAppend("<h2>" + WWHFrame.WWHJavaScript.mMessages.mSearchSearchingMessage + "</h2>\n");

      // Handle single book search
      //
      BookList = WWHFrame.WWHHelp.mBooks.mBookList;
      if (this.mSavedSearchScope > 0)
      {
        StartBookIndex = this.mSearchScopeInfo.mEntries[this.mSavedSearchScope - 1].mStartBookIndex;
        MaxBookIndex   = this.mSearchScopeInfo.mEntries[this.mSavedSearchScope - 1].mEndBookIndex + 1;
      }
      else
      {
        StartBookIndex = 0;
        MaxBookIndex   = BookList.length;
      }

      // Generate search actions
      //
      this.mBookIndex = StartBookIndex;
      for (BookIndex = StartBookIndex ; BookIndex < MaxBookIndex ; BookIndex += 1)
      {
        BookDirectory = BookList[BookIndex].mDirectory;

        for (MaxIndex = this.mBookSearchInfoList[BookIndex].mSearchFileCount, Index = 0 ; Index < MaxIndex ; Index += 1)
        {
          HTML.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + BookDirectory + "wwhdata/js/search/search" + Index + ".js" + VarParameters + "\"></script>\n");
          HTML.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/scripts/search2s.js" + VarParameters + "\"></script>\n");
        }

        HTML.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/scripts/search3s.js" + VarParameters + "\"></script>\n");
      }

      HTML.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/scripts/search4s.js" + VarParameters + "\"></script>\n");
    }
    else if (this.mSearchState === "pairs")
    {
      // Display searching message
      //
      HTML.fAppend("<h2>" + WWHFrame.WWHJavaScript.mMessages.mSearchSearchingMessage + "</h2>\n");

      BookList = WWHFrame.WWHHelp.mBooks.mBookList;

      // Generate search actions
      //
      for (MaxIndex = this.mCombinedResults.mEntries.length, Index = 0 ; Index < MaxIndex ; Index += 1)
      {
        Entry = this.mCombinedResults.mEntries[Index];

        MultiPhraseEntry = this.mMultiPhraseList[Entry.mBookIndex];
        if (MultiPhraseEntry.fPhraseCount() > 0)
        {
          BookDirectory = BookList[Entry.mBookIndex].mDirectory;

          HTML.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + BookDirectory + "wwhdata/js/search/pairs/pair" + Entry.mFileIndex + ".js" + VarParameters + "\"></script>\n");
          HTML.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/scripts/search5s.js" + VarParameters + "\"></script>\n");
        }

        HTML.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/scripts/search6s.js" + VarParameters + "\"></script>\n");
      }

      HTML.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/scripts/search7s.js" + VarParameters + "\"></script>\n");
    }
    else
    {
      // Define accessor functions to reduce file size
      //
      HTML.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\">\n");
      HTML.fAppend(" <!--\n");
      HTML.fAppend("  function  fC(ParamEntryID)\n");
      HTML.fAppend("  {\n");
      HTML.fAppend("    WWHFrame.WWHSearch.fShowEntry(ParamEntryID);\n");
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

      // Display search message and/or prepare results for display
      //
      if (this.mSavedSearchWords.length === 0)
      {
        HTML.fAppend("<h3>" + WWHFrame.WWHJavaScript.mMessages.mSearchDefaultMessage + "</h3>\n");
      }
      else if ((this.mCombinedResults.mEntries !== undefined) &&
               (this.mCombinedResults.mEntries.length > 0))
      {
        // Determine if book name should be displayed about results
        //
        if ((WWHFrame.WWHHelp.mBooks.mBookList.length === 1) ||
            ((this.mSavedSearchScope > 0) &&
             (this.mSearchScopeInfo.mEntries[this.mSavedSearchScope - 1].mStartBookIndex === this.mSearchScopeInfo.mEntries[this.mSavedSearchScope - 1].mEndBookIndex)))
        {
          // Single book scope selected, do not display book titles
          //
          bDisplayBookTitles = false;
        }
        else
        {
          // More than one book in search scope, display book titles
          //
          bDisplayBookTitles = true;
        }

        this.mCombinedResults.fDisplayReset(bDisplayBookTitles);
      }
      else
      {
        HTML.fAppend("<h3>" + WWHFrame.WWHJavaScript.mMessages.mSearchNothingFoundMessage + "</h3>\n");
      }
    }
  }

  return HTML.fGetBuffer();
}

function  WWHSearch_AdvanceHTMLSegment()
{
  var  bSegmentCreated = false;


  if (this.mbPanelInitialized)
  {
    if (this.mSearchState === null)
    {
      bSegmentCreated = this.mCombinedResults.fDisplayAdvance();
    }
  }

  return bSegmentCreated;
}

function  WWHSearch_GetHTMLSegment()
{
  return this.mCombinedResults.mHTMLSegment.fGetBuffer();
}

function  WWHSearch_EndHTMLSegments()
{
  return "";
}

function  WWHSearch_FocusSearchWords()
{
  var VarPanelNavigationFrame, VarSearchForm;

  WWHFrame.WWHHelp.fFocus("WWHPanelNavigationFrame");
  VarPanelNavigationFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHPanelNavigationFrame"));
  VarSearchForm = VarPanelNavigationFrame.document.forms["WWHSearchForm"];
  VarSearchForm.elements["WWHSearchWordsText"].focus();
}

function  WWHSearch_PanelNavigationLoaded()
{
  // Set focus
  //
  this.fFocusSearchWords();

  // Focus on which frame?
  //
  if ((this.mCombinedResults.mEntries !== undefined) &&
      (this.mCombinedResults.mEntries.length > 0))
  {
    if ( ! this.mbChangingToSearch)
    {
      WWHFrame.WWHHelp.fFocus("WWHPanelViewFrame");
    }
  }

  // Changing tabs?
  //
  this.mbChangingToSearch = WWHFrame.WWHJavaScript.mPanels.mbChangingPanels;

  // Set accessibility title
  //
  if (WWHFrame.WWHHelp.mbAccessible)
  {
    WWHFrame.WWHHelp.fSetFrameName("WWHPanelNavigationFrame");
  }
}

function  WWHSearch_PanelViewLoaded()
{
  // Display search results if necessary
  //
  if (this.mSearchState === "words")
  {
    this.mSearchState = "pairs";

    WWHFrame.WWHJavaScript.mPanels.fReloadView();
  }
  else if (this.mSearchState === "pairs")
  {
    this.mSearchState = null;

    WWHFrame.WWHJavaScript.mPanels.fReloadView();
  }
  else
  {
    // Ensure search form message is up-to-date
    //
    if (this.mbChangingToSearch)
    {
      this.fFocusSearchWords();

      this.mbChangingToSearch = false;
    }
    else
    {
      WWHFrame.WWHJavaScript.mPanels.fReloadNavigation();
    }
  }

  // Set accessibility title
  //
  if (WWHFrame.WWHHelp.mbAccessible)
  {
    WWHFrame.WWHHelp.fSetFrameName("WWHPanelViewFrame");
  }
}

function  WWHSearch_HoverTextTranslate(ParamEntryID)
{
  var  HTML     = "";
  var  BookList = WWHFrame.WWHHelp.mBooks.mBookList;
  var  Settings = WWHFrame.WWHJavaScript.mSettings.mSearch;
  var  Messages = WWHFrame.WWHJavaScript.mMessages;
  var  Entry;
  var  Rank = "";
  var  Title;
  var  Book = "";
  var  Format;


  // Retrieve specified entry
  //
  Entry = this.mCombinedResults.mEntries[ParamEntryID];

  // Get Rank
  //
  if (Settings.mbShowRank)
  {
    Rank = Math.floor((Entry.mScore / this.mCombinedResults.mMaxScore) * 100) + "%";
  }

  // Get Title
  //
  Title = Entry.mTitle;

  // Get Book
  //
  if ((BookList.length > 1) &&                 // More than one book exists
      (this.mCombinedResults.mSortedBy === 1))  // By Score
  {
    Book = BookList[Entry.mBookIndex].mTitle;
  }

  // Format for display
  //
  if ((Rank.length === 0) &&
      (Book.length === 0))
  {
    // Simple format, just the title
    //
    HTML = Title;
  }
  else
  {
    Format = " align=\"left\" valign=\"top\"><span style=\"" + WWHFrame.WWHJavaScript.mSettings.mHoverText.mFontStyle + "\">";

    // Complex format, requires a table
    //
    HTML += "<table width=\"100%\" border=\"0\" cellpadding=\"4\" cellspacing=\"0\">";
    if (Rank.length > 0)
    {
      HTML += "<tr>";
      HTML += "<th style=\"white-space: nowrap;\"" + Format + Messages.mSearchRankLabel + "</span></th>";
      HTML += "<td" + Format + Rank + "</span></td>";
      HTML += "</tr>";
    }
    HTML += "<tr>";
    HTML += "<th style=\"white-space: nowrap;\"" + Format + Messages.mSearchTitleLabel + "</span></th>";
    HTML += "<td" + Format + Title + "</span></td>";
    HTML += "</tr>";
    if (Book.length > 0)
    {
      HTML += "<tr>";
      HTML += "<th style=\"white-space: nowrap;\"" + Format + Messages.mSearchBookLabel + "</span></th>";
      HTML += "<td" + Format + Book + "</span></td>";
      HTML += "</tr>";
    }
    HTML += "</table>";

    // IE 5.0 on the Macintosh drops the last table for some reason
    //
    if (WWHFrame.WWHBrowser.mbMacIE50)
    {
      HTML += "<table><tr><td></td></tr></table>";
    }
  }

  return HTML;
}

function  WWHSearch_HoverTextFormat(ParamWidth,
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

function  WWHSearch_DisplaySearchForm()
{
  var  HTML = "";
  var  AccessibilityMessage;
  var  BookList = WWHFrame.WWHHelp.mBooks.mBookList;
  var  SelectedIndex;
  var  MaxIndex;
  var  Index;
  var  SearchScopeEntry;
  var  MaxLevel;
  var  Level;


  HTML += "<form name=\"WWHSearchForm\" onsubmit=\"WWHFrame.WWHSearch.fSubmit();\">\n";
  HTML += "<nobr>\n";

  // Accessibility support
  //
  if (WWHFrame.WWHHelp.mbAccessible)
  {
    // Determine message to display
    //
    if (this.mSavedSearchWords.length === 0)
    {
      AccessibilityMessage = WWHFrame.WWHJavaScript.mMessages.mSearchDefaultMessage;
    }
    else
    {
      if ((this.mCombinedResults.mEntries !== undefined) &&
          (this.mCombinedResults.mEntries.length > 0))
      {
        AccessibilityMessage = WWHFrame.WWHJavaScript.mMessages.mSearchDefaultMessage;
      }
      else
      {
        AccessibilityMessage = WWHFrame.WWHJavaScript.mMessages.mSearchNothingFoundMessage;
      }
    }

    // Label search words input field
    //
    HTML += "<label for=\"WWHSearchWordsText\">";
    HTML += AccessibilityMessage;
    HTML += "</label>";
    HTML += "<br />";
  }

  HTML += "<input tabindex=\"1\" type=\"text\" name=\"WWHSearchWordsText\" size=\"20\" value=\"" + WWHStringUtilities_EscapeHTML(this.mSavedSearchWords) + "\" onkeydown=\"WWHFrame.WWHHelp.fIgnoreNextKeyPress((document.all||document.getElementById||document.layers)?event:null);\" onfocus=\"WWHFrame.WWHSearch.fOnFocus_SearchWords(this);\">\n";
  HTML += "<input tabindex=\"2\" type=\"submit\" value=\"" + WWHFrame.WWHJavaScript.mMessages.mSearchButtonLabel + "\">\n";
  HTML += "</nobr>\n";

  if (BookList.length > 1)
  {
    SelectedIndex = this.mSavedSearchScope - 1;

    HTML += "<br>\n";
    HTML += "<select name=\"WWHSearchScope\">\n";
    HTML += "<option>" + WWHFrame.WWHJavaScript.mMessages.mSearchScopeAllLabel + "</option>\n";
    for (MaxIndex = this.mSearchScopeInfo.mEntries.length, Index = 0 ; Index < MaxIndex ; Index += 1)
    {
      // Access current search scope entry
      //
      SearchScopeEntry = this.mSearchScopeInfo.mEntries[Index];

      // Restore selection
      //
      if (Index === SelectedIndex)
      {
        HTML += "<option selected>";
      }
      else
      {
        HTML += "<option>";
      }

      // Indent to show different levels
      //
      for (MaxLevel = SearchScopeEntry.mLevel, Level = 0 ; Level < MaxLevel ; Level += 1)
      {
        HTML += "- ";
      }

      // Close out entry
      //
      HTML += SearchScopeEntry.mTitle + "</option>\n";
    }
    HTML += "</select>\n";
  }

  HTML += "</form>\n";

  return HTML;
}

function  WWHSearch_OnFocus_SearchWords(ParamTextInput)
{
  var  VarTextRange;
  var  VarValue;

  if (ParamTextInput.createTextRange)
  {
    VarTextRange = ParamTextInput.createTextRange();
    VarTextRange.moveStart('character', 0);
    VarTextRange.moveEnd('character', ParamTextInput.value.length);
    VarTextRange.select();
  }
  else if (ParamTextInput.setSelectionRange)
  {
    ParamTextInput.setSelectionRange(0, ParamTextInput.value.length);
  }
  else
  {
    VarValue = ParamTextInput.value;
    ParamTextInput.value = '';
    ParamTextInput.value = VarValue;
  }
}

function  WWHSearch_Submit()
{
  var  VarPanelNavigationFrame;
  var  SearchForm;
  var  NewSearchWords;
  var  NewSearchScope;
  var  MaxIndex;
  var  Index;


  if ((WWHFrame.WWHHandler.fIsReady()) &&
      (this.mSearchState === null))
  {
    VarPanelNavigationFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHPanelNavigationFrame"));
    SearchForm = VarPanelNavigationFrame.document.forms["WWHSearchForm"];

    // Update search words
    //
    NewSearchWords = SearchForm.elements["WWHSearchWordsText"].value;
    if (NewSearchWords !== this.mSavedSearchWords)
    {
      this.mSavedSearchWords = NewSearchWords;

      this.mSearchState = "words";
    }

    // Update search scope
    //
    if (WWHFrame.WWHHelp.mBooks.mBookList.length > 1)
    {
      NewSearchScope = SearchForm.elements["WWHSearchScope"].selectedIndex;
      if (NewSearchScope !== this.mSavedSearchScope)
      {
        this.mSavedSearchScope = NewSearchScope;

        this.mSearchState = "words";
      }
    }

    // Perform search if something changed
    //
    if (this.mSearchState === "words")
    {
      if (this.mSavedSearchWords.length > 0)
      {
        // Clear previous results
        //
        for (MaxIndex = this.mBookMatchesList.length, Index = 0 ; Index < MaxIndex ; Index += 1)
        {
          this.mBookMatchesList[Index].fClear();
        }
        this.mCombinedResults.fClear();

        // Perform search
        //
        this.fSetSearchWords(this.mSavedSearchWords);
        WWHFrame.WWHJavaScript.mPanels.fClearScrollPosition();

        // Submit will cause navigation area to reload which will trigger the view pane
        // to reload and perform the search.
        //
      }
    }
  }

  return (this.mSearchState === "words");
}

function  WWHSearch_ApplyWordBreaks(ParamSearchWordsString)
{
  var  VarResult = "";
  var  VarMaxIndex;
  var  VarIndex;
  var  VarBreak;

  // Apply Unicode rules for word breaking
  // These rules taken from http://www.unicode.org/unicode/reports/tr29/
  //
  for (VarMaxIndex = ParamSearchWordsString.length, VarIndex = 0; VarIndex < VarMaxIndex ; VarIndex += 1)
  {
    // Break?
    //
    VarBreak = WWHUnicode_CheckBreakAtIndex(ParamSearchWordsString, VarIndex);
    if (VarBreak)
    {
      VarResult += " " + ParamSearchWordsString.charAt(VarIndex);
    }
    else
    {
      VarResult += ParamSearchWordsString.charAt(VarIndex);
    }
  }
  
  return VarResult;
}

function  WWHSearch_SetSearchWords(ParamSearchWordsString)
{
  var  SearchWordAndPhraseList;
  var  SearchWordAndPhraseEntry;
  var  SearchWordList;
  var  MaxIndex;
  var  Index;
  var  MaxWordIndex;
  var  WordIndex;
  var  SearchWord;
  var  SearchRegExpPattern;


  // Clear search words
  //
  this.mSearchWordList.length = 0;
  this.mSearchWordRegExpList.length = 0;

  // Add search words to hash
  //
  SearchWordAndPhraseList = WWHStringUtilities_ParseWordsAndPhrases(ParamSearchWordsString.toLowerCase());
  for (MaxIndex = SearchWordAndPhraseList.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    SearchWordAndPhraseEntry = this.fApplyWordBreaks(SearchWordAndPhraseList[Index]);
    SearchWordList = WWHStringUtilities_ParseWordsAndPhrases(SearchWordAndPhraseEntry);
    for (MaxWordIndex = SearchWordList.length, WordIndex = 0 ; WordIndex < MaxWordIndex ; WordIndex += 1)
    {
      SearchWord = SearchWordList[WordIndex];

      // Skip 0 length words
      //
      if (SearchWord.length > 0)
      {
        // Add to search words hash
        //
        SearchRegExpPattern = WWHStringUtilities_WordToRegExpPattern(SearchWord);

        this.mSearchWordList[this.mSearchWordList.length] = SearchWord;
        this.mSearchWordRegExpList[this.mSearchWordRegExpList.length] = new RegExp(SearchRegExpPattern, "i");
      }
    }
  }

  // Create a items in mMultiPhraseList array that parallel the mBookSearchInfoList array
  // each MultiPhrase item is passed the corresponding index from mBookSearchInfoList and
  // the search words
  //
  var BookSearchIndex;
  var NewMultiPhrase;
  var CurrentBookSearch;
  this.mMultiPhraseList.length = 0;
  for(BookSearchIndex = 0; BookSearchIndex < this.mBookSearchInfoList.length; BookSearchIndex += 1)
  {
    CurrentBookSearch = this.mBookSearchInfoList[BookSearchIndex];
    NewMultiPhrase = new MultiPhrase_Object(ParamSearchWordsString.toLowerCase(), CurrentBookSearch);
    NewMultiPhrase.fParse();
    this.mMultiPhraseList[this.mMultiPhraseList.length] = NewMultiPhrase;
  }
}

function  WWHSearch_CheckForMatch(ParamSearchFunc)
{
  var  Count;
  var  MaxIndex;
  var  Index;
  var  BookMatchesListEntry;
  var  SearchPattern;


  Count = 0;
  for (MaxIndex = this.mSearchWordList.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    if (this.mBookSearchInfoList[this.mBookIndex].fValidSearchWord(this.mSearchWordList[Index]))
    {
      BookMatchesListEntry = this.mBookMatchesList[this.mBookIndex];

      BookMatchesListEntry.fSetMatchedWordIndex(Count);

      SearchPattern = this.mSearchWordRegExpList[Index];
      SearchPattern.t = SearchPattern.test;

      ParamSearchFunc(SearchPattern, BookMatchesListEntry);

      Count += 1;
    }
  }
}

function  WWHSearch_SearchWordsComplete()
{
  // Combine results for display
  //
  this.fCombineResults();
  this.mCombinedResultsIndex = 0;
}

function  WWHSearch_CheckForPhraseMatch(ParamSearchFunc)
{
  var  Entry;
  var  BookIndex;
  var  MultiPhraseEntry;

  Entry            = this.mCombinedResults.mEntries[this.mCombinedResultsIndex];
  BookIndex        = Entry.mBookIndex;
  MultiPhraseEntry = this.mMultiPhraseList[BookIndex];

  // Reset the incremented values of the matching word pairs
  // for this multiphrase object for a new round of testing
  //
  MultiPhraseEntry.fResetMatches();

  // Perform Test
  //
  ParamSearchFunc(MultiPhraseEntry);

  // Match?
  //
  if ( ! MultiPhraseEntry.fCheckForMatch())
  {
    // Remove from results
    //
    this.mCombinedResults.mEntries[this.mCombinedResultsIndex] = null;
  }
}

function  WWHSearch_SearchPairsComplete()
{
  var  Entries = [];
  var  MaxIndex;
  var  Index;

  // Remove failed matches
  //
  for (MaxIndex = this.mCombinedResults.mEntries.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    if (this.mCombinedResults.mEntries[Index] !== null)
    {
      Entries[Entries.length] = this.mCombinedResults.mEntries[Index];
    }
  }
  this.mCombinedResults.mEntries = Entries;

  // Sort results based on single or multi-book display
  //
  if ((WWHFrame.WWHJavaScript.mSettings.mSearch.mbResultsByBook) ||
      ((WWHFrame.WWHHelp.mBooks.mBookList.length === 1) ||
       ((this.mSavedSearchScope > 0) &&
        (this.mSearchScopeInfo.mEntries[this.mSavedSearchScope - 1].mStartBookIndex === this.mSearchScopeInfo.mEntries[this.mSavedSearchScope - 1].mEndBookIndex))))
  {
    this.mCombinedResults.fSortByBookIndex();
  }
  else
  {
    this.mCombinedResults.fSortByScore();
  }
}

function  WWHSearch_CombineResults()
{
  var  MaxBookIndex;
  var  BookIndex;
  var  BookMatches;
  var  BookListEntry;
  var  FileID;
  var  FileIndex;


  this.mCombinedResults.fClear();
  for (MaxBookIndex = this.mBookMatchesList.length, BookIndex = 0 ; BookIndex < MaxBookIndex ; BookIndex += 1)
  {
    BookMatches = this.mBookMatchesList[BookIndex];
    BookListEntry = WWHFrame.WWHHelp.mBooks.mBookList[BookIndex];

    // Add results
    //
    BookMatches.fJoinFileScores();
    for (FileID in BookMatches.mFileScores)
    {
      FileIndex = parseInt(FileID.substring(1, FileID.length), 10);

      this.mCombinedResults.fAddEntry(BookIndex, FileIndex, BookMatches.mFileScores[FileID], BookListEntry.mFiles.fFileIndexToTitle(FileIndex));
    }
  }
}

function  WWHSearch_ShowEntry(ParamIndex)
{
  this.mCombinedResults.fShowEntry(ParamIndex);
}

function  WWHSearchScope_Entry_Object(ParamLevel,
                                      ParamTitle,
                                      ParamBookIndex)
{
  this.mLevel          = ParamLevel;
  this.mTitle          = ParamTitle;
  this.mStartBookIndex = ParamBookIndex;
  this.mEndBookIndex   = ParamBookIndex;
}

function  WWHSearchScope_Object()
{
  this.mEntries = [];
  this.mGroupStack = [];
  this.mBookIndex = 0;

  this.fAddScopeEntries = WWHSearchScope_AddScopeEntries;

  // Set scope entries
  //
  this.fAddScopeEntries(WWHFrame.WWHHelp.mBookGroups);
}

function  WWHSearchScope_AddScopeEntries(ParamGroup)
{
  var  MaxIndex;
  var  Index;
  var  MaxGroupStackIndex;
  var  GroupStackIndex;
  var  ScopeEntry;


  for (MaxIndex = ParamGroup.mChildren.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    if (ParamGroup.mChildren[Index].mbGrouping)
    {
      // Add an entry
      //
      ScopeEntry = new WWHSearchScope_Entry_Object(this.mGroupStack.length, ParamGroup.mChildren[Index].mTitle, -1);
      this.mEntries[this.mEntries.length] = ScopeEntry;

      // Push this entry onto the group stack
      //
      this.mGroupStack[this.mGroupStack.length] = ScopeEntry;

      // Process group entries
      //
      this.fAddScopeEntries(ParamGroup.mChildren[Index]);

      // Pop this entry off the group stack
      //
      this.mGroupStack.length -= 1;
    }
    else
    {
      // Add an entry
      //
      this.mEntries[this.mEntries.length] = new WWHSearchScope_Entry_Object(this.mGroupStack.length, WWHFrame.WWHHelp.mBooks.mBookList[this.mBookIndex].mTitle, this.mBookIndex);

      // Process all entries in the group stack, updating start/end book indicies
      //
      for (MaxGroupStackIndex = this.mGroupStack.length, GroupStackIndex = 0 ; GroupStackIndex < MaxGroupStackIndex ; GroupStackIndex += 1)
      {
        ScopeEntry = this.mGroupStack[GroupStackIndex];

        // Update start
        //
        if (ScopeEntry.mStartBookIndex === -1)
        {
          ScopeEntry.mStartBookIndex = this.mBookIndex;
        }

        // Update end
        //
        ScopeEntry.mEndBookIndex = this.mBookIndex;
      }

      // Increment book index
      //
      this.mBookIndex += 1;
    }
  }
}

function  WWHBookSearchInfo_Object(ParamSearchFileCount,
                                   ParamMinimumWordLength)
{
  this.mSearchFileCount   = ParamSearchFileCount;
  this.mMinimumWordLength = ParamMinimumWordLength;
  this.mSkipWords         = new WWHBookSearchInfo_SkipWords_Object();

  this.fAddSkipWord     = WWHBookSearchInfo_AddSkipWord;
  this.fA               = WWHBookSearchInfo_AddSkipWord;
  this.fValidSearchWord = WWHBookSearchInfo_ValidSearchWord;
}

function  WWHBookSearchInfo_AddSkipWord(ParamSkipWord)
{
  if (ParamSkipWord.length > 0)
  {
    this.mSkipWords[ParamSkipWord + "~"] = 1;
  }
}

function  WWHBookSearchInfo_ValidSearchWord(ParamSearchWord)
{
  var  bValid = true;

  if (ParamSearchWord.length === 0)
  {
    bValid = false;
  }
  else if ((ParamSearchWord.length < this.mMinimumWordLength) && (ParamSearchWord.indexOf("*") < 0))
  {
    bValid = false;
  }
  else if (typeof(this.mSkipWords[ParamSearchWord + "~"]) === "number")
  {
    bValid = false;
  }

  return bValid;
}

function  WWHBookSearchInfo_SkipWords_Object()
{
}

function  WWHSearchBookMatches_Object()
{
  this.mFirstMatchedWordIndex = -1;
  this.mMatchedWordIndex      = -1;
  this.mWordFileScores        = [];
  this.mFileScores            = new WWHSearchBookMatches_FileScores_Object();

  this.fClear               = WWHSearchBookMatches_Clear;
  this.fSetMatchedWordIndex = WWHSearchBookMatches_SetMatchedWordIndex;
  this.fAddMatches          = WWHSearchBookMatches_AddMatches;
  this.f                    = WWHSearchBookMatches_AddMatches;  // For smaller search files
  this.fJoinFileScores      = WWHSearchBookMatches_JoinFileScores;
}

function  WWHSearchBookMatches_Clear()
{
  this.mFirstMatchedWordIndex = -1;
  this.mMatchedWordIndex      = -1;
  this.mWordFileScores.length = 0;
  this.mFileScores            = new WWHSearchBookMatches_FileScores_Object();
}

function  WWHSearchBookMatches_SetMatchedWordIndex(ParamMatchedWordIndex)
{
  this.mMatchedWordIndex = ParamMatchedWordIndex;
  if (ParamMatchedWordIndex === this.mWordFileScores.length)
  {
    this.mWordFileScores[this.mWordFileScores.length] = new WWHSearchBookMatches_FileScores_Object();
  }
}

function  WWHSearchBookMatches_AddMatches(ParamMatchString)
{
  var  MatchList = null;
  var  WordFileScoresEntry;
  var  MaxIndex;
  var  Index;
  var  FileID;
  var  Score;


  if (ParamMatchString !== undefined)
  {
    MatchList = ParamMatchString.split(",");
  }

  if ((MatchList !== null) &&
      (MatchList.length > 0))
  {
    WordFileScoresEntry = this.mWordFileScores[this.mMatchedWordIndex];

    // Add all entries to word file score entry
    //
    for (MaxIndex = MatchList.length, Index = 0 ; Index < MaxIndex ; Index += 2)
    {
      FileID = "i" + MatchList[Index];
      Score  = MatchList[Index + 1];

      if (WordFileScoresEntry[FileID] === undefined)
      {
        WordFileScoresEntry[FileID] = parseInt(Score, 10);
      }
      else
      {
        WordFileScoresEntry[FileID] += parseInt(Score, 10);
      }
    }
  }
}

function  WWHSearchBookMatches_JoinFileScores()
{
  var  MaxIndex;
  var  Index;
  var  WordFileScoresEntry;
  var  FileID;


  this.mFileScores = new WWHSearchBookMatches_FileScores_Object();
  for (MaxIndex = this.mWordFileScores.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    WordFileScoresEntry = this.mWordFileScores[Index];

    if (Index === 0)
    {
      // Add all entries if first entry
      //
      this.mFileScores = WordFileScoresEntry;
    }
    else
    {
      // Remove all entries not found in results set
      //
      for (FileID in this.mFileScores)
      {
        if (typeof(WordFileScoresEntry[FileID]) === "number")
        {
          this.mFileScores[FileID] += WordFileScoresEntry[FileID];
        }
        else
        {
          delete this.mFileScores[FileID];
        }
      }
    }
  }
}

function  WWHSearchBookMatches_FileScores_Object()
{
}

function  WWHSearchResults_Object()
{
  this.mSortedBy     = null;
  this.mEntries      = [];
  this.mMaxScore     = 0;
  this.mDisplayIndex = 0;
  this.mByBookDetect = -1;
  this.mHTMLSegment  = new WWHStringBuffer_Object();
  this.mEventString  = WWHPopup_EventString();

  this.fClear           = WWHSearchResults_Clear;
  this.fAddEntry        = WWHSearchResults_AddEntry;
  this.fSortByScore     = WWHSearchResults_SortByScore;
  this.fSortByBookIndex = WWHSearchResults_SortByBookIndex;
  this.fDisplayReset    = WWHSearchResults_DisplayReset;
  this.fDisplayAdvance  = WWHSearchResults_DisplayAdvance;
  this.fGetPopupAction  = WWHSearchResults_GetPopupAction;
  this.fShowEntry       = WWHSearchResults_ShowEntry;
}

function  WWHSearchResults_Clear()
{
  this.mSortedBy       = null;
  this.mEntries.length = 0;
  this.mMaxScore       = 0;
}

function  WWHSearchResults_AddEntry(ParamBookIndex,
                                    ParamFileIndex,
                                    ParamScore,
                                    ParamTitle)
{
  // Add a new entry
  //
  this.mEntries[this.mEntries.length] = new WWHSearchResultsEntry_Object(ParamBookIndex,
                                                                         ParamFileIndex,
                                                                         ParamScore,
                                                                         ParamTitle);

  // Bump mMaxScore if necessary
  //
  if (ParamScore > this.mMaxScore)
  {
    this.mMaxScore = ParamScore;
  }
}

function  WWHSearchResults_SortByScore()
{
  this.mSortedBy = 1;  // By Score

  if (this.mEntries.length > 0)
  {
    this.mEntries = this.mEntries.sort(WWHSearchResultsEntry_ByScoreByBookIndexByTitleFileIndexURL);
  }
}

function  WWHSearchResults_SortByBookIndex()
{
  this.mSortedBy = 2;  // By BookIndex

  if (this.mEntries.length > 0)
  {
    this.mEntries = this.mEntries.sort(WWHSearchResultsEntry_ByBookIndexByScoreByTitleFileIndexURL);
  }
}

function  WWHSearchResults_DisplayReset(bParamDisplayBookTitles)
{
  this.mDisplayIndex = 0;
  this.mByBookDetect = -1;

  if ( ! bParamDisplayBookTitles)
  {
    this.mByBookDetect = -2;
  }
}

function  WWHSearchResults_DisplayAdvance()
{
  var  bSegmentCreated = false;
  var  Settings = WWHFrame.WWHJavaScript.mSettings.mSearch;
  var  HTML;
  var  MaxHTMLSegmentSize;
  var  BookList;
  var  MaxIndex;
  var  Index;
  var  Entry;
  var  VarAccessibilityTitle = "";
  var  VarPercent;


  // Insure that there is something to display
  //
  if ((this.mSortedBy !== null) &&
      (this.mEntries.length > 0))
  {
    MaxHTMLSegmentSize = WWHFrame.WWHJavaScript.mMaxHTMLSegmentSize;
    this.mHTMLSegment.fReset();
    BookList = WWHFrame.WWHHelp.mBooks.mBookList;

    // If this is the first entry, display the headers and open the list
    //
    if (this.mDisplayIndex === 0)
    {
      HTML = "";

      HTML += "<p><nobr><b>";

      // Display column headers
      //
      if (Settings.mbShowRank)
      {
        HTML += WWHFrame.WWHJavaScript.mMessages.mSearchRankLabel + " ";
      }
      HTML += WWHFrame.WWHJavaScript.mMessages.mSearchTitleLabel;
      if ((BookList.length > 1) &&  // More than one book exists
          (this.mSortedBy === 1))    // By Score
      {
        HTML += ", " + WWHFrame.WWHJavaScript.mMessages.mSearchBookLabel;
      }
      HTML += "</b></nobr></p>\n";

      HTML += "<ol>\n";

      this.mHTMLSegment.fAppend(HTML);
    }

    // Display result entries
    //
    MaxIndex = this.mEntries.length;
    Index = this.mDisplayIndex;
    while ((this.mHTMLSegment.fSize() < MaxHTMLSegmentSize) &&
           (Index < MaxIndex))
    {
      HTML = "";

      Entry = this.mEntries[Index];

      // Display Book
      //
      if ((BookList.length > 1) &&  // More than one book exists
          (this.mSortedBy === 2))    // By BookIndex
      {
        if (this.mByBookDetect !== -2)
        {
          if (this.mByBookDetect !== Entry.mBookIndex)
          {
            // Close list for previous book
            //
            if (Index > 0)
            {
              HTML += "</ol>\n";
            }

            HTML += "<p><nobr>&nbsp;</nobr></p>";
            HTML += "<p class=\"BookTitle\"><nobr>" + BookList[Entry.mBookIndex].mTitle + "</nobr></p>";

            this.mByBookDetect = Entry.mBookIndex;

            // Open new list for next book
            //
            HTML += "<ol>\n";
          }
        }
      }

      // Accessibility support
      //
      if (WWHFrame.WWHHelp.mbAccessible)
      {
        VarAccessibilityTitle = "";

        // Rank
        //
        if (Settings.mbShowRank)
        {
          VarPercent = Math.floor((Entry.mScore / this.mMaxScore) * 100);

          // Some browsers do not allow value attributes to be 0
          //
          if (VarPercent < 1)
          {
            VarPercent = 1;
          }

          VarAccessibilityTitle += WWHStringUtilities_EscapeHTML(WWHFrame.WWHJavaScript.mMessages.mSearchRankLabel + " " + VarPercent + ", ");
        }

        // Title
        //
        VarAccessibilityTitle += WWHStringUtilities_EscapeHTML(WWHFrame.WWHJavaScript.mMessages.mSearchTitleLabel + " " + Entry.mTitle);

        // Book
        //
        if (BookList.length > 1)  // More than one book exists
        {
          VarAccessibilityTitle += WWHStringUtilities_EscapeHTML(WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " " + WWHFrame.WWHJavaScript.mMessages.mSearchBookLabel + " " + BookList[Entry.mBookIndex].mTitle);
        }

        VarAccessibilityTitle = " title=\"" + VarAccessibilityTitle + "\"";
      }

      // Display Rank
      //
      if (Settings.mbShowRank)
      {
        VarPercent = Math.floor((Entry.mScore / this.mMaxScore) * 100);

        // Some browsers do not allow value attributes to be 0
        //
        if (VarPercent < 1)
        {
          VarPercent = 1;
        }

        HTML += "<li value=\"" + VarPercent + "\">";
      }
      else
      {
        HTML += "<li>";
      }

      // Display Title
      //
      HTML += "<a href=\"javascript:fC(" + Index + ");\"" + this.fGetPopupAction(Index) + VarAccessibilityTitle + ">";
      HTML += Entry.mTitle;
      HTML += "</a>";

      // Display Book
      //
      if ((BookList.length > 1) &&  // More than one book exists
          (this.mSortedBy === 1))    // By Score
      {
        HTML += ", " + BookList[Entry.mBookIndex].mTitle;
      }

      HTML += "</li>\n";

      this.mHTMLSegment.fAppend(HTML);

      Index += 1;
    }

    // Record current display index so we can pick up where we left off
    //
    this.mDisplayIndex = Index;
    if (this.mHTMLSegment.fSize() > 0)
    {
      bSegmentCreated = true;
    }

    // If this is the last entry, close the list
    //
    if (this.mDisplayIndex === this.mEntries.length)
    {
      this.mHTMLSegment.fAppend("</ol>\n");
    }
  }

  return bSegmentCreated;
}

function  WWHSearchResults_GetPopupAction(ParamEntryIndex)
{
  var  PopupAction = "";


  if (WWHFrame.WWHJavaScript.mSettings.mHoverText.mbEnabled)
  {
    PopupAction += " onmouseover=\"fS('" + ParamEntryIndex + "', " + this.mEventString + ");\"";
    PopupAction += " onmouseout=\"fH();\"";
  }

  return PopupAction;
}

function  WWHSearchResults_ShowEntry(ParamIndex)
{
  var  Entry;
  var  URL;


  // Update highlight words
  //
  WWHFrame.WWHHighlightWords.fSetWordList(WWHStringUtilities_ParseWordsAndPhrases(WWHFrame.WWHSearch.mSavedSearchWords));

  // Display document
  //
  Entry = this.mEntries[ParamIndex];
  URL = WWHFrame.WWHHelp.fGetBookIndexFileIndexURL(Entry.mBookIndex, Entry.mFileIndex, null);
  WWHFrame.WWHHelp.fSetDocumentHREF(URL, false);
}

function  WWHSearchResultsEntry_Object(ParamBookIndex,
                                       ParamFileIndex,
                                       ParamScore,
                                       ParamTitle)
{
  this.mBookIndex = ParamBookIndex;
  this.mFileIndex = ParamFileIndex;
  this.mScore     = ParamScore;
  this.mTitle     = ParamTitle;
}

function  WWHSearchResultsEntry_ByScoreByBookIndexByTitleFileIndexURL(ParamAlphaEntry,
                                                                      ParamBetaEntry)
{
  var  Result;


  Result = WWHSearchResultsEntry_CompareByScore(ParamAlphaEntry, ParamBetaEntry);
  if (Result === 0)
  {
    Result = WWHSearchResultsEntry_CompareByBookIndex(ParamAlphaEntry, ParamBetaEntry);
  }
  if (Result === 0)
  {
    Result = WWHSearchResultsEntry_CompareByTitleFileIndexURL(ParamAlphaEntry, ParamBetaEntry);
  }

  return Result;
}

function  WWHSearchResultsEntry_ByBookIndexByScoreByTitleFileIndexURL(ParamAlphaEntry,
                                                                      ParamBetaEntry)
{
  var  Result;


  Result = WWHSearchResultsEntry_CompareByBookIndex(ParamAlphaEntry, ParamBetaEntry);
  if (Result === 0)
  {
    Result = WWHSearchResultsEntry_CompareByScore(ParamAlphaEntry, ParamBetaEntry);
  }
  if (Result === 0)
  {
    Result = WWHSearchResultsEntry_CompareByTitleFileIndexURL(ParamAlphaEntry, ParamBetaEntry);
  }

  return Result;
}

function  WWHSearchResultsEntry_CompareByScore(ParamAlphaEntry,
                                               ParamBetaEntry)
{
  var  Result = 0;


  // Sort by score
  //
  if (ParamAlphaEntry.mScore < ParamBetaEntry.mScore)
  {
    Result = 1;
  }
  else if (ParamAlphaEntry.mScore > ParamBetaEntry.mScore)
  {
    Result = -1;
  }

  return Result;
}

function  WWHSearchResultsEntry_CompareByBookIndex(ParamAlphaEntry,
                                                   ParamBetaEntry)
{
  var  Result = 0;


  if (ParamAlphaEntry.mBookIndex < ParamBetaEntry.mBookIndex)
  {
    Result = -1;
  }
  else if (ParamAlphaEntry.mBookIndex > ParamBetaEntry.mBookIndex)
  {
    Result = 1;
  }

  return Result;
}

function  WWHSearchResultsEntry_CompareByTitleFileIndexURL(ParamAlphaEntry,
                                                           ParamBetaEntry)
{
  var  Result = 0;
  var  BookList;
  var  AlphaBookEntry;
  var  BetaBookEntry;
  var  AlphaURL;
  var  BetaURL;


  // Sort by Title
  //
  if (ParamAlphaEntry.mTitle < ParamBetaEntry.mTitle)
  {
    Result = -1;
  }
  else if (ParamAlphaEntry.mTitle > ParamBetaEntry.mTitle)
  {
    Result = 1;
  }
  // Sort by FileIndex
  //
  else if (ParamAlphaEntry.mFileIndex < ParamBetaEntry.mFileIndex)
  {
    Result = -1;
  }
  else if (ParamAlphaEntry.mFileIndex > ParamBetaEntry.mFileIndex)
  {
    Result = 1;
  }
  // Sort by URL
  //
  else
  {
    BookList = WWHFrame.WWHHelp.mBooks.mBookList;

    AlphaBookEntry = BookList[ParamAlphaEntry.mBookIndex];
    BetaBookEntry  = BookList[ParamBetaEntry.mBookIndex];

    AlphaURL = WWHFrame.WWHHelp.mBaseURL + AlphaBookEntry.mDirectory + AlphaBookEntry.mFiles.fFileIndexToHREF(ParamAlphaEntry.mFileIndex);
    BetaURL  = WWHFrame.WWHHelp.mBaseURL + BetaBookEntry.mDirectory + BetaBookEntry.mFiles.fFileIndexToHREF(ParamBetaEntry.mFileIndex);

    if (AlphaURL < BetaURL)
    {
      Result = -1;
    }
    else if (AlphaURL > BetaURL)
    {
      Result = 1;
    }
  }

  return Result;
}
