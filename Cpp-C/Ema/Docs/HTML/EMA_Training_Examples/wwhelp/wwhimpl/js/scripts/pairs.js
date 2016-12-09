// Copyright (c) 2005-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global Pairs_CreateHash */
/*global Pairs_GetPairs */
/*global Pairs_IsMatch */
/*global Pairs_ResetMatches */
/*global Pairs_TestPair */

function Pairs_Object(ParamArray)
{
  // Store Original Array
  //
  this.mOriginalArray = ParamArray;

  // Hash of word pairs to test, keys are prefixed by '~'
  //
  this.mPairsHash = {};

  // Define Functions for this object
  //
  this.fGetPairs     = Pairs_GetPairs;
  this.fResetMatches = Pairs_ResetMatches;
  this.fTestPair     = Pairs_TestPair;
  this.fIsMatch      = Pairs_IsMatch;
  this.fCreateHash   = Pairs_CreateHash;
}

// Create the Hash of Word Pairs
//
function Pairs_CreateHash()
{
  var index = 0;
  var innerHash;
  var lastWord;
  var currentWord;
  var currentInnerHash;
  var innerHashValue;

  for(index = 0; index < this.mOriginalArray.length; index += 1)
  {
    innerHash = {};
    if (index === 0)
    {
      lastWord = "~" + this.mOriginalArray[index];
    }
    else
    {
      currentWord = "~" + this.mOriginalArray[index];
      currentInnerHash = this.mPairsHash[lastWord];
      if (currentInnerHash !== undefined) 
      {
        innerHashValue = currentInnerHash[currentWord];
        if (innerHashValue === undefined)
        {
          currentInnerHash[currentWord] = 0;
        }
      }
      else
      {
        innerHash[currentWord] = 0;
        this.mPairsHash[lastWord] = innerHash;
      }

      lastWord = currentWord;
    }
  }
}

// Accessor function to return the hash
//
function Pairs_GetPairs()
{
  return this.mPairsHash;
}

// After each doc is tested for the occurrence
// of the pairs in the search phrase, the recorded
// matches in the hash can be reset for the next
// doc to test
//
function Pairs_ResetMatches()
{
  var outerKey;
  var innerKey;
  var innerHash;

  for(outerKey in this.mPairsHash)
  {
    innerHash = this.mPairsHash[outerKey];
    for(innerKey in innerHash)
    {
      innerHash[innerKey] = 0;
    }
  }
}

// The list of word pairs generated during output
// calls this function with each word pair in the doc
// if the word pair is present in the hash created
// from the search phrase, then that match is recorded.
//
function Pairs_TestPair(ParamFirst, ParamSecond)
{
  if (this.mPairsHash["~" + ParamFirst] !== undefined &&
      this.mPairsHash["~" + ParamFirst]["~" + ParamSecond] !== undefined)
  {
    this.mPairsHash["~" + ParamFirst]["~" + ParamSecond] += 1;
  }
}

// IsMatch iterates all keys of mPairsHash testing
// whether the inner hashes have values greater than 0
// If all inner hash values are greater than 0, then
// There is a match on the phrase in the doc.
//
function Pairs_IsMatch()
{
  var result = true;

  var outerKey;
  var innerKey;
  var matchValue;

  for(outerKey in this.mPairsHash)
  {
    for(innerKey in this.mPairsHash[outerKey])
    {
      matchValue = this.mPairsHash[outerKey][innerKey];

      if(matchValue <= 0)
      {
        result = false;
        break;
      }
    }

    if(!result)
    {
      break;
    }
  }

  return result;
}
