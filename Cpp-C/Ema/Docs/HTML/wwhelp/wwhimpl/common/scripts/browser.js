// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global window, escape, unescape */
/*global WWHFrame */
/*global WWHBrowser_CookiesEnabled */
/*global WWHBrowser_DeleteCookie */
/*global WWHBrowser_Focus */
/*global WWHBrowser_GetCookie */
/*global WWHBrowser_Initialize */
/*global WWHBrowser_NormalizeURL */
/*global WWHBrowser_ReloadLocation */
/*global WWHBrowser_ReplaceLocation */
/*global WWHBrowser_SetCookie */
/*global WWHBrowser_SetCookiePath */
/*global WWHBrowser_SetLocation */
/*global WWHBrowser_ValidateFrameReference */
/*global WWHStringUtilities_GetBaseURL */

function  WWHBrowserUtilities_SearchReplace(ParamString,
                                            ParamSearchString,
                                            ParamReplaceString)
{
  var  ResultString;
  var  Index;


  ResultString = ParamString;

  if ((ParamSearchString.length > 0) &&
      (ResultString.length > 0))
  {
    Index = ResultString.indexOf(ParamSearchString, 0);
    while (Index !== -1)
    {
      ResultString = ResultString.substring(0, Index) + ParamReplaceString + ResultString.substring(Index + ParamSearchString.length, ResultString.length);
      Index += ParamReplaceString.length;

      Index = ResultString.indexOf(ParamSearchString, Index);
    }
  }

  return ResultString;
}

function  WWHBrowserUtilities_EscapeURLForJavaScriptAnchor(ParamURL)
{
  var  EscapedURL = ParamURL;


  // Escape problematic characters
  // \ " ' < >
  //
  EscapedURL = WWHBrowserUtilities_SearchReplace(EscapedURL, "\\", "\\\\");
  EscapedURL = WWHBrowserUtilities_SearchReplace(EscapedURL, "\"", "\\u0022");
  EscapedURL = WWHBrowserUtilities_SearchReplace(EscapedURL, "'", "\\u0027");
  EscapedURL = WWHBrowserUtilities_SearchReplace(EscapedURL, "<", "\\u003c");
  EscapedURL = WWHBrowserUtilities_SearchReplace(EscapedURL, ">", "\\u003e");

  return EscapedURL;
}

function  WWHBrowser_Object()
{
  this.mLocale                 = "en";
  this.mPlatform               = 0;      // Shorthand for Unknown
  this.mBrowser                = 0;      // Shorthand for Unknown
  this.mCookiePath             = "/";
  this.mbCookiesEnabled        = null;
  this.mbSupportsFocus         = false;
  this.mbSupportsPopups        = true;
  this.mbSupportsIFrames       = false;
  this.mbSupportsFrameRenaming = true;
  this.mbWindowIE40            = false;  // Needed for special case handling
  this.mbMacIE45               = false;  // Needed for special case handling
  this.mbMacIE50               = false;  // Needed for special case handling
  this.mbWindowsIE60           = false;  // Needed for special case handling
  this.mbUnsupported           = false;

  this.fInitialize      = WWHBrowser_Initialize;
  this.fNormalizeURL    = WWHBrowser_NormalizeURL;
  this.fValidateFrameReference = WWHBrowser_ValidateFrameReference;
  this.fSetLocation     = WWHBrowser_SetLocation;
  this.fReplaceLocation = WWHBrowser_ReplaceLocation;
  this.fReloadLocation  = WWHBrowser_ReloadLocation;
  this.fSetCookiePath   = WWHBrowser_SetCookiePath;
  this.fCookiesEnabled  = WWHBrowser_CookiesEnabled;
  this.fSetCookie       = WWHBrowser_SetCookie;
  this.fGetCookie       = WWHBrowser_GetCookie;
  this.fDeleteCookie    = WWHBrowser_DeleteCookie;
  this.fFocus           = WWHBrowser_Focus;

  // Initialize object
  //
  this.fInitialize();
}

function  WWHBrowser_Initialize()
{
  var  Agent;
  var  MajorVersion;
  var  VersionString;
  var  MSIEVersionString;
  var  Version;


  // Reset locale to correct language value
  //
  if ((window.navigator.language !== undefined) &&
      (window.navigator.language !== null))
  {
    this.mLocale = window.navigator.language;
  }
  else if ((window.navigator.userLanguage !== undefined) &&
           (window.navigator.userLanguage !== null))
  {
    this.mLocale = window.navigator.userLanguage;
  }

  // Convert everything to lowercase
  //
  this.mLocale = this.mLocale.toLowerCase();

  // Replace '-'s with '_'s
  //
  this.mLocale = WWHBrowserUtilities_SearchReplace(this.mLocale, "-", "_");

  // Get browser info
  //
  Agent = window.navigator.userAgent.toLowerCase();

  // Determine platform
  //
  if ((Agent.indexOf("win") !== -1) ||
      (Agent.indexOf("16bit") !== -1))
  {
    this.mPlatform = 1;  // Shorthand for Windows
  }
  else if (Agent.indexOf("mac") !== -1)
  {
    this.mPlatform = 2;  // Shorthand for Macintosh
  }

  // Determine browser
  //
  if ((Agent.indexOf("mozilla") !== -1) &&
      (Agent.indexOf("spoofer") === -1) &&
      (Agent.indexOf("compatible") === -1))
  {
    MajorVersion = parseInt(window.navigator.appVersion, 10);
    if (MajorVersion >= 5)
    {
      this.mBrowser = 4;  // Shorthand for Netscape 6.0
      this.mbSupportsIFrames = true;
      this.mbSupportsFocus = true;

      // Netscape 6.0 is unsupported
      //
      if (window.navigator.userAgent.indexOf("m18") !== -1)
      {
        this.mbUnsupported = true;
      }
    }
    else if (MajorVersion >= 4)
    {
      this.mBrowser = 1;  // Shorthand for Netscape

      this.mbSupportsFrameRenaming = false;
    }
  }
  else if (Agent.indexOf("msie") !== -1)
  {
    MajorVersion = parseInt(window.navigator.appVersion, 10);
    if (MajorVersion >= 4)
    {
      this.mBrowser = 2;  // Shorthand for IE
      this.mbSupportsFocus = true;

      // Additional info needed for popups
      //
      VersionString = window.navigator.appVersion.toLowerCase();
      MSIEVersionString = VersionString.substring(VersionString.indexOf("msie") + 4);
      Version = parseFloat(MSIEVersionString);
      if ((Version >= 4.0) &&
          (Version < 4.1))
      {
        if (this.mPlatform === 1)  // Shorthand for Windows
        {
          this.mbWindowsIE40 = true;
        }
      }
      else if ((Version >= 4.5) &&
               (Version < 4.6))
      {
        if (this.mPlatform === 2)  // Shorthand for Macintosh
        {
          this.mbMacIE45 = true;
        }
      }
      else if ((Version >= 5.0) &&
               (Version < 5.1))
      {
        if (this.mPlatform === 2)  // Shorthand for Macintosh
        {
          this.mbMacIE50 = true;
        }
      }
      else if ((Version >= 5.5) &&
               (Version < 6.0))
      {
        this.mbSupportsIFrames = true;
      }
      else if (Version >= 6.0)
      {
        this.mbSupportsIFrames = true;
        this.mbWindowsIE60 = true;
      }
    }
  }
  else if (Agent.indexOf("icab") !== -1)
  {
    this.mBrowser = 3;  // Shorthand for iCab

    this.mbSupportsPopups = false;
  }

  // Safari may spoof as just about anything
  //
  if (Agent.indexOf("applewebkit") !== -1)
  {
    this.mBrowser = 5;  // Shorthand for Safari

    this.mbSupportsPopups = true;
    this.mbSupportsIFrames = true;
    this.mbSupportsFocus = false;
  }
}

function  WWHBrowser_NormalizeURL(ParamURL)
{
  var  URL = ParamURL;
  var  Parts;
  var  MaxIndex;
  var  Index;
  var  DrivePattern;
  var  DrivePatternMatch;


  // Standardize protocol case
  //
  if (URL.indexOf(":") !== -1)
  {
    Parts = URL.split(":");

    URL = Parts[0].toLowerCase();
    for (MaxIndex = Parts.length, Index = 1 ; Index < MaxIndex ; Index += 1)
    {
      URL += ":" + Parts[Index];
    }
  }

  // Handle drive letters under Windows
  //
  if (this.mPlatform === 1)  // Shorthand for Windows
  {
    DrivePattern = new RegExp("^file:[/]+([a-zA-Z])[:\|][/](.*)$", "i");
    DrivePatternMatch = DrivePattern.exec(URL);
    if (DrivePatternMatch !== null)
    {
      URL = "file:///" + DrivePatternMatch[1] + ":/" + DrivePatternMatch[2];
    }
  }

  // Deal with Safari stupidity
  //
  URL = WWHBrowserUtilities_SearchReplace(URL, " ", "%20");

  // Deal with Safari perfect encoding issue
  //
  URL = WWHBrowserUtilities_SearchReplace(URL, "%23", "#");

  return URL;
}

function  WWHBrowser_ValidateFrameReference(ParamFrameReference, ParamAction)
{
  var  VarFrame;

  // Ensure required frames can be resolved (required for Mac Help)
  //
  if (ParamFrameReference !== undefined)
  {
    try
    {
      // Frame not yet created?
      //
      VarFrame = eval(ParamFrameReference);
      if (VarFrame !== undefined)
      {
        // File not yet loaded?
        //
        if (VarFrame.location.href === "about:blank")
        {
          // Try to set the appropriate file path
          //
          VarFrame.location.replace(WWHStringUtilities_GetBaseURL(window.location.href) + "wwhelp/wwhimpl/common/html/blank.htm");

          // Try again in a bit
          //
          window.setTimeout(function () {
            WWHFrame.WWHBrowser.fValidateFrameReference(ParamFrameReference, ParamAction);
          }, 1000);
        }
        else
        {
          // Perform action!
          //
          ParamAction();
        }
      }
      else
      {
        // Try again in a bit
        //
        window.setTimeout(function () {
          WWHFrame.WWHBrowser.fValidateFrameReference(ParamFrameReference, ParamAction);
        }, 10);
      }
    }
    catch (err)
    {
      // Try again in a bit
      //
      window.setTimeout(function () {
        WWHFrame.WWHBrowser.fValidateFrameReference(ParamFrameReference, ParamAction);
      }, 10);
    }
  }
}

function  WWHBrowser_SetLocation(ParamFrameReference,
                                 ParamURL,
                                 ParamTimeout)
{
  this.fValidateFrameReference(ParamFrameReference,
    function () {
      var VarFrame, VarTimeout;

      VarFrame = eval(ParamFrameReference);
      VarTimeout = 1;
      if (ParamTimeout !== undefined) {
        VarTimeout = ParamTimeout;
      }
      window.setTimeout(function () {
        VarFrame.location.assign(ParamURL);
      }, VarTimeout);
    });
}

function  WWHBrowser_ReplaceLocation(ParamFrameReference,
                                     ParamURL,
                                     ParamTimeout)
{
  this.fValidateFrameReference(ParamFrameReference,
    function () {
      var VarFrame, VarTimeout;

      VarFrame = eval(ParamFrameReference);
      VarTimeout = 1;
      if (ParamTimeout !== undefined) {
        VarTimeout = ParamTimeout;
      }
      window.setTimeout(function () {
        VarFrame.location.replace(ParamURL);
      }, VarTimeout);
    });
}

function  WWHBrowser_ReloadLocation(ParamFrameReference,
                                    ParamTimeout)
{
  var  VarFrame;


  VarFrame = eval(ParamFrameReference);
  this.fReplaceLocation(ParamFrameReference, VarFrame.location.href, ParamTimeout);
}

function  WWHBrowser_SetCookiePath(ParamURL)
{
  var  Pathname;
  var  WorkingURL;
  var  Parts;
  var  Index;
  var  Protocol = "";


  // Initialize return value
  //
  Pathname = "/";

  // Remove URL parameters
  //
  WorkingURL = ParamURL;
  if (WorkingURL.indexOf("?") !== -1)
  {
    Parts = WorkingURL.split("?");
    WorkingURL = Parts[0];
  }
  else if (WorkingURL.indexOf("#") !== -1)
  {
    Parts = WorkingURL.split("#");
    WorkingURL = Parts[0];
  }

  // Remove last entry if path does not end with /
  //
  Index = WorkingURL.lastIndexOf("/");
  if ((Index + 1) < WorkingURL.length)
  {
    WorkingURL = WorkingURL.substring(0, Index);
  }

  // Remove protocol
  //
  Index = -1;
  if (WorkingURL.indexOf("http:/") === 0)
  {
    Index = WorkingURL.indexOf("/", 6);
    Protocol = "http";
  }
  else if (WorkingURL.indexOf("ftp:/") === 0)
  {
    Index = WorkingURL.indexOf("/", 5);
    Protocol = "ftp";
  }
  else if (WorkingURL.indexOf("file:///") === 0)
  {
    Index = 7;
    Protocol = "file";
  }

  // Set base URL pathname
  //
  if (Index !== -1)
  {
    Pathname = WorkingURL.substring(Index, WorkingURL.length);

    // Clean up pathname
    //
    if (Protocol === "file")
    {
      if (this.mPlatform === 1)  // Shorthand for Windows
      {
        if (this.mBrowser === 2)  // Shorthand for IE
        {
          // file URLs must have slashes replaced with backslashes, except the first one
          //
          if (Pathname.length > 1)
          {
            Pathname = unescape(Pathname);
            Pathname = WWHBrowserUtilities_SearchReplace(Pathname, "/", "\\");
            if (Pathname.indexOf("\\") === 0)
            {
              Pathname = "/" + Pathname.substring(1, Pathname.length);
            }
          }
        }
      }
    }
    else
    {
      // Trim server info
      //
      Index = Pathname.indexOf("/", Index);
      if (Index !== -1)
      {
        Pathname = Pathname.substring(Index, Pathname.length);
      }
      else
      {
        Pathname = "/";
      }
    }
  }

  // Set cookie path
  //
  this.mCookiePath = Pathname;
}

function  WWHBrowser_CookiesEnabled()
{
  // Cache result
  //
  if (this.mbCookiesEnabled === null)
  {
    // Default to disabled
    //
    this.mbCookiesEnabled = false;

    // Try setting a cookie
    //
    this.fSetCookie("WWHBrowser_CookiesEnabled", "True");

    // Retrieve the cookie
    //
    if (this.fGetCookie("WWHBrowser_CookiesEnabled") !== null)
    {
      // Delete the test cookie
      //
      this.fDeleteCookie("WWHBrowser_CookiesEnabled");

      // Success!
      //
      this.mbCookiesEnabled = true;
    }
  }

  return this.mbCookiesEnabled;
}

function  WWHBrowser_SetCookie(ParamName,
                               ParamValue,
                               ParamExpiration)
{
  var  VarFormattedCookie;
  var  VarExpirationDate;


  // Format the cookie
  //
  VarFormattedCookie = escape(ParamName) + "=" + escape(ParamValue);

  // Add path
  //
  VarFormattedCookie += "; path=" + this.mCookiePath;

  // Add expiration day, if specified
  //
  if ((ParamExpiration !== undefined) &&
      (ParamExpiration !== null) &&
      (ParamExpiration !== 0))
  {
    VarExpirationDate = new Date();
    VarExpirationDate.setTime(VarExpirationDate.getTime() + (ParamExpiration * 1000 * 60 * 60 * 24));
    VarFormattedCookie += "; expires=" + VarExpirationDate.toGMTString();
  }

  // Set the cookie for the specified document
  //
  window.document.cookie = VarFormattedCookie;
}

function  WWHBrowser_GetCookie(ParamName)
{
  var  VarValue;
  var  VarCookies;
  var  VarKey;
  var  VarStartIndex;
  var  VarEndIndex;


  // Initialize return value
  //
  VarValue = null;

  // Get document cookies
  //
  VarCookies = window.document.cookie;

  // Parse out requested cookie
  //

  // Try first position
  //
  VarKey = escape(ParamName) + "=";
  VarStartIndex = VarCookies.indexOf(VarKey);
  if (VarStartIndex !== 0)
  {
    // Try any other position
    //
    VarKey = "; " + escape(ParamName) + "=";
    VarStartIndex = VarCookies.indexOf(VarKey);
  }

  // Match found?
  //
  if (VarStartIndex !== -1)
  {
    // Advance past cookie key
    //
    VarStartIndex += VarKey.length;

    // Find end
    //
    VarEndIndex = VarCookies.indexOf(";", VarStartIndex);
    if (VarEndIndex === -1)
    {
      VarEndIndex = VarCookies.length;
    }
    VarValue = unescape(VarCookies.substring(VarStartIndex, VarEndIndex));
  }

  return VarValue;
}

function  WWHBrowser_DeleteCookie(ParamName)
{
  // Set cookie to expire yesterday
  //
  this.fSetCookie(ParamName, "", -1);
}

function  WWHBrowser_Focus(ParamFrameReference,
                           ParamAnchorName)
{
  var  VarFrame;
  var  VarAnchor;
  var  VarMaxIndex;
  var  VarIndex;


  if (this.mbSupportsFocus)
  {
    if (ParamFrameReference.length > 0)
    {
      // Access frame
      //
      VarFrame = eval(ParamFrameReference);

      // Focus frame
      //
      VarFrame.focus();

      // Focusing anchor?
      //
      if ((ParamAnchorName !== undefined) &&
          (ParamAnchorName !== null) &&
          (ParamAnchorName.length > 0))
      {
        // Focus anchor
        //
        VarAnchor = VarFrame.document.anchors[ParamAnchorName];
        if ((VarAnchor !== undefined) &&
            (VarAnchor !== null))
        {
          VarAnchor.focus();
        }
        else
        {
          for (VarMaxIndex = VarFrame.document.anchors.length, VarIndex = 0 ; VarIndex < VarMaxIndex ; VarIndex += 1)
          {
            if (VarFrame.document.anchors[VarIndex].name === ParamAnchorName)
            {
              VarFrame.document.anchors[VarIndex].focus();

              // Exit loop
              //
              VarIndex = VarMaxIndex;
            }
          }
        }
      }
    }
  }
}
