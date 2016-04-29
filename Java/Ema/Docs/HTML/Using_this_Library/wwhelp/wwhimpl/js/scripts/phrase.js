// Copyright (c) 2005-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global Pairs_Object */
/*global Phrase_GetPairs */
/*global Phrase_GetWords */
/*global Phrase_IsMatch */
/*global Phrase_IsValidPhrase */
/*global Phrase_Parse */
/*global Phrase_ResetMatches */
/*global Phrase_TestPhrase */

function Phrase_Object(ParamPhrase)
{
  // Array of search words
  //
  this.mWords = [];

  // Original Search Phrase
  //
  this.mPhrase = ParamPhrase;

  // Pairs object containing the word pairs of the phrase
  //
  this.mPairs = null;

  this.fTestPhrase    = Phrase_TestPhrase;
  this.fIsMatch       = Phrase_IsMatch;
  this.fParse         = Phrase_Parse;
  this.fIsValidPhrase = Phrase_IsValidPhrase;

  // Accessors and other methods for testing
  //
  this.fGetWords      = Phrase_GetWords;
  this.fGetPairsHash  = Phrase_GetPairs;
  this.fResetMatches  = Phrase_ResetMatches;
}

// Tests the word pair passed in as parameter to check
// if it exists in the word pair hash
//
function Phrase_TestPhrase(ParamFirst, ParamSecond)
{
  this.mPairs.fTestPair(ParamFirst, ParamSecond);
}

// Calls the Pairs object's IsMatch function to see
// if all word pairs are present in the search text
//
function Phrase_IsMatch()
{
  return this.mPairs.fIsMatch();
}

// Parses out the words in the phrase adding them to the
// Pairs object if they are valid search words for the current book
//
function Phrase_Parse()
{
  var StringWithSpace = "x x";
  var phraseSplit;
  var index;
  var currentSplit;

  phraseSplit = this.mPhrase.split(StringWithSpace.substring(1, 2));

  for(index = 0; index < phraseSplit.length; index += 1)
  {
    currentSplit = phraseSplit[index];
    if(currentSplit.length > 0)
    {
      this.mWords[this.mWords.length] = currentSplit;
    }
  }

  if(this.mWords.length > 0)
  {
    this.mPairs = new Pairs_Object(this.mWords);
    this.mPairs.fCreateHash();
  }
  else
  {
    this.mPairs = null;
  }
}

// Returns the word array that is the phrase
// minus the skip wors
//
function Phrase_GetWords()
{
  return this.mWords;
}

// Returns the stored hash of pairs from the pair object
//
function Phrase_GetPairs()
{
  return this.mPairs.fGetPairs();
}

// Resets the match count for the pairs object
//
function Phrase_ResetMatches()
{
  this.mPairs.fResetMatches();
}

// Tests to see if any word pairs exist for this phrase
// Returns true if there are any word pairs, meaning there
// is a valid phrase object
//
function Phrase_IsValidPhrase()
{
  return this.mPairs !== null;
}
