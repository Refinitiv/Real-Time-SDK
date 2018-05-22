///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.unittest;

import junit.framework.TestCase;

import java.nio.ByteBuffer;
import java.util.List;

import org.junit.*;

import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.JUnitTestConnect;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.Series;
import com.thomsonreuters.ema.rdm.EmaRdm;
import com.thomsonreuters.ema.rdm.DataDictionary;
import com.thomsonreuters.ema.rdm.DictionaryEntry;
import com.thomsonreuters.ema.rdm.DictionaryUtility;
import com.thomsonreuters.ema.rdm.EnumTypeTable;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.ema.rdm.EnumType;

public class DataDictionaryJunitTest extends TestCase {
	
	private static com.thomsonreuters.upa.codec.DataDictionary globalEtaDataDictionary;
	private static DataDictionary globalDataDictionary;
	private static com.thomsonreuters.upa.transport.Error rsslError = com.thomsonreuters.upa.transport.TransportFactory.createError();
	
	private static String fieldDictionaryFileName = "./src/test/resources/com/thomsonreuters/ema/unittest/DataDictionaryTest/RDMTestDictionary";
	private static String enumTableFileName = "./src/test/resources/com/thomsonreuters/ema/unittest/DataDictionaryTest/testenumtype.def";
//	private static String fieldDictionaryFileName = "./Java/Ema/TestTools/UnitTests/TestData/RDMTestDictionary";
//	private static String enumTableFileName = "./Java/Ema/TestTools/UnitTests/TestData/testenumtype.def";
	
	static
	{	
		globalEtaDataDictionary = com.thomsonreuters.upa.codec.CodecFactory.createDataDictionary();
		globalDataDictionary = com.thomsonreuters.ema.access.EmaFactory.createDataDictionary();
		
		globalEtaDataDictionary.clear();
		
		if ( globalEtaDataDictionary.loadFieldDictionary(fieldDictionaryFileName, rsslError) < CodecReturnCodes.SUCCESS )
		{
			TestUtilities.checkResult(false, "Failed to load dictionary information with com.thomsonreuters.upa.codec.DataDictionary");
		}
		
		try
		{
			globalDataDictionary.loadFieldDictionary(fieldDictionaryFileName);
		}
		catch(OmmException excp)
		{
			TestUtilities.checkResult(false, "DataDictionary.loadFieldDictionary() failed to load dictionary information - exception not expected");
		}
		
		if ( globalEtaDataDictionary.loadEnumTypeDictionary(enumTableFileName, rsslError) < CodecReturnCodes.SUCCESS )
		{
			TestUtilities.checkResult(false, "Failed to load enumerated types information with com.thomsonreuters.upa.codec.DataDictionary");
		}
		
		try
		{
			globalDataDictionary.loadEnumTypeDictionary(enumTableFileName);
		}
		catch(OmmException excp)
		{
			TestUtilities.checkResult(false, "DataDictionary.loadEnumTypeDictionary() failed to load denumerated types information - exception not expected");
		}
	}
	
	public DataDictionaryJunitTest(String name)
	{
		super(name);
	}
	
	boolean comparingEnumType(com.thomsonreuters.upa.codec.EnumType rsslEnumType, EnumType enumType, StringBuilder failedText, boolean payloadOnly)
	{
		if(rsslEnumType != null)
		{		
			if ( rsslEnumType.value() != enumType.value() )
			{
				failedText.append(" Value: ").append(enumType.value()).append(" is not equal");
				return false;
			}
			
			if ( rsslEnumType.display().toString().equals(enumType.display()) == false )
			{
				failedText.append(" Display: ").append(enumType.display()).append(" is not equal");
				return false;
			}
			
			if ( !payloadOnly && rsslEnumType.meaning().toString().equals(enumType.meaning()) == false )
			{
				failedText.append(" Meaning: ").append(enumType.meaning()).append(" is not equal");
				return false;
			}
		
			return true;
		}
		else
		{
			failedText.append("com.thomsonreuters.upa.codec.EnumTyp is null");
		}
		
		return false;
	}
	
	boolean comparingEnumTable(com.thomsonreuters.upa.codec.EnumTypeTable rsslEnumTypeTable, EnumTypeTable enumTypeTable, StringBuilder failedText,
			boolean payloadOnly)
	{
		if ( rsslEnumTypeTable != null )
		{
			com.thomsonreuters.upa.codec.EnumType rsslEnumType;
			List<EnumType> enumTypeList = enumTypeTable.enumTypes();
			int enumTypeListIndex = 0;
			
			for(int index = 0; index <= rsslEnumTypeTable.maxValue(); index++ )
			{
				rsslEnumType = rsslEnumTypeTable.enumTypes()[index];
				
				if ( rsslEnumType != null )
				{
					if( comparingEnumType(rsslEnumType, enumTypeList.get(enumTypeListIndex), failedText, payloadOnly) == false )
					{
						return false;
					}
				
					enumTypeListIndex++;
				}
			}
			
			List<Integer> fieldIdList = enumTypeTable.FidReferences();
			
			for(int index = 0; index < rsslEnumTypeTable.fidReferenceCount(); index ++)
			{
				if ( rsslEnumTypeTable.fidReferences()[index] != fieldIdList.get(index))
				{
					failedText.append(" Field ID: ").append( fieldIdList.get(index)).append(" in the list of EnumTypeTable is not equal");
					return false;
				}	
			}
			
			return true;
		}
		
		return false;
	}
	
	boolean comparingDictionaryEntry(com.thomsonreuters.upa.codec.DictionaryEntry rsslDictionaryEntry, DictionaryEntry dictionaryEntry, StringBuilder failedText,
			boolean payloadOnly)
	{
		if ( rsslDictionaryEntry != null)
		{
			failedText.setLength(0);
			failedText.append("Fid: ").append(dictionaryEntry.fid());
			
			if ( rsslDictionaryEntry.acronym().toString().equals(dictionaryEntry.acronym()) == false )
			{
				failedText.append(" Acronym: ").append(dictionaryEntry.acronym()).append(" is not equal");
				return false;
			}
			
			if ( rsslDictionaryEntry.ddeAcronym().toString().equals(dictionaryEntry.ddeAcronym()) == false )
			{
				failedText.append(" DDEAcronym: ").append(dictionaryEntry.ddeAcronym()).append(" is not equal");
			}
			

			if (rsslDictionaryEntry.enumLength() != dictionaryEntry.enumLength())
			{
				failedText.append(" EnumLength: ").append(dictionaryEntry.enumLength()).append(" is not equal");
				return false;
			}

			if (rsslDictionaryEntry.fid() != dictionaryEntry.fid())
			{
				failedText.append(" is not equal");
				return false;
			}

			if (rsslDictionaryEntry.fieldType() != dictionaryEntry.fieldType())
			{
				failedText.append(" FieldType: ").append(dictionaryEntry.fieldType()).append(" is not equal");
				return false;
			}

			if (rsslDictionaryEntry.length() != dictionaryEntry.length())
			{
				failedText.append(" Length: ").append(dictionaryEntry.length()).append(" is not equal");
				return false;
			}

			if (rsslDictionaryEntry.enumTypeTable() != null )
			{
				if (comparingEnumTable(rsslDictionaryEntry.enumTypeTable(), dictionaryEntry.enumTypeTable(), failedText, payloadOnly) != true)
					return false;
			}
			else
			{
				if ( dictionaryEntry.hasEnumTypeTable() )
				{
					failedText.append(" Eta's EnumTypeTable is null while Ema's EnumTypeTable exists");
					return false;
				}
			}

			if (rsslDictionaryEntry.rippleToField() != dictionaryEntry.rippleToField())
			{
				failedText.append(" RippleToField: ").append(dictionaryEntry.rippleToField()).append(" is not equal");
				return false;
			}

			if (rsslDictionaryEntry.rwfLength() != dictionaryEntry.rwfLength())
			{
				failedText.append(" RwfLength: ").append(dictionaryEntry.rwfLength()).append(" is not equal");
				return false;
			}

			if (rsslDictionaryEntry.rwfType() != dictionaryEntry.rwfType())
			{
				failedText.append(" RwfType: ").append(dictionaryEntry.rwfType()).append(" is not equal");
				return false;
			}
			
			return true;
		}
		else
		{
			failedText.setLength(0);
			failedText.append("com.thomsonreuters.upa.codec.DictionaryEntry is null");
		}
		
		return false;
	}
	
	void comparingDataDictionary(com.thomsonreuters.upa.codec.DataDictionary rsslDataDictionary, 
			DataDictionary dataDictionary, boolean payloadOnly)
	{
		if(!payloadOnly)
		{
			TestUtilities.checkResult( rsslDataDictionary.infoEnumDate().toString().equals(dataDictionary.enumDate()) , 
					"Comparing DataDictionary.infoEnumDate() with com.thomsonreuters.upa.codec.DataDictionary.infoEnumDate()");
			
			TestUtilities.checkResult(rsslDataDictionary.infoEnumDesc().toString().equals(dataDictionary.enumDescription()), 
					"Comparing DataDictionary.infoEnumDesc() with com.thomsonreuters.upa.codec.DataDictionary.infoEnumDesc()");
			
			TestUtilities.checkResult(rsslDataDictionary.infoEnumFilename().toString().equals(dataDictionary.enumFilename()), 
					"Comparing DataDictionary.infoEnumFilename() with com.thomsonreuters.upa.codec.DataDictionary.infoEnumFilename()");
			
			TestUtilities.checkResult(rsslDataDictionary.infoFieldBuild().toString().equals(dataDictionary.fieldBuild()), 
					"Comparing DataDictionary.infoFieldBuild() with com.thomsonreuters.upa.codec.DataDictionary.infoFieldBuild()");
			
			TestUtilities.checkResult(rsslDataDictionary.infoFieldDate().toString().equals(dataDictionary.fieldDate()), 
					"Comparing DataDictionary.infoFieldDate() with com.thomsonreuters.upa.codec.DataDictionary.infoFieldDate()");
			
			TestUtilities.checkResult(rsslDataDictionary.infoFieldDesc().toString().equals(dataDictionary.fieldDescription()), 
					"Comparing DataDictionary.infoFieldDesc() with com.thomsonreuters.upa.codec.DataDictionary.infoFieldDesc()");
			
			TestUtilities.checkResult(rsslDataDictionary.infoFieldFilename().toString().equals(dataDictionary.fieldFilename()), 
					"Comparing DataDictionary.infoFieldFilename() with com.thomsonreuters.upa.codec.DataDictionary.infoFieldFilename()");
		}
		
		TestUtilities.checkResult(rsslDataDictionary.infoEnumDTVersion().toString().equals(dataDictionary.enumDisplayTemplateVersion()), 
				"Comparing DataDictionary.infoEnumDTVersion() with com.thomsonreuters.upa.codec.DataDictionary.infoEnumDTVersion()");
		
		TestUtilities.checkResult(rsslDataDictionary.infoEnumRTVersion().toString().equals(dataDictionary.enumRecordTemplateVersion()), 
				"Comparing DataDictionary.infoEnumRTVersion() with com.thomsonreuters.upa.codec.DataDictionary.infoEnumRTVersion()");
		
		TestUtilities.checkResult(rsslDataDictionary.infoFieldVersion().toString().equals(dataDictionary.fieldVersion()), 
				"Comparing DataDictionary.infoinfoFieldVersion() with com.thomsonreuters.upa.codec.DataDictionary.infoFieldVersion()");
		
		TestUtilities.checkResult(rsslDataDictionary.infoDictionaryId() == dataDictionary.dictionaryId(), 
				"Comparing DataDictionary.infoDictionaryId() with com.thomsonreuters.upa.codec.DataDictionary.infoDictionaryId()");
		
		TestUtilities.checkResult(rsslDataDictionary.numberOfEntries() == dataDictionary.entries().size() , 
				"Comparing DataDictionary.entries().size() with com.thomsonreuters.upa.codec.DataDictionary.numberOfEntries()");
		
		TestUtilities.checkResult(rsslDataDictionary.enumTableCount() == dataDictionary.enumTables().size() , 
				"Comparing DataDictionary.enumTables().size() with com.thomsonreuters.upa.codec.DataDictionary.enumTableCount()");
		
		TestUtilities.checkResult(rsslDataDictionary.maxFid() == dataDictionary.maxFid() , 
				"Comparing DataDictionary.maxFid() with com.thomsonreuters.upa.codec.DataDictionary.maxFid()");
		
		TestUtilities.checkResult(rsslDataDictionary.minFid() == dataDictionary.minFid() , 
				"Comparing DataDictionary.minFid() with com.thomsonreuters.upa.codec.DataDictionary.minFid()");
		
		// Comparing for all DictionaryEntry from DataDictionary.entries()
		int fid = rsslDataDictionary.minFid();
		com.thomsonreuters.upa.codec.DictionaryEntry rsslDictionaryEntry;
		StringBuilder errorText = new StringBuilder(256);
		boolean result = false;
		int index = 0;
		
		List<DictionaryEntry> dictionaryEntryList = dataDictionary.entries();
		
		for(; fid <= rsslDataDictionary.maxFid(); fid++)
		{
			rsslDictionaryEntry = rsslDataDictionary.entry(fid);
			
			if(rsslDictionaryEntry != null)
			{
				result = comparingDictionaryEntry(rsslDictionaryEntry, dictionaryEntryList.get(index), errorText,payloadOnly);
				if ( result == false)
				{
					break;
				}
				
				++index;
			}
		}
		
		// Make sure that there is no other DictionaryEntry in dictionaryEntryList
		TestUtilities.checkResult(  index ==  dictionaryEntryList.size(), "There is no others Ema's DictionaryEntry beyond RsslDataDictionary" ); 
		
		
		if(result)
		{
			TestUtilities.checkResult(result, "All DictionaryEntry from DataDictionary.entries() is equal with all entries in com.thomsonreuters.upa.codec.DataDictionary");
		}
		else
		{
			TestUtilities.checkResult(result, errorText.toString());
		}
		
		errorText.setLength(0);
		
		// Comparing for all EnumTypeTable from DataDictionary.enumTables()
		index = 0;
		List<EnumTypeTable> enumTypeTableList = dataDictionary.enumTables();
		com.thomsonreuters.upa.codec.EnumTypeTable rsslEnumTypeTable;
		
		for(; index < rsslDataDictionary.enumTableCount(); index++)
		{
			rsslEnumTypeTable = rsslDataDictionary.enumTables()[index];
			
			result = comparingEnumTable(rsslEnumTypeTable, enumTypeTableList.get(index), errorText, payloadOnly);
			
			if ( result == false )
			{
				break;
			}
		}
		
		if (result)
		{
			TestUtilities.checkResult(result, "All EnumTypeTable from DataDictionary.enumTables() is equal with com.thomsonreuters.upa.codec.enumTables()");
		}
		else
		{
			TestUtilities.checkResult(result, errorText.toString());
		}
	}
	
	@Test
	public void testDataDictionary_loadDictionaryFromFile()
	{
		TestUtilities.printTestHead("testDataDictionary_loadDictionaryFromFile()","Test load dictionary information from local file");
		
		comparingDataDictionary(globalEtaDataDictionary, globalDataDictionary, false);
	}
	
	@Test
	public void testDataDictionary_loadDictionaryFrom_Invalid_File()
	{
		TestUtilities.printTestHead("testDataDictionary_loadDictionaryFrom_Invalid_File()","Test load dictionary information from invalid local file");
		
		DataDictionary dataDictionary = EmaFactory.createDataDictionary();
		
		try
		{
			dataDictionary.loadFieldDictionary("Invalid_RDMFieldDictionary");
			TestUtilities.checkResult(false, "DataDictionary.loadFieldDictionary() with invalid RDMFieldDictionary - exception expected");
		}
		catch(OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='Unable to load field dictionary from file named Invalid_RDMFieldDictionary\n\tCurrent working directory " 
			+ System.getProperty("user.dir") + "\n\tReason='Can't open file: Invalid_RDMFieldDictionary''"),
				"DataDictionary.loadFieldDictionary() with invalid RDMFieldDictionary - exception expected");
		}
		
		try
		{
			dataDictionary.loadEnumTypeDictionary("Invalid_enumtype.def");
			TestUtilities.checkResult(false, "DataDictionary.loadEnumTypeDictionary() with invalid enumtype.def - exception expected");
		}
		catch(OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='Unable to load enumerated type definition from file named Invalid_enumtype.def\n\tCurrent working directory " 
			+ System.getProperty("user.dir") + "\n\tReason='Can't open file: Invalid_enumtype.def''"),
				"DataDictionary.loadEnumTypeDictionary() with invalid enumtype.def - exception expected");
		}
	}
	
	@Test
	public void testDataDictionary_Uninitialize()
	{
		TestUtilities.printTestHead("testDataDictionary_Uninitialize()",
				"Test to call querying and encoding interfaces of DataDictionary while the internal RsslDataDictionary is not initialized yet.");
		
		DataDictionary dataDictionary = com.thomsonreuters.ema.access.EmaFactory.createDataDictionary();
		
		TestUtilities.checkResult(dataDictionary.toString().equals("DataDictionary is not initialized"), "DataDictionary.toString()");

		TestUtilities.checkResult(dataDictionary.entries().size() == 0, "DataDictionary.entries().size()");
		TestUtilities.checkResult(dataDictionary.enumTables().size() == 0, "DataDictionary.enumTables().size()");

		TestUtilities.checkResult(dataDictionary.hasEntry(1) == false, "DataDictionary.hasEntry(fid)");
		TestUtilities.checkResult(dataDictionary.hasEntry("PROD_PERM") == false, "DataDictionary.hasEntry(name)");
		
		try
		{
			dataDictionary.entry(1); // Getting from non existing DictionaryEntry
			TestUtilities.checkResult(false, "DataDictionary.entry(fid) cannot get DictionaryEntry - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The field dictionary information was not loaded'"),
				"DataDictionary.entry(fid) cannot get DictionaryEntry  - exception expected");
		}
		
		try
		{
			dataDictionary.entry("PROD_PERM"); // Getting from non existing DictionaryEntry
			TestUtilities.checkResult(false, "DataDictionary.entry(name) cannot get DictionaryEntry - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The field dictionary information was not loaded'"),
				"DataDictionary.entry(name) cannot get DictionaryEntry  - exception expected");
		}

		TestUtilities.checkResult(dataDictionary.hasEnumType(1, 1) == false, "DataDictionary.hasEnumType(fid,value)");

		try
		{
			dataDictionary.enumType(4, 2); // Getting from non existing EnumType
			TestUtilities.checkResult(false, "DataDictionary.enumType(fid,value) cannot get EnumType - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The enumerated types dictionary was not loaded'"),
				"DataDictionary::getEnumType(fid,value) cannot get EnumType  - exception expected");
		}

		TestUtilities.checkResult(dataDictionary.dictionaryId() == 0, "DataDictionary.infoDictionaryId()");

		TestUtilities.checkResult( dataDictionary.enumDate().equals(""), "DataDictionary.infoEnumDate()");

		TestUtilities.checkResult( dataDictionary.enumDescription().equals(""), "DataDictionary.infoEnumDesc()");

		TestUtilities.checkResult( dataDictionary.enumDisplayTemplateVersion().equals(""), "DataDictionary.infoEnumDTVersion()");

		TestUtilities.checkResult( dataDictionary.enumFilename().equals(""), "DataDictionary.infoEnumFilename()");

		TestUtilities.checkResult( dataDictionary.enumRecordTemplateVersion().equals(""), "DataDictionary.infoEnumRTVersion()");

		TestUtilities.checkResult( dataDictionary.fieldBuild().equals(""), "DataDictionary.infoFieldBuild()");

		TestUtilities.checkResult( dataDictionary.fieldDate().equals(""), "DataDictionary.infoFieldDate()");

		TestUtilities.checkResult( dataDictionary.fieldDescription().equals(""), "DataDictionary.infoFieldDesc()");

		TestUtilities.checkResult( dataDictionary.fieldFilename().equals(""), "DataDictionary.infoFieldFilename()");

		TestUtilities.checkResult( dataDictionary.fieldVersion().equals(""), "DataDictionary.infoFieldVersion()");

		TestUtilities.checkResult( dataDictionary.maxFid() == 0, "DataDictionary.maxFid()");

		TestUtilities.checkResult( dataDictionary.minFid() == 0, "DataDictionary.minFid()");

		try
		{
			dataDictionary.encodeEnumTypeDictionary(EmaFactory.createSeries(), EmaRdm.DICTIONARY_NORMAL);
			TestUtilities.checkResult(false, "DataDictionary.encodeEnumTypeDictionary() - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The enumerated types dictionary was not loaded'"),
				"DataDictionary.encodeEnumTypeDictionary()  - exception expected");
		}

		int count = 0;

		try
		{
			dataDictionary.encodeEnumTypeDictionary(EmaFactory.createSeries(), count, EmaRdm.DICTIONARY_NORMAL, 555);
			TestUtilities.checkResult(false, "DataDictionary.encodeEnumTypeDictionary(fragmentation) - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The enumerated types dictionary was not loaded'"),
				"DataDictionary.encodeEnumTypeDictionary(fragmentation)  - exception expected");
		}

		try
		{
			dataDictionary.encodeFieldDictionary(EmaFactory.createSeries(), EmaRdm.DICTIONARY_NORMAL);
			TestUtilities.checkResult(false, "DataDictionary.encodeFieldDictionary() - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The field dictionary information was not loaded'"),
				"DataDictionary.encodeFieldDictionary()  - exception expected");
		}

		try
		{
			dataDictionary.encodeFieldDictionary(EmaFactory.createSeries(), count, EmaRdm.DICTIONARY_NORMAL, 555);
			TestUtilities.checkResult(false, "DataDictionary.encodeFieldDictionary(fragmentation) - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The field dictionary information was not loaded'"),
				"DataDictionary.encodeFieldDictionary(fragmentation)  - exception expected");
		}
	}
	
	@Test
	public void testDataDictionary_FieldDictionary_EnumTypeDef_encode_decode()
	{
		TestUtilities.printTestHead("testDataDictionary_FieldDictionary_EnumTypeDef_encode_decode()", "Test encode and decode dictionary payload.");
		
		Series series = EmaFactory.createSeries();
		
		try
		{
			globalDataDictionary.encodeFieldDictionary(series, EmaRdm.DICTIONARY_VERBOSE);			
		}
		catch(OmmException excp)
		{
			TestUtilities.checkResult(false, "DataDictionary.encodeFieldDictionary() failed to encode field dictionary information - exception not expected");
		}
		
		Series decodeSeries = JUnitTestConnect.createSeries();
		
		JUnitTestConnect.setRsslData(decodeSeries, series, Codec.majorVersion(), Codec.minorVersion(), null, null);
		
		DataDictionary dataDictionary = EmaFactory.createDataDictionary();
		
		try
		{
			dataDictionary.decodeFieldDictionary(decodeSeries, EmaRdm.DICTIONARY_VERBOSE);
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(false, "DataDictionary.decodeFieldDictionary() failed to decode field dictionary information - exception not expected");
		}

		try
		{
			globalDataDictionary.encodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_VERBOSE);
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(false, "DataDictionary.encodeFieldDictionary() failed to encode field dictionary information - exception not expected");
		}
		
		decodeSeries.clear();

		JUnitTestConnect.setRsslData(decodeSeries, series, Codec.majorVersion(), Codec.minorVersion(), null, null);
		
		try
		{
			dataDictionary.decodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_VERBOSE);
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(false, "DataDictionary.decodeEnumTypeDictionary() failed to decode field dictionary information - exception not expected");
		}

		comparingDataDictionary(globalEtaDataDictionary, globalDataDictionary, true);
	}
	
	@Test
	public void testDataDictionary_FieldDictionary_encode_with_fragmentation()
	{
		TestUtilities.printTestHead("testDataDictionary_FieldDictionary_encode_with_fragmentation()", "Test encode and decode Field dictionary payload with fragmentation size.");
		
		int fragmentationSize = 5120;
		int currentFid = globalDataDictionary.minFid();
		DataDictionary dataDictionary = EmaFactory.createDataDictionary();
		
		Series series = EmaFactory.createSeries();
		Series decodeSeries = JUnitTestConnect.createSeries();
		
		try
		{
			while (true)
			{
				currentFid = globalDataDictionary.encodeFieldDictionary(series, currentFid, EmaRdm.DICTIONARY_VERBOSE, fragmentationSize);

				JUnitTestConnect.setRsslData(decodeSeries, series, Codec.majorVersion(), Codec.minorVersion(), null, null);

				try
				{
					dataDictionary.decodeFieldDictionary(decodeSeries, EmaRdm.DICTIONARY_VERBOSE);
				}
				catch (OmmException excp)
				{
					TestUtilities.checkResult(false, "DataDictionary.decodeFieldDictionary() failed to decode field dictionary information - exception not expected");
				}

				if (dataDictionary.maxFid() > currentFid)
				{
					TestUtilities.checkResult(false, "DataDictionary.maxFid() must be less than the current fid of multi-part payload");
				}

				if (decodeSeries.hasTotalCountHint())
				{
					TestUtilities.checkResult(decodeSeries.totalCountHint() == globalDataDictionary.entries().size(), "Series.totalCountHint() of the first multi-part payload is equal to the number of entry of the globalDataDictionary");
					TestUtilities.checkResult(dataDictionary.minFid() == globalDataDictionary.minFid(), "DataDictionary.minFid() of the first multi-part payload is equal to the globalDataDictionary");
				}

				if (dataDictionary.maxFid() == currentFid)
					break;
			}

			TestUtilities.checkResult(dataDictionary.maxFid() == globalDataDictionary.maxFid(), "DataDictionary::maxFid() of the last multipart payload is equal with the globalDataDictionary.");

		}
		catch (OmmException excep)
		{
			TestUtilities.checkResult(false, "DataDictionary::encodeFieldDictionary(fragmentationSize) failed to encode field dictionary information - exception not expected");
		}
	}
	
	@Test
	public void testDataDictionary_DictionaryUtility()
	{
		TestUtilities.printTestHead("testDataDictionary_DictionaryUtility()", "Test to retrieve DataDictionary from the FieldList for decoding.");

		FieldList fieldList = EmaFactory.createFieldList();
		
		fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2) );
		fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2) );
		fieldList.add( EmaFactory.createFieldEntry().real(30, 9, OmmReal.MagnitudeType.EXPONENT_0) );
		fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0) );
		
		FieldList decodeFieldList = JUnitTestConnect.createFieldList();
		
		JUnitTestConnect.setRsslData(decodeFieldList, fieldList, Codec.majorVersion(), Codec.minorVersion(), globalEtaDataDictionary, null);
		
		DictionaryUtility dictionaryUtility = EmaFactory.createDictionaryUtility();
		
		comparingDataDictionary(globalEtaDataDictionary, dictionaryUtility.dataDictionary(decodeFieldList), true);
		
		try
		{
			dictionaryUtility.dataDictionary(decodeFieldList).decodeFieldDictionary(EmaFactory.createSeries(), EmaRdm.DICTIONARY_VERBOSE);
			TestUtilities.checkResult(false, "Calling DataDictionary.decodeFieldDictionary() - exception expected");
		}
		catch(OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='This DataDictionary instance is used for query data dictionary information only'"),
				"Calling DataDictionary.decodeFieldDictionary()  - exception expected");
		}
	}
	
	@Test
	public void testDataDictionary_Clear_For_File()
	{
		TestUtilities.printTestHead("testDataDictionary_Clear_For_File()", "Test to load data dictionary information loading from file after clearing it.");
		
		DataDictionary dataDictionary = EmaFactory.createDataDictionary();
		
		try
		{
			dataDictionary.loadFieldDictionary(fieldDictionaryFileName);
			dataDictionary.entries();
			dataDictionary.enumTables();
			dataDictionary.clear();
			TestUtilities.checkResult(dataDictionary.entries().size() == 0, "The number of DictionaryEntry is 0 after calling the clear()");
			TestUtilities.checkResult(dataDictionary.enumTables().size() == 0, "The number of EnumTable is 0 after calling the clear()");
			
			TestUtilities.checkResult(dataDictionary.hasEntry(1) == false, "Checking DataDictionary.hasEntry(fid) after calling the clear()"); 
			TestUtilities.checkResult(dataDictionary.hasEntry("PROD_PERM") == false, "Checking DataDictionary.hasEntry(name) after calling the clear()");
			
			try
			{
				dataDictionary.entry(1); // Getting from non existing DictionaryEntry
				TestUtilities.checkResult(false, "DataDictionary.entry(fid) cannot get DictionaryEntry - exception expected");
			}
			catch (OmmException excp)
			{
				TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
				TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The field dictionary information was not loaded'"),
					"DataDictionary.entry(fid) cannot get DictionaryEntry  - exception expected");
			}
			
			try
			{
				dataDictionary.entry("PROD_PERM"); // Getting from non existing DictionaryEntry
				TestUtilities.checkResult(false, "DataDictionary.entry(name) cannot get DictionaryEntry - exception expected");
			}
			catch (OmmException excp)
			{
				TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
				TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The field dictionary information was not loaded'"),
					"DataDictionary.entry(name) cannot get DictionaryEntry  - exception expected");
			}
			
			TestUtilities.checkResult(dataDictionary.hasEnumType(4, 2) == false, "Checking DataDictionary.hasEnumType(fid, enum value) after calling the clear()"); 
			
			try
			{
				dataDictionary.enumType(4, 2); // Getting from non existing EnumType
				TestUtilities.checkResult(false, "DataDictionary.enumType(fid,value) cannot get EnumType - exception expected");
			}
			catch (OmmException excp)
			{
				TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
				TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The enumerated types dictionary was not loaded'"),
					"DataDictionary::getEnumType(fid,value) cannot get EnumType  - exception expected");
			}
			
			dataDictionary.loadFieldDictionary(fieldDictionaryFileName);
			TestUtilities.checkResult(dataDictionary.entries().size() == globalEtaDataDictionary.numberOfEntries(), "The number of DictionaryEntry is euqal");
			TestUtilities.checkResult(true, "Calling DataDictionary.loadFieldDictionary() multiple times after clearing the data dictionary - exception not expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(false, "Calling DataDictionary.loadFieldDictionary() multiple times after clearing the data dictionary - exception not expected");
		}
		
		try
		{
			dataDictionary.loadEnumTypeDictionary(enumTableFileName);
			dataDictionary.entries().size();
			dataDictionary.enumTables().size();
			dataDictionary.clear();
			TestUtilities.checkResult(dataDictionary.entries().size() == 0, "The number of DictionaryEntry is 0 after calling the clear()");
			TestUtilities.checkResult(dataDictionary.enumTables().size() == 0, "The number of EnumTable is 0 after calling the clear()");
			dataDictionary.loadEnumTypeDictionary(enumTableFileName);
			TestUtilities.checkResult(dataDictionary.enumTables().size() == globalEtaDataDictionary.enumTableCount(), "The number of EnumTable is euqal");
			TestUtilities.checkResult(true, "Calling DataDictionary.loadEnumTypeDictionary() multiple times after clearing the data dictionary - exception not expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(false, "Calling DataDictionary.loadEnumTypeDictionary() multiple times after clearing the data dictionary - exception not expected");
		}
	}

	@Test
	public void testDataDictionary_Clear_For_Payload()
	{
		TestUtilities.printTestHead("testDataDictionary_Clear_For_Payload()", "Test to load data dictionary information decoding from dictionary payload after clearing it.");
		
		DataDictionary dataDictionary = EmaFactory.createDataDictionary();
		
		Series series = EmaFactory.createSeries();
		
		Series decodeSeries = JUnitTestConnect.createSeries();
		
		globalDataDictionary.encodeFieldDictionary(series, EmaRdm.DICTIONARY_NORMAL);
		
		try
		{
			JUnitTestConnect.setRsslData(decodeSeries, series, Codec.majorVersion(), Codec.minorVersion(), null, null);
			dataDictionary.decodeFieldDictionary(series, EmaRdm.DICTIONARY_NORMAL);
			dataDictionary.clear();
			TestUtilities.checkResult(dataDictionary.entries().size() == 0, "The number of DictionaryEntry is 0 after calling the clear()");
			TestUtilities.checkResult(dataDictionary.enumTables().size() == 0, "The number of EnumTable is 0 after calling the clear()");
			dataDictionary.decodeFieldDictionary(series, EmaRdm.DICTIONARY_NORMAL);
			TestUtilities.checkResult(dataDictionary.entries().size() == globalEtaDataDictionary.numberOfEntries(), "The number of DictionaryEntry is euqal");
			TestUtilities.checkResult(true, "Calling DataDictionary::decodeFieldDictionary() multiple times after clearing the data dictionary - exception not expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(false, "Calling DataDictionary::decodeFieldDictionary() multiple times after clearing the data dictionary - exception not expected");
		}
		
		globalDataDictionary.encodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_NORMAL);

		try
		{
			JUnitTestConnect.setRsslData(decodeSeries, series, Codec.majorVersion(), Codec.minorVersion(), null, null);
			dataDictionary.decodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_NORMAL);
			dataDictionary.clear();
			TestUtilities.checkResult(dataDictionary.entries().size() == 0, "The number of DictionaryEntry is 0 after calling the clear()");
			TestUtilities.checkResult(dataDictionary.enumTables().size() == 0, "The number of EnumTable is 0 after calling the clear()");
			dataDictionary.decodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_NORMAL);
			TestUtilities.checkResult(dataDictionary.enumTables().size() == globalEtaDataDictionary.enumTableCount(), "The number of EnumTable is euqal");
			TestUtilities.checkResult(true, "Calling DataDictionary.decodeEnumTypeDictionary() multiple times after clearing the data dictionary - exception not expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(false, "Calling DataDictionary.decodeEnumTypeDictionary() multiple times after clearing the data dictionary - exception not expected");
		}
	}
	
	@Test
	public void testDataDictionary_Duplicate_FromFile()
	{
		TestUtilities.printTestHead("testDataDictionary_Duplicate_FromFile()", "Test to load duplicate data dictionary from file.");
		
		DataDictionary dataDictionary = EmaFactory.createDataDictionary();
		
		try
		{
			dataDictionary.loadFieldDictionary(fieldDictionaryFileName);
			dataDictionary.loadFieldDictionary(fieldDictionaryFileName);
			TestUtilities.checkResult(false, "Calling DataDictionary.loadFieldDictionary() multiple times - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.getMessage().indexOf("Unable to load field dictionary from file named " + fieldDictionaryFileName ) != -1 &&
					excp.getMessage().indexOf("Reason='Duplicate definition for fid -22 (Line=41).'") != -1,
					"Calling DataDictionary.loadFieldDictionary() - exception expected");
		}
		
		try
		{
			dataDictionary.loadEnumTypeDictionary(enumTableFileName);
			dataDictionary.loadEnumTypeDictionary(enumTableFileName);
			TestUtilities.checkResult(dataDictionary.enumTables().size() == globalEtaDataDictionary.enumTableCount(), "The number of EnumTable is euqal");
			TestUtilities.checkResult(false, "Calling DataDictionary.loadEnumTypeDictionary() multiple times - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.getMessage().indexOf("Unable to load enumerated type definition from file named " + enumTableFileName) != -1 && 
					excp.getMessage().indexOf("Reason='FieldId 4 has duplicate Enum Table reference'") != -1 ,
					"Calling DataDictionary.loadEnumTypeDictionary() - exception expected");
		}
	}

	@Test
	public void testDataDictionary_Duplicate_FromDictionaryPayload()
	{
		TestUtilities.printTestHead("testDataDictionary_Duplicate_FromDictionaryPayload()", "Test to load duplicate data dictionary from dictionary payload.");
	
		DataDictionary dataDictionary = EmaFactory.createDataDictionary();
		
		Series series = EmaFactory.createSeries();
		
		Series decodeSeries = JUnitTestConnect.createSeries();
		
		globalDataDictionary.encodeFieldDictionary(series, EmaRdm.DICTIONARY_NORMAL);
		
		try
		{
			JUnitTestConnect.setRsslData(decodeSeries, series, Codec.majorVersion(), Codec.minorVersion(), null, null);
			dataDictionary.decodeFieldDictionary(series, EmaRdm.DICTIONARY_NORMAL);
			dataDictionary.decodeFieldDictionary(series, EmaRdm.DICTIONARY_NORMAL);
			TestUtilities.checkResult(false, "Calling DataDictionary.decodeFieldDictionary() multiple times - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.getMessage().indexOf("Failed to decode the field dictionary information. Reason='Duplicate definition for fid -22.'") != -1, "Calling DataDictionary.decodeFieldDictionary() - exception expected");
		}
		
		globalDataDictionary.encodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_NORMAL);

		try
		{
			JUnitTestConnect.setRsslData(decodeSeries, series, Codec.majorVersion(), Codec.minorVersion(), null, null);
			dataDictionary.decodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_NORMAL);
			dataDictionary.decodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_NORMAL);
			TestUtilities.checkResult(false, "Calling DataDictionary.decodeEnumTypeDictionary() multiple times - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.getMessage().indexOf("Failed to decode the enumerated types dictionary. Reason='FieldId 4 has duplicate Enum Table reference'") != -1, "Calling DataDictionary.decodeEnumTypeDictionary() - exception expected");
		}
	}
	
	@Test
	public void testDataDictionary_DictionaryUtility_FromEncodingOnly()
	{
		TestUtilities.printTestHead("testDataDictionary_DictionaryUtility_FromEncodingOnly()", "Test to retrieve DataDictionary from the FieldList from encoding only.");
		
		FieldList fieldList = EmaFactory.createFieldList();
		
		fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2) );
		fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2) );
		fieldList.add( EmaFactory.createFieldEntry().real(30, 9, OmmReal.MagnitudeType.EXPONENT_0) );
		fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0) );
		
		try
		{
			EmaFactory.createDictionaryUtility().dataDictionary(fieldList);
			TestUtilities.checkResult(false, "DictionaryUtility.dataDictionary(FieldList) - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals( "Exception Type='OmmInvalidUsageException', Text='Failed to extract DataDictionary from the passed in FieldList'"),
				"DictionaryUtility.dataDictionary(FieldList) from encoding a FieldList  - exception expected");
		}	
	}
	
	@Test
	public void testDataDictionary_CloneDataDictionary()
	{
		TestUtilities.printTestHead("testDataDictionary_CloneDataDictionary()","Test to clone DataDictonary from another DataDictionary instance.");

		try
		{
			DataDictionary dataDictionary = EmaFactory.createDataDictionary(globalDataDictionary);
			comparingDataDictionary(globalEtaDataDictionary, dataDictionary, true);
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(false, "DataDictionary.DataDictionary(DataDictionary) failed to clone dictionary - exception not expected");
		}
	}
	
	@Test
	public void testDataDictionary_CloneDataDictionary_From_Uninitialized()
	{
		TestUtilities.printTestHead("testDataDictionary_CloneDataDictionary_From_Uninitialized()","Test to clone DataDictonary from another uninitialized DataDictionary instance");
		
		DataDictionary uninitializedDataDictionary = EmaFactory.createDataDictionary();
		
		try
		{
			DataDictionary dataDictionary = EmaFactory.createDataDictionary(uninitializedDataDictionary);
			TestUtilities.checkResult( dataDictionary.entries().size() == 0, "The number of DictionaryEntry must be zero"  );
			TestUtilities.checkResult( dataDictionary.enumTables().size() == 0, "The number of EnumTypeTable must be zero"  );
			TestUtilities.checkResult(dataDictionary.toString().equals("DataDictionary is not initialized"), "Clone from uninitialized DataDictionary");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(false, "DataDictionary.DataDictionary(DataDictionary) failed to clone from uninitialized DataDictionary - exception not expected");
		}
	}
	
	@Test
	public void testDataDictionary_toString()
	{
		TestUtilities.printTestHead("testDataDictionary_toString()", "Test to call toString() from an instance of DataDictionary loading from local file.");

		String toStringValue = globalDataDictionary.toString();

		TestUtilities.checkResult(toStringValue.indexOf("Data Dictionary Dump: MinFid=-22 MaxFid=32766 NumEntries 13860") != -1, "Calling DataDictionary.toString() to check for summary data");

		TestUtilities.checkResult(toStringValue.indexOf("Version=\"4.20.03\"") != -1, "Calling DataDictionary.toString() to check field dictionary version");

		TestUtilities.checkResult(toStringValue.indexOf("RT_Version=\"4.20.03\"") != -1, "Calling DataDictionary.toString() to check record tempalte version");

		TestUtilities.checkResult(toStringValue.indexOf("DT_Version=\"13.81\"") != -1, "Calling DataDictionary.toString() to check display template version");

		TestUtilities.checkResult(toStringValue.indexOf("Fid=1 'PROD_PERM' 'PERMISSION' Type=1 RippleTo=0 Len=5 EnumLen=0 RwfType=4 RwfLen=2") != -1,
			"Calling DataDictionary.toString() to check the first fied dictionary");

		TestUtilities.checkResult(toStringValue.indexOf("Fid=32761 'CD_CEILING' 'CD_CEILING' Type=1 RippleTo=0 Len=10 EnumLen=0 RwfType=4 RwfLen=5") != -1,
			"Calling DataDictionary.toString() to check the last field dictionary");

		TestUtilities.checkResult(toStringValue.indexOf("value=1 display=\"ASE\" meaning=\"NYSE AMEX\"") != -1, "Calling DataDictionary.toString() to check the first enum value");

		TestUtilities.checkResult(toStringValue.lastIndexOf("(Referenced by Fid 14157)") != -1, "DataDictionary.toString() to check fid of the last enum value");

		TestUtilities.checkResult(toStringValue.lastIndexOf("value=1 display=\"AUT  \" meaning=\"Authorised\"") != -1, "Calling DataDictionary.toString() to check the last enum value");
	}
	
	@Test
	public void testDataDictionary_ExtractDictionaryType()
	{
		TestUtilities.printTestHead("testDataDictionary_ExtractDictionaryType()","Test to extract dictionary type from dictionary payload's summary data.\n");

		int dictionaryType = 5;

		Series series = EmaFactory.createSeries();

		ElementList summaryData = EmaFactory.createElementList();
		summaryData.add(EmaFactory.createElementEntry().buffer("Version", ByteBuffer.wrap("40.20".getBytes())));
		summaryData.add(EmaFactory.createElementEntry().uintValue("Type", dictionaryType));
		summaryData.add(EmaFactory.createElementEntry().intValue("DictionaryId", 5));
		
		series.summaryData(summaryData).totalCountHint(0).add(EmaFactory.createSeriesEntry().elementList(EmaFactory.createElementList()));

		TestUtilities.checkResult(EmaFactory.createDataDictionary().extractDictionaryType(series) == dictionaryType, "DataDictionary.extractDictionaryType()");
	}
	
	@Test
	public void testDataDictionary_EnumTypeDef_encode_with_fragmentation()
	{
		TestUtilities.printTestHead("testDataDictionary_EnumTypeDef_encode_with_fragmentation()", "Test to encode and decode enumerated types payload with fragmentation size.\n");

		int fragmentationSize = 12800;
		int currentCount = 0;
		DataDictionary dataDictionary = EmaFactory.createDataDictionary();

		Series series = EmaFactory.createSeries();
		int result;

		try
		{
			while (true)
			{
				result = globalDataDictionary.encodeEnumTypeDictionary(series, currentCount, EmaRdm.DICTIONARY_VERBOSE, fragmentationSize);
				
				try
				{
					dataDictionary.decodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_VERBOSE);
				}
				catch (OmmException excp)
				{
					TestUtilities.checkResult(false, "DataDictionary.decodeEnumTypeDictionary() failed to decode enumerated type dictionary - exception not expected");
				}

				if (result != globalDataDictionary.enumTables().size())
				{
					if ( dataDictionary.enumTables().size() != result )
					{
						TestUtilities.checkResult(false, "DataDictionary.enumTables().size() must be equal to the index of current table count of multi-part payload");
					}
				}

				if (series.hasTotalCountHint())
				{
					TestUtilities.checkResult(series.totalCountHint() == globalDataDictionary.enumTables().size(), "Series.TotalCountHint() of the first multi-part payload is equal to the number of EnumTable of the globalDataDictionary");
				}
				
				currentCount = result;

				if (result == globalDataDictionary.enumTables().size())
					break;
			}

			TestUtilities.checkResult(dataDictionary.enumTables().size() == globalDataDictionary.enumTables().size() , "DataDictionary.enumTables().size() of the last multipart payload is equal with the globalDataDictionary.");

		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(false, "DataDictionary.encodeEnumTypeDictionary(fragmentationSize) failed to encode enumerated type dictionary - exception not expected");
		}
	}
	
	@Test
	public void testDictionaryEntry_toString()
	{
		TestUtilities.printTestHead("testDictionaryEntry_toString()", "Test to call toString() from an instance of DictionaryEntry");

		DictionaryEntry dictionaryEntry = globalDataDictionary.entry(14154);

		StringBuilder expected = new StringBuilder(256);
		expected.append("Fid=14154 'OPEN_GRADE' 'OPEN GRADE' Type=6 RippleTo=0 Len=3 EnumLen=5 RwfType=14 RwfLen=1");
		expected.append("\n\nEnum Type Table:\n").
			append("(Referenced by Fid 14145)\n").
			append("(Referenced by Fid 14146)\n").
			append("(Referenced by Fid 14147)\n").
			append("(Referenced by Fid 14148)\n").
			append("(Referenced by Fid 14149)\n").
			append("(Referenced by Fid 14150)\n").
			append("(Referenced by Fid 14151)\n").
			append("(Referenced by Fid 14152)\n").
			append("(Referenced by Fid 14153)\n").
			append("(Referenced by Fid 14154)\n").
			append("value=0 display=\"     \" meaning=\"Undefined\"\n").
			append("value=1 display=\"    0\" meaning=\"Valid\"\n").
			append("value=2 display=\"    1\" meaning=\"Suspect\"\n");

		TestUtilities.checkResult(dictionaryEntry.toString().equals(expected.toString()), "Calling DictionaryEntry.toString() for a fid");
	}

	@Test
	public void testDictionaryEntry_hasEnumTable_getEnumTable()
	{
		TestUtilities.printTestHead("testDictionaryEntry_hasEnumTable_getEnumTable()","Test to check and get EnumTable from DictionaryEntry.");

		DictionaryEntry dictionaryEntryFid1 = globalDataDictionary.entry(1);

		TestUtilities.checkResult(dictionaryEntryFid1.hasEnumTypeTable() == false, "Calling DictionaryEntry.hasEnumTypeTable() for non existing EnumTypeTable.");

		try
		{
			dictionaryEntryFid1.enumTypeTable();
			TestUtilities.checkResult(false, "Calling DictionaryEntry::getEnumTypeTable() for non existing EnumTypeTable - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The EnumTypeTable does not exist for the Field ID 1'"),
				"Calling DictionaryEntry::getEnumType() for non existing EnumTypeTable - exception expected");
		}

		DictionaryEntry dictionaryEntryFid4 = globalDataDictionary.entry(4);

		TestUtilities.checkResult(dictionaryEntryFid4.hasEnumTypeTable()== true, "Calling DictionaryEntry.hasEnumTypeTable() for an existing EnumTypeTable.");

		try
		{
			dictionaryEntryFid4.enumTypeTable();
			TestUtilities.checkResult(true, "Calling DictionaryEntry::enumType() for an existing EnumType - exception not expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(false, "Calling DictionaryEntry::getEnumTypeTable() for an existing EnumTypeTable - exception not expected");
		}
	}

	@Test
	public void testDictionaryEntry_hasEnumType_getEnumType()
	{
		TestUtilities.printTestHead("testDictionaryEntry_hasEnumType_getEnumType()","Test to check and get EnumType from DitionaryEntry.");

		DictionaryEntry dictionaryEntryFid1 = globalDataDictionary.entry(1);

		TestUtilities.checkResult(dictionaryEntryFid1.hasEnumType(5555) == false, "Calling DictionaryEntry.hasEnumType() for non existing EnumType.");

		try
		{
			dictionaryEntryFid1.enumType(5555);
			TestUtilities.checkResult(false, "Calling DictionaryEntry.enumType() for non existing EnumType - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The enum value 5555 for the Field ID 1 does not exist in enumerated type definitions'"),
				"Calling DictionaryEntry::getEnumType() for non existing EnumType - exception expected");
		}

		DictionaryEntry dictionaryEntryFid4 = globalDataDictionary.entry(4);

		TestUtilities.checkResult(dictionaryEntryFid4.hasEnumType(5) == true, "Calling DictionaryEntry.hasEnumType() for an existing EnumType.");

		try
		{
			dictionaryEntryFid4.enumType(5);
			TestUtilities.checkResult(true, "Calling DictionaryEntry::getEnumType() for an existing EnumType - exception not expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(false, "Calling DictionaryEntry.enumType() for an existing EnumType - exception not expected");
		}
	}
	
	@Test
	public void testDataDictionary_hasEntry_getEntry_by_FieldID()
	{
		TestUtilities.printTestHead("testDataDictionary_hasEntry_getEntry_by_FieldID()","Test to check and get DictionaryEntry by field ID from DataDictionary.");

		TestUtilities.checkResult(globalDataDictionary.hasEntry(-555) == false, "Check from non existing DictionaryEntry");

		try
		{
			globalDataDictionary.entry(-555);
			TestUtilities.checkResult(false, "Calling DictionaryUtility.entry(fid) from non existing DictionaryEntry - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The Field ID -555 does not exist in the field dictionary'"),
				"DictionaryUtility.entry(fid) from non existing  DictionaryEntry - exception expected");
		}

		TestUtilities.checkResult(globalDataDictionary.hasEntry(3) == true, "Check from existing DictionaryEntry");

		try
		{
			DictionaryEntry dictionaryEntry = globalDataDictionary.entry(3);
			TestUtilities.checkResult(dictionaryEntry.acronym().equals("DSPLY_NAME"), "Calling DictionaryEntry.acronym() is correct for FID 3");
			TestUtilities.checkResult(true, "Calling DictionaryUtility.entry(fid) from an existing DictionaryEntry - exception not expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(false,"Calling DictionaryUtility.entry(fid) from an existing DictionaryEntry - exception not expected");
		}
	}
	
	@Test
	public void testDataDictionary_hasEntry_getEntry_by_FieldName()
	{
		TestUtilities.printTestHead("testDataDictionary_hasEntry_getEntry_by_FieldName()","Test to check and get DictionaryEntry by field name from DataDictionary.");

		TestUtilities.checkResult(globalDataDictionary.hasEntry("UNKNOWN_FID") == false, "Check from non existing DictionaryEntry");

		try
		{
			globalDataDictionary.entry("UNKNOWN_FID");
			TestUtilities.checkResult(false, "Calling DictionaryUtility.entry(name) from non existing DictionaryEntry - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType() == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals("Exception Type='OmmInvalidUsageException', Text='The Field name UNKNOWN_FID does not exist in the field dictionary'"),
				"DictionaryUtility.entry(name) from non existing  DictionaryEntry - exception expected");
		}

		TestUtilities.checkResult(globalDataDictionary.hasEntry("DSPLY_NAME") == true, "Check from existing DictionaryEntry");

		try
		{
			DictionaryEntry dictionaryEntry = globalDataDictionary.entry("DSPLY_NAME");
			TestUtilities.checkResult(dictionaryEntry.fid() == 3, "Calling DictionaryEntry.fid() is correct for field name DSPLY_NAME");
			TestUtilities.checkResult(true, "Calling DictionaryUtility.entry(fid) from an existing DictionaryEntry - exception not expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(false,"Calling DictionaryUtility.entry(fid) from an existing DictionaryEntry - exception not expected");
		}
	}

	@Test
	public void testDataDictionary_hasEnumType_getEnumType()
	{
		TestUtilities.printTestHead("testDataDictionary_hasEnumType_getEnumType()","Test to check and get EnumType from DataDictionary.");
	
		TestUtilities.checkResult(globalDataDictionary.hasEnumType( 4, 5555 ) == false, "Check from non existing EnumType");

		try
		{
			globalDataDictionary.enumType(4, 5555);
			TestUtilities.checkResult(false, "Calling DictionaryUtility.enumType(fid, enumValue) from non existing EnumType - exception expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(excp.exceptionType()  == OmmException.ExceptionType.OmmInvalidUsageException, "OmmException.exceptionType()");
			TestUtilities.checkResult(excp.toString().equals( "Exception Type='OmmInvalidUsageException', Text='The enum value 5555 for the Field ID 4 does not exist in enumerated type definitions'"),
				"DictionaryUtility.enumType(fid, enumvalue) from non existing EnumType - exception expected");
		}

		TestUtilities.checkResult(globalDataDictionary.hasEnumType(4, 15) == true, "Check from existing DictionaryEntry");

		try
		{
			globalDataDictionary.enumType(4, 15);
			TestUtilities.checkResult(true, "Calling DictionaryUtility.enumType(fid, enumvalue) from an existing EnumType - exception not expected");
		}
		catch (OmmException excp)
		{
			TestUtilities.checkResult(false, "Calling DictionaryUtility.enumType(fid, enumvalue) from an existing EnumType - exception not expected");
		}
	}
}
