// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global WWHFrame */
/*global WWHALinksBookLinks_Object */
/*global WWHALinksEntry_AddLinks */
/*global WWHALinksEntry_Object */
/*global WWHALinksPopup_Format */
/*global WWHALinksPopup_Translate */
/*global WWHALinks_Add */
/*global WWHALinks_GotoALink */
/*global WWHALinks_HTML */
/*global WWHALinks_Hide */
/*global WWHALinks_InlineHTML */
/*global WWHALinks_PopupHTML */
/*global WWHALinks_SetALinks */
/*global WWHALinks_Show */
/*global WWHPopup_Object */
/*global WWHRelatedTopicEntry_Object */
/*global WWHRelatedTopicsPopup_Format */
/*global WWHRelatedTopicsPopup_Translate */
/*global WWHRelatedTopics_Add */
/*global WWHRelatedTopics_Clear */
/*global WWHRelatedTopics_DisplayTopic */
/*global WWHRelatedTopics_HTML */
/*global WWHRelatedTopics_HasRelatedTopics */
/*global WWHRelatedTopics_Hide */
/*global WWHRelatedTopics_InlineHTML */
/*global WWHRelatedTopics_PopupHTML */
/*global WWHRelatedTopics_Show */
/*global WWHRelatedTopics_ShowAtEvent */
/*global WWHStringBuffer_Object */
/*global WWHStringUtilities_ExtractStyleAttribute */

function  WWHRelatedTopics_Object()
{
  this.mRelatedTopicList = [];
  this.mPopup            = new WWHPopup_Object("WWHFrame.WWHRelatedTopics.mPopup",
                                               WWHFrame.WWHHelp.fGetFrameReference("WWHDocumentFrame"),
                                               WWHRelatedTopicsPopup_Translate,
                                               WWHRelatedTopicsPopup_Format,
                                               "WWHRelatedTopicsDIV", "WWHRelatedTopicsText", 10, 0, 0,
                                               WWHFrame.WWHHelp.mSettings.mRelatedTopics.mWidth);

  this.fHasRelatedTopics = WWHRelatedTopics_HasRelatedTopics;
  this.fClear            = WWHRelatedTopics_Clear;
  this.fAdd              = WWHRelatedTopics_Add;
  this.fHTML             = WWHRelatedTopics_HTML;
  this.fDisplayTopic     = WWHRelatedTopics_DisplayTopic;
  this.fShow             = WWHRelatedTopics_Show;
  this.fShowAtEvent      = WWHRelatedTopics_ShowAtEvent;
  this.fHide             = WWHRelatedTopics_Hide;
  this.fInlineHTML       = WWHRelatedTopics_InlineHTML;
  this.fPopupHTML        = WWHRelatedTopics_PopupHTML;
}

function  WWHRelatedTopics_HasRelatedTopics()
{
  var  bVarHasRelatedTopics = false;


  if (this.mRelatedTopicList.length > 0)
  {
    bVarHasRelatedTopics = true;
  }

  return bVarHasRelatedTopics;
}

function  WWHRelatedTopics_Clear()
{
  this.mRelatedTopicList.length = 0;
}

function  WWHRelatedTopics_Add(ParamText,
                               ParamContext,
                               ParamFileURL)
{
  this.mRelatedTopicList[this.mRelatedTopicList.length] = new WWHRelatedTopicEntry_Object(ParamText, ParamContext, ParamFileURL);
}

function  WWHRelatedTopics_HTML()
{
  var  HTML = new WWHStringBuffer_Object();
  var  Settings = WWHFrame.WWHHelp.mSettings.mRelatedTopics;
  var  FontFamily = "";
  var  FontSize;
  var  MaxIndex;
  var  Index;
  var  ContextBook;


  if ( ! WWHFrame.WWHBrowser.mbSupportsPopups)
  {
    // Determine font family if running Netscape 4.x
    // Required due to errors processing style attributes
    //
    if (WWHFrame.WWHBrowser.mBrowser === 1)  // Shorthand for Netscape 4.x
    {
      FontFamily = WWHStringUtilities_ExtractStyleAttribute("font-family", Settings.mInlineFontStyle);
      FontSize   = WWHStringUtilities_ExtractStyleAttribute("font-size", Settings.mInlineFontStyle);
    }
  }

  for (MaxIndex = this.mRelatedTopicList.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    ContextBook = WWHFrame.WWHHelp.mBooks.fGetContextBook(this.mRelatedTopicList[Index].mContext);
    if (ContextBook !== null)
    {
      if (WWHFrame.WWHBrowser.mbSupportsPopups)
      {
        HTML.fAppend("<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"2\">");
        HTML.fAppend("<tr>");
        HTML.fAppend("<td width=\"17\" valign=\"middle\">");
        HTML.fAppend("<a");
        HTML.fAppend(" href=\"javascript:WWHFrame.WWHRelatedTopics.fDisplayTopic(" + Index + ");\">");
        HTML.fAppend("<img border=\"0\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/images/doc.gif\" width=\"17\" height=\"17\" alt=\"\">");
        HTML.fAppend("</a>");
        HTML.fAppend("</td>");
        HTML.fAppend("<td width=\"100%\" align=\"left\" valign=\"middle\">");
        HTML.fAppend("<a");
        HTML.fAppend(" href=\"javascript:WWHFrame.WWHRelatedTopics.fDisplayTopic(" + Index + ");\"");
        HTML.fAppend(" style=\"text-decoration: none ; color: " + Settings.mForegroundColor + " ; " + Settings.mFontStyle + "\">");
        HTML.fAppend(this.mRelatedTopicList[Index].mText);
        HTML.fAppend("</a>");
        HTML.fAppend("</td>");
        HTML.fAppend("</tr>");
        HTML.fAppend("</table>\n");
      }
      else
      {
        if (HTML.fSize() === 0)
        {
          HTML.fAppend("<ul>\n");
        }

        HTML.fAppend("<li>");
        if (FontFamily.length > 0)
        {
          HTML.fAppend("<font face=\"" + FontFamily + "\" point-size=\"" + FontSize + "\" color=\"" + Settings.mForegroundColor + "\">");
        }
        HTML.fAppend("<a");
        HTML.fAppend(" href=\"javascript:WWHFrame.WWHRelatedTopics.fDisplayTopic(" + Index + ");\"");
        if (FontFamily.length === 0)
        {
          HTML.fAppend(" style=\"text-decoration: none ; color: " + Settings.mForegroundColor + " ; " + Settings.mFontStyle + "\"");
        }
        HTML.fAppend(">");
        HTML.fAppend(this.mRelatedTopicList[Index].mText);
        HTML.fAppend("</a>");
        if (FontFamily.length > 0)
        {
          HTML.fAppend("</font>");
        }
        HTML.fAppend("</li>\n");
      }
    }
  }

  if ( ! WWHFrame.WWHBrowser.mbSupportsPopups)
  {
    if (HTML.fSize() > 0)
    {
      HTML.fAppend("</ul>\n");
    }
  }

  return HTML.fGetBuffer();
}

function  WWHRelatedTopics_DisplayTopic(ParamIndex)
{
  var  ContextBook;
  var  RelatedTopicURL = null;


  ContextBook = WWHFrame.WWHHelp.mBooks.fGetContextBook(this.mRelatedTopicList[ParamIndex].mContext);
  if (ContextBook !== null)
  {
    RelatedTopicURL = WWHFrame.WWHHelp.mBaseURL + ContextBook.mDirectory + this.mRelatedTopicList[ParamIndex].mFileURL;

    // Hide popup to prevent JavaScript errors before displaying target document
    //
    this.fHide();

    WWHFrame.WWHHelp.fSetDocumentHREF(RelatedTopicURL, false);
  }
}

function  WWHRelatedTopicEntry_Object(ParamText,
                                      ParamContext,
                                      ParamFileURL)
{
  this.mText    = ParamText;
  this.mContext = ParamContext;
  this.mFileURL = ParamFileURL;
}

function  WWHRelatedTopics_Show()
{
  var  FakeEvent;
  var  VarDocumentFrame;


  // Create dummy event to pass to popup show command
  //
  FakeEvent = {};

  VarDocumentFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHDocumentFrame"));

  // Assign coordinates to event base on browser type
  // Place event at far right and allow popup code to handle repositioning for display
  //
  if (WWHFrame.WWHBrowser.mBrowser === 1)  // Shorthand for Netscape 4.x
  {
    FakeEvent.layerX = VarDocumentFrame.innerWidth + VarDocumentFrame.pageXOffset;
    FakeEvent.layerY = VarDocumentFrame.pageYOffset;
  }
  else if (WWHFrame.WWHBrowser.mBrowser === 2)  // Shorthand for IE
  {
    if ((VarDocumentFrame.document.documentElement !== undefined) &&
        (VarDocumentFrame.document.documentElement.clientWidth !== undefined) &&
        (VarDocumentFrame.document.documentElement.clientHeight !== undefined) &&
        ((VarDocumentFrame.document.documentElement.clientWidth !== 0) ||
         (VarDocumentFrame.document.documentElement.clientHeight !== 0)))
    {
      FakeEvent.x = VarDocumentFrame.document.documentElement.clientWidth;
      FakeEvent.y = 0;
    }
    else
    {
      FakeEvent.x = VarDocumentFrame.document.body.clientWidth;
      FakeEvent.y = 0;
    }
  }
  else if ((WWHFrame.WWHBrowser.mBrowser === 4) ||  // Shorthand for Netscape 6.x (Mozilla)
           (WWHFrame.WWHBrowser.mBrowser === 5))    // Shorthand for Safari
  {
    FakeEvent.layerX = VarDocumentFrame.innerWidth + VarDocumentFrame.pageXOffset;
    FakeEvent.layerY = VarDocumentFrame.pageYOffset;
  }

  // Show popup
  //
  this.fShowAtEvent(FakeEvent);
}

function  WWHRelatedTopics_ShowAtEvent(ParamEvent)
{
  var  RelatedTopicsHTML;


  // Show popup
  //
  RelatedTopicsHTML = this.fHTML();
  if (RelatedTopicsHTML.length > 0)
  {
    this.mPopup.fShow(RelatedTopicsHTML, ParamEvent);
  }
}

function  WWHRelatedTopics_Hide()
{
  this.mPopup.fHide();
}

function  WWHRelatedTopics_InlineHTML()
{
  var  HTML = "";
  var  Settings;
  var  FontFamily = "";
  var  FontSize;
  var  ForegroundColor;
  var  ImageDir;
  var  AnchorAttributes;


  if (this.fHasRelatedTopics())
  {
    Settings = WWHFrame.WWHHelp.mSettings.mRelatedTopics;

    // Determine font family if running Netscape 4.x
    // Required due to errors processing style attributes
    //
    if (WWHFrame.WWHBrowser.mBrowser === 1)  // Shorthand for Netscape 4.x
    {
      FontFamily = WWHStringUtilities_ExtractStyleAttribute("font-family", Settings.mInlineFontStyle);
      FontSize   = WWHStringUtilities_ExtractStyleAttribute("font-size", Settings.mInlineFontStyle);
    }

    ForegroundColor = Settings.mInlineForegroundColor;
    ImageDir        = WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/images";

    if (WWHFrame.WWHBrowser.mbSupportsPopups)
    {
      if (Settings.mbInlineEnabled)
      {
        AnchorAttributes = "href=\"javascript:WWHDoNothingHREF();\"";
        AnchorAttributes += " onclick=\"WWHShowRelatedTopicsPopup((document.all||document.getElementById||document.layers)?event:null);\"";

        HTML += "<div class=\"WWHInlineRelatedTopics\">";
        HTML += "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\">";
        HTML += "<tr>";
        HTML += "<td valign=\"bottom\">";
        HTML += "<nobr>";
        if (FontFamily.length > 0)
        {
          HTML += "<font face=\"" + FontFamily + "\" point-size=\"" + FontSize + "\" color=\"" + ForegroundColor + "\">";
        }
        HTML += "<a";
        if (FontFamily.length === 0)
        {
          HTML += " style=\"text-decoration: none ; color: " + ForegroundColor + " ; " + Settings.mInlineFontStyle + "\"";
        }
        HTML += " " + AnchorAttributes + ">";
        HTML += WWHFrame.WWHHelp.mMessages.mRelatedTopicsIconLabel;
        HTML += "</a>";
        HTML += "&nbsp;";
        if (FontFamily.length > 0)
        {
          HTML += "</font>";
        }
        HTML += "</nobr>";
        HTML += "</td>";
        HTML += "<td valign=\"bottom\">";
        HTML += "<a " + AnchorAttributes + ">";
        HTML += "<img border=\"0\" src=\"" + ImageDir + "/relatedi.gif\" alt=\"\">";
        HTML += "</a>";
        HTML += "</td>";
        HTML += "</tr>";
        HTML += "</table>";
        HTML += "</div>";
      }
    }
    else
    {
      // Display inline without popups
      //
      if ((WWHFrame.WWHHelp.mSettings.mbRelatedTopicsEnabled) ||
          (Settings.mbInlineEnabled))
      {
        // Emit title
        //
        HTML += "<hr>";

        if (FontFamily.length > 0)
        {
          HTML += "<p>";
          HTML += "<font face=\"" + FontFamily + "\" point-size=\"" + FontSize + "\" color=\"" + ForegroundColor + "\">";
        }
        else
        {
          HTML += "<p style=\"text-decoration: none ; color: " + ForegroundColor + " ; " + Settings.mInlineFontStyle + "\">";
        }
        HTML += WWHFrame.WWHHelp.mMessages.mRelatedTopicsIconLabel + "<a name=\"WWHRelatedTopics\">&nbsp;</a>";
        if (FontFamily.length > 0)
        {
          HTML += "</font>";
        }
        HTML += "</p>";

        // Get formatted HTML
        //
        HTML += this.fHTML();
      }
    }
  }

  return HTML;
}

function  WWHRelatedTopics_PopupHTML()
{
  var  VarHTML = "";


  if (WWHFrame.WWHBrowser.mbSupportsPopups)
  {
    VarHTML = this.mPopup.fDivTagText();
  }

  return VarHTML;
}

function  WWHRelatedTopicsPopup_Translate(ParamText)
{
  return ParamText;
}

function  WWHRelatedTopicsPopup_Format(ParamWidth,
                                       ParamTextID,
                                       ParamText)
{
  var  FormattedText        = "";
  var  Settings             = WWHFrame.WWHHelp.mSettings.mRelatedTopics;
  var  ImageDir             = WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/images";
  var  BackgroundColor      = Settings.mBackgroundColor;
  var  BorderColor          = Settings.mBorderColor;
  var  TitleForegroundColor = Settings.mTitleForegroundColor;
  var  TitleBackgroundColor = Settings.mTitleBackgroundColor;
  var  ReqSpacer1w2h        = "<img src=\"" + ImageDir + "/spc1w2h.gif\" width=1 height=2 alt=\"\">";
  var  ReqSpacer2w1h        = "<img src=\"" + ImageDir + "/spc2w1h.gif\" width=2 height=1 alt=\"\">";
  var  ReqSpacer4w4h        = "<img src=\"" + ImageDir + "/spacer4.gif\" width=4 height=4 alt=\"\">";
  var  Spacer1w2h           = ReqSpacer1w2h;
  var  Spacer2w1h           = ReqSpacer2w1h;
  var  Spacer4w4h           = ReqSpacer4w4h;


  // Netscape 6.x (Mozilla) renders table cells with graphics
  // incorrectly inside of <div> tags that are rewritten on the fly
  //
  if (WWHFrame.WWHBrowser.mBrowser === 4)  // Shorthand for Netscape 6.x (Mozilla)
  {
    Spacer1w2h = "";
    Spacer2w1h = "";
    Spacer4w4h = "";
  }

  FormattedText += "<table width=\"" + ParamWidth + "\" border=0 cellspacing=0 cellpadding=0 bgcolor=\"" + BackgroundColor + "\">";
  FormattedText += " <tr>";
  FormattedText += "  <td height=2 colspan=6 bgcolor=\"" + BorderColor + "\">" + Spacer1w2h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += "  <td height=4 colspan=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += "  <td height=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 colspan=2 bgcolor=\"" + TitleBackgroundColor + "\">" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += "  <td>" + ReqSpacer4w4h + "</td>";
  FormattedText += "  <td bgcolor=\"" + TitleBackgroundColor + "\" width=\"100%\" align=\"left\" valign=\"middle\"><nobr><span style=\"" + Settings.mTitleFontStyle + " ; color: " + TitleForegroundColor + "\">" + ReqSpacer4w4h + WWHFrame.WWHHelp.mMessages.mRelatedTopicsIconLabel + "</span></nobr></td>";
  FormattedText += "  <td bgcolor=\"" + TitleBackgroundColor + "\" width=\"16\" align=\"right\" valign=\"middle\"><nobr><a href=\"javascript:WWHFrame.WWHRelatedTopics.fHide();\"><img src=\"" + ImageDir + "/close.gif\" border=0 width=16 height=15 alt=\"\"></a>" + ReqSpacer4w4h + "</nobr></td>";
  FormattedText += "  <td>" + ReqSpacer4w4h + "</td>";
  FormattedText += "  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += "  <td height=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 colspan=2 bgcolor=\"" + TitleBackgroundColor + "\">" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += "  <td height=4 colspan=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += "  <td>" + ReqSpacer4w4h + "</td>";
  FormattedText += "  <td colspan=2 width=\"100%\" id=\"" + ParamTextID + "\">" + ParamText + "</td>";
  FormattedText += "  <td>" + ReqSpacer4w4h + "</td>";
  FormattedText += "  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += "  <td height=4 colspan=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td height=2 colspan=6 bgcolor=\"" + BorderColor + "\">" + Spacer1w2h + "</td>";
  FormattedText += " </tr>";
  FormattedText += "</table>";

  return FormattedText;
}

function  WWHALinks_Object()
{
  this.mALinksHash = {};
  this.mPopup      = new WWHPopup_Object("WWHFrame.WWHALinks.mPopup",
                                         WWHFrame.WWHHelp.fGetFrameReference("WWHDocumentFrame"),
                                         WWHALinksPopup_Translate,
                                         WWHALinksPopup_Format,
                                         "WWHALinksDIV", "WWHALinksText", 10, 0, 0,
                                         WWHFrame.WWHHelp.mSettings.mALinks.mWidth);
  this.mALinks     = [];
  this.mbSingle    = false;

  this.fAdd        = WWHALinks_Add;
  this.fA          = WWHALinks_Add;
  this.fSetALinks  = WWHALinks_SetALinks;
  this.fHTML       = WWHALinks_HTML;
  this.fGotoALink  = WWHALinks_GotoALink;
  this.fShow       = WWHALinks_Show;
  this.fHide       = WWHALinks_Hide;
  this.fInlineHTML = WWHALinks_InlineHTML;
  this.fPopupHTML  = WWHALinks_PopupHTML;
}

function  WWHALinks_Add(ParamKeyword,
                        ParamALinksArray)
{
  var  VarALinksEntry;


  // Access alink entry
  //
  VarALinksEntry = this.mALinksHash[ParamKeyword + "~"];
  if ((VarALinksEntry === undefined) ||
      (VarALinksEntry === null))
  {
    VarALinksEntry = new WWHALinksEntry_Object();
    this.mALinksHash[ParamKeyword + "~"] = VarALinksEntry;
  }

  // Add links
  //
  VarALinksEntry.fAddLinks(WWHFrame.WWHHelp.mBooks.mInitIndex, ParamALinksArray);
}

function  WWHALinks_SetALinks(ParamKeywordsArray)
{
  var  bVarFirstLink = true;
  var  VarMaxIndex;
  var  VarIndex;
  var  VarKeyword;
  var  VarALinksEntry;
  var  VarMaxBookLinksIndex;
  var  VarBookLinksIndex;
  var  VarBookLinks;
  var  VarCheckHashArray = [];
  var  VarALinkEntry;
  var  VarCheckHash;
  var  VarMaxLinkIndex;
  var  VarLinkIndex;
  var  VarCheckLink;


  // Reset alinks info
  //
  this.mALinks  = [];
  this.mbSingle = false;

  // Get links for each keyword
  //
  for (VarMaxIndex = ParamKeywordsArray.length, VarIndex = 0 ; VarIndex < VarMaxIndex ; VarIndex += 1)
  {
    VarKeyword = ParamKeywordsArray[VarIndex];

    // Get keyword links
    //
    VarALinksEntry = this.mALinksHash[VarKeyword + "~"];
    if ((VarALinksEntry !== undefined) &&
        (VarALinksEntry !== null))
    {
      // Add links
      //
      for (VarMaxBookLinksIndex = VarALinksEntry.mBookLinks.length, VarBookLinksIndex = 0 ; VarBookLinksIndex < VarMaxBookLinksIndex ; VarBookLinksIndex += 1)
      {
        // Access book info
        //
        VarBookLinks = VarALinksEntry.mBookLinks[VarBookLinksIndex];

        // Access book entries
        //
        while (this.mALinks.length <= VarBookLinks.mBookIndex)
        {
          this.mALinks[this.mALinks.length] = [];
          VarCheckHashArray[VarCheckHashArray.length] = {};
        }
        VarALinkEntry = this.mALinks[VarBookLinks.mBookIndex];
        VarCheckHash  = VarCheckHashArray[VarBookLinks.mBookIndex];

        // Add entries
        //
        for (VarMaxLinkIndex = VarBookLinks.mLinks.length, VarLinkIndex = 0 ; VarLinkIndex < VarMaxLinkIndex ; VarLinkIndex += 1)
        {
          // Confirm link will not be added more than once per book
          //
          VarCheckLink = VarCheckHash[VarBookLinks.mLinks[VarLinkIndex] + "~"];
          if ((VarCheckLink === undefined) ||
              (VarCheckLink === null))
          {
            // Add the link
            //
            VarALinkEntry[VarALinkEntry.length] = VarBookLinks.mLinks[VarLinkIndex];
            VarCheckHash[VarBookLinks.mLinks[VarLinkIndex] + "~"] = "1";

            // Single link?
            //
            if (bVarFirstLink)
            {
              this.mbSingle = true;
              bVarFirstLink = false;
            }
            else
            {
              this.mbSingle = false;
            }
          }
        }
      }
    }
  }
}

function  WWHALinks_HTML(bParamReplace)
{
  var  VarHTML = new WWHStringBuffer_Object();
  var  VarImageDirectory = WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/images";
  var  VarSettings = WWHFrame.WWHHelp.mSettings.mALinks;
  var  VarMaxIndex;
  var  VarIndex;
  var  VarLinks;
  var  VarMaxLinkIndex, VarLinkIndex;
  var  VarLink;
  var  VarParts;
  var  VarLinkFileIndex;


  // Format and display as HTML
  //
  VarHTML.fReset();
  for (VarMaxIndex = this.mALinks.length, VarIndex = 0; VarIndex < VarMaxIndex ; VarIndex += 1)
  {
    VarLinks = this.mALinks[VarIndex];
    if (VarLinks.length > 0)
    {
      // Emit book title
      //
      if (VarSettings.mbShowBook)
      {
        // Emit the book title
        //
        VarHTML.fAppend("<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"2\">");
        VarHTML.fAppend("<tr>");
        VarHTML.fAppend("<td width=\"17\" valign=\"middle\">");
        VarHTML.fAppend("<img border=\"0\" src=\"" + VarImageDirectory + "/fo.gif\" width=\"17\" height=\"17\" alt=\"\">");
        VarHTML.fAppend("</td>");
        VarHTML.fAppend("<td width=\"100%\" align=\"left\" valign=\"middle\">");
        VarHTML.fAppend("<span ");
        VarHTML.fAppend(" style=\"text-decoration: none ; color: " + VarSettings.mForegroundColor + " ; " + VarSettings.mFontStyle + "\">");
        VarHTML.fAppend(WWHFrame.WWHHelp.mBooks.fGetBookTitle(VarIndex));
        VarHTML.fAppend("</span>");
        VarHTML.fAppend("</td>");
        VarHTML.fAppend("</tr>");
        VarHTML.fAppend("</table>\n");

        // Open indentation
        //
        VarHTML.fAppend("<div style=\"margin-left: " + VarSettings.mIndent + "pt\">\n");
      }

      for (VarMaxLinkIndex = VarLinks.length, VarLinkIndex = 0 ; VarLinkIndex < VarMaxLinkIndex ; VarLinkIndex += 1)
      {
        // Get link info
        //
        VarLink = VarLinks[VarLinkIndex];
        VarParts = VarLink.split("#");
        VarLinkFileIndex = parseInt(VarParts[0], 10);

        // Emit link entry
        //
        VarHTML.fAppend("<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"2\">");
        VarHTML.fAppend("<tr>");
        VarHTML.fAppend("<td width=\"17\" valign=\"middle\">");
        VarHTML.fAppend("<a");
        VarHTML.fAppend(" href=\"javascript:WWHFrame.WWHALinks.fGotoALink(" + VarIndex + "," + VarLinkIndex + ", " + bParamReplace + ");\"");
        VarHTML.fAppend(">");
        VarHTML.fAppend("<img border=\"0\" src=\"" + VarImageDirectory + "/doc.gif\" width=\"17\" height=\"17\" alt=\"\">");
        VarHTML.fAppend("</a>");
        VarHTML.fAppend("</td>");
        VarHTML.fAppend("<td width=\"100%\" align=\"left\" valign=\"middle\">");
        VarHTML.fAppend("<a");
        VarHTML.fAppend(" href=\"javascript:WWHFrame.WWHALinks.fGotoALink(" + VarIndex + "," + VarLinkIndex + ", " + bParamReplace + ");\"");
        VarHTML.fAppend(" style=\"text-decoration: none ; color: " + VarSettings.mForegroundColor + " ; " + VarSettings.mFontStyle + "\">");
        VarHTML.fAppend(WWHFrame.WWHHelp.mBooks.fBookIndexFileIndexToTitle(VarIndex, VarLinkFileIndex));
        VarHTML.fAppend("</a>");
        VarHTML.fAppend("</td>");
        VarHTML.fAppend("</tr>");
        VarHTML.fAppend("</table>\n");
      }

      if (VarSettings.mbShowBook)
      {
        // Close indendation
        //
        VarHTML.fAppend("</div>\n");
      }
    }
  }

  return VarHTML.fGetBuffer();
}

function  WWHALinks_GotoALink(ParamBookIndex,
                              ParamLinkIndex,
                              bParamReplace)
{
  var  VarLinks;
  var  VarLink;
  var  VarParts;
  var  VarLinkFileIndex;
  var  VarLinkAnchor;
  var  VarDocumentURL;


  // Hide popup to prevent JavaScript errors before displaying target document
  //
  if (WWHFrame.WWHBrowser.mbSupportsPopups)
  {
    this.fHide();
  }

  // Determine document URL
  //
  VarLinks = this.mALinks[ParamBookIndex];
  VarLink = VarLinks[ParamLinkIndex];
  VarParts = VarLink.split("#");
  VarLinkFileIndex = parseInt(VarParts[0], 10);
  VarLinkAnchor = null;
  if (VarParts.length > 1)
  {
    if (VarParts[1].length > 0)
    {
      VarLinkAnchor = VarParts[1];
    }
  }
  VarDocumentURL = WWHFrame.WWHHelp.fGetBookIndexFileIndexURL(ParamBookIndex, VarLinkFileIndex, VarLinkAnchor);

  // Reset alinks list
  //
  this.mALinks  = [];
  this.mbSingle = false;

  // Goto document
  //
  WWHFrame.WWHHelp.fSetDocumentHREF(VarDocumentURL, bParamReplace);
}

function  WWHALinks_Show(ParamKeywordArray,
                         ParamEvent)
{
  var  VarMaxIndex;
  var  VarIndex;
  var  VarLinks;
  var  VarALinksHTML;


  // Set alinks for given keywords
  //
  this.fSetALinks(ParamKeywordArray);
  if (this.mALinks.length > 0)
  {
    if (this.mbSingle)
    {
      // Just go to single target
      //
      for (VarMaxIndex = this.mALinks.length, VarIndex = 0 ; VarIndex < VarMaxIndex ; VarIndex += 1)
      {
        VarLinks = this.mALinks[VarIndex];
        if (VarLinks.length > 0)
        {
          // Display single link
          //
          this.fGotoALink(VarIndex, 0, false);

          // Exit loop
          //
          VarIndex = VarMaxIndex;
        }
      }
    }
    else
    {
      if (WWHFrame.WWHBrowser.mbSupportsPopups)
      {
        // Show popup
        //
        VarALinksHTML = this.fHTML(false);
        if (VarALinksHTML.length > 0)
        {
          this.mPopup.fShow(VarALinksHTML, ParamEvent);
        }
      }
      else
      {
        // Display selection page in browser window
        //
        WWHFrame.WWHHelp.fSetDocumentHREF(WWHFrame.WWHHelp.mBaseURL + "wwhelp/wwhimpl/common/html/alinks.htm", false);
      }
    }
  }
}

function  WWHALinks_Hide()
{
  this.mPopup.fHide();
}

function  WWHALinks_InlineHTML()
{
  var  VarHTML = new WWHStringBuffer_Object();
  var  VarSettings          = WWHFrame.WWHHelp.mSettings.mALinks;
  var  ImageDir             = WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/images";
  var  BackgroundColor      = VarSettings.mBackgroundColor;
  var  BorderColor          = VarSettings.mBorderColor;
  var  TitleForegroundColor = VarSettings.mTitleForegroundColor;
  var  TitleBackgroundColor = VarSettings.mTitleBackgroundColor;
  var  ReqSpacer1w2h        = "<img src=\"" + ImageDir + "/spc1w2h.gif\" width=1 height=2 alt=\"\">";
  var  ReqSpacer2w1h        = "<img src=\"" + ImageDir + "/spc2w1h.gif\" width=2 height=1 alt=\"\">";
  var  ReqSpacer4w4h        = "<img src=\"" + ImageDir + "/spacer4.gif\" width=4 height=4 alt=\"\">";
  var  Spacer1w2h           = ReqSpacer1w2h;
  var  Spacer2w1h           = ReqSpacer2w1h;
  var  Spacer4w4h           = ReqSpacer4w4h;


  VarHTML.fReset();

  // Netscape 6.x (Mozilla) renders table cells with graphics
  // incorrectly inside of <div> tags that are rewritten on the fly
  //
  if (WWHFrame.WWHBrowser.mBrowser === 4)  // Shorthand for Netscape 6.x (Mozilla)
  {
    Spacer1w2h = "";
    Spacer2w1h = "";
    Spacer4w4h = "";
  }

  VarHTML.fAppend("<table border=0 cellspacing=0 cellpadding=0 bgcolor=\"" + BackgroundColor + "\">");
  VarHTML.fAppend(" <tr>");
  VarHTML.fAppend("  <td height=2 colspan=5 bgcolor=\"" + BorderColor + "\">" + Spacer1w2h + "</td>");
  VarHTML.fAppend(" </tr>");

  VarHTML.fAppend(" <tr>");
  VarHTML.fAppend("  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>");
  VarHTML.fAppend("  <td height=4 colspan=3>" + Spacer4w4h + "</td>");
  VarHTML.fAppend("  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>");
  VarHTML.fAppend(" </tr>");

  VarHTML.fAppend(" <tr>");
  VarHTML.fAppend("  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>");
  VarHTML.fAppend("  <td height=4>" + Spacer4w4h + "</td>");
  VarHTML.fAppend("  <td height=4 bgcolor=\"" + TitleBackgroundColor + "\">" + Spacer4w4h + "</td>");
  VarHTML.fAppend("  <td height=4>" + Spacer4w4h + "</td>");
  VarHTML.fAppend("  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>");
  VarHTML.fAppend(" </tr>");

  VarHTML.fAppend(" <tr>");
  VarHTML.fAppend("  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>");
  VarHTML.fAppend("  <td>" + ReqSpacer4w4h + "</td>");
  VarHTML.fAppend("  <td bgcolor=\"" + TitleBackgroundColor + "\" align=\"left\" valign=\"middle\"><nobr><span style=\"" + VarSettings.mTitleFontStyle + " ; color: " + TitleForegroundColor + "\">" + ReqSpacer4w4h + WWHFrame.WWHHelp.mMessages.mSeeAlsoLabel + "</span></nobr></td>");
  VarHTML.fAppend("  <td>" + ReqSpacer4w4h + "</td>");
  VarHTML.fAppend("  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>");
  VarHTML.fAppend(" </tr>");

  VarHTML.fAppend(" <tr>");
  VarHTML.fAppend("  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>");
  VarHTML.fAppend("  <td height=4>" + Spacer4w4h + "</td>");
  VarHTML.fAppend("  <td height=4 bgcolor=\"" + TitleBackgroundColor + "\">" + Spacer4w4h + "</td>");
  VarHTML.fAppend("  <td height=4>" + Spacer4w4h + "</td>");
  VarHTML.fAppend("  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>");
  VarHTML.fAppend(" </tr>");

  VarHTML.fAppend(" <tr>");
  VarHTML.fAppend("  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>");
  VarHTML.fAppend("  <td height=4 colspan=3>" + Spacer4w4h + "</td>");
  VarHTML.fAppend("  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>");
  VarHTML.fAppend(" </tr>");

  VarHTML.fAppend(" <tr>");
  VarHTML.fAppend("  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>");
  VarHTML.fAppend("  <td>" + ReqSpacer4w4h + "</td>");
  VarHTML.fAppend("  <td>" + WWHFrame.WWHALinks.fHTML(true) + "</td>");
  VarHTML.fAppend("  <td>" + ReqSpacer4w4h + "</td>");
  VarHTML.fAppend("  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>");
  VarHTML.fAppend(" </tr>");

  VarHTML.fAppend(" <tr>");
  VarHTML.fAppend("  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>");
  VarHTML.fAppend("  <td height=4 colspan=3>" + Spacer4w4h + "</td>");
  VarHTML.fAppend("  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>");
  VarHTML.fAppend(" </tr>");

  VarHTML.fAppend(" <tr>");
  VarHTML.fAppend("  <td height=2 colspan=5 bgcolor=\"" + BorderColor + "\">" + Spacer1w2h + "</td>");
  VarHTML.fAppend(" </tr>");
  VarHTML.fAppend("</table>");

  return VarHTML.fGetBuffer();
}

function  WWHALinks_PopupHTML()
{
  var  VarHTML = "";


  if (WWHFrame.WWHBrowser.mbSupportsPopups)
  {
    VarHTML = this.mPopup.fDivTagText();
  }

  return VarHTML;
}

function  WWHALinksPopup_Translate(ParamText)
{
  return ParamText;
}

function  WWHALinksPopup_Format(ParamWidth,
                                ParamTextID,
                                ParamText)
{
  var  FormattedText        = "";
  var  VarSettings          = WWHFrame.WWHHelp.mSettings.mALinks;
  var  ImageDir             = WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/images";
  var  BackgroundColor      = VarSettings.mBackgroundColor;
  var  BorderColor          = VarSettings.mBorderColor;
  var  TitleForegroundColor = VarSettings.mTitleForegroundColor;
  var  TitleBackgroundColor = VarSettings.mTitleBackgroundColor;
  var  ReqSpacer1w2h        = "<img src=\"" + ImageDir + "/spc1w2h.gif\" width=1 height=2 alt=\"\">";
  var  ReqSpacer2w1h        = "<img src=\"" + ImageDir + "/spc2w1h.gif\" width=2 height=1 alt=\"\">";
  var  ReqSpacer4w4h        = "<img src=\"" + ImageDir + "/spacer4.gif\" width=4 height=4 alt=\"\">";
  var  Spacer1w2h           = ReqSpacer1w2h;
  var  Spacer2w1h           = ReqSpacer2w1h;
  var  Spacer4w4h           = ReqSpacer4w4h;


  // Netscape 6.x (Mozilla) renders table cells with graphics
  // incorrectly inside of <div> tags that are rewritten on the fly
  //
  if (WWHFrame.WWHBrowser.mBrowser === 4)  // Shorthand for Netscape 6.x (Mozilla)
  {
    Spacer1w2h = "";
    Spacer2w1h = "";
    Spacer4w4h = "";
  }

  FormattedText += "<table width=\"" + ParamWidth + "\" border=0 cellspacing=0 cellpadding=0 bgcolor=\"" + BackgroundColor + "\">";
  FormattedText += " <tr>";
  FormattedText += "  <td height=2 colspan=6 bgcolor=\"" + BorderColor + "\">" + Spacer1w2h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += "  <td height=4 colspan=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += "  <td height=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 colspan=2 bgcolor=\"" + TitleBackgroundColor + "\">" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += "  <td>" + ReqSpacer4w4h + "</td>";
  FormattedText += "  <td bgcolor=\"" + TitleBackgroundColor + "\" width=\"100%\" align=\"left\" valign=\"middle\"><nobr><span style=\"" + VarSettings.mTitleFontStyle + " ; color: " + TitleForegroundColor + "\">" + ReqSpacer4w4h + WWHFrame.WWHHelp.mMessages.mSeeAlsoLabel + "</span></nobr></td>";
  FormattedText += "  <td bgcolor=\"" + TitleBackgroundColor + "\" width=\"16\" align=\"right\" valign=\"middle\"><nobr><a href=\"javascript:WWHFrame.WWHALinks.fHide();\"><img src=\"" + ImageDir + "/close.gif\" border=0 width=16 height=15 alt=\"\"></a>" + ReqSpacer4w4h + "</nobr></td>";
  FormattedText += "  <td>" + ReqSpacer4w4h + "</td>";
  FormattedText += "  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += "  <td height=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 colspan=2 bgcolor=\"" + TitleBackgroundColor + "\">" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += "  <td height=4 colspan=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += "  <td>" + ReqSpacer4w4h + "</td>";
  FormattedText += "  <td colspan=2 width=\"100%\" id=\"" + ParamTextID + "\">" + ParamText + "</td>";
  FormattedText += "  <td>" + ReqSpacer4w4h + "</td>";
  FormattedText += "  <td bgcolor=\"" + BorderColor + "\">" + ReqSpacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += "  <td height=4 colspan=4>" + Spacer4w4h + "</td>";
  FormattedText += "  <td height=4 bgcolor=\"" + BorderColor + "\">" + Spacer2w1h + "</td>";
  FormattedText += " </tr>";

  FormattedText += " <tr>";
  FormattedText += "  <td height=2 colspan=6 bgcolor=\"" + BorderColor + "\">" + Spacer1w2h + "</td>";
  FormattedText += " </tr>";
  FormattedText += "</table>";

  return FormattedText;
}

function  WWHALinksEntry_Object()
{
  this.mBookLinks = [];

  this.fAddLinks = WWHALinksEntry_AddLinks;
}

function  WWHALinksEntry_AddLinks(ParamBookIndex,
                                  ParamLinksArray)
{
  var  VarBookLinks;


  VarBookLinks = new WWHALinksBookLinks_Object(ParamBookIndex, ParamLinksArray);
  this.mBookLinks[this.mBookLinks.length] = VarBookLinks;
}

function  WWHALinksBookLinks_Object(ParamBookIndex,
                                    ParamLinksArray)
{
  this.mBookIndex = ParamBookIndex;
  this.mLinks     = ParamLinksArray;
}
