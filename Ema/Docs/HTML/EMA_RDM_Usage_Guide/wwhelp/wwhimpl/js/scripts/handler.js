// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global Image */
/*global WWHFrame */
/*global WWHHandler_FavoritesCurrent */
/*global WWHHandler_Finalize */
/*global WWHHandler_GetCurrentTab */
/*global WWHHandler_GetFrameName */
/*global WWHHandler_GetFrameReference */
/*global WWHHandler_Init */
/*global WWHHandler_IsReady */
/*global WWHHandler_ProcessAccessKey */
/*global WWHHandler_SetCurrentTab */
/*global WWHHandler_SyncTOC */
/*global WWHHandler_Update */
/*global WWHStringUtilities_EscapeHTML */
/*global WWHStringUtilities_FormatMessage */

function  WWHHandler_Object()
{
  this.mbInitialized = false;
  this.mImages       = [];

  this.fInit              = WWHHandler_Init;
  this.fFinalize          = WWHHandler_Finalize;
  this.fGetFrameReference = WWHHandler_GetFrameReference;
  this.fGetFrameName      = WWHHandler_GetFrameName;
  this.fIsReady           = WWHHandler_IsReady;
  this.fUpdate            = WWHHandler_Update;
  this.fSyncTOC           = WWHHandler_SyncTOC;
  this.fFavoritesCurrent  = WWHHandler_FavoritesCurrent;
  this.fProcessAccessKey  = WWHHandler_ProcessAccessKey;
  this.fGetCurrentTab     = WWHHandler_GetCurrentTab;
  this.fSetCurrentTab     = WWHHandler_SetCurrentTab;
}

function  WWHHandler_Init()
{
  WWHFrame.WWHJavaScript.fInit();
}

function  WWHHandler_Finalize()
{
  var  VarImageDirectory = WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/js/images";

  // Preload graphics
  //
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_bg.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_bgx.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_e.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_ex.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_n.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_ne.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_nex.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_nw.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_nwx.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_nx.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_s.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_se.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_sex.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_sw.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_swx.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_sx.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_w.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/btn_wx.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/navbg.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/spc_e.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/spc_n.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/spc_s.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/spc_sx.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/spc_tabl.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/spc_tabm.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/spc_tabr.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/spc_top.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/spc_topx.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/spc_w.gif";
  this.mImages[this.mImages.length] = new Image(); this.mImages[this.mImages.length - 1].src = VarImageDirectory + "/viewbg.gif";

  // Display tab and panel
  //
  if (WWHFrame.WWHHelp.mInitialTabName !== null)
  {
    WWHFrame.WWHJavaScript.fStartChangeTab(WWHFrame.WWHJavaScript.mInitialTab);
  }
  else
  {
    WWHFrame.WWHJavaScript.fClickedChangeTab(WWHFrame.WWHJavaScript.mInitialTab);
  }
}

function  WWHHandler_GetFrameReference(ParamFrameName)
{
  var  VarFrameReference;


  switch (ParamFrameName)
  {
    case "WWHTabsFrame":
      // WWHFrame.WWHNavigationFrame.WWHTabsFrame
      //
      VarFrameReference = WWHFrame.WWHHelp.fGetFrameReference("WWHNavigationFrame") + ".frames[0]";
      break;

    case "WWHPanelFrame":
      // WWHFrame.WWHNavigationFrame.WWHPanelFrame
      //
      VarFrameReference = WWHFrame.WWHHelp.fGetFrameReference("WWHNavigationFrame") + ".frames[1]";
      break;

    case "WWHPanelNavigationFrame":
      // WWHFrame.WWHNavigationFrame.WWHPanelFrame.WWHPanelNavigationFrame
      //
      VarFrameReference = WWHFrame.WWHHelp.fGetFrameReference("WWHPanelFrame") + ".frames[0]";
      break;

    case "WWHPanelViewFrame":
      // WWHFrame.WWHNavigationFrame.WWHPanelFrame.WWHPanelViewFrame
      //
      if (WWHFrame.WWHJavaScript.mPanels.fGetCurrentPanelObject().mPanelFilename === "panelvie.htm")
      {
        VarFrameReference = WWHFrame.WWHHelp.fGetFrameReference("WWHPanelFrame");
      }
      else
      {
        VarFrameReference = WWHFrame.WWHHelp.fGetFrameReference("WWHPanelFrame") + ".frames[1]";
      }
      break;
  }

  return VarFrameReference;
}

function  WWHHandler_GetFrameName(ParamFrameName)
{
  var  VarName = null;


  // Determine name for this frame
  //
  switch (ParamFrameName)
  {
    case "WWHTabsFrame":
      VarName = "";

      if (WWHFrame.WWHJavaScript.mSettings.mTOC.mbShow)
      {
        if (VarName.length > 0)
        {
          VarName += WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " ";
        }
        VarName += WWHFrame.WWHJavaScript.mMessages.mTabsTOCLabel;
      }

      if (WWHFrame.WWHJavaScript.mSettings.mIndex.mbShow)
      {
        if (VarName.length > 0)
        {
          VarName += WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " ";
        }
        VarName += WWHFrame.WWHJavaScript.mMessages.mTabsIndexLabel;
      }

      if (WWHFrame.WWHJavaScript.mSettings.mSearch.mbShow)
      {
        if (VarName.length > 0)
        {
          VarName += WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " ";
        }
        VarName += WWHFrame.WWHJavaScript.mMessages.mTabsSearchLabel;
      }

      if (WWHFrame.WWHJavaScript.mSettings.mFavorites.mbShow)
      {
        if (VarName.length > 0)
        {
          VarName += WWHFrame.WWHHelp.mMessages.mAccessibilityListSeparator + " ";
        }
        VarName += WWHFrame.WWHJavaScript.mMessages.mTabsFavoritesLabel;
      }

      VarName = WWHStringUtilities_FormatMessage(WWHFrame.WWHJavaScript.mMessages.mAccessibilityTabsFrameName, VarName);
      VarName = WWHStringUtilities_EscapeHTML(VarName);
      break;

    case "WWHPanelFrame":
      // Nothing to do
      //
      break;

    case "WWHPanelNavigationFrame":
      VarName = WWHStringUtilities_FormatMessage(WWHFrame.WWHJavaScript.mMessages.mAccessibilityNavigationFrameName,
                                                 WWHFrame.WWHJavaScript.mPanels.fGetCurrentPanelObject().mPanelTabTitle);
      VarName = WWHStringUtilities_EscapeHTML(VarName);
      break;

    case "WWHPanelViewFrame":
      VarName = WWHFrame.WWHJavaScript.mPanels.fGetCurrentPanelObject().mPanelTabTitle;
      break;
  }

  return VarName;
}

function  WWHHandler_IsReady()
{
  var  bVarIsReady = true;


  if ((WWHFrame.WWHJavaScript.mbChangingTabs) ||
      (WWHFrame.WWHJavaScript.mPanels.mbChangingPanels) ||
      (WWHFrame.WWHJavaScript.mPanels.mbLoading))
  {
    bVarIsReady = false;
  }

  return bVarIsReady;
}

function  WWHHandler_Update(ParamBookIndex,
                            ParamFileIndex)
{
}

function  WWHHandler_SyncTOC(ParamBookIndex,
                             ParamFileIndex,
                             ParamAnchor,
                             bParamReportError)
{
  WWHFrame.WWHJavaScript.fSyncTOC(ParamBookIndex,
                                  ParamFileIndex,
                                  ParamAnchor,
                                  bParamReportError);
}

function  WWHHandler_FavoritesCurrent(ParamBookIndex,
                                      ParamFileIndex)
{
  WWHFrame.WWHJavaScript.fFavoritesCurrent(ParamBookIndex,
                                           ParamFileIndex);
}

function  WWHHandler_ProcessAccessKey(ParamAccessKey)
{
  switch (ParamAccessKey)
  {
    case 1:
      this.fSetCurrentTab("contents");
      break;

    case 2:
      this.fSetCurrentTab("index");
      break;

    case 3:
      this.fSetCurrentTab("search");
      break;
  }
}

function  WWHHandler_GetCurrentTab()
{
  var  VarCurrentTab;


  // Initialize return value
  //
  VarCurrentTab = "";

  if (WWHFrame.WWHJavaScript.mPanels.mCurrentPanel === WWHFrame.WWHOutline.mPanelTabIndex)
  {
    VarCurrentTab = "contents";
  }
  else if (WWHFrame.WWHJavaScript.mPanels.mCurrentPanel === WWHFrame.WWHIndex.mPanelTabIndex)
  {
    VarCurrentTab = "index";
  }
  else if (WWHFrame.WWHJavaScript.mPanels.mCurrentPanel === WWHFrame.WWHSearch.mPanelTabIndex)
  {
    VarCurrentTab = "search";
  }
  else if (WWHFrame.WWHJavaScript.mPanels.mCurrentPanel === WWHFrame.WWHFavorites.mPanelTabIndex)
  {
    VarCurrentTab = "favorites";
  }

  return VarCurrentTab;
}

function  WWHHandler_SetCurrentTab(ParamTabName)
{
  switch (ParamTabName)
  {
    case "contents":
      // Contents exists?
      //
      if (WWHFrame.WWHOutline.mPanelTabIndex !== -1)
      {
        // SyncTOC if possible
        //
        if (WWHFrame.WWHControls.fCanSyncTOC())
        {
          WWHFrame.WWHControls.fClickedSyncTOC();
        }
        else
        {
          // Focus if visible, otherwise switch panels
          //
          if (WWHFrame.WWHJavaScript.mPanels.mCurrentPanel === WWHFrame.WWHOutline.mPanelTabIndex)
          {
            WWHFrame.WWHJavaScript.mPanels.fGetCurrentPanelObject().fPanelViewLoaded();
          }
          else
          {
            WWHFrame.WWHJavaScript.fClickedChangeTab(WWHFrame.WWHOutline.mPanelTabIndex);
          }
        }
      }
      break;

    case "index":
      // Index exists?
      //
      if (WWHFrame.WWHIndex.mPanelTabIndex !== -1)
      {
        // Focus if visible, otherwise switch panels
        //
        if (WWHFrame.WWHJavaScript.mPanels.mCurrentPanel === WWHFrame.WWHIndex.mPanelTabIndex)
        {
          WWHFrame.WWHJavaScript.mPanels.fGetCurrentPanelObject().fPanelNavigationLoaded();
        }
        else
        {
          WWHFrame.WWHJavaScript.fClickedChangeTab(WWHFrame.WWHIndex.mPanelTabIndex);
        }
      }
      break;

    case "search":
      // Search exists?
      //
      if (WWHFrame.WWHSearch.mPanelTabIndex !== -1)
      {
        // Focus if visible, otherwise switch panels
        //
        if (WWHFrame.WWHJavaScript.mPanels.mCurrentPanel === WWHFrame.WWHSearch.mPanelTabIndex)
        {
          WWHFrame.WWHJavaScript.mPanels.fGetCurrentPanelObject().fPanelNavigationLoaded();
        }
        else
        {
          WWHFrame.WWHJavaScript.fClickedChangeTab(WWHFrame.WWHSearch.mPanelTabIndex);
        }
      }
      break;

    case "favorites":
      // Favorites exists?
      //
      if (WWHFrame.WWHFavorites.mPanelTabIndex !== -1)
      {
        // Focus if visible, otherwise switch panels
        //
        if (WWHFrame.WWHJavaScript.mPanels.mCurrentPanel === WWHFrame.WWHFavorites.mPanelTabIndex)
        {
          WWHFrame.WWHJavaScript.mPanels.fGetCurrentPanelObject().fPanelNavigationLoaded();
        }
        else
        {
          WWHFrame.WWHJavaScript.fClickedChangeTab(WWHFrame.WWHFavorites.mPanelTabIndex);
        }
      }
      break;
  }
}
