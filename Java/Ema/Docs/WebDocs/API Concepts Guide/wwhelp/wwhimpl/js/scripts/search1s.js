// Copyright (c) 2000-2014 Quadralay Corporation.  All rights reserved.
//
/*jslint newcap: true, sloppy: true, vars: true, white: true */
/*global WWHFrame */
/*global WWHBookData_SearchFileCount */
/*global WWHBookData_SearchMinimumWordLength */
/*global WWHBookData_SearchSkipWords */

// Load book search info
//
WWHFrame.WWHSearch.fInitLoadBookSearchInfo(WWHBookData_SearchFileCount(),
                                           WWHBookData_SearchMinimumWordLength(),
                                           WWHBookData_SearchSkipWords);
