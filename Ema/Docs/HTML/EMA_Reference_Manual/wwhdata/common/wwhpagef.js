// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global window */
/*global WWHFrame */
/*global WWHToWWHelpDirectory */
/*global WWHBookData_Context */
/*global WWHCheckHash */

function  WWHGetWWHFrame(ParamToBookDir,
                         ParamRedirect)
{
  var  Frame = null;


  // Set reference to top level help frame
  //
  try
  {
    if ((window.parent.WWHHelp !== undefined) &&
        (window.parent.WWHHelp !== null))
    {
      Frame = window.parent;
    }
    else if ((window.parent.parent.WWHHelp !== undefined) &&
             (window.parent.parent.WWHHelp !== null))
    {
      Frame = window.parent.parent;
    }
  }
  catch (ignore)
  {
    // Assume we've got a security situation on our hands
    //
  }

  // Redirect if Frame is null
  //
  if ((Frame === null) && ParamRedirect)
  {
    var  bPerformRedirect = true;
    var  Agent;


    // No redirect if running Netscape 4.x
    //
    Agent = window.navigator.userAgent.toLowerCase();
    if ((Agent.indexOf("mozilla") !== -1) &&
        (Agent.indexOf("spoofer") === -1) &&
        (Agent.indexOf("compatible") === -1))
    {
      var  MajorVersion;


      MajorVersion = parseInt(window.navigator.appVersion, 10);
      if (MajorVersion < 5)
      {
        bPerformRedirect = false;  // Skip redirect for Netscape 4.x
      }
    }

    if (bPerformRedirect)
    {
      var  BaseFilename;
      var  RedirectURI;

      BaseFilename = window.location.href.substring(window.location.href.lastIndexOf("/") + 1, window.location.href.length);

      if (ParamToBookDir.length > 0)
      {
        var  RelativePathList = ParamToBookDir.split("/");
        var  PathList         = window.location.href.split("/");
        var  BaseList = [];
        var  MaxIndex;
        var  Index;
        var  RelativePathComponent;

        // Trim base file component
        //
        PathList.length -= 1;
        for (MaxIndex = RelativePathList.length, Index = 0 ; Index < MaxIndex ; Index += 1)
        {
          RelativePathComponent = RelativePathList[Index];

          if (
              (RelativePathComponent !== "")
               &&
              (RelativePathComponent !== ".")
             )
          {
            if (RelativePathComponent === "..")
            {
              BaseList[BaseList.length] = PathList[PathList.length - 1];
              PathList.length = PathList.length - 1;
            }
            else
            {
              BaseList[BaseList.length] = RelativePathComponent;
            }
          }
        }

        // Reverse compoment list before determining base file path
        //
        BaseList.reverse();

        // Build path
        //
        if (BaseList.length > 0)
        {
          BaseFilename = BaseList.join("/") + "/" + BaseFilename;
        }
      }

      // Redirect
      //
      RedirectURI = WWHToWWHelpDirectory() + ParamToBookDir + "wwhelp/wwhimpl/api.htm?context=" + WWHBookData_Context() + "&file=" + BaseFilename + "&single=true";
      window.setTimeout(function () {
        window.location.replace(RedirectURI);
      }, 50);
    }
  }

  return Frame;
}

function  WWHShowPopup(ParamContext,
                       ParamLink,
                       ParamEvent)
{
  if (WWHFrame !== null)
  {
    if ((ParamEvent === null) &&
        (window.event !== undefined))
    {
      ParamEvent = window.event;  // Older IE browsers only store event in window.event
    }

    WWHFrame.WWHHelp.fShowPopup(ParamContext, ParamLink, ParamEvent);
  }
}

function  WWHPopupLoaded()
{
  if (WWHFrame !== null)
  {
    WWHFrame.WWHHelp.fPopupLoaded();
  }
}

function  WWHHidePopup()
{
  if (WWHFrame !== null)
  {
    WWHFrame.WWHHelp.fHidePopup();
  }
}

function  WWHClickedPopup(ParamContext,
                          ParamLink,
                          ParamPopupLink)
{
  if (WWHFrame !== null)
  {
    WWHFrame.WWHHelp.fClickedPopup(ParamContext, ParamLink, ParamPopupLink);
  }
}

function  WWHShowTopic(ParamContext,
                       ParamTopic)
{
  if (WWHFrame !== null)
  {
    WWHFrame.WWHHelp.fShowTopic(ParamContext, ParamTopic);
  }
}

function  WWHUpdate()
{
  var  bVarSuccess = true;


  if (WWHFrame !== null)
  {
    bVarSuccess = WWHFrame.WWHHelp.fUpdate(window.location.href);

    // Only update if "?" is not present (and therefore has priority)
    //
    if (window.location.href.indexOf("?") === -1)
    {
      // Start check hash polling
      //
      if ((WWHFrame.WWHBrowser.mBrowser === 2) ||  // Shorthand for IE
          (WWHFrame.WWHBrowser.mBrowser === 4))    // Shorthand for Netscape 6.0 (Mozilla)
      {
        if (WWHFrame.location.hash.length > 0)
        {
          WWHFrame.WWHHelp.mLastHash = WWHFrame.location.hash;
          WWHFrame.WWHHelp.mCheckHashTimeoutID = window.setTimeout(WWHCheckHash, 100);
        }
      }
    }
  }

  return bVarSuccess;
}

function  WWHCheckHash()
{
  // Clear timeout ID
  //
  WWHFrame.WWHHelp.mCheckHashTimeoutID = null;

  // Change detected?
  //
  if ((WWHFrame.WWHHelp.mLastHash.length > 0) &&
      (WWHFrame.location.hash !== WWHFrame.WWHHelp.mLastHash))
  {
    if ((WWHFrame.WWHHelp.mLastHash.indexOf("topic=") > 0) &&
        (WWHFrame.location.hash.indexOf("#href=") === 0))
    {
      // Context-sensitive link resolved
      // Update last hash and keep polling
      //
      WWHFrame.WWHHelp.mLastHash = WWHFrame.location.hash;
      WWHFrame.WWHHelp.mCheckHashTimeoutID = window.setTimeout(WWHCheckHash, 100);
    }
    else
    {
      // Set context document
      //
      WWHFrame.WWHHelp.mLastHash = "";
      WWHFrame.WWHHelp.fSetContextDocument(WWHFrame.location.href);
    }
  }
  else
  {
    // Keep polling
    //
    WWHFrame.WWHHelp.mLastHash = WWHFrame.location.hash;
    WWHFrame.WWHHelp.mCheckHashTimeoutID = window.setTimeout(WWHCheckHash, 100);
  }
}

function  WWHUnload()
{
  var  bVarSuccess = true;


  if (WWHFrame !== null)
  {
    // Stop check hash polling
    //
    if ((WWHFrame.WWHHelp.mCheckHashTimeoutID !== null) &&
        (WWHFrame.WWHHelp.mCheckHashTimeoutID !== undefined))
    {
      window.clearTimeout(WWHFrame.WWHHelp.mCheckHashTimeoutID);
      WWHFrame.WWHHelp.mCheckHashTimeoutID = null;
      WWHFrame.WWHHelp.mLastHash = "";
    }

    if (WWHFrame.WWHHelp !== undefined)
    {
      bVarSuccess = WWHFrame.WWHHelp.fUnload();
    }
  }

  return bVarSuccess;
}

function  WWHHandleKeyDown(ParamEvent)
{
  var  bVarSuccess = true;


  if (WWHFrame !== null)
  {
    bVarSuccess = WWHFrame.WWHHelp.fHandleKeyDown(ParamEvent);
  }

  return bVarSuccess;
}

function  WWHHandleKeyPress(ParamEvent)
{
  var  bVarSuccess = true;


  if (WWHFrame !== null)
  {
    bVarSuccess = WWHFrame.WWHHelp.fHandleKeyPress(ParamEvent);
  }

  return bVarSuccess;
}

function  WWHHandleKeyUp(ParamEvent)
{
  var  bVarSuccess = true;


  if (WWHFrame !== null)
  {
    bVarSuccess = WWHFrame.WWHHelp.fHandleKeyUp(ParamEvent);
  }

  return bVarSuccess;
}

function  WWHClearRelatedTopics()
{
  if (WWHFrame !== null)
  {
    WWHFrame.WWHRelatedTopics.fClear();
  }
}

function  WWHAddRelatedTopic(ParamText,
                             ParamContext,
                             ParamFileURL)
{
  if (WWHFrame !== null)
  {
    WWHFrame.WWHRelatedTopics.fAdd(ParamText, ParamContext, ParamFileURL);
  }
}

function  WWHRelatedTopicsInlineHTML()
{
  var  HTML = "";


  if (WWHFrame !== null)
  {
    HTML = WWHFrame.WWHRelatedTopics.fInlineHTML();
  }

  return HTML;
}

function  WWHDoNothingHREF()
{
  // Nothing to do.
  //
}

function  WWHShowRelatedTopicsPopup(ParamEvent)
{
  if (WWHFrame !== null)
  {
    if ((ParamEvent === null) &&
        (window.event !== undefined))
    {
      ParamEvent = window.event;  // Older IE browsers only store event in window.event
    }

    WWHFrame.WWHRelatedTopics.fShowAtEvent(ParamEvent);
  }
}

function  WWHShowALinksPopup(ParamKeywordArray,
                             ParamEvent)
{
  if (WWHFrame !== null)
  {
    if ((ParamEvent === null) &&
        (window.event !== undefined))
    {
      ParamEvent = window.event;  // Older IE browsers only store event in window.event
    }

    WWHFrame.WWHALinks.fShow(ParamKeywordArray, ParamEvent);
  }
}

function  WWHRelatedTopicsDivTag()
{
  var  RelatedTopicsDivTag = "";


  if (WWHFrame !== null)
  {
    RelatedTopicsDivTag = WWHFrame.WWHRelatedTopics.fPopupHTML();
  }

  return RelatedTopicsDivTag;
}

function  WWHPopupDivTag()
{
  var  PopupDivTag = "";


  if (WWHFrame !== null)
  {
    PopupDivTag = WWHFrame.WWHHelp.fPopupHTML();
  }

  return PopupDivTag;
}

function  WWHALinksDivTag()
{
  var  ALinksDivTag = "";


  if (WWHFrame !== null)
  {
    ALinksDivTag = WWHFrame.WWHALinks.fPopupHTML();
  }

  return ALinksDivTag;
}
