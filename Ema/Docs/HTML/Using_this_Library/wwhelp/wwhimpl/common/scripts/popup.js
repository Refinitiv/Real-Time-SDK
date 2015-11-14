// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint maxerr: 50, indent: 2, vars: true, white: true */
/*global window */
/*global WWHFrame */
/*global WWHPopup_DivTagText */
/*global WWHPopup_EventString */
/*global WWHPopup_Hide */
/*global WWHPopup_Load */
/*global WWHPopup_Position */
/*global WWHPopup_PositionAt */
/*global WWHPopup_Reveal */
/*global WWHPopup_Show */

function  WWHPopup_Object(ParamThisPopupRef,
                          ParamWindowRef,
                          ParamPopupTranslateFunc,
                          ParamPopupFormatFunc,
                          ParamDivID,
                          ParamTextID,
                          ParamTimeout,
                          ParamOffsetX,
                          ParamOffsetY,
                          ParamWidth)
{
  'use strict';

  this.mThisPopupRef = ParamThisPopupRef;
  this.mWindowRef    = ParamWindowRef;
  this.mDivID        = ParamDivID;
  this.mTextID       = ParamTextID;
  this.mTimeout      = (ParamTimeout > 0) ? ParamTimeout : 0;
  this.mOffsetX      = ParamOffsetX;
  this.mOffsetY      = ParamOffsetY;
  this.mWidth        = ParamWidth;


  // Updated when popup triggered
  //
  this.mbVisible     = false;
  this.mPositionX    = 0;
  this.mPositionY    = 0;
  this.mText         = "";
  this.mSetTimeoutID = null;

  this.fTranslate     = ParamPopupTranslateFunc;
  this.fFormat        = ParamPopupFormatFunc;
  this.fEventString   = WWHPopup_EventString;
  this.fDivTagText    = WWHPopup_DivTagText;
  this.fShow          = WWHPopup_Show;
  this.fLoad          = WWHPopup_Load;
  this.fPositionAt    = WWHPopup_PositionAt;
  this.fPosition      = WWHPopup_Position;
  this.fReveal        = WWHPopup_Reveal;
  this.fHide          = WWHPopup_Hide;
}

function  WWHPopup_EventString()
{
  'use strict';

  var  EventString = "null";
  var  Browser = WWHFrame.WWHBrowser.mBrowser;


  // Set event string based on browser type
  //
  if ((Browser === 1) ||  // Shorthand for Netscape
      (Browser === 2) ||  // Shorthand for IE
      (Browser === 4) ||  // Shorthand for Netscape 6.0 (Mozilla)
      (Browser === 5))    // Shorthand for Safari
  {
    EventString = "event";
  }
  else
  {
    EventString = "null";
  }

  return EventString;
}

function  WWHPopup_DivTagText()
{
  'use strict';

  var  DivTagText = "";
  var  Browser = WWHFrame.WWHBrowser.mBrowser;
  var  VisibleAttribute = "visibility: hidden";


  // Update VisibleAttribute based on browser
  //
  if ((Browser === 2) ||  // Shorthand for Internet Explorer
      (Browser === 3) ||  // Shorthand for iCab
      (Browser === 4) ||  // Shorthand for Netscape 6.0 (Mozilla)
      (Browser === 5))    // Shorthand for Safari
  {
    VisibleAttribute += " ; display: none";
  }

  // Open DIV tag
  //
  DivTagText += "<div id=\"" + this.mDivID + "\" style=\"position: absolute ; z-index: -1 ; " + VisibleAttribute + " ; top: 0px ; left: 0px\">\n";

  // Expand out popup in browsers that support innerHTML accessor
  //
  if ((Browser === 2) ||  // Shortcut for IE
      (Browser === 3) ||  // Shortcut for iCab
      (Browser === 4) ||  // Shorthand for Netscape 6.0 (Mozilla)
      (Browser === 5))    // Shorthand for Safari
  {
    DivTagText += this.fFormat(this.mWidth, this.mTextID,
                               "Popup");
  }

  // Close out DIV tag
  //
  DivTagText += "</div>\n";

  return DivTagText;
}

function  WWHPopup_Show(ParamText,
                        ParamEvent)
{
  'use strict';

  var  Browser = WWHFrame.WWHBrowser.mBrowser;
  var  bLoad = false;
  var  PopupDocument = eval(this.mWindowRef + ".document");
  var  TranslatedText;


  // Hide popup
  //
  this.fHide();

  // Position at 0,0
  //
  this.fPositionAt(0, 0);

  // Reset the timeout operation to display the popup
  //
  if (this.mSetTimeoutID !== null)
  {
    window.clearTimeout(this.mSetTimeoutID);
    this.mSetTimeoutID = null;
  }

  // Check to see if there is anything to display
  //
  if ((ParamText !== null) &&
      (ParamEvent !== null))
  {
    if (Browser === 1)  // Shorthand for Netscape 4.x
    {
      this.mPositionX = ParamEvent.layerX;
      this.mPositionY = ParamEvent.layerY;

      this.mText = ParamText;

      bLoad = true;
    }
    else if (Browser === 2)  // Shorthand for IE
    {
      if ((PopupDocument.documentElement !== undefined) &&
          (PopupDocument.documentElement.clientWidth !== undefined) &&
          (PopupDocument.documentElement.clientHeight !== undefined) &&
          ((PopupDocument.documentElement.scrollLeft !== 0) ||
           (PopupDocument.documentElement.scrollTop !== 0)))
      {
        this.mPositionX = PopupDocument.documentElement.scrollLeft + ParamEvent.x;
        this.mPositionY = PopupDocument.documentElement.scrollTop  + ParamEvent.y;
      }
      else
      {
        this.mPositionX = PopupDocument.body.scrollLeft + ParamEvent.x;
        this.mPositionY = PopupDocument.body.scrollTop  + ParamEvent.y;
      }

      // Workaround for IE 4.0 on Windows
      //
      if (WWHFrame.WWHBrowser.mbWindowsIE40)
      {
        this.mPositionX = ParamEvent.x;
        this.mPositionY = ParamEvent.y;
      }

      this.mText = ParamText;

      if (WWHFrame.WWHBrowser.mPlatform === 2)  // Shorthand for Macintosh
      {
        // Setting the position here before it is displayed
        // corrects a bug under IE 5 on the Macintosh
        //
        PopupDocument.all[this.mDivID].style.pixelLeft = 0;
        PopupDocument.all[this.mDivID].style.pixelTop  = 0;
        TranslatedText = this.fTranslate(this.mText);
        PopupDocument.all[this.mTextID].innerHTML = TranslatedText;
        this.fPosition();
      }

      bLoad = true;
    }
    else if ((Browser === 4) ||  // Shorthand for Netscape 6.0 (Mozilla)
             (Browser === 5))    // Shorthand for Safari
    {
      this.mPositionX = ParamEvent.layerX;
      this.mPositionY = ParamEvent.layerY;

      this.mText = ParamText;

      bLoad = true;
    }

    // Load popup
    //
    if (bLoad === true)
    {
      this.fLoad();
    }
  }
}

function  WWHPopup_Load()
{
  'use strict';

  var  PopupDocument = eval(this.mWindowRef + ".document");
  var  Browser = WWHFrame.WWHBrowser.mBrowser;
  var  FormattedText;
  var  TranslatedText;
  var  VarPopup;


  if (WWHFrame.WWHHandler.fIsReady())
  {
    if (Browser === 1)  // Shorthand for Netscape 4.x
    {
      // Format popup contents for browser
      //
      FormattedText = this.fFormat(this.mWidth, this.mTextID,
                                   this.fTranslate(this.mText));

      // Set popup contents
      //
      PopupDocument.layers[this.mDivID].document.open();
      PopupDocument.layers[this.mDivID].document.write(FormattedText);
      PopupDocument.layers[this.mDivID].document.close();
    }
    else if ((Browser === 2) ||  // Shorthand for IE
             (Browser === 3))    // Shorthand for iCab
    {
      // Format popup contents for browser
      // Set popup contents
      //
      TranslatedText = this.fTranslate(this.mText);
      PopupDocument.all[this.mTextID].innerHTML = TranslatedText;

      // Block display mode
      //
      PopupDocument.all[this.mDivID].style.display = "block";
    }
    else if ((Browser === 4) ||  // Shorthand for Netscape 6.0 (Mozilla)
             (Browser === 5))    // Shorthand for Safari
    {
      // Format popup contents for browser
      // Set popup contents
      //
      TranslatedText = this.fTranslate(this.mText);
      PopupDocument.getElementById(this.mTextID).innerHTML = TranslatedText;

      // Block display mode
      //
      PopupDocument.getElementById(this.mDivID).style.display = "block";
    }

    // Reveal
    //
    VarPopup = eval(this.mThisPopupRef);
    this.mSetTimeoutID = window.setTimeout(function () {
      VarPopup.fReveal();
    }, this.mTimeout);
  }
}

function  WWHPopup_PositionAt(ParamX,
                              ParamY)
{
  'use strict';

  var  PopupDocument = eval(this.mWindowRef + ".document");
  var  Browser = WWHFrame.WWHBrowser.mBrowser;


  if (Browser === 1)  // Shorthand for Netscape 4.x
  {
    // Set popup position
    //
    PopupDocument.layers[this.mDivID].left = ParamX;
    PopupDocument.layers[this.mDivID].top  = ParamY;
  }
  else if (Browser === 2)  // Shorthand for IE
  {
    // Set popup position
    //
    PopupDocument.all[this.mDivID].style.pixelLeft = ParamX;
    PopupDocument.all[this.mDivID].style.pixelTop  = ParamY;
  }
  else if ((Browser === 4) ||  // Shorthand for Netscape 6.0 (Mozilla)
           (Browser === 5))    // Shorthand for Safari
  {
    // Set popup position
    //
    PopupDocument.getElementById(this.mDivID).style.left = ParamX + "px";
    PopupDocument.getElementById(this.mDivID).style.top  = ParamY + "px";
  }
}

function  WWHPopup_Position()
{
  'use strict';

  var  PopupWindow   = eval(this.mWindowRef);
  var  PopupDocument = eval(this.mWindowRef + ".document");
  var  Browser = WWHFrame.WWHBrowser.mBrowser;
  var  Margin = 8;
  var  NewPositionX;
  var  NewPositionY;
  var  VisibleOffsetX;
  var  VisibleOffsetY;
  var  ScrollTop;
  var  PopupWidth;
  var  PopupHeight;
  var  DeltaY;


  // Calculate new position for popup
  //
  NewPositionX = this.mPositionX + this.mOffsetX;
  NewPositionY = this.mPositionY + this.mOffsetY;

  if (Browser === 1)  // Shorthand for Netscape 4.x
  {
    // Attempt to determine DIV tag dimensions
    //
    PopupWidth = this.mWidth;
    if (PopupDocument.layers[this.mDivID].clip.width > PopupWidth)
    {
      PopupWidth = PopupDocument.layers[this.mDivID].clip.width;
    }
    PopupHeight = 60;  // Guess a value
    if (PopupDocument.layers[this.mDivID].clip.height > PopupHeight)
    {
      PopupHeight = PopupDocument.layers[this.mDivID].clip.height;
    }

    // Calculate maximum values for X and Y such that the
    // popup will remain visible
    //
    VisibleOffsetX = PopupWindow.innerWidth  - this.mOffsetX - PopupWidth - Margin;
    if (VisibleOffsetX < 0)
    {
      VisibleOffsetX = 0;
    }
    VisibleOffsetY = PopupWindow.innerHeight - this.mOffsetY - PopupHeight - Margin;
    if (VisibleOffsetY < 0)
    {
      VisibleOffsetY = 0;
    }

    // Confirm popup will be visible and adjust if necessary
    //
    if (NewPositionX > (PopupWindow.pageXOffset + VisibleOffsetX))
    {
      NewPositionX = PopupWindow.pageXOffset + VisibleOffsetX;
    }
    ScrollTop = PopupWindow.pageYOffset;
    if (NewPositionY > (PopupWindow.pageYOffset + VisibleOffsetY))
    {
      NewPositionY = PopupWindow.pageYOffset + VisibleOffsetY;
    }

    // Relocate popup if it will overlay the current mouse position
    //
    if ((this.mPositionY >= NewPositionY) &&
        (this.mPositionY <= (NewPositionY + PopupHeight)))
    {
      DeltaY = (NewPositionY + PopupHeight) - this.mPositionY;
      if (NewPositionY - (DeltaY + Margin) > ScrollTop)
      {
        NewPositionY -= DeltaY + Margin;
      }
    }

    // Set popup position
    //
    this.fPositionAt(NewPositionX, NewPositionY);
  }
  else if (Browser === 2)  // Shorthand for IE
  {
    // Attempt to determine DIV tag dimensions
    //
    PopupWidth = this.mWidth;
    if (PopupDocument.all[this.mDivID].offsetWidth > PopupWidth)
    {
      PopupWidth = PopupDocument.all[this.mDivID].offsetWidth;
    }
    PopupHeight = 60;  // Guess a value
    if (PopupDocument.all[this.mDivID].offsetHeight > PopupHeight)
    {
      PopupHeight = PopupDocument.all[this.mDivID].offsetHeight;
    }

    // Calculate maximum values for X and Y such that the
    // popup will remain visible
    //
    if ((PopupDocument.documentElement !== undefined) &&
        (PopupDocument.documentElement.clientWidth !== undefined) &&
        (PopupDocument.documentElement.clientHeight !== undefined) &&
        ((PopupDocument.documentElement.clientWidth !== 0) ||
         (PopupDocument.documentElement.clientHeight !== 0)))
    {
      VisibleOffsetX = PopupDocument.documentElement.clientWidth  - this.mOffsetX - PopupWidth - Margin;
      VisibleOffsetY = PopupDocument.documentElement.clientHeight - this.mOffsetY - PopupHeight - Margin;
    }
    else
    {
      VisibleOffsetX = PopupDocument.body.clientWidth  - this.mOffsetX - PopupWidth - Margin;
      VisibleOffsetY = PopupDocument.body.clientHeight - this.mOffsetY - PopupHeight - Margin;
    }
    if (VisibleOffsetX < 0)
    {
      VisibleOffsetX = 0;
    }
    if (VisibleOffsetY < 0)
    {
      VisibleOffsetY = 0;
    }

    // Confirm popup will be visible and adjust if necessary
    //
    if ((PopupDocument.documentElement !== undefined) &&
        (PopupDocument.documentElement.clientWidth !== undefined) &&
        (PopupDocument.documentElement.clientHeight !== undefined) &&
        ((PopupDocument.documentElement.scrollLeft !== 0) ||
         (PopupDocument.documentElement.scrollTop !== 0)))
    {
      if (NewPositionX > (PopupDocument.documentElement.scrollLeft + VisibleOffsetX))
      {
        NewPositionX = PopupDocument.documentElement.scrollLeft + VisibleOffsetX;
      }
      ScrollTop = PopupDocument.documentElement.scrollTop;
      if (NewPositionY > (PopupDocument.documentElement.scrollTop + VisibleOffsetY))
      {
        NewPositionY = PopupDocument.documentElement.scrollTop + VisibleOffsetY;
      }
    }
    else
    {
      if (NewPositionX > (PopupDocument.body.scrollLeft + VisibleOffsetX))
      {
        NewPositionX = PopupDocument.body.scrollLeft + VisibleOffsetX;
      }
      ScrollTop = PopupDocument.body.scrollTop;
      if (NewPositionY > (PopupDocument.body.scrollTop + VisibleOffsetY))
      {
        NewPositionY = PopupDocument.body.scrollTop + VisibleOffsetY;
      }
    }

    // Relocate popup if it will overlay the current mouse position
    //
    if ((this.mPositionY >= NewPositionY) &&
        (this.mPositionY <= (NewPositionY + PopupHeight)))
    {
      DeltaY = (NewPositionY + PopupHeight) - this.mPositionY;
      if (NewPositionY - (DeltaY + Margin) > ScrollTop)
      {
        NewPositionY -= DeltaY + Margin;
      }
    }

    // Set popup position
    //
    this.fPositionAt(NewPositionX, NewPositionY);
  }
  else if ((Browser === 4) ||  // Shorthand for Netscape 6.0 (Mozilla)
           (Browser === 5))    // Shorthand for Safari
  {
    // Attempt to determine DIV tag dimensions
    //
    PopupWidth = this.mWidth;
    if (PopupDocument.getElementById(this.mDivID).offsetWidth > PopupWidth)
    {
      PopupWidth = PopupDocument.getElementById(this.mDivID).offsetWidth;
    }
    PopupHeight = 60;  // Guess a value
    if (PopupDocument.getElementById(this.mDivID).offsetHeight > PopupHeight)
    {
      PopupHeight = PopupDocument.getElementById(this.mDivID).offsetHeight;
    }

    // Calculate maximum values for X and Y such that the
    // popup will remain visible
    // Throw in a bit extra for vertical scroll bars when determinine the horizontal position
    //
    VisibleOffsetX = PopupWindow.innerWidth  - this.mOffsetX - PopupWidth - 16 - Margin;
    if (VisibleOffsetX < 0)
    {
      VisibleOffsetX = 0;
    }
    VisibleOffsetY = PopupWindow.innerHeight - this.mOffsetY - PopupHeight - Margin;
    if (VisibleOffsetY < 0)
    {
      VisibleOffsetY = 0;
    }

    // Confirm popup will be visible and adjust if necessary
    //
    if (NewPositionX > (PopupWindow.scrollX + VisibleOffsetX))
    {
      NewPositionX = PopupWindow.scrollX + VisibleOffsetX;
    }
    ScrollTop = PopupWindow.scrollY;
    if (NewPositionY > (PopupWindow.scrollY + VisibleOffsetY))
    {
      NewPositionY = PopupWindow.scrollY + VisibleOffsetY;
    }

    // Relocate popup if it will overlay the current mouse position
    //
    if ((this.mPositionY >= NewPositionY) &&
        (this.mPositionY <= (NewPositionY + PopupHeight)))
    {
      DeltaY = (NewPositionY + PopupHeight) - this.mPositionY;
      if (NewPositionY - (DeltaY + Margin) > ScrollTop)
      {
        NewPositionY -= DeltaY + Margin;
      }
    }

    // Set popup position
    //
    this.fPositionAt(NewPositionX, NewPositionY);
  }
}

function  WWHPopup_Reveal()
{
  'use strict';

  var  PopupDocument = eval(this.mWindowRef + ".document");
  var  Browser = WWHFrame.WWHBrowser.mBrowser;


  if ((WWHFrame.WWHHandler.fIsReady()) &&
      (this.mSetTimeoutID !== null))
  {
    if (Browser === 1)  // Shorthand for Netscape 4.x
    {
      // Position the popup
      //
      this.fPosition();

      // Show the popup
      //
      PopupDocument.layers[this.mDivID].zIndex = 1;
      PopupDocument.layers[this.mDivID].visibility = "visible";
      this.mbVisible = true;
    }
    else if ((Browser === 2) ||  // Shorthand for IE
             (Browser === 3))    // Shorthand for iCab
    {
      // Position the popup
      //
      this.fPosition();

      // Show the popup
      //
      PopupDocument.all[this.mDivID].style.zIndex = 1;
      PopupDocument.all[this.mDivID].style.visibility = "visible";
      this.mbVisible = true;
    }
    else if ((Browser === 4) ||  // Shorthand for Netscape 6.0 (Mozilla)
             (Browser === 5))    // Shorthand for Safari
    {
      // Initial popup positioning before object size can be determined
      //
      this.fPosition();

      // Show the popup
      //
      PopupDocument.getElementById(this.mDivID).style.zIndex = 1;
      PopupDocument.getElementById(this.mDivID).style.visibility = "visible";
      this.mbVisible = true;

      // Position the popup
      // Offset calculations may be off so we might need to reposition the popup
      //
      this.fPosition();
    }
  }

  // Clear the setTimeout ID tracking field
  // to indicate that we're done.
  //
  this.mSetTimeoutID = null;
}

function  WWHPopup_Hide()
{
  'use strict';

  var  Browser = WWHFrame.WWHBrowser.mBrowser;
  var  PopupDocument;


  // Cancel the setTimeout value that would have
  // displayed the popup
  //
  if (this.mSetTimeoutID !== null)
  {
    window.clearTimeout(this.mSetTimeoutID);
    this.mSetTimeoutID = null;
  }

  // Shutdown the popup
  //
  if (this.mbVisible === true)
  {
    PopupDocument = eval(this.mWindowRef + ".document");

    if (Browser === 1)  // Shorthand for Netscape 4.x
    {
      PopupDocument.layers[this.mDivID].zIndex = -1;
      PopupDocument.layers[this.mDivID].visibility = "hidden";
    }
    else if ((Browser === 2) ||  // Shorthand for IE
             (Browser === 3))    // Shorthand for iCab
    {
      PopupDocument.all[this.mDivID].style.zIndex = -1;
      PopupDocument.all[this.mDivID].style.visibility = "hidden";
      PopupDocument.all[this.mDivID].style.display    = "none";
    }
    else if ((Browser === 4) ||  // Shorthand for Netscape 6.0 (Mozilla)
             (Browser === 5))    // Shorthand for Safari
    {
      PopupDocument.getElementById(this.mDivID).style.zIndex = -1;
      PopupDocument.getElementById(this.mDivID).style.visibility = "hidden";
      PopupDocument.getElementById(this.mDivID).style.display    = "none";
    }
  }

  this.mbVisible = false;
}
