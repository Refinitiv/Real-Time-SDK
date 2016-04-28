// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global window */
/*global WWHFrame */
/*global WWHPanelHoverText_Format */
/*global WWHPanelHoverText_Translate */
/*global WWHPanels_ChangePanel */
/*global WWHPanels_ClearScrollPosition */
/*global WWHPanels_GetCurrentPanelEntry */
/*global WWHPanels_GetCurrentPanelObject */
/*global WWHPanels_InitBodyHTML */
/*global WWHPanels_InitHeadHTML */
/*global WWHPanels_InitLoaded */
/*global WWHPanels_JumpToAnchor */
/*global WWHPanels_PanelNavigationLoaded */
/*global WWHPanels_PanelViewLoaded */
/*global WWHPanels_PopupHTML */
/*global WWHPanels_ReloadNavigation */
/*global WWHPanels_ReloadPanel */
/*global WWHPanels_ReloadView */
/*global WWHPanels_RestoreScrollPosition */
/*global WWHPanels_SaveScrollPosition */
/*global WWHPopup_Object */
/*global WWHStringUtilities_EscapeURLForJavaScriptAnchor */

function  WWHPanelsEntry_Object(ParamPanelObject)
{
  this.mPanelObject      = ParamPanelObject;
  this.mScrollPosition   = [ 0, 0 ];
}

function  WWHPanels_Object()
{
  this.mCurrentPanel = 0;
  this.mPanelEntries = [];
  this.mPopup        = new WWHPopup_Object("WWHFrame.WWHJavaScript.mPanels.mPopup",
                                           "REPLACE ME",
                                           WWHPanelHoverText_Translate, WWHPanelHoverText_Format,
                                           "WWHPanelPopupDIV", "WWHPanelPopupText", 1000, 12, 20,
                                           WWHFrame.WWHJavaScript.mSettings.mHoverText.mWidth);
  this.mbChangingPanels       = false;
  this.mbLoading              = false;
  this.mbReloadOnlyNavigation = false;

  this.fGetCurrentPanelEntry  = WWHPanels_GetCurrentPanelEntry;
  this.fGetCurrentPanelObject = WWHPanels_GetCurrentPanelObject;
  this.fInitHeadHTML          = WWHPanels_InitHeadHTML;
  this.fInitBodyHTML          = WWHPanels_InitBodyHTML;
  this.fInitLoaded            = WWHPanels_InitLoaded;
  this.fPopupHTML             = WWHPanels_PopupHTML;
  this.fClearScrollPosition   = WWHPanels_ClearScrollPosition;
  this.fSaveScrollPosition    = WWHPanels_SaveScrollPosition;
  this.fRestoreScrollPosition = WWHPanels_RestoreScrollPosition;
  this.fJumpToAnchor          = WWHPanels_JumpToAnchor;
  this.fChangePanel           = WWHPanels_ChangePanel;
  this.fReloadPanel           = WWHPanels_ReloadPanel;
  this.fReloadNavigation      = WWHPanels_ReloadNavigation;
  this.fReloadView            = WWHPanels_ReloadView;
  this.fPanelNavigationLoaded = WWHPanels_PanelNavigationLoaded;
  this.fPanelViewLoaded       = WWHPanels_PanelViewLoaded;

  // Add visible panels
  //
  if (WWHFrame.WWHJavaScript.mSettings.mTOC.mbShow)
  {
    WWHFrame.WWHOutline.mPanelTabIndex = this.mPanelEntries.length;
    this.mPanelEntries[this.mPanelEntries.length] = new WWHPanelsEntry_Object(WWHFrame.WWHOutline);
  }
  if (WWHFrame.WWHJavaScript.mSettings.mIndex.mbShow)
  {
    WWHFrame.WWHIndex.mPanelTabIndex = this.mPanelEntries.length;
    this.mPanelEntries[this.mPanelEntries.length] = new WWHPanelsEntry_Object(WWHFrame.WWHIndex);
  }
  if (WWHFrame.WWHJavaScript.mSettings.mSearch.mbShow)
  {
    WWHFrame.WWHSearch.mPanelTabIndex = this.mPanelEntries.length;
    this.mPanelEntries[this.mPanelEntries.length] = new WWHPanelsEntry_Object(WWHFrame.WWHSearch);
  }
  if ((WWHFrame.WWHHelp.fCookiesEnabled()) &&
      (WWHFrame.WWHJavaScript.mSettings.mFavorites.mbShow))
  {
    WWHFrame.WWHFavorites.mPanelTabIndex = this.mPanelEntries.length;
    this.mPanelEntries[this.mPanelEntries.length] = new WWHPanelsEntry_Object(WWHFrame.WWHFavorites);
  }
}

function  WWHPanels_GetCurrentPanelEntry()
{
  return this.mPanelEntries[this.mCurrentPanel];
}

function  WWHPanels_GetCurrentPanelObject()
{
  return this.mPanelEntries[this.mCurrentPanel].mPanelObject;
}

function  WWHPanels_InitHeadHTML()
{
  var  HTML = "";
  var  PanelEntry;


  if (WWHFrame.WWHHelp.mInitStage > 0)
  {
    // Access panel entry
    //
    PanelEntry = this.fGetCurrentPanelEntry();

    HTML = PanelEntry.mPanelObject.fInitHeadHTML();
  }

  return HTML;
}

function  WWHPanels_InitBodyHTML()
{
  var  HTML = "";
  var  PanelEntry;


  if (WWHFrame.WWHHelp.mInitStage > 0)
  {
    // Access panel entry
    //
    PanelEntry = this.fGetCurrentPanelEntry();

    HTML = PanelEntry.mPanelObject.fInitBodyHTML();
  }

  return HTML;
}

function  WWHPanels_InitLoaded()
{
  this.fClearScrollPosition();
  this.mbLoading = false;
  this.fReloadPanel();
}

function  WWHPanels_PopupHTML()
{
  var  VarHTML = "";


  if (WWHFrame.WWHBrowser.mbSupportsPopups)
  {
    VarHTML = this.mPopup.fDivTagText();
  }

  return VarHTML;
}

function  WWHPanels_ClearScrollPosition()
{
  var  PanelEntry;


  PanelEntry = this.fGetCurrentPanelEntry();

  PanelEntry.mScrollPosition[0] = 0;
  PanelEntry.mScrollPosition[1] = 0;
}

function  WWHPanels_SaveScrollPosition()
{
  var  PanelEntry;
  var  PanelObject;
  var  VarPanelViewFrame;


  if ( ! this.mbLoading)
  {
    // Access panel object
    //
    PanelEntry  = this.fGetCurrentPanelEntry();
    PanelObject = this.fGetCurrentPanelObject();
    if (PanelObject.mbPanelInitialized)
    {
      VarPanelViewFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHPanelViewFrame"));

      if ((WWHFrame.WWHBrowser.mBrowser === 1) ||  // Shorthand for Netscape
          (WWHFrame.WWHBrowser.mBrowser === 3) ||  // Shorthand for iCab
          (WWHFrame.WWHBrowser.mBrowser === 4) ||  // Shorthand for Netscape 6.0 (Mozilla)
          (WWHFrame.WWHBrowser.mBrowser === 5))    // Shorthand for Safari
      {
        PanelEntry.mScrollPosition[0] = VarPanelViewFrame.window.pageXOffset;
        PanelEntry.mScrollPosition[1] = VarPanelViewFrame.window.pageYOffset;
      }
      else if (WWHFrame.WWHBrowser.mBrowser === 2)  // Shorthand for IE
      {
        // Test required to avoid JavaScript error under IE5.5 on Windows
        //
        if (VarPanelViewFrame.document.body === undefined)
        {
          PanelEntry.mScrollPosition[0] = 0;
          PanelEntry.mScrollPosition[1] = 0;
        }
        else
        {
          if ((VarPanelViewFrame.document.documentElement !== undefined) &&
              (VarPanelViewFrame.document.documentElement.scrollLeft !== undefined) &&
              (VarPanelViewFrame.document.documentElement.scrollTop !== undefined) &&
              ((VarPanelViewFrame.document.documentElement.scrollLeft !== 0) ||
               (VarPanelViewFrame.document.documentElement.scrollTop !== 0)))
          {
            PanelEntry.mScrollPosition[0] = VarPanelViewFrame.document.documentElement.scrollLeft;
            PanelEntry.mScrollPosition[1] = VarPanelViewFrame.document.documentElement.scrollTop;
          }
          else
          {
            PanelEntry.mScrollPosition[0] = VarPanelViewFrame.document.body.scrollLeft;
            PanelEntry.mScrollPosition[1] = VarPanelViewFrame.document.body.scrollTop;
          }
        }
      }
    }
  }
}

function  WWHPanels_RestoreScrollPosition()
{
  var  PanelEntry;
  var  ScrollPosition;
  var  VarFrame;


  if ( ! this.mbLoading)
  {
    // Access panel entry
    //
    PanelEntry = this.fGetCurrentPanelEntry();

    // See if a target position has been specified
    //
    if (PanelEntry.mPanelObject.mPanelAnchor !== null)
    {
      this.fJumpToAnchor();
    }
    else
    {
      // Restore scroll position
      //
      ScrollPosition = PanelEntry.mScrollPosition;

      // setTimeout required for correct operation in Netscape 6.0
      //
      VarFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHPanelViewFrame"));
      window.setTimeout(function () {
        VarFrame.window.scrollTo(ScrollPosition[0], ScrollPosition[1]);
      }, 10);
    }
  }
}

function  WWHPanels_JumpToAnchor()
{
  var  PanelObject;
  var  bVarEnableNavigatorWorkaround;
  var  VarPanelViewFrame;


  if ( ! this.mbLoading)
  {
    // Access panel object
    //
    PanelObject = this.fGetCurrentPanelObject();
    if ((PanelObject.mbPanelInitialized) &&
        (PanelObject.mPanelAnchor !== null))
    {
      if (WWHFrame.WWHBrowser.mbSupportsFocus)
      {
        // Use focus() method
        //
        WWHFrame.WWHHelp.fFocus("WWHPanelViewFrame", PanelObject.mPanelAnchor);

        PanelObject.mPanelAnchor = null;
      }
      else
      {
        // Navigator reloads the page if the hash isn't already defined
        //
        bVarEnableNavigatorWorkaround = false;
        VarPanelViewFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHPanelViewFrame"));
        if (WWHFrame.WWHBrowser.mBrowser === 1)  // Shorthand for Netscape
        {
          if (VarPanelViewFrame.location.hash.length === 0)
          {
            bVarEnableNavigatorWorkaround = true;
          }
        }

        // Jump to anchor
        //
        VarPanelViewFrame.location.hash = PanelObject.mPanelAnchor;

        // Navigator reloads the page if the hash isn't already defined
        //
        if ( ! bVarEnableNavigatorWorkaround)
        {
          PanelObject.mPanelAnchor = null;
        }
      }
    }
  }
}

function  WWHPanels_ChangePanel(ParamPanelIndex)
{
  if (( ! this.mbChangingPanels) &&
      ( ! this.mbLoading))
  {
    // Set flag
    //
    this.mbChangingPanels = true;

    // Save scroll position
    //
    this.fSaveScrollPosition();

    // Close down any popups we had going to prevent JavaScript errors
    //
    this.mPopup.fHide();

    // Set panel index
    //
    this.mCurrentPanel = ParamPanelIndex;

    // Update current panel
    //
    this.fReloadPanel();
  }
}

function  WWHPanels_ReloadPanel()
{
  var  PanelObject;
  var  HTMLFilename;


  if ( ! this.mbLoading)
  {
    // Set flag
    //
    this.mbLoading = true;

    // Access panel object
    //
    PanelObject = this.fGetCurrentPanelObject();

    // Determine file to load
    //
    if ( ! PanelObject.mbPanelInitialized)
    {
      HTMLFilename = "panelmsg.htm";
    }
    else
    {
      HTMLFilename = PanelObject.mPanelFilename;
    }

    // Redirect to the correct file
    //
    WWHFrame.WWHHelp.fReplaceLocation("WWHPanelFrame", WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/html/" + HTMLFilename);
  }
}

function  WWHPanels_ReloadNavigation()
{
  if ( ! this.mbLoading)
  {
    // Set flag
    //
    this.mbLoading = true;

    this.mbReloadOnlyNavigation = true;

    WWHFrame.WWHHelp.fReplaceLocation("WWHPanelNavigationFrame", WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/html/panelnav.htm");
  }
}

function  WWHPanels_ReloadView()
{
  var VarPanels = this;

  WWHFrame.WWHBrowser.fValidateFrameReference(WWHFrame.WWHHelp.fGetFrameReference("WWHPanelViewFrame"),
    function () {
      var  VarPanelViewFrameReference;
      var  VarPanelViewFrame;

      if ( ! VarPanels.mbLoading)
      {
        // Save scroll position
        //
        VarPanels.fSaveScrollPosition();

        // Set flag
        //
        VarPanels.mbLoading = true;

        // Close down any popups we had going to prevent JavaScript errors
        //
        VarPanels.mPopup.fHide();

        // Get frame reference
        //
        VarPanelViewFrameReference = WWHFrame.WWHHelp.fGetFrameReference("WWHPanelViewFrame");
        VarPanelViewFrame = eval(VarPanelViewFrameReference);

        // Reload view
        //
        window.setTimeout(function () {
          // Handle browser specific issues
          //
          if ((WWHFrame.WWHBrowser.mBrowser === 1) ||  // Shorthand for Netscape
              (WWHFrame.WWHBrowser.mBrowser === 4))    // Shorthand for Netscape 6.0 (Mozilla)
          {
            // Navigator has trouble if the hash is defined
            //
            if (VarPanelViewFrame.location.hash.length !== 0)
            {
              VarPanelViewFrame.location.hash = "";

              if (WWHFrame.WWHBrowser.mBrowser === 4)  // Shorthand for Netscape 6.0 (Mozilla)
              {
                VarPanelViewFrame.location.replace(WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/html/panelvie.htm");
              }
            }
          }

          VarPanelViewFrame.location.replace(WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/html/panelvie.htm");
        }, 1);
      }
    });
}

function  WWHPanels_PanelNavigationLoaded()
{
  var  PanelObject;


  // Access panel object
  //
  PanelObject = this.fGetCurrentPanelObject();

  // Set frame name for accessibility
  //
  if (WWHFrame.WWHHelp.mbAccessible)
  {
    WWHFrame.WWHHelp.fSetFrameName("WWHPanelNavigationFrame");
  }

  // Update flag
  //
  if (this.mbReloadOnlyNavigation)
  {
    this.mbLoading = false;
  }

  // Notify panel object navigation loaded
  //
  PanelObject.fPanelNavigationLoaded();

  // Load view panel
  //
  if (this.mbReloadOnlyNavigation)
  {
    this.mbReloadOnlyNavigation = false;
  }
  else
  {
    WWHFrame.WWHHelp.fReplaceLocation("WWHPanelViewFrame", WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/html/panelvie.htm");
  }
}

function  WWHPanels_PanelViewLoaded()
{
  var  PanelObject;


  // Set frame name for accessibility
  //
  if (WWHFrame.WWHHelp.mbAccessible)
  {
    WWHFrame.WWHHelp.fSetFrameName("WWHPanelViewFrame");
  }

  // Update flag
  //
  this.mbLoading = false;

  // Restore window position
  //
  this.fRestoreScrollPosition();

  // Update popup window reference
  //
  this.mPopup.mWindowRef = WWHFrame.WWHHelp.fGetFrameReference("WWHPanelViewFrame");

  // Access panel object
  //
  PanelObject = this.fGetCurrentPanelObject();

  // Notify panel object view loaded
  //
  PanelObject.fPanelViewLoaded();

  // Complete changing tabs
  //
  if (this.mbChangingPanels)
  {
    // Notify end of tab change
    //
    WWHFrame.WWHJavaScript.fEndChangeTab();

    this.mbChangingPanels = false;
  }
}

function  WWHPanelHoverText_Translate(ParamEntryID)
{
  var  Text = "";
  var  PanelObject;


  // Access panel object
  //
  PanelObject = WWHFrame.WWHJavaScript.mPanels.fGetCurrentPanelObject();
  if (PanelObject.mbPanelInitialized)
  {
    Text = PanelObject.fHoverTextTranslate(ParamEntryID);
  }

  return Text;
}

function  WWHPanelHoverText_Format(ParamWidth,
                                   ParamTextID,
                                   ParamText)
{
  var  HTML = "";
  var  PanelObject;


  // Access panel object
  //
  PanelObject = WWHFrame.WWHJavaScript.mPanels.fGetCurrentPanelObject();
  if (PanelObject.mbPanelInitialized)
  {
    HTML = PanelObject.fHoverTextFormat(ParamWidth, ParamTextID, ParamText);
  }

  return HTML;
}
