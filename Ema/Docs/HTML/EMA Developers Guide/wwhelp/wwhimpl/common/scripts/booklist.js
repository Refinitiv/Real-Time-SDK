// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint sloppy: true, vars: true, white: true */
/*global WWHFrame */
/*global WWHBookList_BookFileIndiciesToHREF */
/*global WWHBookList_BookIndexFileIndexToTitle */
/*global WWHBookList_GetBook */
/*global WWHBookList_GetBookIndexFileHREF */
/*global WWHBookList_GetBookTitle */
/*global WWHBookList_GetContextBook */
/*global WWHBookList_GetContextIndex */
/*global WWHBookList_GetSyncPrevNext */
/*global WWHBookList_HREFToBookIndexFileIndexAnchor */
/*global WWHBookList_HREFToFileIndex */
/*global WWHBookList_HREFToTitle */
/*global WWHBookList_Init_AddBook */
/*global WWHBookList_Init_AddBookDir */
/*global WWHBookList_Init_BookData_Script */
/*global WWHBookList_Init_IncrementIndex */
/*global WWHBook_Init */
/*global WWHFileList_Object */
/*global WWHStringBuffer_Object */

function  WWHBook_Object(ParamDirectory)
{
  // Set values from callbacks
  //
  this.mDirectory = null;
  this.mTitle     = null;
  this.mContext   = null;
  this.mFiles     = new WWHFileList_Object();

  // Fix up directory
  //
  if (ParamDirectory === ".")
  {
    this.mDirectory = "";
  }
  else
  {
    this.mDirectory = ParamDirectory + "/";
  }

  this.fInit = WWHBook_Init;
}

function  WWHBook_Init(ParamTitle,
                       ParamContext,
                       ParamFilesFunction,
                       ParamALinksFunction)
{
  this.mTitle   = ParamTitle;
  this.mContext = ParamContext;

  // Load files
  //
  ParamFilesFunction(this.mFiles);

  // Load alinks
  //
  ParamALinksFunction(WWHFrame.WWHALinks);
}

function  WWHBookList_Object()
{
  this.mInitIndex          = 0;
  this.mBookList           = [];
  this.mBookContextToIndex = {};

  this.fInit_AddBookDir                = WWHBookList_Init_AddBookDir;
  this.fInit_BookData_Script           = WWHBookList_Init_BookData_Script;
  this.fInit_AddBook                   = WWHBookList_Init_AddBook;
  this.fInit_IncrementIndex            = WWHBookList_Init_IncrementIndex;
  this.fGetBook                        = WWHBookList_GetBook;
  this.fGetBookTitle                   = WWHBookList_GetBookTitle;
  this.fHREFToFileIndex                = WWHBookList_HREFToFileIndex;
  this.fHREFToTitle                    = WWHBookList_HREFToTitle;
  this.fBookIndexFileIndexToTitle      = WWHBookList_BookIndexFileIndexToTitle;
  this.fGetBookIndexFileHREF           = WWHBookList_GetBookIndexFileHREF;
  this.fBookFileIndiciesToHREF         = WWHBookList_BookFileIndiciesToHREF;
  this.fHREFToBookIndexFileIndexAnchor = WWHBookList_HREFToBookIndexFileIndexAnchor;
  this.fGetSyncPrevNext                = WWHBookList_GetSyncPrevNext;
  this.fGetContextIndex                = WWHBookList_GetContextIndex;
  this.fGetContextBook                 = WWHBookList_GetContextBook;
}

function  WWHBookList_Init_AddBookDir(ParamBookDir)
{
  this.mBookList[this.mBookList.length] = new WWHBook_Object(ParamBookDir);
}

function  WWHBookList_Init_BookData_Script()
{
  var  Scripts  = new WWHStringBuffer_Object();
  var  VarParameters;
  var  MaxIndex = 0;
  var  Index    = 0;
  var  BookDirectory;


  // Workaround Safari reload bug
  //
  VarParameters = "";
  if (WWHFrame.WWHBrowser.mBrowser === 5)  // Shorthand for Safari
  {
    VarParameters = "?" + (new Date() * 1);
  }

  this.mInitIndex = 0;
  for (MaxIndex = this.mBookList.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    BookDirectory = WWHFrame.WWHHelp.mHelpURLPrefix + this.mBookList[Index].mDirectory;

    Scripts.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"" + BookDirectory + "wwhdata/common/title.js" + VarParameters + "\"></script>\n");
    Scripts.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"" + BookDirectory + "wwhdata/common/context.js" + VarParameters + "\"></script>\n");
    Scripts.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"" + BookDirectory + "wwhdata/common/files.js" + VarParameters + "\"></script>\n");
    Scripts.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"" + BookDirectory + "wwhdata/common/alinks.js" + VarParameters + "\"></script>\n");

    Scripts.fAppend("<script type=\"text/javascript\" language=\"JavaScript1.2\" src=\"" + WWHFrame.WWHHelp.mHelpURLPrefix + "wwhelp/wwhimpl/common/scripts/bklist1s.js" + VarParameters + "\"></script>\n");
  }

  return Scripts.fGetBuffer();
}

function  WWHBookList_Init_AddBook(ParamTitle,
                                   ParamContext,
                                   ParamFilesFunction,
                                   ParamALinksFunction)
{
  // Update book information
  //
  this.mBookList[this.mInitIndex].fInit(ParamTitle, ParamContext,
                                        ParamFilesFunction,
                                        ParamALinksFunction);
  this.mBookContextToIndex[ParamContext + "~"] = this.mInitIndex;
}

function  WWHBookList_Init_IncrementIndex()
{
  this.mInitIndex += 1;
}

function  WWHBookList_GetBook(ParamIndex)
{
  return this.mBookList[ParamIndex];
}

function  WWHBookList_GetBookTitle(ParamIndex)
{
  return this.mBookList[ParamIndex].mTitle;
}

function  WWHBookList_HREFToFileIndex(ParamIndex,
                                      ParamHREF)
{
  return this.mBookList[ParamIndex].mFiles.fHREFToIndex(ParamHREF);
}

function  WWHBookList_HREFToTitle(ParamIndex,
                                  ParamHREF)
{
  return this.mBookList[ParamIndex].mFiles.fHREFToTitle(ParamHREF);
}

function  WWHBookList_BookIndexFileIndexToTitle(ParamBookIndex,
                                                ParamFileIndex)
{
  return this.mBookList[ParamBookIndex].mFiles.fFileIndexToTitle(ParamFileIndex);
}

function  WWHBookList_GetBookIndexFileHREF(ParamHREF)
{
  var  ResultArray = [ -1, null ];
  var  LongestMatchIndex;
  var  MaxIndex;
  var  Index;
  var  FileHREF;


  // Find the book directory
  //
  LongestMatchIndex = -1;
  for (MaxIndex = this.mBookList.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    if (ParamHREF.indexOf(this.mBookList[Index].mDirectory) === 0)
    {
      if (LongestMatchIndex === -1)
      {
        LongestMatchIndex = Index;
      }
      else if (this.mBookList[Index].mDirectory.length > this.mBookList[LongestMatchIndex].mDirectory.length)
      {
        LongestMatchIndex = Index;
      }
    }
  }

  // If LongestMatchIndex is valid, we found our book directory
  //
  if (LongestMatchIndex !== -1)
  {
    // Set FileHREF to be just the file portion
    //
    if (this.mBookList[LongestMatchIndex].mDirectory.length > 0)
    {
      FileHREF = ParamHREF.substring(this.mBookList[LongestMatchIndex].mDirectory.length, ParamHREF.length);
    }
    else
    {
      FileHREF = ParamHREF;
    }

    ResultArray[0] = LongestMatchIndex;
    ResultArray[1] = FileHREF;
  }

  return ResultArray;
}

function  WWHBookList_BookFileIndiciesToHREF(ParamBookIndex,
                                             ParamFileIndex)
{
  return this.mBookList[ParamBookIndex].mDirectory + this.mBookList[ParamBookIndex].mFiles.fFileIndexToHREF(ParamFileIndex);
}

function  WWHBookList_HREFToBookIndexFileIndexAnchor(ParamHREF)
{
  var  ResultArray = [ -1, -1, "" ];
  var  Parts;
  var  TrimmedHREF;
  var  Anchor;
  var  BookIndex;
  var  FileIndex;


  // Record anchor
  //
  Parts = ParamHREF.split("#");
  TrimmedHREF = Parts[0];
  Anchor = "";
  if (Parts.length > 1)
  {
    if (Parts[1].length > 0)
    {
      Anchor = Parts[1];
    }
  }

  // Determine book index
  //
  Parts = this.fGetBookIndexFileHREF(TrimmedHREF);
  if (Parts[0] >= 0)
  {
    BookIndex = Parts[0];
    FileIndex = this.fHREFToFileIndex(BookIndex, Parts[1]);

    if (FileIndex >= 0)
    {
      ResultArray[0] = BookIndex;
      ResultArray[1] = FileIndex;
      ResultArray[2] = Anchor;
    }
  }

  return ResultArray;
}

function  WWHBookList_GetSyncPrevNext(ParamHREF)
{
  var  ResultArray = [ null, null, null ];
  var  Parts;
  var  BookIndex;
  var  FileIndex;


  // Determine current book index and file index
  //
  Parts = this.fHREFToBookIndexFileIndexAnchor(ParamHREF);
  BookIndex = Parts[0];
  FileIndex = Parts[1];

  // Set return results
  //
  if ((BookIndex >= 0) &&
      (FileIndex >= 0))
  {
    // Set sync
    //
    ResultArray[0] = ParamHREF;  // Indicates file found, sync possible

    // Set previous
    //
    if (FileIndex > 0)
    {
      ResultArray[1] = this.fBookFileIndiciesToHREF(BookIndex, FileIndex - 1);
    }
    else
    {
      if (BookIndex > 0)
      {
        ResultArray[1] = this.fBookFileIndiciesToHREF(BookIndex - 1, this.mBookList[BookIndex - 1].mFiles.mFileList.length - 1);
      }
    }

    // Set next
    //
    if ((FileIndex + 1) < this.mBookList[BookIndex].mFiles.mFileList.length)
    {
      ResultArray[2] = this.fBookFileIndiciesToHREF(BookIndex, FileIndex + 1);
    }
    else
    {
      if (((BookIndex + 1) < this.mBookList.length) &&
          (this.mBookList[BookIndex + 1].mFiles.mFileList.length > 0))
      {
        ResultArray[2] = this.fBookFileIndiciesToHREF(BookIndex + 1, 0);
      }
    }
  }

  return ResultArray;
}

function  WWHBookList_GetContextIndex(ParamContext)
{
  var  RetIndex = -1;

  if (typeof(this.mBookContextToIndex[ParamContext + "~"]) === "number")
  {
    RetIndex = this.mBookContextToIndex[ParamContext + "~"];
  }

  return RetIndex;
}

function  WWHBookList_GetContextBook(ParamContext)
{
  var  ResultBook = null;
  var  MaxIndex;
  var  Index;


  for (MaxIndex = this.mBookList.length, Index = 0 ; Index < MaxIndex ; Index += 1)
  {
    if (this.mBookList[Index].mContext === ParamContext)
    {
      ResultBook = this.mBookList[Index];
    }
  }

  return ResultBook;
}
