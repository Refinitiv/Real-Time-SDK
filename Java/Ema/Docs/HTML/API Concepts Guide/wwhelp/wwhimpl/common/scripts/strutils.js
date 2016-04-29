// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global WWHStringBuffer_Append */
/*global WWHStringBuffer_GetBuffer */
/*global WWHStringBuffer_Reset */
/*global WWHStringBuffer_Size */
/*global WWHStringUtilities_HexDigit */

function  WWHStringUtilities_GetBaseURL(ParamURL)
{
  var  BaseURL;
  var  Parts;


  // Remove URL parameters
  //
  BaseURL = ParamURL;
  if (BaseURL.indexOf("?") !== -1)
  {
    Parts = BaseURL.split("?");
    BaseURL = Parts[0];
  }
  else if (BaseURL.indexOf("#") !== -1) 
  {
    Parts = BaseURL.split("#");
    BaseURL = Parts[0];
  }

  // Trim down to last referenced directory
  //
  BaseURL = ParamURL.substring(0, ParamURL.lastIndexOf("/"));

  // Attempt to match known WWHelp directories
  //
  Parts = BaseURL.split("/wwhelp/wwhimpl/common/html");
  if (Parts[0] === BaseURL)
  {
    Parts = BaseURL.split("/wwhelp/wwhimpl/js/html");
  }

  // Append trailing slash for this directory
  //
  BaseURL = Parts[0] + "/";

  return BaseURL;
}

function  WWHStringUtilities_SearchReplace(ParamString,
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

function  WWHStringUtilities_FormatMessage(ParamMessage,
                                           ParamReplacement1,
                                           ParamReplacement2,
                                           ParamReplacement3,
                                           ParamReplacement4)
{
  var  VarFormattedMessage;
  var  VarSearchString;
  var  VarReplacementStringIndex;
  var  VarIndex;
  var  VarReplacementString;


  VarFormattedMessage = ParamMessage;
  if (VarFormattedMessage.length > 0)
  {
    VarSearchString = "%s";
    VarReplacementStringIndex = 1;
    VarIndex = VarFormattedMessage.indexOf(VarSearchString, 0);
    while (VarIndex !== -1)
    {
      VarReplacementString = null;
      if (VarReplacementStringIndex <= 4)
      {
        if (VarReplacementStringIndex === 1) { VarReplacementString = ParamReplacement1; }
        if (VarReplacementStringIndex === 2) { VarReplacementString = ParamReplacement2; }
        if (VarReplacementStringIndex === 3) { VarReplacementString = ParamReplacement3; }
        if (VarReplacementStringIndex === 4) { VarReplacementString = ParamReplacement4; }
      }

      if ((VarReplacementString !== undefined) &&
          (VarReplacementString !== null))
      {
        VarFormattedMessage = VarFormattedMessage.substring(0, VarIndex) + VarReplacementString + VarFormattedMessage.substring(VarIndex + VarSearchString.length, VarFormattedMessage.length);

        VarIndex += VarReplacementString.length;
      }
      else
      {
        VarIndex += VarSearchString.length;
      }

      VarReplacementStringIndex += 1;

      VarIndex = VarFormattedMessage.indexOf(VarSearchString, VarIndex);
    }
  }

  return VarFormattedMessage;
}

function  WWHStringUtilities_EscapeHTML(ParamHTML)
{
  var  EscapedHTML = ParamHTML;


  // Escape problematic characters
  // & < > "
  //
  EscapedHTML = WWHStringUtilities_SearchReplace(EscapedHTML, "&", "&amp;");
  EscapedHTML = WWHStringUtilities_SearchReplace(EscapedHTML, "<", "&lt;");
  EscapedHTML = WWHStringUtilities_SearchReplace(EscapedHTML, ">", "&gt;");
  EscapedHTML = WWHStringUtilities_SearchReplace(EscapedHTML, "\"", "&quot;");

  return EscapedHTML;
}

function  WWHStringUtilities_UnescapeHTML(ParamHTML)
{
  var  Text = ParamHTML;
  var  EscapedExpression;
  var  EscapedCharacterMatches;
  var  EscapeSequence;
  var  CharacterCode;
  var  JavaScriptCharacter;


  // Unescape problematic characters
  //
  // & < > "
  //
  Text = WWHStringUtilities_SearchReplace(Text, "&amp;", "&");
  Text = WWHStringUtilities_SearchReplace(Text, "&lt;", "<");
  Text = WWHStringUtilities_SearchReplace(Text, "&gt;", ">");
  Text = WWHStringUtilities_SearchReplace(Text, "&quot;", "\"");

  // If any still exist, replace them with normal character
  //
  if (Text.indexOf("&#") !== -1)
  {
    EscapedExpression = new RegExp("&#([0-9]+);");
    EscapedCharacterMatches = EscapedExpression.exec(Text);
    while (EscapedCharacterMatches !== null)
    {
      EscapeSequence = EscapedCharacterMatches[0];
      CharacterCode = parseInt(EscapedCharacterMatches[1], 10);

      // Turn character code into escaped JavaScript character
      //
      JavaScriptCharacter = String.fromCharCode(CharacterCode);

      // Replace in string
      //
      Text = WWHStringUtilities_SearchReplace(Text, EscapeSequence, JavaScriptCharacter);

      // Find more matches
      //
      EscapedCharacterMatches = EscapedExpression.exec(Text);
    }
  }

  return Text;
}

function  WWHStringUtilities_DecimalToHex(ParamNumber)
{
  var  HexNumber = "";


  HexNumber += WWHStringUtilities_HexDigit(ParamNumber >> 12);
  HexNumber += WWHStringUtilities_HexDigit(ParamNumber >>  8);
  HexNumber += WWHStringUtilities_HexDigit(ParamNumber >>  4);
  HexNumber += WWHStringUtilities_HexDigit(ParamNumber >>  0);

  return HexNumber;
}

function  WWHStringUtilities_HexDigit(ParamDigit)
{
  var  HexDigit;
  var  MaskedDigit = ParamDigit & 0x0F;


  // Translate to hex characters 'a' - 'f' if necessary
  //
  if (MaskedDigit === 10)
  {
    HexDigit = "a";
  }
  else if (MaskedDigit === 11)
  {
    HexDigit = "b";
  }
  else if (MaskedDigit === 12)
  {
    HexDigit = "c";
  }
  else if (MaskedDigit === 13)
  {
    HexDigit = "d";
  }
  else if (MaskedDigit === 14)
  {
    HexDigit = "e";
  }
  else if (MaskedDigit === 15)
  {
    HexDigit = "f";
  }
  else
  {
    HexDigit = MaskedDigit;
  }

  return HexDigit;
}

function  WWHStringUtilities_GetURLFilePathOnly(ParamURL)
{
  var  VarFilePathOnly;
  var  VarIndex;


  VarFilePathOnly = ParamURL;

  // Trim off any parameters
  //
  VarIndex = VarFilePathOnly.indexOf("?");
  if (VarIndex !== -1)
  {
    VarFilePathOnly = VarFilePathOnly.substring(0, VarIndex);
  }

  // Trim off named anchor
  //
  VarIndex = VarFilePathOnly.indexOf("#");
  if (VarIndex !== -1)
  {
    VarFilePathOnly = VarFilePathOnly.substring(0, VarIndex);
  }

  return VarFilePathOnly;
}

function  WWHStringUtilities_EscapeURLForJavaScriptAnchor(ParamURL)
{
  var  EscapedURL = ParamURL;


  // Escape problematic characters
  // \ " ' < >
  //
  EscapedURL = WWHStringUtilities_SearchReplace(EscapedURL, "\\", "\\\\");
  EscapedURL = WWHStringUtilities_SearchReplace(EscapedURL, "\"", "\\u0022");
  EscapedURL = WWHStringUtilities_SearchReplace(EscapedURL, "'", "\\u0027");
  EscapedURL = WWHStringUtilities_SearchReplace(EscapedURL, "<", "\\u003c");
  EscapedURL = WWHStringUtilities_SearchReplace(EscapedURL, ">", "\\u003e");

  return EscapedURL;
}

function  WWHStringUtilities_EscapeForJavaScript(ParamString)
{
  var  EscapedString = ParamString;


  // Escape problematic characters
  // \ " '
  //
  EscapedString = WWHStringUtilities_SearchReplace(EscapedString, "\\", "\\\\");
  EscapedString = WWHStringUtilities_SearchReplace(EscapedString, "\"", "\\u0022");
  EscapedString = WWHStringUtilities_SearchReplace(EscapedString, "'", "\\u0027");
  EscapedString = WWHStringUtilities_SearchReplace(EscapedString, "\n", "\\u000a");
  EscapedString = WWHStringUtilities_SearchReplace(EscapedString, "\r", "\\u000d");

  return EscapedString;
}

function  WWHStringUtilities_EscapeRegExp(ParamWord)
{
  var  WordRegExpPattern = ParamWord;


  // Escape special characters
  // \ ( ) [ ] . ? + ^ $
  //
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, "\\", "\\\\");
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, ".", "\\.");
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, "?", "\\?");
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, "+", "\\+");
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, "|", "\\|");
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, "^", "\\^");
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, "$", "\\$");
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, "(", "\\(");
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, ")", "\\)");
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, "{", "\\{");
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, "}", "\\}");
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, "[", "\\[");
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, "]", "\\]");

  // Windows IE 4.0 is brain dead
  //
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, "/", "[/]");

  // Convert * to .*
  //
  WordRegExpPattern = WWHStringUtilities_SearchReplace(WordRegExpPattern, "*", ".*");

  return WordRegExpPattern;
}

function  WWHStringUtilities_WordToRegExpPattern(ParamWord)
{
  var  WordRegExpPattern;


  // Escape special characters
  // Convert * to .*
  //
  WordRegExpPattern = WWHStringUtilities_EscapeRegExp(ParamWord);

  // Add ^ and $ to force whole string match
  //
  WordRegExpPattern = "^" + WordRegExpPattern + "$";

  return WordRegExpPattern;
}

function  WWHStringUtilities_WordToRegExpWithSpacePattern(ParamWord)
{
  var  WordRegExpPattern;


  // Escape special characters
  // Convert * to .*
  //
  WordRegExpPattern = WWHStringUtilities_EscapeRegExp(ParamWord);

  // Add ^ and $ to force whole string match
  // Allow trailing whitespace
  //
  WordRegExpPattern = "^" + WordRegExpPattern + " *$";

  return WordRegExpPattern;
}

function  WWHStringUtilities_ExtractStyleAttribute(ParamAttribute,
                                                   ParamFontStyle)
{
  var  Attribute = "";
  var  AttributeIndex;
  var  AttributeStart;
  var  AttributeEnd;


  AttributeIndex = ParamFontStyle.indexOf(ParamAttribute, 0);
  if (AttributeIndex !== -1)
  {
    AttributeStart = ParamFontStyle.indexOf(":", AttributeIndex);

    if (AttributeStart !== -1)
    {
      AttributeStart += 1;

      AttributeEnd = ParamFontStyle.indexOf(";", AttributeStart);
      if (AttributeEnd === -1)
      {
        AttributeEnd = ParamFontStyle.length;
      }

      Attribute = ParamFontStyle.substring(AttributeStart + 1, AttributeEnd);
    }
  }

  return Attribute;
}

function  WWHStringBuffer_Object()
{
  this.mStringList        = [];
  this.mStringListEntries = 0;
  this.mSize              = 0;

  this.fSize      = WWHStringBuffer_Size;
  this.fReset     = WWHStringBuffer_Reset;
  this.fAppend    = WWHStringBuffer_Append;
  this.fGetBuffer = WWHStringBuffer_GetBuffer;
}

function  WWHStringBuffer_Size()
{
  return this.mSize;
}

function  WWHStringBuffer_Reset()
{
  this.mStringListEntries = 0;
  this.mSize              = 0;
}

function  WWHStringBuffer_Append(ParamString)
{
  this.mSize += ParamString.length;
  this.mStringList[this.mStringListEntries] = ParamString;
  this.mStringListEntries += 1;
}

function  WWHStringBuffer_GetBuffer()
{
  this.mStringList.length = this.mStringListEntries;

  return this.mStringList.join("");
}

function WWHStringUtilities_ParseWordsAndPhrases(ParamInput)
{
  var WordSplits      = [];
  var Results         = [];
  var StringWithSpace = "x x";
  var CurrentPhrase   = "";
  var CurrentWord     = "";
  var WordIndex       = 0;
  var StartQuotes     = false;

  if(ParamInput.length > 0)
  {
    WordSplits = ParamInput.split(StringWithSpace.substring(1, 2));
    for(WordIndex = 0; WordIndex < WordSplits.length; WordIndex += 1)
    {
      CurrentWord = WordSplits[WordIndex];
      if(CurrentWord.length > 0)
      {
        // If the current word does not start with or end with a double quote
        // and a phrase has not been started, then add it to the result word list
        // and continue
        //
        if(CurrentWord.charAt(0) === '"')
        {
          if(StartQuotes)
          {
            // This entry ends the current phrase and the word following
            // the quote will be added as a separate word, unless there is
            // a second quote at the start that will start a new phrase
            //
            Results[Results.length] = CurrentPhrase.substring(0, CurrentPhrase.length - 1);
            CurrentPhrase = "";

            while ((CurrentWord.length > 0) &&
                     (CurrentWord.charAt(0) === '"'))
            {
              CurrentWord = CurrentWord.substring(1, CurrentWord.length);
            }
            if(CurrentWord.length > 0)
            {
              CurrentPhrase += CurrentWord + " ";
            }
          }
          else
          {
            StartQuotes = true;

            // Strip off the leading quotes and process the word
            //
            while ((CurrentWord.length > 0) &&
                     (CurrentWord.charAt(0) === '"'))
            {
              CurrentWord = CurrentWord.substring(1, CurrentWord.length);
            }

            if(CurrentWord.length > 0)
            {
              // One Word Phrase - Add it as a word and set StartQuotes to false
              //
              if(CurrentWord.charAt(CurrentWord.length - 1) === '"')
              {
                StartQuotes = false;
                // Strip off trailing quotes and add it as a word
                //
                while ((CurrentWord.length > 0) &&
                       (CurrentWord.charAt(CurrentWord.length - 1) === '"'))
                {
                  CurrentWord = CurrentWord.substring(0, CurrentWord.length - 1);
                }

                // Add the Word to the result array
                //
                Results[Results.length] = CurrentWord;
              }
              else
              {
                // The current word starts a phrase
                //
                CurrentPhrase += CurrentWord + " ";
              }
            }
          }
        }
        else if(CurrentWord.charAt(CurrentWord.length - 1) === '"')
        {
          // Strip off trailing quotes regardless
          //
          while ((CurrentWord.length > 0) &&
                 (CurrentWord.charAt(CurrentWord.length - 1) === '"'))
          {
            CurrentWord = CurrentWord.substring(0, CurrentWord.length - 1);
          }

          // Only process the word if the length is greater than 0 after
          // stripping the trailing quotes
          //
          if(CurrentWord.length > 0)
          {
            if(StartQuotes)
            {
              CurrentPhrase += CurrentWord;

              Results[Results.length] = CurrentPhrase;
              StartQuotes = false;
              CurrentPhrase = "";
            }
            else
            {
              // The phrase is not started
              //
              Results[Results.length] = CurrentWord;
            }
          }
        }
        else
        {
          // The word is either a single word or in the middle of a phrase
          //
          if(StartQuotes)
          {
            CurrentPhrase += CurrentWord + " ";
          }
          else
          {
            Results[Results.length] = CurrentWord;
          }
        }
      }
    }
  }

  return Results;
}
