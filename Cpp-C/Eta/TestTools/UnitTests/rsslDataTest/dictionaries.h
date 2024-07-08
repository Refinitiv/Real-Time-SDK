///*
// *|---------------------------------------------------------------
// *|		Copyright (C) 2019 LSEG,			  --
// *| All rights reserved. Duplication or distribution prohibited --
// *|---------------------------------------------------------------
// */

/* Contains dictionaries to be used for testing. */

#ifndef _RSSL_TEST_DICTIONARIES_H
#define _RSSL_TEST_DICTIONARIES_H

#include <stdio.h>
#include "rtr/rsslDataDictionary.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char rsslTmpFile[] = "rsslTmpFile.txt";


/*** Utilities for writing a dictionary string to a file (so it can be loaded)***/

void _createTmpFile(const char *str, RsslUInt32 length)
{
	FILE * fp = fopen(rsslTmpFile, "w");
	fwrite(str, 1, length, fp);
	fclose(fp);
}

void _deleteTestFile()
{
	remove(rsslTmpFile);
}

/*** Bad dictionary files -- should also run through memory leak checker ***/

static char rsslTestEnumDef_Err_NoFids[] =
      "0        \"   \"   undefined\n"
      "1        \"ASE\"   NYSE AMEX\n"
      "2        \"NYS\"   New York Stock Exchange\n";

static char rsslTestEnumDef_Err_NoTable[] =
	"RDN_EXCHID     4\n";

static char rsslTestEnumDef_Err_NoTable2[] =
	"RDN_EXCHID     4"
	"0        \"   \"   undefined\n"
	"1        \"ASE\"   NYSE AMEX\n"
	"2        \"NYS\"   New York Stock Exchange\n"
	"PRCTCK_1      14\n"
	"LST_PRCTCK  1791\n"
	"TICK_1      3111\n"
	"TICK_2      3112\n"
	"TICK_3      3113\n"
	"TICK_4      3114\n"
	"TICK_5      3115\n";

static char rsslTestEnumDef_Err_WrongType[] =
	"!tag Type 1\n"
	"RDN_EXCHID     4\n"
	"0        \"   \"   undefined\n"
	"1        \"ASE\"   NYSE AMEX\n"
	"2        \"NYS\"   New York Stock Exchange\n";

static char rsslTestEnumDef_Err_IsFieldDictionary[] =
	"PROD_PERM  \"PERMISSION\"             1  NULL        INTEGER             5  UINT32           2\n"
	"RDNDISPLAY \"DISPLAYTEMPLATE\"        2  NULL        INTEGER             3  UINT32           1\n";

static char rsslTestEnumDef_Err_NoFid[] =
	"RDN_EXCHID     \n"
	"0        \"   \"   undefined\n"
	"1        \"ASE\"   NYSE AMEX\n"
	"2        \"NYS\"   New York Stock Exchange\n";

static char rsslTestEnumDef_Err_NoValue[] =
	"RDN_EXCHID     4\n"
	"0        \"   \"   undefined\n"
	"         \"ASE\"   NYSE AMEX\n"
	"2        \"NYS\"   New York Stock Exchange\n";

static char rsslTestEnumDef_Err_NoDisplay[] =
	"RDN_EXCHID     4\n"
	"0        \"   \"   undefined\n"
	"1                  NYSE AMEX\n"
	"2        \"NYS\"   New York Stock Exchange\n";

static char rsslTestEnumDef_Err_BadHex1[] = /* Odd length */
	"RDN_EXCHID     4\n"
	"0        \"   \"   undefined\n"
	"1        #FFF#   NYSE AMEX\n";

static char rsslTestEnumDef_Err_BadHex2[] = /* G is not hex digit */
	"RDN_EXCHID     4\n"
	"0        #DE#   undefined\n"
	"1        #FG#   NYSE AMEX\n";

/* Make an RsslBuffer from a string literal. */
#define STR_TO_RSSLBUF(dictData) { sizeof(dictData)/sizeof(char), const_cast<char*>((dictData)) }

/*** Error cases table ***/
typedef struct
{
	RsslBuffer dictData;	/* The string of dictionary data */
	const char *errorText;	/* Very brief description of the expected error */
} rsslTest_DictLoadErrorCases;

static rsslTest_DictLoadErrorCases dictErrorCases[] =
{
	{ STR_TO_RSSLBUF(rsslTestEnumDef_Err_NoFids), "No fids" },
	{ STR_TO_RSSLBUF(rsslTestEnumDef_Err_NoTable), "No table" },
	{ STR_TO_RSSLBUF(rsslTestEnumDef_Err_NoTable2), "No table(2)" },
	{ STR_TO_RSSLBUF(rsslTestEnumDef_Err_WrongType), "Wrong type" },
	{ STR_TO_RSSLBUF(rsslTestEnumDef_Err_IsFieldDictionary), "(Trying field dict data to see what happens)" },
	{ STR_TO_RSSLBUF(rsslTestEnumDef_Err_NoFid), "No FieldId"},
	{ STR_TO_RSSLBUF(rsslTestEnumDef_Err_NoValue), "No Value" },
	{ STR_TO_RSSLBUF(rsslTestEnumDef_Err_NoDisplay), "No Display" },
	{ STR_TO_RSSLBUF(rsslTestEnumDef_Err_BadHex1), "Odd Hex" },
	{ STR_TO_RSSLBUF(rsslTestEnumDef_Err_BadHex2), "Bad Hex" }
};
const int dictErrorCases_length = sizeof(dictErrorCases) / sizeof(rsslTest_DictLoadErrorCases);

/*** Well-formed enum tables ***/

typedef struct
{
	int expectedFidCount;
	int expectedEnumCount;
} RsslTest_EnumTable;

static char rsslTestEnumDef_OneTable[] =
	"RDN_EXCHID     4\n"
	"0        \"   \"   undefined\n"
	"1        \"ASE\"   NYSE AMEX\n";
static RsslTest_EnumTable rsslTestEnumTables_OneTable[] = { {1, 2} };

static char rsslTestEnumDef_OneTable_2[] =
	"RDN_EXCHID     4\n"
	"0        \"   \"   undefined\n"
	"1        \"ASE\"   NYSE AMEX\n"
	"2        \"NYS\"   New York Stock Exchange\n";
static RsslTest_EnumTable rsslTestEnumTables_OneTable_2[] = { {1, 3} };

static char rsslTestEnumDef_TwoTables[] =
	"RDN_EXCHID     4\n"
	"0        \"   \"   undefined\n"
	"1        \"ASE\"   NYSE AMEX\n"
	"SPS_FD_STS  6474\n"
	"SPS_GP_DSC  6476\n"
	"SPS_PV_STS  6479\n"
	"0       \"    \"   undefined\n";
static RsslTest_EnumTable rsslTestEnumTables_TwoTables[] = { {1, 2}, {3, 1} };

typedef struct
{
	RsslBuffer dictData;
	RsslTest_EnumTable *table;
	int	length;
} RsslTest_EnumTableList;

static RsslTest_EnumTableList rsslTestEnumTables[] =
{
	{STR_TO_RSSLBUF(rsslTestEnumDef_OneTable), rsslTestEnumTables_OneTable, sizeof(rsslTestEnumTables_OneTable)/sizeof(RsslTest_EnumTable)},
	{STR_TO_RSSLBUF(rsslTestEnumDef_OneTable_2), rsslTestEnumTables_OneTable_2, sizeof(rsslTestEnumTables_OneTable_2)/sizeof(RsslTest_EnumTable)},
	{STR_TO_RSSLBUF(rsslTestEnumDef_TwoTables), rsslTestEnumTables_TwoTables, sizeof(rsslTestEnumTables_TwoTables)/sizeof(RsslTest_EnumTable)},
};
const int rsslTestEnumTables_length = sizeof(rsslTestEnumTables)/sizeof(RsslTest_EnumTableList);

#ifdef __cplusplus
}
#endif

#endif
