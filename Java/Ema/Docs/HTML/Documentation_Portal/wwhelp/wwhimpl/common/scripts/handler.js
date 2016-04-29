// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint sloppy: true, vars: true, white: true */
/*global window */
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

function  WWHHandler_Object()
{
  this.mbInitialized = false;

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
  this.mbInitialized = true;
  WWHFrame.WWHHelp.fHandlerInitialized();
}

function  WWHHandler_Finalize()
{
}

function  WWHHandler_GetFrameReference(ParamFrameName)
{
  // Nothing to do
  //
  return null;
}

function  WWHHandler_GetFrameName(ParamFrameName)
{
  var  VarName = null;


  // Nothing to do
  //

  return VarName;
}

function  WWHHandler_IsReady()
{
  var  bVarIsReady = true;


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
  window.setTimeout(function () {
    WWHFrame.WWHControls.fSwitchToNavigation();
  }, 1);
}

function  WWHHandler_FavoritesCurrent(ParamBookIndex,
                                      ParamFileIndex)
{
}

function  WWHHandler_ProcessAccessKey(ParamAccessKey)
{
  switch (ParamAccessKey)
  {
    case 1:
      WWHFrame.WWHControls.fSwitchToNavigation("contents");
      break;

    case 2:
      WWHFrame.WWHControls.fSwitchToNavigation("index");
      break;

    case 3:
      WWHFrame.WWHControls.fSwitchToNavigation("search");
      break;
  }
}

function  WWHHandler_GetCurrentTab()
{
  var  VarCurrentTab;


  // Initialize return value
  //
  VarCurrentTab = "";

  return VarCurrentTab;
}

function  WWHHandler_SetCurrentTab(ParamTabName)
{
}
