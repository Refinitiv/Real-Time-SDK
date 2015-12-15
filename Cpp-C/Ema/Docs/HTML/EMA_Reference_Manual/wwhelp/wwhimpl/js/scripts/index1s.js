// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global WWHFrame */
/*global WWHBookData_AddIndexEntries */
/*global WWHBookData_MaxIndexLevel */

// Load book index
//
if (WWHBookData_MaxIndexLevel() > WWHFrame.WWHIndex.mMaxLevel)
{
  WWHFrame.WWHIndex.mMaxLevel = WWHBookData_MaxIndexLevel();
}
WWHFrame.WWHIndex.fInitLoadBookIndex(WWHBookData_AddIndexEntries);
