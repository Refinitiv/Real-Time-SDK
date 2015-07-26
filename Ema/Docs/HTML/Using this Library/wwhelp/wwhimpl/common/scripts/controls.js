// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global escape */
/*global WWHFrame */
/*global WWHControlEntry_GetHTML */
/*global WWHControlEntry_GetIconURL */
/*global WWHControlEntry_GetLabel */
/*global WWHControlEntry_SetStatus */
/*global WWHControlEntry_UpdateIcon */
/*global WWHControls_AddControl */
/*global WWHControls_CanSyncTOC */
/*global WWHControls_ClickedEmail */
/*global WWHControls_ClickedNext */
/*global WWHControls_ClickedPDF */
/*global WWHControls_ClickedPrevious */
/*global WWHControls_ClickedPrint */
/*global WWHControls_ClickedRelatedTopics */
/*global WWHControls_ClickedShowNavigation */
/*global WWHControls_ClickedSyncTOC */
/*global WWHControls_ControlsLoaded */
/*global WWHControls_Email */
/*global WWHControls_GetControl */
/*global WWHControls_HasPDFLink */
/*global WWHControls_Initialize */
/*global WWHControls_LeftFrameTitle */
/*global WWHControls_LeftHTML */
/*global WWHControls_Next */
/*global WWHControls_PDF */
/*global WWHControls_Previous */
/*global WWHControls_Print */
/*global WWHControls_ProcessAccessKey */
/*global WWHControls_RecordFocus */
/*global WWHControls_RelatedTopics */
/*global WWHControls_ReloadControls */
/*global WWHControls_RestoreFocus */
/*global WWHControls_RightFrameTitle */
/*global WWHControls_RightHTML */
/*global WWHControls_SansNavigation */
/*global WWHControls_ShowNavigation */
/*global WWHControls_SwitchToNavigation */
/*global WWHControls_SyncTOC */
/*global WWHControls_TopSpacerHTML */
/*global WWHControls_UpdateHREF */
/*global WWHStringUtilities_EscapeHTML */
/*global WWHStringUtilities_FormatMessage */
/*global WWHStringUtilities_GetURLFilePathOnly */

function  WWHControlEntry_Object(ParamControlName,
                                 bParamEnabled,
                                 bParamStatus,
                                 ParamLabel,
                                 ParamIconEnabled,
                                 ParamIconDisabled,
                                 ParamAnchorMethod,
                                 ParamFrameName)
{
  this.mControlName  = ParamControlName;
  this.mbEnabled     = bParamEnabled;
  this.mbStatus      = bParamStatus;
  this.mLabel        = ParamLabel;
  this.mIconEnabled  = ParamIconEnabled;
  this.mIconDisabled = ParamIconDisabled;
  this.mAnchorMethod = ParamAnchorMethod;
  this.mFrameName    = ParamFrameName;

  this.fSetStatus  = WWHControlEntry_SetStatus;
  this.fGetIconURL = WWHControlEntry_GetIconURL;
  this.fGetHTML    = WWHControlEntry_GetHTML;
  this.fGetLabel   = WWHControlEntry_GetLabel;
  this.fUpdateIcon = WWHControlEntry_UpdateIcon;
}

function  WWHControlEntry_SetStatus(bParamStatus)
{
  if (this.mbEnabled)
  {
    this.mbStatus = bParamStatus;
  }
  else
  {
    this.mbStatus = false;
  }
}

function  WWHControlEntry_GetIconURL()
{
  var  VarIconURL = "";


  if (this.mbEnabled)
  {
    // Create absolute path to icon
    //
    VarIconURL += WWHFrame.WWHHelp.mHelpURLPrefix;
    VarIconURL += "wwhelp/wwhimpl/common/images/";

    // Determine which icon to return
    //
    if (this.mbStatus)
    {
      VarIconURL += this.mIconEnabled;
    }
    else
    {
      VarIconURL += this.mIconDisabled;
    }
  }

  return VarIconURL;
}

function  WWHControlEntry_GetHTML()
{
  var  VarHTML = "";
  var  VarStyleAttribute;
  var  VarLabel;


  // Set style attribute to insure small image height
  //
  if (WWHFrame.WWHBrowser.mBrowser === 1)  // Shorthand for Netscape
  {
    VarStyleAttribute = "";
  }
  else
  {
    VarStyleAttribute = " style=\"font-size: 1px; line-height: 1px;\"";
  }

  if (this.mbEnabled)
  {
    // Set label
    //
    VarLabel = this.mLabel;
    if (WWHFrame.WWHHelp.mbAccessible)
    {
      if ( ! this.mbStatus)
      {
        VarLabel = WWHStringUtilities_FormatMessage(WWHFrame.WWHHelp.mMessages.mAccessibilityDisabledNavigationButton, this.mLabel);
        VarLabel = WWHStringUtilities_EscapeHTML(VarLabel);
      }
    }
    VarLabel = WWHStringUtilities_EscapeHTML(VarLabel);

    // Display control
    //
    VarHTML += "  <td width=\"23\">";
    VarHTML += "<div" + VarStyleAttribute + ">";
    VarHTML += "<a name=\"" + this.mControlName + "\" href=\"javascript:WWHFrame.WWHControls." + this.mAnchorMethod + "();\" title=\"" + VarLabel + "\">";
    VarHTML += "<img name=\"" + this.mControlName + "\" alt=\"" + VarLabel + "\" border=\"0\" src=\"" + this.fGetIconURL() + "\" width=\"23\" height=\"21\">";
    VarHTML += "</a>";
    VarHTML +=" </div>";
    VarHTML += "</td>\n";
  }

  return VarHTML;
}

function  WWHControlEntry_GetLabel()
{
  var  VarLabel = "";


  if (this.mbEnabled)
  {
    // Set label
    //
    VarLabel = this.mLabel;
  }

  return VarLabel;
}

function  WWHControlEntry_UpdateIcon()
{
  var  VarControlDocument;


  if (this.mbEnabled)
  {
    // Access control document
    //
    VarControlDocument = eval(WWHFrame.WWHHelp.fGetFrameReference(this.mFrameName) + ".document");

    // Update icon
    //
    VarControlDocument.images[this.mControlName].src = this.fGetIconURL();
  }
}

function  WWHControlEntries_Object()
{
}

function  WWHControls_Object()
{
  this.mControls      = new WWHControlEntries_Object();
  this.mSyncPrevNext  = [ null, null, null ];
  this.mFocusedFrame  = "";
  this.mFocusedAnchor = "";

  this.fReloadControls        = WWHControls_ReloadControls;
  this.fControlsLoaded        = WWHControls_ControlsLoaded;
  this.fAddControl            = WWHControls_AddControl;
  this.fGetControl            = WWHControls_GetControl;
  this.fInitialize            = WWHControls_Initialize;
  this.fSansNavigation        = WWHControls_SansNavigation;
  this.fCanSyncTOC            = WWHControls_CanSyncTOC;
  this.fTopSpacerHTML         = WWHControls_TopSpacerHTML;
  this.fLeftHTML              = WWHControls_LeftHTML;
  this.fRightHTML             = WWHControls_RightHTML;
  this.fLeftFrameTitle        = WWHControls_LeftFrameTitle;
  this.fRightFrameTitle       = WWHControls_RightFrameTitle;
  this.fUpdateHREF            = WWHControls_UpdateHREF;
  this.fRecordFocus           = WWHControls_RecordFocus;
  this.fRestoreFocus          = WWHControls_RestoreFocus;
  this.fSwitchToNavigation    = WWHControls_SwitchToNavigation;
  this.fHasPDFLink            = WWHControls_HasPDFLink;
  this.fClickedShowNavigation = WWHControls_ClickedShowNavigation;
  this.fClickedSyncTOC        = WWHControls_ClickedSyncTOC;
  this.fClickedPrevious       = WWHControls_ClickedPrevious;
  this.fClickedNext           = WWHControls_ClickedNext;
  this.fClickedPDF            = WWHControls_ClickedPDF;
  this.fClickedRelatedTopics  = WWHControls_ClickedRelatedTopics;
  this.fClickedEmail          = WWHControls_ClickedEmail;
  this.fClickedPrint          = WWHControls_ClickedPrint;
  this.fShowNavigation        = WWHControls_ShowNavigation;
  this.fSyncTOC               = WWHControls_SyncTOC;
  this.fPrevious              = WWHControls_Previous;
  this.fNext                  = WWHControls_Next;
  this.fPDF                   = WWHControls_PDF;
  this.fRelatedTopics         = WWHControls_RelatedTopics;
  this.fEmail                 = WWHControls_Email;
  this.fPrint                 = WWHControls_Print;
  this.fProcessAccessKey      = WWHControls_ProcessAccessKey;
}

function  WWHControls_ReloadControls()
{
  // Load the left frame it it will cascade and load the other frames
  //
  WWHFrame.WWHHelp.fReplaceLocation("WWHControlsLeftFrame", WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/html/controll.htm");
}

function  WWHControls_ControlsLoaded(ParamDescription)
{
  if (ParamDescription === "left")
  {
    // Set frame name for accessibility
    //
    if (WWHFrame.WWHHelp.mbAccessible)
    {
      WWHFrame.WWHHelp.fSetFrameName("WWHControlsLeftFrame");
    }

    WWHFrame.WWHHelp.fReplaceLocation("WWHControlsRightFrame", WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/html/controlr.htm");
  }
  else if (ParamDescription === "right")
  {
    // Set frame name for accessibility
    //
    if (WWHFrame.WWHHelp.mbAccessible)
    {
      WWHFrame.WWHHelp.fSetFrameName("WWHControlsRightFrame");
    }

    WWHFrame.WWHHelp.fReplaceLocation("WWHTitleFrame", WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/html/title.htm");
  }
  else  // (ParamDescription === "title")
  {
    // Set frame name for accessibility
    //
    if (WWHFrame.WWHHelp.mbAccessible)
    {
      WWHFrame.WWHHelp.fSetFrameName("WWHTitleFrame");
    }

    if ( ! WWHFrame.WWHHelp.mbInitialized)
    {
      // All control frames are now loaded
      //
      WWHFrame.WWHHelp.fInitStage(5);
    }
    else
    {
      // Restore previous focus
      //
      this.fRestoreFocus();
    }
  }
}

function  WWHControls_AddControl(ParamControlName,
                                 bParamEnabled,
                                 bParamStatus,
                                 ParamLabel,
                                 ParamIconEnabled,
                                 ParamIconDisabled,
                                 ParamAnchorMethod,
                                 ParamFrameName)
{
  var  VarControlEntry;


  VarControlEntry = new WWHControlEntry_Object(ParamControlName,
                                               bParamEnabled,
                                               bParamStatus,
                                               ParamLabel,
                                               ParamIconEnabled,
                                               ParamIconDisabled,
                                               ParamAnchorMethod,
                                               ParamFrameName);

  this.mControls[ParamControlName + "~"] = VarControlEntry;
}

function  WWHControls_GetControl(ParamControlName)
{
  var  VarControlEntry;


  VarControlEntry = this.mControls[ParamControlName + "~"];
  if (VarControlEntry === undefined)
  {
    VarControlEntry = null;
  }

  return VarControlEntry;
}

function  WWHControls_Initialize()
{
  var  VarSettings;
  var  VarDocumentFrame;


  // Access settings
  //
  VarSettings = WWHFrame.WWHHelp.mSettings;

  // Confirm Sync TOC can be enabled
  //
  if (this.fSansNavigation())
  {
    VarSettings.mbSyncContentsEnabled = false;
  }

  // Confirm E-mail can be enabled
  //
  if (VarSettings.mbEmailEnabled)
  {
    VarSettings.mbEmailEnabled = ((typeof(VarSettings.mEmailAddress) === "string") &&
                                  (VarSettings.mEmailAddress.length > 0));
  }

  // Confirm Print can be enabled
  //
  if (VarSettings.mbPrintEnabled)
  {
    VarDocumentFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHTitleFrame"));
    VarSettings.mbPrintEnabled = ((VarDocumentFrame.focus !== undefined) &&
                                  (VarDocumentFrame.print !== undefined));
  }

  // Create control entries
  //
  this.fAddControl("WWHFrameSetIcon", this.fSansNavigation(), this.fSansNavigation(),
                   WWHFrame.WWHHelp.mMessages.mShowNavigationIconLabel,
                   "shownav.gif", "shownav.gif", "fClickedShowNavigation", "WWHControlsLeftFrame");
  this.fAddControl("WWHSyncTOCIcon", VarSettings.mbSyncContentsEnabled, false,
                   WWHFrame.WWHHelp.mMessages.mSyncIconLabel,
                   "sync.gif", "syncx.gif", "fClickedSyncTOC", "WWHControlsLeftFrame");
  this.fAddControl("WWHPrevIcon", VarSettings.mbPrevEnabled, false,
                   WWHFrame.WWHHelp.mMessages.mPrevIconLabel,
                   "prev.gif", "prevx.gif", "fClickedPrevious", "WWHControlsLeftFrame");
  this.fAddControl("WWHNextIcon", VarSettings.mbNextEnabled, false,
                   WWHFrame.WWHHelp.mMessages.mNextIconLabel,
                   "next.gif", "nextx.gif", "fClickedNext", "WWHControlsLeftFrame");
  this.fAddControl("WWHPDFIcon", VarSettings.mbPDFEnabled, false,
                   WWHFrame.WWHHelp.mMessages.mPDFIconLabel,
                   "pdf.gif", "pdfx.gif", "fClickedPDF", "WWHControlsRightFrame");
  this.fAddControl("WWHRelatedTopicsIcon", VarSettings.mbRelatedTopicsEnabled, false,
                   WWHFrame.WWHHelp.mMessages.mRelatedTopicsIconLabel,
                   "related.gif", "relatedx.gif", "fClickedRelatedTopics", "WWHControlsRightFrame");
  this.fAddControl("WWHEmailIcon", VarSettings.mbEmailEnabled, false,
                   WWHFrame.WWHHelp.mMessages.mEmailIconLabel,
                   "email.gif", "emailx.gif", "fClickedEmail", "WWHControlsRightFrame");
  this.fAddControl("WWHPrintIcon", VarSettings.mbPrintEnabled, false,
                   WWHFrame.WWHHelp.mMessages.mPrintIconLabel,
                   "print.gif", "printx.gif", "fClickedPrint", "WWHControlsRightFrame");

  // Load control frames
  //
  this.fReloadControls();
}

function  WWHControls_SansNavigation()
{
  var  bSansNavigation = false;


  if (WWHFrame.WWHHelp.fSingleTopic())
  {
    bSansNavigation = true;
  }

  return bSansNavigation;
}

function  WWHControls_CanSyncTOC()
{
  var  bVarCanSyncTOC = false;


  if (this.mSyncPrevNext[0] !== null)
  {
    bVarCanSyncTOC = true;
  }

  return bVarCanSyncTOC;
}

function  WWHControls_TopSpacerHTML()
{
  var  VarHTML = "";
  var  VarStyleAttribute;

  // Set style attribute to insure small image height
  //
  if (WWHFrame.WWHBrowser.mBrowser === 1)  // Shorthand for Netscape
  {
    VarStyleAttribute = "";
  }
  else
  {
    VarStyleAttribute = " style=\"font-size: 1px; line-height: 1px;\"";
  }

  VarHTML += "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" role=\"presentation\">\n";
  VarHTML += " <tr>\n";
  VarHTML += "  <td><div" + VarStyleAttribute + "><img src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/images/spc_tb_t.gif" + "\" alt=\"\"></div></td>\n";
  VarHTML += " </tr>\n";
  VarHTML += "</table>\n";

  return VarHTML;
}

function  WWHControls_LeftHTML()
{
  var  VarHTML = "";
  var  VarEnabledControls;
  var  VarControl;
  var  VarMaxIndex;
  var  VarIndex;

  // Confirm user did not reload the frameset
  //
  if (this.fGetControl("WWHFrameSetIcon") !== null)
  {
    // Determine active controls
    //
    VarEnabledControls = [];
    VarControl = this.fGetControl("WWHFrameSetIcon");
    if (VarControl.mbEnabled)
    {
      VarEnabledControls[VarEnabledControls.length] = VarControl;
    }
    VarControl = this.fGetControl("WWHSyncTOCIcon");
    if (VarControl.mbEnabled)
    {
      VarEnabledControls[VarEnabledControls.length] = VarControl;
    }
    VarControl = this.fGetControl("WWHPrevIcon");
    if (VarControl.mbEnabled)
    {
      VarEnabledControls[VarEnabledControls.length] = VarControl;
    }
    VarControl = this.fGetControl("WWHNextIcon");
    if (VarControl.mbEnabled)
    {
      VarEnabledControls[VarEnabledControls.length] = VarControl;
    }

    // Emit HTML for controls
    //
    VarHTML += this.fTopSpacerHTML();
    if (VarEnabledControls.length > 0)
    {
      VarHTML += "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" role=\"presentation\">\n";
      VarHTML += " <tr>\n";

      VarHTML += "  <td><div><img src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/images/spc_tb_l.gif" + "\" alt=\"\"></div></td>\n";
      for (VarMaxIndex = VarEnabledControls.length, VarIndex = 0 ; VarIndex < VarMaxIndex ; VarIndex += 1)
      {
        VarHTML += VarEnabledControls[VarIndex].fGetHTML();
        if ((VarIndex + 1) < VarMaxIndex)
        {
          VarHTML += "  <td><div><img src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/images/spc_tb_m.gif" + "\" alt=\"\"></div></td>\n";
        }
      }

      VarHTML += " </tr>\n";
      VarHTML += "</table>\n";
    }
  }

  return VarHTML;
}

function  WWHControls_RightHTML()
{
  var  VarHTML = "";
  var  VarEnabledControls;
  var  VarControl;
  var  VarMaxIndex;
  var  VarIndex;

  // Confirm user did not reload the frameset
  //
  if (this.fGetControl("WWHRelatedTopicsIcon") !== null)
  {
    // Determine active controls
    //
    VarEnabledControls = [];
    VarControl = this.fGetControl("WWHPDFIcon");
    if (VarControl.mbEnabled)
    {
      VarEnabledControls[VarEnabledControls.length] = VarControl;
    }
    VarControl = this.fGetControl("WWHRelatedTopicsIcon");
    if (VarControl.mbEnabled)
    {
      VarEnabledControls[VarEnabledControls.length] = VarControl;
    }
    VarControl = this.fGetControl("WWHEmailIcon");
    if (VarControl.mbEnabled)
    {
      VarEnabledControls[VarEnabledControls.length] = VarControl;
    }
    VarControl = this.fGetControl("WWHPrintIcon");
    if (VarControl.mbEnabled)
    {
      VarEnabledControls[VarEnabledControls.length] = VarControl;
    }

    // Emit HTML for controls
    //
    VarHTML += this.fTopSpacerHTML();
    if (VarEnabledControls.length > 0)
    {
      VarHTML += "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" role=\"presentation\">\n";
      VarHTML += " <tr>\n";

      for (VarMaxIndex = VarEnabledControls.length, VarIndex = 0 ; VarIndex < VarMaxIndex ; VarIndex += 1)
      {
        VarHTML += VarEnabledControls[VarIndex].fGetHTML();
        if ((VarIndex + 1) < VarMaxIndex)
        {
          VarHTML += "  <td><div><img src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/images/spc_tb_m.gif" + "\" alt=\"\"></div></td>\n";
        }
      }
      VarHTML += "  <td><div><img src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/images/spc_tb_r.gif" + "\" alt=\"\"></div></td>\n";

      VarHTML += " </tr>\n";
      VarHTML += "</table>\n";
    }
  }

  return VarHTML;
}

function  WWHControls_LeftFrameTitle()
{
  var  VarTitle = "";


  if (this.fGetControl("WWHFrameSetIcon").fGetLabel().length > 0)
  {
    if (VarTitle.length > 0)
    {
      VarTitle += WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " ";
    }
    VarTitle += this.fGetControl("WWHFrameSetIcon").fGetLabel();
  }

  if (this.fGetControl("WWHSyncTOCIcon").fGetLabel().length > 0)
  {
    if (VarTitle.length > 0)
    {
      VarTitle += WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " ";
    }
    VarTitle += this.fGetControl("WWHSyncTOCIcon").fGetLabel();
  }

  if (this.fGetControl("WWHPrevIcon").fGetLabel().length > 0)
  {
    if (VarTitle.length > 0)
    {
      VarTitle += WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " ";
    }
    VarTitle += this.fGetControl("WWHPrevIcon").fGetLabel();
  }

  if (this.fGetControl("WWHNextIcon").fGetLabel().length > 0)
  {
    if (VarTitle.length > 0)
    {
      VarTitle += WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " ";
    }
    VarTitle += this.fGetControl("WWHNextIcon").fGetLabel();
  }

  return VarTitle;
}

function  WWHControls_RightFrameTitle()
{
  var  VarTitle = "";


  if (this.fGetControl("WWHPDFIcon").fGetLabel().length > 0)
  {
    if (VarTitle.length > 0)
    {
      VarTitle += WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " ";
    }
    VarTitle += this.fGetControl("WWHPDFIcon").fGetLabel();
  }

  if (this.fGetControl("WWHRelatedTopicsIcon").fGetLabel().length > 0)
  {
    if (VarTitle.length > 0)
    {
      VarTitle += WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " ";
    }
    VarTitle += this.fGetControl("WWHRelatedTopicsIcon").fGetLabel();
  }

  if (this.fGetControl("WWHEmailIcon").fGetLabel().length > 0)
  {
    if (VarTitle.length > 0)
    {
      VarTitle += WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " ";
    }
    VarTitle += this.fGetControl("WWHEmailIcon").fGetLabel();
  }

  if (this.fGetControl("WWHPrintIcon").fGetLabel().length > 0)
  {
    if (VarTitle.length > 0)
    {
      VarTitle += WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " ";
    }
    VarTitle += this.fGetControl("WWHPrintIcon").fGetLabel();
  }

  return VarTitle;
}

function  WWHControls_UpdateHREF(ParamHREF)
{
  // Update sync/prev/next array
  //
  this.mSyncPrevNext = WWHFrame.WWHHelp.fGetSyncPrevNext(ParamHREF);

  // Update status
  //
  this.fGetControl("WWHFrameSetIcon").fSetStatus(this.fSansNavigation());
  this.fGetControl("WWHSyncTOCIcon").fSetStatus(this.fCanSyncTOC());
  this.fGetControl("WWHPrevIcon").fSetStatus(this.mSyncPrevNext[1] !== null);
  this.fGetControl("WWHNextIcon").fSetStatus(this.mSyncPrevNext[2] !== null);
  this.fGetControl("WWHPDFIcon").fSetStatus(this.fHasPDFLink());
  this.fGetControl("WWHRelatedTopicsIcon").fSetStatus(WWHFrame.WWHRelatedTopics.fHasRelatedTopics());
  this.fGetControl("WWHEmailIcon").fSetStatus(this.fCanSyncTOC());
  this.fGetControl("WWHPrintIcon").fSetStatus(this.fCanSyncTOC());

  // Update controls
  //
  if (WWHFrame.WWHHelp.mbAccessible)
  {
    // Reload control frames
    //
    this.fReloadControls();
  }
  else
  {
    // Update icons in place
    //
    this.fGetControl("WWHFrameSetIcon").fUpdateIcon();
    this.fGetControl("WWHSyncTOCIcon").fUpdateIcon();
    this.fGetControl("WWHPrevIcon").fUpdateIcon();
    this.fGetControl("WWHNextIcon").fUpdateIcon();
    this.fGetControl("WWHPDFIcon").fUpdateIcon();
    this.fGetControl("WWHRelatedTopicsIcon").fUpdateIcon();
    this.fGetControl("WWHEmailIcon").fUpdateIcon();
    this.fGetControl("WWHPrintIcon").fUpdateIcon();

    // Restore previous focus
    //
    this.fRestoreFocus();
  }
}

function  WWHControls_RecordFocus(ParamFrameName,
                                  ParamAnchorName)
{
  this.mFocusedFrame  = ParamFrameName;
  this.mFocusedAnchor = ParamAnchorName;
}

function  WWHControls_RestoreFocus()
{
  if ((this.mFocusedFrame.length > 0) &&
      (this.mFocusedAnchor.length > 0))
  {
    WWHFrame.WWHHelp.fFocus(this.mFocusedFrame, this.mFocusedAnchor);
  }

  this.mFocusedFrame  = "";
  this.mFocusedAnchor = "";
}

function  WWHControls_SwitchToNavigation(ParamTabName)
{
  var  VarDocumentFrame;
  var  VarDocumentURL;
  var  VarSwitchURL;


  // Switch to navigation
  //
  VarDocumentFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHDocumentFrame"));
  VarDocumentURL = WWHFrame.WWHBrowser.fNormalizeURL(VarDocumentFrame.location.href);
  VarDocumentURL = WWHFrame.WWHHelp.fGetBookFileHREF(VarDocumentURL);
  VarSwitchURL = WWHFrame.WWHHelp.mHelpURLPrefix + "/wwhelp/wwhimpl/common/html/switch.htm?href=" + VarDocumentURL;
  if (WWHFrame.WWHHelp.mbAccessible)
  {
    VarSwitchURL += "&accessible=true";
  }
  if ((ParamTabName !== undefined) &&
      (ParamTabName !== null))
  {
    VarSwitchURL += "&tab=" + ParamTabName;
  }
  WWHFrame.WWHSwitch.fExec(false, VarSwitchURL);
}

function  WWHControls_HasPDFLink()
{
  var  VarHasPDFLink = false;
  var  VarDocumentFrame;

  VarDocumentFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHDocumentFrame"));
  VarHasPDFLink = ((typeof VarDocumentFrame.WWHPDFLink) === "function");

  return VarHasPDFLink;
}

function  WWHControls_ClickedShowNavigation()
{
  this.fShowNavigation();
}

function  WWHControls_ClickedSyncTOC()
{
  this.fSyncTOC(true);
}

function  WWHControls_ClickedPrevious()
{
  // Record focused icon
  //
  this.fRecordFocus("WWHControlsLeftFrame", "WWHPrevIcon");

  this.fPrevious();
}

function  WWHControls_ClickedNext()
{
  // Record focused icon
  //
  this.fRecordFocus("WWHControlsLeftFrame", "WWHNextIcon");

  this.fNext();
}

function  WWHControls_ClickedPDF()
{
  this.fPDF();
}

function  WWHControls_ClickedRelatedTopics()
{
  this.fRelatedTopics();
}

function  WWHControls_ClickedEmail()
{
  this.fEmail();
}

function  WWHControls_ClickedPrint()
{
  this.fPrint();
}

function  WWHControls_ShowNavigation()
{
  if (WWHFrame.WWHHandler.fIsReady())
  {
    this.fSwitchToNavigation();
  }
}

function  WWHControls_SyncTOC(bParamReportError)
{
  if (this.fCanSyncTOC())
  {
    if (WWHFrame.WWHHandler.fIsReady())
    {
      WWHFrame.WWHHelp.fSyncTOC(this.mSyncPrevNext[0], bParamReportError);
    }
  }
}

function  WWHControls_Previous()
{
  if (this.mSyncPrevNext[1] !== null)
  {
    WWHFrame.WWHHelp.fSetDocumentHREF(this.mSyncPrevNext[1], false);
  }
}

function  WWHControls_Next()
{
  if (this.mSyncPrevNext[2] !== null)
  {
    WWHFrame.WWHHelp.fSetDocumentHREF(this.mSyncPrevNext[2], false);
  }
}

function  WWHControls_PDF()
{
  var  VarDocumentFrame;
  var  VarIndex;
  var  VarDocumentParentURL;
  var  VarPDFLink;
  var  VarPDFURL;

  VarDocumentFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHDocumentFrame"));
  if ((typeof VarDocumentFrame.WWHPDFLink) === "function")
  {
    VarIndex = VarDocumentFrame.location.href.lastIndexOf("/");
    VarDocumentParentURL = VarDocumentFrame.location.href.substring(0, VarIndex);
    VarPDFLink = VarDocumentFrame.WWHPDFLink();
    VarPDFURL = VarDocumentParentURL + "/" + VarPDFLink;

    WWHFrame.WWHHelp.fSetLocation("WWHDocumentFrame", VarPDFURL);
  }
}

function  WWHControls_RelatedTopics()
{
  var  VarDocumentFrame;
  var  VarDocumentURL;


  if (WWHFrame.WWHRelatedTopics.fHasRelatedTopics())
  {
    if (WWHFrame.WWHBrowser.mbSupportsPopups)
    {
      WWHFrame.WWHRelatedTopics.fShow();
    }
    else
    {
      VarDocumentFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHDocumentFrame"));

      VarDocumentURL = WWHFrame.WWHBrowser.fNormalizeURL(VarDocumentFrame.location.href);
      VarDocumentURL = WWHStringUtilities_GetURLFilePathOnly(VarDocumentURL);

      WWHFrame.WWHHelp.fSetLocation("WWHDocumentFrame", VarDocumentURL + "#WWHRelatedTopics");
    }
  }
}

function  WWHControls_Email()
{
  var  VarLocation;
  var  VarMessage;
  var  VarMailTo;


  if (this.fCanSyncTOC())
  {
    VarLocation = escape(this.mSyncPrevNext[0]);
    VarMessage = "Feedback: " + VarLocation;
    VarMailTo = "mailto:" + WWHFrame.WWHHelp.mSettings.mEmailAddress + "?subject=" + VarMessage + "&body=" + VarMessage;

    WWHFrame.WWHHelp.fSetLocation("WWHDocumentFrame", VarMailTo);
  }
}

function  WWHControls_Print()
{
  var  VarDocumentFrame;


  if (this.fCanSyncTOC())
  {
    VarDocumentFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHDocumentFrame"));

    VarDocumentFrame.focus();
    VarDocumentFrame.print();
  }
}

function  WWHControls_ProcessAccessKey(ParamAccessKey)
{
  switch (ParamAccessKey)
  {
    case 4:
      this.fClickedPrevious();
      break;

    case 5:
      this.fClickedNext();
      break;

    case 6:
      this.fClickedRelatedTopics();
      break;

    case 7:
      this.fClickedEmail();
      break;

    case 8:
      this.fClickedPrint();
      break;
  }
}
