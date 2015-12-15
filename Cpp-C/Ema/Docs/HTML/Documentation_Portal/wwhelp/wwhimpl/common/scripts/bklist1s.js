// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint sloppy: true, vars: true, white: true */
/*global WWHFrame */
/*global WWHBookData_Title, WWHBookData_Context, WWHBookData_Files, WWHBookData_ALinks */

// Load book data
//
WWHFrame.WWHHelp.mBooks.fInit_AddBook(WWHBookData_Title(),
                                      WWHBookData_Context(),
                                      WWHBookData_Files,
                                      WWHBookData_ALinks);

// Increment mInitIndex
//
WWHFrame.WWHHelp.mBooks.fInit_IncrementIndex();
