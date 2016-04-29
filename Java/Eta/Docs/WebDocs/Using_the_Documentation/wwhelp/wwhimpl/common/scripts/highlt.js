// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global WWHFrame */
/*global WWHHighlightWords_Exec */
/*global WWHHighlightWords_ExpressionEntry_Object */
/*global WWHHighlightWords_SetWordList */
/*global WWHStringUtilities_WordToRegExpWithSpacePattern */

function  WWHHighlightWords_Object()
{
  this.mWordList = null;

  this.fSetWordList = WWHHighlightWords_SetWordList;
  this.fExec        = WWHHighlightWords_Exec;
}

function  WWHHighlightWords_SetWordList(ParamWordList)
{
  this.mWordList = ParamWordList;
}

function  WWHHighlightWords_Exec()
{
  var  MaxIndex;
  var  Index;
  var  WordExpressions;
  var  LongestWordExpression;
  var  MaxExpressionIndex;
  var  ExpressionIndex;
  var  ExpressionHash = null;
  var  ExpressionEntry;
  var  Expression;
  var  VarDocumentFrame;
  var  LongestWordExpressionKey;
  var  NewHighlightedWords = null;
  var  HighlightedWords = null;
  var  TextRange;
  var  LastCharTextRange;
  var  bMatchFound;
  var  bFirstMatch;
  var  NewHighlightedWordsKey;


  if ((WWHFrame.WWHHelp.mSettings.mbHighlightingEnabled) &&
      (this.mWordList !== null))
  {
    // Only works under IE on Windows
    //
    if ((WWHFrame.WWHBrowser.mBrowser === 2) &&  // Shorthand for IE
        (WWHFrame.WWHBrowser.mPlatform === 1))   // Shorthand for Windows
    {
      ExpressionHash   = {};
      HighlightedWords = {};
      bFirstMatch = true;

      // Access search words
      //
      for (MaxIndex = this.mWordList.length, Index = 0 ; Index < MaxIndex ; Index += 1)
      {
        if (this.mWordList[Index].length > 0)
        {
          // Determine longest sub-expression between '*'
          //
          WordExpressions = this.mWordList[Index].split("*");
          LongestWordExpression = "";
          for (MaxExpressionIndex = WordExpressions.length, ExpressionIndex = 0 ; ExpressionIndex < MaxExpressionIndex ; ExpressionIndex += 1)
          {
            if (WordExpressions[ExpressionIndex].length > LongestWordExpression.length)
            {
              LongestWordExpression = WordExpressions[ExpressionIndex];
            }
          }

          // Store search expression keyed by longest sub-expression
          //
          ExpressionEntry = ExpressionHash[LongestWordExpression + "~"];
          if (ExpressionEntry === undefined)
          {
            ExpressionEntry = new WWHHighlightWords_ExpressionEntry_Object();
            ExpressionHash[LongestWordExpression + "~"] = ExpressionEntry;
          }
          Expression = WWHStringUtilities_WordToRegExpWithSpacePattern(this.mWordList[Index]);
          ExpressionEntry.mExpressions[ExpressionEntry.mExpressions.length] = new RegExp(Expression, "i");
        }
      }

      // Ensure document window has focus to avoid "Incompatible markup pointers" error
      // http://xopus.com/devblog/2008/invalid-markup-pointers-revisited
      //
      VarDocumentFrame = eval(WWHFrame.WWHHelp.fGetFrameReference("WWHDocumentFrame"));
      VarDocumentFrame.focus();

      // Search document based on longest sub-expressions
      //
      for (LongestWordExpressionKey in ExpressionHash)
      {
        LongestWordExpression = LongestWordExpressionKey.substring(0, LongestWordExpressionKey.length - 1);
        NewHighlightedWords = {};

        TextRange = VarDocumentFrame.document.body.createTextRange();
        TextRange.collapse();
        while (TextRange.findText(LongestWordExpression, 1))
        {
          TextRange.expand("word");
          ExpressionEntry = ExpressionHash[LongestWordExpression + "~"];

          // Check word against search expression
          //
          bMatchFound = false;
          MaxExpressionIndex = ExpressionEntry.mExpressions.length;
          ExpressionIndex = 0;
          while (( ! bMatchFound) &&
                 (ExpressionIndex < MaxExpressionIndex))
          {
            if (ExpressionEntry.mExpressions[ExpressionIndex].test(TextRange.text))
            {
              // Highlight text if not processed already
              //
              if (HighlightedWords[TextRange.text + "~"] === undefined)
              {
                // Record text highlighted for this expression
                //
                NewHighlightedWords[TextRange.text + "~"] = true;

                // Try to trim off trailing whitespace or .s
                //
                LastCharTextRange = TextRange.duplicate();
                LastCharTextRange.moveStart("character", TextRange.text.length - 1);

                if ((LastCharTextRange.text === " ") ||
                    (LastCharTextRange.text === ",") ||
                    (LastCharTextRange.text === "."))
                {
                  // Prevent infinite loops if search is for "," or "."
                  //
                  if (LastCharTextRange.text !== LongestWordExpression)
                  {
                    TextRange.moveEnd("character", -1);
                  }
                }

                // Highlight words in text range
                //
                TextRange.execCommand("ForeColor", false, WWHFrame.WWHHelp.mSettings.mHighlightingForegroundColor);
                TextRange.execCommand("BackColor", false, WWHFrame.WWHHelp.mSettings.mHighlightingBackgroundColor);

                if (bFirstMatch)
                {
                  TextRange.scrollIntoView();

                  bFirstMatch = false;
                }

                bMatchFound = true;
              }
            }

            ExpressionIndex += 1;
          }

          TextRange.collapse(false);
        }

        // Add highlighted words to hash
        //
        for (NewHighlightedWordsKey in NewHighlightedWords)
        {
          HighlightedWords[NewHighlightedWordsKey] = true;
        }
      }
    }
  }

  // Highlight words only once
  //
  this.mWordList = null;
}

function  WWHHighlightWords_ExpressionEntry_Object()
{
  this.mExpressions = [];
}
