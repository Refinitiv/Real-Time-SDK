// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global unescape */
/*global WWHFileList_AddFile */
/*global WWHFileList_Entries */
/*global WWHFileList_FileIndexToHREF */
/*global WWHFileList_FileIndexToTitle */
/*global WWHFileList_HREFToIndex */
/*global WWHFileList_HREFToTitle */
/*global WWHStringUtilities_EscapeHTML */

function  WWHFile_Object(ParamTitle,
                         ParamHREF)
{
  this.mTitle = ParamTitle;
  this.mHREF  = ParamHREF;
}

function  WWHFileList_Object()
{
  this.mFileList = [];
  this.mFileHash = {};

  this.fEntries          = WWHFileList_Entries;
  this.fAddFile          = WWHFileList_AddFile;
  this.fA                = WWHFileList_AddFile;
  this.fHREFToIndex      = WWHFileList_HREFToIndex;
  this.fHREFToTitle      = WWHFileList_HREFToTitle;
  this.fFileIndexToHREF  = WWHFileList_FileIndexToHREF;
  this.fFileIndexToTitle = WWHFileList_FileIndexToTitle;
}

function  WWHFileList_Entries()
{
  return this.mFileList.length;
}

function  WWHFileList_AddFile(ParamTitle,
                              ParamHREF)
{
  // Store unescaped to avoid browser specific auto-unescape behaviors
  //
  this.mFileHash[unescape(ParamHREF) + "~"] = this.mFileList.length;
  this.mFileList[this.mFileList.length] = new WWHFile_Object(ParamTitle, ParamHREF);
}

function  WWHFileList_HREFToIndex(ParamHREF)
{
  var  MatchIndex = -1;
  var  Match;


  // Query unescaped to avoid browser specific auto-unescape behaviors
  //
  Match = this.mFileHash[unescape(ParamHREF) + "~"];
  if (Match !== undefined)
  {
    MatchIndex = Match;
  }

  return MatchIndex;
}

function  WWHFileList_HREFToTitle(ParamHREF)
{
  var  Title = "";
  var  MatchIndex;


  MatchIndex = this.fHREFToIndex(ParamHREF);
  if (MatchIndex !== -1)
  {
    Title = this.mFileList[MatchIndex].mTitle;
  }
  else
  {
    Title = WWHStringUtilities_EscapeHTML(ParamHREF);
  }

  return Title;
}

function  WWHFileList_FileIndexToHREF(ParamIndex)
{
  return this.mFileList[ParamIndex].mHREF;
}

function  WWHFileList_FileIndexToTitle(ParamIndex)
{
  return this.mFileList[ParamIndex].mTitle;
}
