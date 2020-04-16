/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "Utilities.h"
#include <sstream>

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

const char* fieldDictionaryFileName = "RDMFieldDictionaryTest";
const char* enumTableFileName = "enumtypeTest.def";

bool
comparingEnumType( RsslEnumType* rsslEnumType, const EnumType& enumType, bool payloadOnly ) {
  EXPECT_TRUE( rsslEnumType != NULL ) << "rsslEnumType is null";
  if ( ::testing::Test::HasFailure() )
    return false;

  EXPECT_EQ( rsslEnumType->value, enumType.getValue() ) << " Value not equal";
  if ( rsslEnumType->value != enumType.getValue() )
    return false;

  if ( ! comparingData( rsslEnumType->display, enumType.getDisplay() ) )
    return false;

  if ( ! payloadOnly )
    if ( ! comparingData( rsslEnumType->meaning, enumType.getMeaning() ) )
      return false;
  return true;
}

bool
comparingEnumTable( RsslEnumTypeTable* rsslEnumTypeTable, const EnumTypeTable& enumTypeTable,
		    bool payloadOnly ) {
  EXPECT_TRUE( rsslEnumTypeTable != NULL ) << "comparingEnumTable: rsslEnumTypeTable argument NULL";

  RsslEnumType* rsslEnumType = 0;
  const EmaVector<EnumType>& enumTypeList = enumTypeTable.getEnumTypes();
  UInt32 enumTypeListIndex = 0;

  bool result;
  for ( UInt32 index = 0; index <= rsslEnumTypeTable->maxValue; ++index ) {
    rsslEnumType = *(rsslEnumTypeTable->enumTypes + index);
    if (rsslEnumType)	{
      result = comparingEnumType( rsslEnumType, enumTypeList[enumTypeListIndex], payloadOnly );
      EXPECT_TRUE( result ) << "comparingEnumType for rsslEnumTypeTable index " << index
			    << " and enumTypeList index " << enumTypeListIndex << " failed";
      if ( ! result )
	return false;
      ++enumTypeListIndex;
    }
  }

  RsslFieldId* rsslField = 0;
  const EmaVector<Int16>& fieldIdList = enumTypeTable.getFidReferences();
  for ( UInt32 index = 0; index < rsslEnumTypeTable->fidReferenceCount; ++index ) {
    rsslField = rsslEnumTypeTable->fidReferences + index;
    EXPECT_EQ ( *rsslField, fieldIdList[index] ) << " rsslField not equal to fieldIdList for index "
						 << index;
    if ( *rsslField != fieldIdList[index] )
      return false;
  }

  return true;
}

bool
comparingDictionaryEntry(RsslDictionaryEntry* rsslDictionaryEntry, const DictionaryEntry& dictionaryEntry,
			 bool payloadOnly) {
  EXPECT_TRUE( rsslDictionaryEntry != NULL ) << "RsslDictionaryEntry is null pointer";
  if ( ::testing::Test::HasFailure() )
    return false;

  string message("Fid ");
  ostringstream traceMsg;
  {
    traceMsg << message << dictionaryEntry.getFid() << " Acronym" << "s not equal";
    SCOPED_TRACE( traceMsg.str() );
    if ( ! comparingData( rsslDictionaryEntry->acronym, dictionaryEntry.getAcronym() ) )
      return false;
  }
  {
    traceMsg.str( message );
    traceMsg << message << dictionaryEntry.getFid() << " DDEAcronym" << "s not equal";
    SCOPED_TRACE( traceMsg.str() );
    if ( ! comparingData( rsslDictionaryEntry->ddeAcronym, dictionaryEntry.getDDEAcronym() ) )
      return false;
  }
  {
    traceMsg.str( message );
    traceMsg << message << dictionaryEntry.getFid() << " EnumLength" << "s not equal";
    SCOPED_TRACE( traceMsg.str() );
    EXPECT_EQ( rsslDictionaryEntry->enumLength, dictionaryEntry.getEnumLength() );
    if ( rsslDictionaryEntry->enumLength != dictionaryEntry.getEnumLength() )
      return false;
  }
  {
    traceMsg.str( message );
    traceMsg << message << dictionaryEntry.getFid() << " Fid" << "s not equal";
    SCOPED_TRACE( traceMsg.str() );
    EXPECT_EQ( rsslDictionaryEntry->fid, dictionaryEntry.getFid() );
    if( rsslDictionaryEntry->fid != dictionaryEntry.getFid() )
      return false;
  }
  {
    traceMsg.str( message );
    traceMsg << message << dictionaryEntry.getFid() << " Field Type" << "s not equal";
    SCOPED_TRACE( traceMsg.str() );
    EXPECT_EQ( rsslDictionaryEntry->fieldType, dictionaryEntry.getFieldType() );
    if( rsslDictionaryEntry->fieldType != dictionaryEntry.getFieldType() )
      return false;
  }
  {
    traceMsg.str( message );
    traceMsg << message << dictionaryEntry.getFid() << " Length" << "s not equal";
    SCOPED_TRACE( traceMsg.str() );
    EXPECT_EQ( rsslDictionaryEntry->length, dictionaryEntry.getLength() );
    if( rsslDictionaryEntry->length != dictionaryEntry.getLength() )
      return false;
  }
  {
    /*
    if (rsslDictionaryEntry->pEnumTypeTable) {
      if (comparingEnumTable(rsslDictionaryEntry->pEnumTypeTable, dictionaryEntry.getEnumTypeTable(),
			     failedText, payloadOnly) != true)
	return false;
    }
    */
  }
  {
    traceMsg.str( message );
    traceMsg << message << dictionaryEntry.getFid() << " RippleToField" << "s not equal";
    SCOPED_TRACE( traceMsg.str() );
    EXPECT_EQ( rsslDictionaryEntry->rippleToField, dictionaryEntry.getRippleToField() );
    if( rsslDictionaryEntry->rippleToField != dictionaryEntry.getRippleToField() )
      return false;
  }
  {
    traceMsg.str( message );
    traceMsg << message << dictionaryEntry.getFid() << " RwfLength" << "s not equal";
    SCOPED_TRACE( traceMsg.str() );
    EXPECT_EQ( rsslDictionaryEntry->rwfLength, dictionaryEntry.getRwfLength() );
    if( rsslDictionaryEntry->rwfLength != dictionaryEntry.getRwfLength() )
      return false;
  }
  {
    traceMsg.str( message );
    traceMsg << message << dictionaryEntry.getFid() << " RwfType" << "s not equal";
    SCOPED_TRACE( traceMsg.str() );
    EXPECT_EQ( rsslDictionaryEntry->rwfType, dictionaryEntry.getRwfType() );
    if( rsslDictionaryEntry->rwfType != dictionaryEntry.getRwfType() )
      return false;
  }
  return true;
}

bool
comparingDataDictionary( RsslDataDictionary* rsslDataDictionary, const DataDictionary& dataDictionary,
			 bool payloadOnly = false ) {
  if ( ! payloadOnly ) {
    EXPECT_TRUE( comparingData( rsslDataDictionary->infoEnum_Date, dataDictionary.getEnumDate() ) )
      << "comparing DataDictionary::getInfoEnumDate() with RsslDataDictionary.infoEnum_Date";
    EXPECT_TRUE( comparingData( rsslDataDictionary->infoEnum_Desc, dataDictionary.getEnumDescription() ) )
      << "comparing DataDictionary::getInfoEnumDesc() with RsslDataDictionary.infoEnum_Desc";
    EXPECT_TRUE( comparingData( rsslDataDictionary->infoEnum_Filename, dataDictionary.getEnumFilename() ) )
      << "comparing DataDictionary::getInfoEnumFilename() with RsslDataDictionary.infoEnum_Filename";
    EXPECT_TRUE( comparingData( rsslDataDictionary->infoField_Build, dataDictionary.getFieldBuild() ) )
      << "comparing DataDictionary::getInfoFieldBuild() with RsslDataDictionary.infoField_Build";
    EXPECT_TRUE( comparingData( rsslDataDictionary->infoField_Date, dataDictionary.getFieldDate() ) )
      << "comparing DataDictionary::getInfoFieldDate() with RsslDataDictionary.infoField_Date";
    EXPECT_TRUE( comparingData( rsslDataDictionary->infoField_Desc, dataDictionary.getFieldDescription() ) )
      << "comparing DataDictionary::getInfoFieldDesc() with RsslDataDictionary.infoField_Desc";
    EXPECT_TRUE( comparingData( rsslDataDictionary->infoField_Filename,
				dataDictionary.getFieldFilename() ) )
      << "comparing DataDictionary::getInfoFieldFilename() with RsslDataDictionary.infoField_Filename";
  }

  EXPECT_TRUE( comparingData( rsslDataDictionary->infoEnum_DT_Version,
			      dataDictionary.getEnumDisplayTemplateVersion() ) )
    << "comparing DataDictionary::getInfoEnumDTVersion() with RsslDataDictionary.infoEnum_DT_Version";
  EXPECT_TRUE( comparingData( rsslDataDictionary->infoEnum_RT_Version,
			      dataDictionary.getEnumRecordTemplateVersion() ) )
    << "comparing DataDictionary::getInfoEnumRTVersion() with RsslDataDictionary.infoEnum_RT_Version";
  EXPECT_TRUE( comparingData( rsslDataDictionary->infoField_Version,
			      dataDictionary.getFieldVersion() ) )
    << "comparing DataDictionary::getInfoFieldVersion() with RsslDataDictionary.infoField_Version";
  EXPECT_EQ( rsslDataDictionary->info_DictionaryId, dataDictionary.getDictionaryId() ) <<
    "Comparing DataDictionary::getInfoDictionaryId() with RsslDataDictionary.info_DictionaryId";
  EXPECT_EQ( rsslDataDictionary->numberOfEntries, dataDictionary.getEntries().size() ) <<
    "Comparing DataDictionary::getEntries().size() with RsslDataDictionary.numberOfEntries";
  EXPECT_EQ( rsslDataDictionary->enumTableCount, dataDictionary.getEnumTables().size() ) <<
    "Comparing DataDictionary::getEnumTables().size() with RsslDataDictionary.enumTableCount";
  EXPECT_EQ( rsslDataDictionary->maxFid, dataDictionary.getMaxFid() ) <<
    "Comparing DataDictionary::getMaxFid()";
  EXPECT_EQ( rsslDataDictionary->minFid, dataDictionary.getMinFid() ) <<
    "Comparing DataDictionary::getMinFid()";
   
  // Comparing for all DictionaryEntry from DataDictionary::getEntries()
  bool result;
  Int32 index = 0;
  RsslDictionaryEntry* rsslDictionaryEntry = 0;
  const EmaVector<DictionaryEntry>& dictionaryEntryList = dataDictionary.getEntries();
  
  for (Int32 fid = rsslDataDictionary->minFid; fid <= rsslDataDictionary->maxFid; fid++) {
    rsslDictionaryEntry = *(rsslDataDictionary->entriesArray + fid);

    if (rsslDictionaryEntry) {
      result = comparingDictionaryEntry( rsslDictionaryEntry, dictionaryEntryList[index], payloadOnly );
      EXPECT_TRUE( result ) << "dictionary entries for fid " << fid << " failed comparison check";
      if ( ! result )
	break;
      ++index;
    }
  }

  // Make sure that there is no other DictionaryEntry in dictionaryEntryList
  EXPECT_EQ( index, dictionaryEntryList.size() ) << "ema dataDictionary has different number of entries than rsslDictionary";

  // EnumTypeTable comparison
  const EmaVector<EnumTypeTable>& enumTypeTableList =  dataDictionary.getEnumTables();
  RsslEnumTypeTable* rsslEnumTypeTable;
  for (index = 0; index < rsslDataDictionary->enumTableCount ; ++index)	{
    rsslEnumTypeTable = *(rsslDataDictionary->enumTables + index);
    result = comparingEnumTable(rsslEnumTypeTable, enumTypeTableList[index], payloadOnly);
    EXPECT_TRUE( result ) << "comparingDataDictionary: comparingEnumTable failed for index " << index;
    if (result == false)
      break;
  }

  return ! ::testing::Test::HasFailure();
}

// this class exists to ensure that the globalRsslDataDictionary and globalDataDictionary are loaded
// before running any of these other tests. We use the variable hasRun so that we load these only once
class DataDictionaryTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    SCOPED_TRACE( "DataDictionaryTest SetUp" );
    if (hasRun)
      return;
    char errTxt[256];
    RsslBuffer errorText = { 255, (char*)errTxt };

    rsslClearDataDictionary(&globalRsslDataDictionary);

    int stuff( RSSL_RET_FAILURE );

    ASSERT_EQ( rsslLoadFieldDictionary(fieldDictionaryFileName, &globalRsslDataDictionary, &errorText), 0) << "loading globalRsslDataDictionary";
    try {
      globalDataDictionary.loadFieldDictionary(fieldDictionaryFileName);
    }
    catch (const OmmException&) {
      ASSERT_TRUE( false ) <<  "DataDictionary::loadFieldDictionary() failed to load dictionary information";
    }
    ASSERT_TRUE( comparingDataDictionary(&globalRsslDataDictionary, globalDataDictionary) ) << "DataDictionarySetUp failed when comparing field dictionaries";

    ASSERT_EQ( rsslLoadEnumTypeDictionary(enumTableFileName, &globalRsslDataDictionary, &errorText), 0) << "Failed to load enumerated types information with RsslDataDictionary";
    try	{
      globalDataDictionary.loadEnumTypeDictionary(enumTableFileName);
    }
    catch (const OmmException&) {
      ASSERT_TRUE( false ) << "DataDictionary::loadEnumTypeDictionary() failed to load enumerated types information - exception not expected";
    }
    ASSERT_TRUE( comparingDataDictionary(&globalRsslDataDictionary, globalDataDictionary) ) << "DataDictionarySetUp failed when comparing enumeration dictionaries";
    hasRun = true;
  }

  static RsslDataDictionary globalRsslDataDictionary;
  static DataDictionary globalDataDictionary;
  static bool hasRun;
};

bool DataDictionaryTest::hasRun(false);
RsslDataDictionary DataDictionaryTest::globalRsslDataDictionary;
DataDictionary DataDictionaryTest::globalDataDictionary;

TEST_F(DataDictionaryTest, LoadDictionaryFromFile) {
  EXPECT_TRUE( true ) << "LoadDictionaryFromFile";
}

TEST_F(DataDictionaryTest, LoadDictionaryFromInvalidFile) {
  DataDictionary dataDictionary;
  try {
    dataDictionary.loadFieldDictionary("Invalid_RDMFieldDictionary");
    EXPECT_FALSE( true ) << "expected exception when loading dictionary from invalid file";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum )
      << "type of exception unexpected when loading dictionary from invalid file";
  }

  try {
    dataDictionary.loadEnumTypeDictionary("Invalid_enumtype.def");
    EXPECT_FALSE( true ) << "expected exception when loading enumtype.def from invalid file";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum )
      << "type of exception unexpected when loading enumtype.def from invalid file";
  }
}

TEST_F(DataDictionaryTest, Uninitialized) {
  DataDictionary dataDictionary;
  EXPECT_STREQ( dataDictionary.toString(), "DataDictionary is not initialized" )
    << "incorrect DataDictionary::toString() for uninitialized dictionary";

  EXPECT_EQ( dataDictionary.getEntries().size(), 0 )
    << "uninitialized DataDictionary should have 0 entries";
  EXPECT_EQ( dataDictionary.getEnumTables().size(), 0 )
    << "uninitialized DataDictionary should have 0 enumTables";

  EXPECT_FALSE( dataDictionary.hasEntry(1) ) << "uninitialized DataDictionary hasEntry(1)";
  try {
    dataDictionary.getEntry(1); // Getting from non existing DictionaryEntry
    EXPECT_FALSE( true ) << "uninitialized DataDictionary::getEntry(1) should have caused exception";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum) <<
      "uninitialized DataDictionary::getEntry(1): unexpected exception type";
  }

  EXPECT_FALSE( dataDictionary.hasEnumType(1, 1) ) << "uninitialzed DataDictionary::hasEnumType(1,1)";
  try {
    dataDictionary.getEnumType(1, 1); // Getting from non existing EnumType
    EXPECT_FALSE( true ) << "uninitialized DataDictionary: getEnumType(1,1) should cause exception";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum)
      << "uninitialized DataDictionary::getEnumType(1, 1): unexpected exception type";
  }

  EXPECT_EQ( dataDictionary.getDictionaryId(), 0 ) << "DataDictionary::getInfoDictionaryId()";
  EXPECT_EQ( strlen(dataDictionary.getEnumDate().c_str()), 0 ) << "DataDictionary::getInfoEnumDate()";
  EXPECT_EQ( strlen(dataDictionary.getEnumDescription().c_str() ), 0)
    << "DataDictionary::getInfoEnumDesc()";
  EXPECT_EQ( strlen(dataDictionary.getEnumDisplayTemplateVersion().c_str()), 0)
    << "DataDictionary::getInfoEnumDTVersion()";
  EXPECT_EQ( strlen(dataDictionary.getEnumFilename().c_str()), 0)
    << "DataDictionary::getInfoEnumFilename()";
  EXPECT_EQ( strlen(dataDictionary.getEnumRecordTemplateVersion().c_str()), 0)
    << "DataDictionary::getInfoEnumRTVersion()";
  EXPECT_EQ( strlen(dataDictionary.getFieldBuild().c_str()), 0)
    << "DataDictionary::getInfoFieldBuild()";
  EXPECT_EQ( strlen(dataDictionary.getFieldDate().c_str()), 0) << "DataDictionary::getInfoFieldDate()";
  EXPECT_EQ( strlen(dataDictionary.getFieldDescription().c_str()), 0)
    << "DataDictionary::getInfoFieldDesc()";
  EXPECT_EQ( strlen(dataDictionary.getFieldFilename().c_str()), 0)
    << "DataDictionary::getInfoFieldFilename()";
  EXPECT_EQ( strlen(dataDictionary.getFieldVersion().c_str()), 0)
    << "DataDictionary::getInfoFieldVersion()";
  EXPECT_EQ( dataDictionary.getMaxFid(), 0) << "DataDictionary::getMaxFid()";
  EXPECT_EQ( dataDictionary.getMinFid(), 0) << "DataDictionary::getMinFid()";

  try {
    Series series;
    dataDictionary.encodeEnumTypeDictionary(series, DICTIONARY_NORMAL);
    EXPECT_FALSE( true ) << "expection exception when calling DataDictionary::encodeEnumTypeDictionary()";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum )
      << "dataDictionary.encodeEnumTypeDictionary: unexpected exception type";
  }

  int count = 0;
  try {
    Series series;
    dataDictionary.encodeEnumTypeDictionary(series, count, DICTIONARY_NORMAL, 555);
    EXPECT_FALSE( true )
      << "expected exception when calling DataDictionary::encodeEnumTypeDictionary (fragmentation)";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum )
      << "DataDictionary::encodeEnumTypeDictionary (fragmentation): unexpected exception type";
  }

  try {
    Series series;
    dataDictionary.encodeFieldDictionary(series, DICTIONARY_NORMAL);
    EXPECT_FALSE( true ) << "expected exception when calling DataDictionary::encodeFieldDictionary()";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum )
      << "dataDictionary.encodeFieldDictionary: unexpected exception type";
  }

  try {
    Series series;
    dataDictionary.encodeFieldDictionary(series, count, DICTIONARY_INFO, 555);
    EXPECT_FALSE( true )
      << "expected exception when calling DataDictionary::encodeFieldDictionary(fragmentation)";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum)
      << "dataDictionary.encodeFieldDictionary: unexpected exception type";
  }
}

TEST_F(DataDictionaryTest, DictionaryEntryToString) {
  EXPECT_TRUE( globalDataDictionary.hasEntry(32303) ) << "expected to find entry for fid 32303";
  if ( ::testing::Test::HasFailure() )
    return;

  const DictionaryEntry& dictionaryEntry = globalDataDictionary.getEntry(32303);

  EmaString expected("Fid=32303 'DBOR_DM_TP' 'DBOR DM TYPE' Type=6 RippleTo=0 Len=3 EnumLen=8 RwfType=14 RwfLen=1");
  expected.append("\n\nEnum Type Table:\n")
    .append("(Referenced by Fid 32303)\n")
    .append("(Referenced by Fid 32304)\n")
    .append("(Referenced by Fid 32312)\n")
    .append("value=0 display=\"        \" meaning=\"Undefined\"\n")
    .append("value=1 display=\"LCM     \" meaning=\"LCM\"\n")
    .append("value=2 display=\"UDM     \" meaning=\"UDM\"\n")
    .append("value=3 display=\"UNSPC_DM\" meaning=\"UNSPEC_DM\"\n");

    EXPECT_EQ( dictionaryEntry.toString(), expected.c_str() ) << "calling DictionaryEntry::toString() for a fid";
}

TEST_F(DataDictionaryTest, DictionaryEntryHasEnumTableGetEnumTable) {
  const DictionaryEntry& dictionaryEntryFid1 = globalDataDictionary.getEntry(1);

  EXPECT_FALSE( dictionaryEntryFid1.hasEnumTypeTable() )
    << "Calling DictionaryEntry::hasEnumTypeTable() for non existing EnumTypeTable.";

  try {
    dictionaryEntryFid1.getEnumTypeTable();
    EXPECT_FALSE( true ) << "expected exception when calling DictionaryEntry::getEnumTypeTable() for non existing EnumTypeTable";
  }
  catch (const OmmException& excp)
  {
      EXPECT_EQ(excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum)
	<< "dictionaryEntryFid1.getEnumTypeTable() caused exception with unexpected type";
  }

  EXPECT_TRUE( globalDataDictionary.hasEntry(4) ) << "expected globalDataDictionary.hasEntry(4)";
  if ( ::testing::Test::HasFailure() )
    return;

  const DictionaryEntry& dictionaryEntryFid4 = globalDataDictionary.getEntry(4);
  EXPECT_TRUE( dictionaryEntryFid4.hasEnumTypeTable() ) << "Calling DictionaryEntry::hasEnumTypeTable() for an existing EnumTypeTable.";

  try {
    dictionaryEntryFid4.getEnumTypeTable();
  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) << "unexpected exception when calling DictionaryEntry::getEnumTypeTable() for an existing EnumTypeTable";
  }
}

TEST_F( DataDictionaryTest, DictionaryEntryHasEnumTypeGetEnumType ) {
  const DictionaryEntry& dictionaryEntryFid1 = globalDataDictionary.getEntry(1);

  EXPECT_FALSE( dictionaryEntryFid1.hasEnumType(5555) ) << "Calling DictionaryEntry::hasEnumType() for non existing EnumType.";

  try {
    dictionaryEntryFid1.getEnumType(5555);
    EXPECT_FALSE( true ) << "Calling DictionaryEntry::getEnumType() for non existing EnumType - exception expected";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum ) << "unexpected exception type for DictionaryEntry::getEnumType()";
  }

  const DictionaryEntry& dictionaryEntryFid4 = globalDataDictionary.getEntry(4);

  EXPECT_TRUE( dictionaryEntryFid4.hasEnumType(5) ) << "Calling DictionaryEntry::hasEnumType() for an existing EnumType.";

  try {
    dictionaryEntryFid4.getEnumType(5);
  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) << "unexpected exception when calling DictionaryEntry::getEnumType() for an existing EnumType";
  }
}

TEST_F( DataDictionaryTest, DictionaryToString ) {
  const EmaString& toStringValue = globalDataDictionary.toString();

  EXPECT_NE( toStringValue.find("Data Dictionary Dump: MinFid=-19 MaxFid=32766 NumEntries 14488"), -1) << "Calling DataDictionary.toString() to check for summary data";
  EXPECT_NE( toStringValue.find("Version=\"4.20.19\""), -1 ) << "Calling DataDictionary.toString() to check field dictionary version";
  EXPECT_NE( toStringValue.find("RT_Version=\"4.20.19\""), -1 ) << "Calling DataDictionary.toString() to check record template version";
  EXPECT_NE( toStringValue.find("DT_Version=\"15.61\""), -1 ) << "Calling DataDictionary.toString() to check display template version";
  EXPECT_NE( toStringValue.find("Fid=1 'PROD_PERM' 'PERMISSION' Type=1 RippleTo=0 Len=5 EnumLen=0 RwfType=4 RwfLen=2"), -1 ) << "Calling DataDictionary.toString() to check the first field dictionary";
  EXPECT_NE( toStringValue.find("Fid=32761 'CD_CEILING' 'CD_CEILING' Type=1 RippleTo=0 Len=10 EnumLen=0 RwfType=4 RwfLen=5"), -1 ) << "Calling DataDictionary.toString() to check the last field dictionary";
  EXPECT_NE( toStringValue.find("value=1 display=\"ASE\" meaning=\"NYSE AMEX\""), -1 ) << "Calling DataDictionary.toString() to check the first enum value";
  EXPECT_NE( toStringValue.findLast("(Referenced by Fid 32322)"), -1 ) << "DataDictionary.toString() to check fid of the last enum value";
  EXPECT_NE( toStringValue.findLast("value=1 display=\"Onpass\" meaning=\"Onpass\""), -1 ) << "Calling DataDictionary.toString() to check the last enum value";
}

TEST_F( DataDictionaryTest, DictionaryDuplicateFromFile ) {
  DataDictionary dataDictionary;

  try {
    dataDictionary.loadFieldDictionary(fieldDictionaryFileName);
    dataDictionary.loadFieldDictionary(fieldDictionaryFileName);
    EXPECT_FALSE( true ) << "expected exception after calling DataDictionary::loadFieldDictionary() multiple times";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum) << "unexpected exception type when calling DataDictionary::loadFieldDictionary() multiple times";
  }

  try {
    dataDictionary.loadEnumTypeDictionary(enumTableFileName);
    dataDictionary.loadEnumTypeDictionary(enumTableFileName);
    EXPECT_FALSE( true ) << "expected exception after calling DataDictionary::loadEnumTypeDictionary() multiple times";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum ) << "unexpected exception type after calling DataDictionary::loadEnumTypeDictionary() multiple times - exception expected";
  }
}

TEST_F( DataDictionaryTest, DictionaryDuplicateFromDictionaryPayload ) {
  DataDictionary dataDictionary;
  Series series;

  globalDataDictionary.encodeFieldDictionary(series, DICTIONARY_NORMAL);

  try {
    StaticDecoder::setData(&series, 0);
    dataDictionary.decodeFieldDictionary(series, DICTIONARY_NORMAL);
    dataDictionary.decodeFieldDictionary(series, DICTIONARY_NORMAL);
    EXPECT_FALSE( true ) << "expected exception after calling DataDictionary::decodeFieldDictionary() multiple times";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum ) << "unexpected exception type after calling DataDictionary::decodeFieldDictionary() multiple times";
  }

  globalDataDictionary.encodeEnumTypeDictionary(series, DICTIONARY_NORMAL);
  try {
    StaticDecoder::setData(&series, 0);
    dataDictionary.decodeEnumTypeDictionary(series, DICTIONARY_NORMAL);
    dataDictionary.decodeEnumTypeDictionary(series, DICTIONARY_NORMAL);
    EXPECT_FALSE( true ) << "expected exception after calling DataDictionary::decodeEnumTypeDictionary() multiple times";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum ) << "unexpected exception type after calling DataDictionary::decodeEnumTypeDictionary() multiple times";
  }
}

TEST_F( DataDictionaryTest, DictionaryClearForFile ) {
  DataDictionary dataDictionary;

  try {
    dataDictionary.loadFieldDictionary(fieldDictionaryFileName);
    dataDictionary.clear();

    EXPECT_EQ( dataDictionary.getEntries().size(), 0 ) << "number of dictionary entries should be 0 after calling clear()";
    EXPECT_EQ( dataDictionary.getEnumTables().size(), 0 ) << "number of enum tables should be 0 after calling clear()";
    EXPECT_FALSE( dataDictionary.hasEntry(1) ) << "dataDictionary.hasEntry(fid) should fail after calling clear()";
    EXPECT_FALSE( dataDictionary.hasEntry("PROD_PERM") ) << "dataDictionary.hasEntry(name) should fail after calling clear()";

    try {
      dataDictionary.getEntry(1); // Getting from non existing DictionaryEntry
      EXPECT_FALSE( true ) << "expected exception after clear for DataDictionary.getEntry(fid)";
    }
    catch (const OmmException& excp)
    {
      EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum )
	<< "unexpected exception type when calling DataDictionary.getEntry(fid) after clear";
    }

    try {
      dataDictionary.getEntry("PROD_PERM"); // Getting from non existing DictionaryEntry
      EXPECT_FALSE( true ) << "expected exception after clear for DataDictionary.getEntry(name)";
    }
    catch (const OmmException& excp)
    {
      EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum )
	<< "unexpected exception type when calling DataDictionary.getEntry(name) after clear";
    }

    EXPECT_FALSE( dataDictionary.hasEnumType(4, 2) ) << "checking DataDictionary.hasEnumType(fid, enum value) after calling clear()";

    try {
      dataDictionary.getEnumType(4, 2); // Getting from non existing EnumType
      EXPECT_FALSE( true ) << "expected exception from DataDictionary.enumType(fid,value) after clear";
    }
    catch (const OmmException& excp)
    {
      EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum )
	<< "unexpected exception type from OmmException.exceptionType() from DataDictionary.enumType(fid,value) after clear";
    }

    dataDictionary.loadFieldDictionary(fieldDictionaryFileName);
    EXPECT_EQ( dataDictionary.getEntries().size(), globalRsslDataDictionary.numberOfEntries )
      << "number of dictionary entries after load should equal globalRsslDataDictionary";
  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) << "unexecpted exception when calling DataDictionary::loadFieldDictionary() multiple times after clearing the data dictionary";
  }

  try {
    dataDictionary.loadEnumTypeDictionary(enumTableFileName);
    dataDictionary.clear();
    EXPECT_EQ( dataDictionary.getEntries().size(), 0) << "number of dictionaryEntries should be 0 after calling clear()";
    EXPECT_EQ( dataDictionary.getEnumTables().size(), 0) << "size of enumTable is 0 after calling clear()";
    dataDictionary.loadEnumTypeDictionary(enumTableFileName);
    EXPECT_EQ( dataDictionary.getEnumTables().size(), globalRsslDataDictionary.enumTableCount)
      <<  "size of EnumTable is equal";
  }
  catch (const OmmException& )
  {
    EXPECT_FALSE( true ) << "unexpected exception after calling DataDictionary::loadEnumTypeDictionary() multiple times after clearing the data dictionary";
  }
}

TEST_F( DataDictionaryTest, DictionaryClearForPayload ) {
  DataDictionary dataDictionary;
  Series series;

  globalDataDictionary.encodeFieldDictionary(series, DICTIONARY_NORMAL);

  try {
    StaticDecoder::setData(&series, 0);
    dataDictionary.decodeFieldDictionary(series, DICTIONARY_NORMAL);
    dataDictionary.clear();
    dataDictionary.decodeFieldDictionary(series, DICTIONARY_NORMAL);
    EXPECT_EQ( dataDictionary.getEntries().size(), globalRsslDataDictionary.numberOfEntries) << "number of Dictionary entries should be euqal";
  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) << "unxpected exception when calling DataDictionary::decodeFieldDictionary() multiple times after clearing the data dictionary";
  }

  globalDataDictionary.encodeEnumTypeDictionary(series, DICTIONARY_NORMAL);
  try {
    StaticDecoder::setData(&series, 0);
    dataDictionary.decodeEnumTypeDictionary(series, DICTIONARY_NORMAL);
    dataDictionary.clear();
    dataDictionary.decodeEnumTypeDictionary(series, DICTIONARY_NORMAL);
    EXPECT_EQ( dataDictionary.getEnumTables().size(), globalRsslDataDictionary.enumTableCount )
      << "size of enum table is equal";
  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) << "unexpected exception after calling DataDictionary::decodeEnumTypeDictionary() multiple times after clearing the data dictionary";
  }
}

TEST_F( DataDictionaryTest, DictionaryHasEntryGetEntryByFieldID ) {
  EXPECT_FALSE( globalDataDictionary.hasEntry(-555) ) << "checking non existing dictionary entry";

  try {
    globalDataDictionary.getEntry(-555);
    EXPECT_FALSE( true ) << "expected exception after calling DictionaryUtility::getEntry(fid) for non-existing dictionary entry";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum) << "unexpected exception type after calling getEntry(fid) from non existing dictionary entry";
  }

  EXPECT_TRUE( globalDataDictionary.hasEntry(3) ) << "checking existing dictionary entry";

  try {
    const DictionaryEntry& dictionaryEntry = globalDataDictionary.getEntry(3);
  }
  catch (const OmmException& )
  {
    EXPECT_FALSE( true ) << "unexpected exception when calling DictionaryUtility::getEntry(fid) for an existing dictionary entry";
  }
}

TEST_F( DataDictionaryTest, DictionaryHasEntryGetEntryByFieldName ) {
  EXPECT_FALSE( globalDataDictionary.hasEntry("UNKNOWN_FID") ) << "check non-existing dictionary entry";

  try {
    globalDataDictionary.getEntry("UNKNOWN_FID");
    EXPECT_FALSE( true ) << "expected exception when calling DictionaryUtility::getEntry(name) for non-existing DictionaryEntry";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum) << "unexpected exception type after calling DictionaryUtility::getEntry(name) for non-existing dictionary entry";
  }

  EXPECT_TRUE( globalDataDictionary.hasEntry("DSPLY_NAME") ) << "check existing dictionary entry";

  try {
    const DictionaryEntry& dictionaryEntry = globalDataDictionary.getEntry("DSPLY_NAME");
  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) << "unexpected exception after calling DictionaryUtility::getEntry(name) for an existing dictionary entry";
  }
}

TEST_F( DataDictionaryTest, DictionaryHasEnumTypeGetEnumType ) {
  EXPECT_FALSE( globalDataDictionary.hasEnumType( 4, 5555 ) ) << "check for non-existing enum type";

  try {
    globalDataDictionary.getEnumType(4, 5555);
    EXPECT_FALSE( true ) << "expected exception when calling DictionaryUtility::getEnumType(fid, enumValue) for non-existing enum type";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum) <<  "unexpected exception type when calling DictionaryUtility::getEnumType(fid, enumvalue) for non-existing enum type";
  }

  EXPECT_TRUE( globalDataDictionary.hasEnumType(4, 15) ) << "check for existing dictionary entry";

  try {
    const EnumType& enumType = globalDataDictionary.getEnumType(4, 15);
  }
  catch (const OmmException& )
  {
    EXPECT_FALSE( true ) << "unexpected exception when calling DictionaryUtility::getEnumType(fid, enumvalue) for an existing enum type";
  }
}

TEST_F( DataDictionaryTest, DictionaryDictionaryUtility ) {
  FieldList fieldList;

  fieldList.addReal(22, 3990, OmmReal::ExponentNeg2Enum).
    addReal(25, 3994, OmmReal::ExponentNeg2Enum).addReal(30, 9, OmmReal::Exponent0Enum).
    addReal(31, 19, OmmReal::Exponent0Enum).complete();
  StaticDecoder::setData(&fieldList, &globalRsslDataDictionary);
  const DataDictionary& dictionaryData = DictionaryUtility::dataDictionary(fieldList);

  EXPECT_TRUE( comparingDataDictionary(&globalRsslDataDictionary, dictionaryData ) )
    << " failure when comparing data dictionary for newly created field list";

  try {
    Series series;
    const_cast<DataDictionary&>(dictionaryData).decodeFieldDictionary(series, DICTIONARY_NORMAL);
    EXPECT_FALSE( true ) << "expected exception when calling DataDictionary::decodeFieldDictionary()";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum ) << "unexpected exception type when calling DataDictionary::decodeFieldDictionary()";
  }
}

TEST_F( DataDictionaryTest, DictionaryDictionaryUtilityFromEncodingOnly ) {
  FieldList fieldList;
  fieldList.addReal(22, 3990, OmmReal::ExponentNeg2Enum).addReal(25, 3994, OmmReal::ExponentNeg2Enum).
    addReal(30, 9, OmmReal::Exponent0Enum).addReal(31, 19, OmmReal::Exponent0Enum).complete();

  try {
    const DataDictionary& dictionaryData = DictionaryUtility::dataDictionary(fieldList);
    EXPECT_FALSE( true ) << "expected exception for DictionaryUtility::dataDictionary(FieldList)";
  }
  catch (const OmmException& excp)
  {
    EXPECT_EQ( excp.getExceptionType(), OmmException::OmmInvalidUsageExceptionEnum ) << "unexpected exception type when calling DictionaryUtility::dataDictionary(FieldList)";
  }	
}

TEST_F( DataDictionaryTest, DictionaryCloneDataDictionary ) {
  try {
    DataDictionary dataDictionary(globalDataDictionary);
    EXPECT_TRUE( comparingDataDictionary(&globalRsslDataDictionary, dataDictionary, true ) ) << "clone dictionary";
  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) <<  " unexpected exception: DataDictionary::DataDictionary(const DataDictionary&) failed to clone dictionary";
  }
}

TEST_F( DataDictionaryTest, DictionaryCloneDataDictionaryFromUninitialized ) {
  DataDictionary uninitializedDataDictionary;
  try {
    DataDictionary dataDictionary(uninitializedDataDictionary);
    EXPECT_STREQ( dataDictionary.toString(), "DataDictionary is not initialized") << "Clone from uninitialized DataDictionary";
  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) << "unexpected exception when cloning from uninitialized DataDictionary";
  }
}

TEST_F( DataDictionaryTest, DictionaryExtractDictionaryType ) {
  UInt64 dictionaryType = 5;
  Series series;
  series.summaryData(ElementList().addBuffer("Version", EmaBuffer("40.20", 5))
		     .addUInt("Type", dictionaryType).addInt("DictionaryId", 5).complete())
    .totalCountHint(0).complete();

  StaticDecoder::setData(&series, 0);

  EXPECT_EQ( DataDictionary().extractDictionaryType(series), dictionaryType) <<  "DataDictionary::extractDictionaryType()";
}

TEST_F( DataDictionaryTest, DictionaryFieldDictionaryEnumTypeDefEncodeDecode ) {
  Series series;
  try {
    globalDataDictionary.encodeFieldDictionary(series, DICTIONARY_VERBOSE);
  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) << "unexpected exception: DataDictionary::encodeFieldDictionary() failed to encode field dictionary information";
  }

  DataDictionary dataDictionary;
  StaticDecoder::setData(&series, 0);
  try {
    dataDictionary.decodeFieldDictionary(series, DICTIONARY_VERBOSE);
  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) << "unexpected exception: DataDictionary::decodeFieldDictionary() failed to decode field dictionary information";
  }
  try {
    globalDataDictionary.encodeEnumTypeDictionary(series, DICTIONARY_VERBOSE);
  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) << "unexpected exception: DataDictionary::encodeFieldDictionary() failed to encode field dictionary information";
  }

  StaticDecoder::setData(&series, 0);
  try {
    dataDictionary.decodeEnumTypeDictionary(series, DICTIONARY_VERBOSE);
  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) << "unexpected exception: DataDictionary::decodeEnumTypeDictionary() failed to decode field dictionary information";
  }

  EXPECT_TRUE( comparingDataDictionary(&globalRsslDataDictionary, dataDictionary, true) );
}

TEST_F( DataDictionaryTest, DictionaryFieldDictionaryEncodeWithFragmentation ) {
  UInt32 fragmentationSize = 5120;
  Int32 currentFid = globalDataDictionary.getMinFid();
  DataDictionary dataDictionary;

  Series series;
  bool result;

  try {
    while (true) {
      result = globalDataDictionary.encodeFieldDictionary(series, currentFid, DICTIONARY_VERBOSE, fragmentationSize);
      StaticDecoder::setData(&series, 0);
      try {
	dataDictionary.decodeFieldDictionary(series, DICTIONARY_VERBOSE);
      }
      catch (const OmmException&)
      {
	EXPECT_FALSE( true ) << "unexpected exception: DataDictionary::decodeFieldDictionary() failed to decode field dictionary information";
      }

      EXPECT_LE( dataDictionary.getMaxFid(), currentFid ) << "DataDictionary::getMaxFid() must be less than the current fid of multi-part payload";

      if (series.hasTotalCountHint()) {
	EXPECT_EQ( series.getTotalCountHint(), globalDataDictionary.getEntries().size() ) << "Series::getTotalCountHint() of the first multi-part payload is equal to the number of entry of the globalDataDictionary";
	EXPECT_EQ( dataDictionary.getMinFid(), globalDataDictionary.getMinFid() ) <<  "DataDictionary::getMinFid() of the first multi-part payload is equal to the globalDataDictionary";
      }

      if (result)
	break;
    }

    EXPECT_EQ( dataDictionary.getMaxFid(), globalDataDictionary.getMaxFid() ) << "DataDictionary::getMaxFid() of the last multipart payload is equal with the globalDataDictionary";

  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) << "unexpected exception: DataDictionary::encodeFieldDictionary(fragmentationSize) failed to encode field dictionary information";
  }
}

TEST_F( DataDictionaryTest, DictionaryEnumTypeDefEncodeWithFragmentation ) {
  UInt32 fragmentationSize = 12800;
  Int32 currentCount = 0;
  DataDictionary dataDictionary;
  Series series;
  bool result;

  try {
    while (true) {
      result = globalDataDictionary.encodeEnumTypeDictionary(series, currentCount, DICTIONARY_VERBOSE, fragmentationSize);

      StaticDecoder::setData(&series, 0);
      try {
	dataDictionary.decodeEnumTypeDictionary(series, DICTIONARY_VERBOSE);
      }
      catch (const OmmException&)
	{
	  EXPECT_FALSE( true ) << "unexpected exception: DataDictionary::decodeEnumTypeDictionary() failed to decode enumerated type dictionary";
	}

      if ( ! result )
	EXPECT_EQ( dataDictionary.getEnumTables().size(), currentCount ) << "DataDictionary::getEnumTables().size() must be equal to the index of current table count of multi-part payload";
      else 
	EXPECT_EQ( dataDictionary.getEnumTables().size(), currentCount + 1 ) << "DataDictionary::getEnumTables().size() must be equal to the index plus one of current table count of the final part";

      if (series.hasTotalCountHint())
	EXPECT_EQ( series.getTotalCountHint(), globalDataDictionary.getEnumTables().size() )
	  << "Series::getTotalCountHint() of the first multi-part payload is equal to the number of EnumTable of the globalDataDictionary";

      if (result)
	break;
    }

    EXPECT_EQ( dataDictionary.getEnumTables().size(), globalDataDictionary.getEnumTables().size() )
      << "DataDictionary::getEnumTables().size() of the last multipart payload should equal the globalDataDictionary.";
  }
  catch (const OmmException&)
  {
    EXPECT_FALSE( true ) <<  "unexpected exception: DataDictionary::encodeEnumTypeDictionary(fragmentationSize) failed to encode enumerated type dictionary";
  }
}
