/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;

using Xunit.Abstractions;

using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;

namespace LSEG.Ema.Access.Tests;

public class DataDictionaryTests
{
    #region Helpers

    public const string FIELD_DICTIONARY_FILENAME = "../../../ComplexTypeTests/RDMFieldDictionary";
    public const string ENUM_TABLE_FILENAME = "../../../ComplexTypeTests/enumtype.def";

    private static Eta.Codec.DataDictionary m_GlobalEtaDataDictionary;
    private static Rdm.DataDictionary m_GlobalDataDictionary;

    private EmaObjectManager manager = new EmaObjectManager();

    private ITestOutputHelper output;

    static DataDictionaryTests()
    {
        m_GlobalEtaDataDictionary = new();
        m_GlobalDataDictionary = new();

        m_GlobalEtaDataDictionary.Clear();

        if (m_GlobalEtaDataDictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILENAME, out var codecError) < Eta.Codec.CodecReturnCode.SUCCESS)
        {
            Assert.True(false, $"Failed to load dictionary information with Eta.Codec.DataDictionary: {codecError}");
        }

        try
        {
            m_GlobalDataDictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILENAME);
        }
        catch (OmmException excp)
        {
            Assert.True(false,
                $"DataDictionary.LoadFieldDictionary() failed to load dictionary information - exception not expected: {excp}");
        }

        if (m_GlobalEtaDataDictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILENAME, out var rsslError) < Eta.Codec.CodecReturnCode.SUCCESS)
        {
            Assert.True(false,
                $"Failed to load enumerated types information with Eta.Codec.DataDictionary: {rsslError}");
        }

        try
        {
            m_GlobalDataDictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILENAME);
        }
        catch (OmmException excp)
        {
            Assert.True(false,
                $"DataDictionary.LoadEnumTypeDictionary() failed to load denumerated types information - exception not expected: {excp}");
        }
    }

    public DataDictionaryTests(ITestOutputHelper output)
    {
        this.output = output;
    }

    private static void AssertEqualEnums(Eta.Codec.IEnumType rsslEnumType, EnumType enumType, bool payloadOnly)
    {
        if (rsslEnumType != null)
        {
            Assert.Equal(rsslEnumType.Value, enumType.Value);

            Assert.Equal(rsslEnumType.Display.ToString(), enumType.Display);

            if (!payloadOnly)
                Assert.Equal(rsslEnumType.Meaning.ToString(), enumType.Meaning);

            return;
        }

        Assert.True(false);
    }

    public static bool AssertEqualEnumTables(Eta.Codec.IEnumTypeTable rsslEnumTypeTable, Ema.Rdm.EnumTypeTable enumTypeTable, bool payloadOnly)
    {
        if (rsslEnumTypeTable != null)
        {
            Eta.Codec.IEnumType rsslEnumType;
            List<EnumType> enumTypeList = enumTypeTable.EnumTypes;
            int enumTypeListIndex = 0;

            for (int index = 0; index <= rsslEnumTypeTable.MaxValue; index++)
            {
                rsslEnumType = rsslEnumTypeTable.EnumTypes[index];

                if (rsslEnumType != null)
                {
                    AssertEqualEnums(rsslEnumType, enumTypeList[enumTypeListIndex], payloadOnly);

                    enumTypeListIndex++;
                }
            }

            List<int> fieldIdList = enumTypeTable.FidReferences;

            for (int index = 0; index < rsslEnumTypeTable.FidReferences.Count; index++)
            {
                Assert.Equal(rsslEnumTypeTable.FidReferences[index], fieldIdList[index]);
            }

            return true;
        }

        return false;
    }

    private static bool AssertEqualDictionaryEntries(Eta.Codec.IDictionaryEntry rsslDictionaryEntry, DictionaryEntry dictionaryEntry, StringBuilder failedText,
            bool payloadOnly)
    {
        if (rsslDictionaryEntry != null)
        {
            Assert.Equal(rsslDictionaryEntry.GetAcronym().ToString(), dictionaryEntry.Acronym);
            Assert.Equal(rsslDictionaryEntry.GetDdeAcronym().ToString(), dictionaryEntry.DdeAcronym);
            Assert.Equal(rsslDictionaryEntry.GetEnumLength(), dictionaryEntry.EnumLength);
            Assert.Equal(rsslDictionaryEntry.GetFid(), dictionaryEntry.FieldId);
            Assert.Equal(rsslDictionaryEntry.GetFieldType(), dictionaryEntry.FieldType);
            Assert.Equal(rsslDictionaryEntry.GetLength(), dictionaryEntry.Length);

            if (rsslDictionaryEntry.GetEnumTypeTable() != null)
            {
                if (AssertEqualEnumTables(rsslDictionaryEntry.GetEnumTypeTable(), dictionaryEntry.GetEnumTypeTable(), payloadOnly) != true)
                    return false;
            }
            else
            {
                if (dictionaryEntry.HasEnumTypeTable)
                {
                    failedText.Append(" Eta's EnumTypeTable is null while Ema's EnumTypeTable exists");
                    return false;
                }
            }

            Assert.Equal(rsslDictionaryEntry.GetRippleToField(), dictionaryEntry.RippleToField);
            Assert.Equal(rsslDictionaryEntry.GetRwfLength(), dictionaryEntry.RwfLength);
            Assert.Equal(rsslDictionaryEntry.GetRwfType(), dictionaryEntry.RwfType);

            return true;
        }
        else
        {
            failedText.Clear();
            failedText.Append("Eta.Codec.DictionaryEntry is null");
        }

        return false;
    }

    internal static void AssertEqualDataDictionary(Eta.Codec.DataDictionary rsslDataDictionary, Rdm.DataDictionary dataDictionary, bool payloadOnly)
    {
        if (!payloadOnly)
        {
            Assert.Equal(rsslDataDictionary.InfoEnumDate.ToString(), dataDictionary.EnumDate);
            Assert.Equal(rsslDataDictionary.InfoEnumDesc.ToString(), dataDictionary.EnumDescription);
            Assert.Equal(rsslDataDictionary.InfoEnumFilename.ToString(), dataDictionary.EnumFilename);
            Assert.Equal(rsslDataDictionary.InfoFieldBuild.ToString(), dataDictionary.FieldBuild);
            Assert.Equal(rsslDataDictionary.InfoFieldDate.ToString(), dataDictionary.FieldDate);
            Assert.Equal(rsslDataDictionary.InfoFieldDesc.ToString(), dataDictionary.FieldDescription);
            Assert.Equal(rsslDataDictionary.InfoFieldFilename.ToString(), dataDictionary.FieldFilename);
        }

        Assert.Equal(rsslDataDictionary.InfoEnumDTVersion.ToString(), dataDictionary.EnumDisplayTemplateVersion);
        Assert.Equal(rsslDataDictionary.InfoEnumRTVersion.ToString(), dataDictionary.EnumRecordTemplateVersion);
        Assert.Equal(rsslDataDictionary.InfoFieldVersion.ToString(), dataDictionary.FieldVersion);
        Assert.Equal(rsslDataDictionary.InfoDictionaryId, dataDictionary.DictionaryId);
        Assert.Equal(rsslDataDictionary.NumberOfEntries, dataDictionary.Entries().Count);
        Assert.Equal(rsslDataDictionary.EnumTableCount, dataDictionary.EnumTables().Count);
        Assert.Equal(rsslDataDictionary.MaxFid, dataDictionary.MaxFid);
        Assert.Equal(rsslDataDictionary.MinFid, dataDictionary.MinFid);

        // Comparing for all DictionaryEntry from DataDictionary.entries()
        Eta.Codec.IDictionaryEntry rsslDictionaryEntry;
        StringBuilder errorText = new StringBuilder(256);
        bool result = false;
        int index = 0;

        List<DictionaryEntry> dictionaryEntryList = dataDictionary.Entries();

        for (int fid = rsslDataDictionary.MinFid; fid <= rsslDataDictionary.MaxFid; fid++)
        {
            rsslDictionaryEntry = rsslDataDictionary.Entry(fid);

            if (rsslDictionaryEntry != null)
            {
                result = AssertEqualDictionaryEntries(rsslDictionaryEntry, dictionaryEntryList[index], errorText, payloadOnly);
                if (result == false)
                {
                    break;
                }

                ++index;
            }
        }

        // Make sure that there is no other DictionaryEntry in dictionaryEntryList
        Assert.Equal(index, dictionaryEntryList.Count);

        if (result)
        {
            Assert.True(result, "All DictionaryEntry from DataDictionary.entries() is equal with all entries in Eta.Codec.DataDictionary");
        }
        else
        {
            Assert.True(result, errorText.ToString());
        }

        errorText.Clear();

        // Comparing for all EnumTypeTable from DataDictionary.enumTables()
        index = 0;
        List<EnumTypeTable> enumTypeTableList = dataDictionary.EnumTables();
        Eta.Codec.IEnumTypeTable rsslEnumTypeTable;

        for (; index < rsslDataDictionary.EnumTableCount; index++)
        {
            rsslEnumTypeTable = rsslDataDictionary.EnumTables[index];

            result = AssertEqualEnumTables(rsslEnumTypeTable, enumTypeTableList[index], payloadOnly);

            if (result == false)
            {
                break;
            }
        }

        if (result)
        {
            Assert.True(result, "All EnumTypeTable from DataDictionary.enumTables() is equal with Eta.Codec.enumTables()");
        }
        else
        {
            Assert.True(result, errorText.ToString());
        }
    }

    private static CodecReturnCode SetRsslData(Data data, Data dataEncoded, Eta.Codec.DataDictionary? dict = null)
    {
        return data.Decode(Eta.Codec.Codec.MajorVersion(), Eta.Codec.Codec.MinorVersion(),
            dataEncoded.Encoder!.m_encodeIterator!.Buffer(),
            dict, null);
    }

    #endregion

    #region Tests

    [Fact]
    public void TestDataDictionary_LoadDictionaryFromFile()
    {
        output.WriteLine("TestDataDictionary_LoadDictionaryFromFile(): Test load dictionary information from local file");

        AssertEqualDataDictionary(m_GlobalEtaDataDictionary, m_GlobalDataDictionary, false);

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_LoadDictionaryFrom_Invalid_File()
    {
        output.WriteLine("TestDataDictionary_LoadDictionaryFrom_Invalid_File(): Test load dictionary information from invalid local file");

        Rdm.DataDictionary dataDictionary = new();

        try
        {
            dataDictionary.LoadFieldDictionary("Invalid_RDMFieldDictionary");
            Assert.True(false, "DataDictionary.loadFieldDictionary() with invalid RDMFieldDictionary - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal(("Exception Type='OmmInvalidUsageException', Text='Unable to load field dictionary from file named Invalid_RDMFieldDictionary\n\tCurrent working directory "
                + System.IO.Directory.GetCurrentDirectory()
                + "\n\tReason='Can't open file: Invalid_RDMFieldDictionary'', Error Code='-1'"),
                excp.ToString());
        }

        try
        {
            dataDictionary.LoadEnumTypeDictionary("Invalid_enumtype.def");
            Assert.True(false, "DataDictionary.loadEnumTypeDictionary() with invalid enumtype.def - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal((
                "Exception Type='OmmInvalidUsageException', Text='Unable to load enumerated type definition from file named Invalid_enumtype.def\n\tCurrent working directory "
                + System.IO.Directory.GetCurrentDirectory()
                + "\n\tReason='Can't open file: Invalid_enumtype.def'', Error Code='-1'"),
            excp.ToString());
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_Uninitialize()
    {
        output.WriteLine("TestDataDictionary_Uninitialize(): Test to call querying and encoding interfaces of DataDictionary while the internal RsslDataDictionary is not initialized yet.");

        Rdm.DataDictionary dataDictionary = new();

        Assert.Equal("DataDictionary is not initialized", dataDictionary.ToString());

        Assert.Empty(dataDictionary.Entries());
        Assert.Empty(dataDictionary.EnumTables());

        Assert.False(dataDictionary.HasEntry(1));
        Assert.False(dataDictionary.HasEntry("PROD_PERM"));

        try
        {
            dataDictionary.Entry(1); // Getting from non existing DictionaryEntry
            Assert.True(false, "DataDictionary.Entry(fid) cannot get DictionaryEntry - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal(excp.ToString(), ("Exception Type='OmmInvalidUsageException', Text='The field dictionary information was not loaded', Error Code='-4048'"));
        }

        try
        {
            dataDictionary.Entry("PROD_PERM"); // Getting from non existing DictionaryEntry
            Assert.True(false, "DataDictionary.Entry(name) cannot get DictionaryEntry - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The field dictionary information was not loaded', Error Code='-4048'",
                excp.ToString());
        }

        Assert.False(dataDictionary.HasEnumType(1, 1), "DataDictionary.hasEnumType(fid,value)");

        try
        {
            dataDictionary.EnumType(4, 2); // Getting from non existing EnumType
            Assert.True(false, "DataDictionary.enumType(fid,value) cannot get EnumType - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The enumerated types dictionary was not loaded', Error Code='-4048'",
                excp.ToString());
        }

        Assert.Equal(0, dataDictionary.DictionaryId);
        Assert.Equal("", dataDictionary.EnumDate);
        Assert.Equal("", dataDictionary.EnumDescription);
        Assert.Equal("", dataDictionary.EnumDisplayTemplateVersion);
        Assert.Equal("", dataDictionary.EnumFilename);
        Assert.Equal("", dataDictionary.EnumRecordTemplateVersion);
        Assert.Equal("", dataDictionary.FieldBuild);
        Assert.Equal("", dataDictionary.FieldDate);
        Assert.Equal("", dataDictionary.FieldDescription);
        Assert.Equal("", dataDictionary.FieldFilename);
        Assert.Equal("", dataDictionary.FieldVersion);
        Assert.Equal(0, dataDictionary.MaxFid);
        Assert.Equal(0, dataDictionary.MinFid);

        Series series = manager.GetOmmSeries();

        try
        {
            dataDictionary.EncodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_NORMAL);
            Assert.True(false, "DataDictionary.EncodeEnumTypeDictionary() - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The enumerated types dictionary was not loaded', Error Code='-4048'",
                excp.ToString());
        }

        int count = 0;
        series.Clear();

        try
        {
            dataDictionary.EncodeEnumTypeDictionary(series, count, EmaRdm.DICTIONARY_NORMAL, 555);
            Assert.True(false, "DataDictionary.EncodeEnumTypeDictionary(fragmentation) - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The enumerated types dictionary was not loaded', Error Code='-4048'",
                excp.ToString());
        }

        series.Clear();

        try
        {
            dataDictionary.EncodeFieldDictionary(series, EmaRdm.DICTIONARY_NORMAL);
            Assert.True(false, "DataDictionary.EncodeFieldDictionary() - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The field dictionary information was not loaded', Error Code='-4048'",
                excp.ToString());
        }

        series.Clear();

        try
        {
            dataDictionary.EncodeFieldDictionary(series, count, EmaRdm.DICTIONARY_NORMAL, 555);
            Assert.True(false, "DataDictionary.EncodeFieldDictionary(fragmentation) - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The field dictionary information was not loaded', Error Code='-4048'",
                excp.ToString());
        }

        series.ClearAndReturnToPool_All();

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_FieldDictionary_EnumTypeDef_Encode_Decode()
    {
        output.WriteLine("TestDataDictionary_FieldDictionary_EnumTypeDef_Encode_Decode(): Test encode and decode dictionary payload.");

        Series series = manager.GetOmmSeries();
        Series decodeSeries = manager.GetOmmSeries();

        try
        {
            m_GlobalDataDictionary.EncodeFieldDictionary(series, EmaRdm.DICTIONARY_VERBOSE);
        }
        catch (OmmException excp)
        {
            Assert.True(false,
                $"DataDictionary.EncodeFieldDictionary() failed to encode field dictionary information - exception not expected: {excp}");
        }

        SetRsslData(decodeSeries, series);

        Rdm.DataDictionary dataDictionary = new();

        try
        {
            dataDictionary.DecodeFieldDictionary(decodeSeries, EmaRdm.DICTIONARY_VERBOSE);
        }
        catch (OmmException excp)
        {
            Assert.True(false, $"DataDictionary.DecodeFieldDictionary() failed to decode field dictionary information - exception not expected: {excp}");
        }

        try
        {
            m_GlobalDataDictionary.EncodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_VERBOSE);
        }
        catch (OmmException excp)
        {
            Assert.True(false, $"DataDictionary.EncodeFieldDictionary() failed to encode field dictionary information - exception not expected: {excp}");
        }

        decodeSeries.Clear();

        SetRsslData(decodeSeries, series);

        try
        {
            dataDictionary.DecodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_VERBOSE);
        }
        catch (OmmException excp)
        {
            Assert.True(false, $"DataDictionary.DecodeEnumTypeDictionary() failed to decode field dictionary information - exception not expected: {excp}");
        }

        AssertEqualDataDictionary(m_GlobalEtaDataDictionary, m_GlobalDataDictionary, true);

        series.ClearAndReturnToPool_All();
        decodeSeries.ClearAndReturnToPool_All();

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_FieldDictionary_Encode_with_Fragmentation()
    {
        output.WriteLine("TestDataDictionary_FieldDictionary_Encode_with_Fragmentation(): Test encode and decode Field dictionary payload with fragmentation size.");

        int fragmentationSize = 5120;
        int currentFid = m_GlobalDataDictionary.MinFid;
        Rdm.DataDictionary dataDictionary = new();

        Series series = manager.GetOmmSeries();
        Series decodeSeries = manager.GetOmmSeries();

        try
        {
            while (true)
            {
                currentFid = m_GlobalDataDictionary.EncodeFieldDictionary(series, currentFid, EmaRdm.DICTIONARY_VERBOSE, fragmentationSize);

                SetRsslData(decodeSeries, series);

                try
                {
                    dataDictionary.DecodeFieldDictionary(decodeSeries, EmaRdm.DICTIONARY_VERBOSE);
                }
                catch (OmmException excp)
                {
                    Assert.True(false, $"DataDictionary.DecodeFieldDictionary() failed to decode field dictionary information - exception not expected: {excp}");
                }

                Assert.False(dataDictionary.MaxFid > currentFid, "DataDictionary.MaxFid() must be less than the current fid of multi-part payload");

                if (decodeSeries.HasTotalCountHint)
                {
                    Assert.Equal(decodeSeries.TotalCountHint(), m_GlobalDataDictionary.Entries().Count);
                    Assert.Equal(m_GlobalDataDictionary.MinFid, dataDictionary.MinFid);
                }

                if (dataDictionary.MaxFid == currentFid)
                    break;
            }

            Assert.Equal(m_GlobalDataDictionary.MaxFid, dataDictionary.MaxFid);

        }
        catch (OmmException excp)
        {
            Assert.True(false,
                $"DataDictionary.EncodeFieldDictionary(fragmentationSize) failed to encode field dictionary information - exception not expected: {excp}");
        }

        series.ClearAndReturnToPool_All();
        decodeSeries.ClearAndReturnToPool_All();

        CheckEtaGlobalPoolSizes();
    }

    [Fact(Skip = "Obsolete")]
    public void TestDataDictionary_DictionaryUtility()
    {
        output.WriteLine("TestDataDictionary_DictionaryUtility(): Test to retrieve DataDictionary from the FieldList for decoding.");

        FieldList fieldList = new FieldList();

        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.Complete();

        FieldList decodeFieldList = new FieldList(new EmaObjectManager());

        SetRsslData(decodeFieldList, fieldList, m_GlobalEtaDataDictionary);

        AssertEqualDataDictionary(m_GlobalEtaDataDictionary, DictionaryUtility.ExtractDataDictionary(decodeFieldList), true);

        Series series = manager.GetOmmSeries();
        try
        {
            DictionaryUtility.ExtractDataDictionary(decodeFieldList).DecodeFieldDictionary(series, EmaRdm.DICTIONARY_VERBOSE);
            Assert.True(false, "Calling DataDictionary.DecodeFieldDictionary() - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='This DataDictionary instance is used for query data dictionary information only', Error Code='-4048'",
                excp.ToString());
        }

        series.ClearAndReturnToPool_All();

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_Clear_For_File()
    {
        output.WriteLine("TestDataDictionary_Clear_For_File(): Test to load data dictionary information loading from file after clearing it.");

        Rdm.DataDictionary dataDictionary = new();

        try
        {
            dataDictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILENAME);
            dataDictionary.Entries();
            dataDictionary.EnumTables();
            dataDictionary.Clear();
            Assert.Empty(dataDictionary.Entries());
            Assert.Empty(dataDictionary.EnumTables());

            Assert.False(dataDictionary.HasEntry(1));
            Assert.False(dataDictionary.HasEntry("PROD_PERM"));

            try
            {
                dataDictionary.Entry(1); // Getting from non existing DictionaryEntry
                Assert.True(false, "DataDictionary.Entry(fid) cannot get DictionaryEntry - exception expected");
            }
            catch (OmmException excp)
            {
                Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
                Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The field dictionary information was not loaded', Error Code='-4048'",
                    excp.ToString());
            }

            try
            {
                dataDictionary.Entry("PROD_PERM"); // Getting from non existing DictionaryEntry
                Assert.True(false, "DataDictionary.Entry(name) cannot get DictionaryEntry - exception expected");
            }
            catch (OmmException excp)
            {
                Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
                Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The field dictionary information was not loaded', Error Code='-4048'",
                    excp.ToString());
            }

            Assert.False(dataDictionary.HasEnumType(4, 2), "Checking DataDictionary.HasEnumType(fid, enum value) after calling the Clear()");

            try
            {
                dataDictionary.EnumType(4, 2); // Getting from non existing EnumType
                Assert.True(false, "DataDictionary.EnumType(fid,value) cannot get EnumType - exception expected");
            }
            catch (OmmException excp)
            {
                Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
                Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The enumerated types dictionary was not loaded', Error Code='-4048'",
                    excp.ToString());
            }

            dataDictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILENAME);
            Assert.Equal(dataDictionary.Entries().Count, m_GlobalEtaDataDictionary.NumberOfEntries);
        }
        catch (OmmException excp)
        {
            Assert.True(false,
                $"Calling DataDictionary.LoadFieldDictionary() multiple times after clearing the data dictionary - exception not expected: {excp}");
        }

        try
        {
            dataDictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILENAME);
            Assert.NotEmpty(dataDictionary.Entries());
            Assert.NotEmpty(dataDictionary.EnumTables());
            dataDictionary.Clear();
            Assert.Empty(dataDictionary.Entries());
            Assert.Empty(dataDictionary.EnumTables());
            dataDictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILENAME);
            Assert.Equal(dataDictionary.EnumTables().Count, m_GlobalEtaDataDictionary.EnumTableCount);
        }
        catch (OmmException excp)
        {
            Assert.True(false,
                $"Calling DataDictionary.LoadEnumTypeDictionary() multiple times after clearing the data dictionary - exception not expected: {excp}");
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_Clear_For_Payload()
    {
        output.WriteLine("TestDataDictionary_Clear_For_Payload(): Test to load data dictionary information decoding from dictionary payload after clearing it.");

        Rdm.DataDictionary dataDictionary = new();

        Series series = new();
        Series decodeSeries = new();

        m_GlobalDataDictionary.EncodeFieldDictionary(series, EmaRdm.DICTIONARY_NORMAL);

        try
        {
            SetRsslData(decodeSeries, series);
            dataDictionary.DecodeFieldDictionary(series, EmaRdm.DICTIONARY_NORMAL);
            dataDictionary.Clear();
            Assert.Empty(dataDictionary.Entries());
            Assert.Empty(dataDictionary.EnumTables());
            dataDictionary.DecodeFieldDictionary(series, EmaRdm.DICTIONARY_NORMAL);
            Assert.Equal(dataDictionary.Entries().Count, m_GlobalEtaDataDictionary.NumberOfEntries);
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.True(false,
                $"Calling DataDictionary.DecodeFieldDictionary() multiple times after clearing the data dictionary - exception not expected: {excp}");
        }

        m_GlobalDataDictionary.EncodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_NORMAL);

        try
        {
            SetRsslData(decodeSeries, series);
            dataDictionary.DecodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_NORMAL);
            dataDictionary.Clear();
            Assert.Empty(dataDictionary.Entries());
            Assert.Empty(dataDictionary.EnumTables());
            dataDictionary.DecodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_NORMAL);
            Assert.Equal(dataDictionary.EnumTables().Count, m_GlobalEtaDataDictionary.EnumTableCount);
        }
        catch (OmmException excp)
        {
            Assert.True(false,
                $"Calling DataDictionary.DecodeEnumTypeDictionary() multiple times after clearing the data dictionary - exception not expected: {excp}");
        }

        series.ClearAndReturnToPool_All();
        decodeSeries.ClearAndReturnToPool_All();

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_Duplicate_FromFile()
    {
        output.WriteLine("TestDataDictionary_Duplicate_FromFile(): Test to load duplicate data dictionary from file.");

        Rdm.DataDictionary dataDictionary = new();

        try
        {
            dataDictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILENAME);
            dataDictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILENAME);
            Assert.True(false, "Calling DataDictionary.LoadFieldDictionary() multiple times - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Contains("Unable to load field dictionary from file named " + FIELD_DICTIONARY_FILENAME, excp.Message);
            Assert.Contains("Reason='Duplicate definition for fid -32768 (Line=41).'", excp.Message);
        }

        try
        {
            dataDictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILENAME);
            dataDictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILENAME);
            Assert.Equal(dataDictionary.EnumTables().Count, m_GlobalEtaDataDictionary.EnumTableCount);
            Assert.True(false, "Calling DataDictionary.loadEnumTypeDictionary() multiple times - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Contains("Unable to load enumerated type definition from file named " + ENUM_TABLE_FILENAME, excp.Message);
            Assert.Contains("Reason='FieldId 32767 has duplicate Enum Table reference'", excp.Message);
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_Duplicate_FromDictionaryPayload()
    {
        output.WriteLine("TestDataDictionary_Duplicate_FromDictionaryPayload(): Test to load duplicate data dictionary from dictionary payload.");

        Rdm.DataDictionary dataDictionary = new();

        Series series = manager.GetOmmSeries();
        Series decodeSeries = manager.GetOmmSeries();

        m_GlobalDataDictionary.EncodeFieldDictionary(series, EmaRdm.DICTIONARY_NORMAL);

        try
        {
            SetRsslData(decodeSeries, series);
            dataDictionary.DecodeFieldDictionary(series, EmaRdm.DICTIONARY_NORMAL);
            dataDictionary.DecodeFieldDictionary(series, EmaRdm.DICTIONARY_NORMAL);
            Assert.True(false, "Calling DataDictionary.DecodeFieldDictionary() multiple times - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.NotEqual("Failed to decode the field dictionary information. Reason='Duplicate definition for fid -22.'", excp.Message);
        }

        m_GlobalDataDictionary.EncodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_NORMAL);

        try
        {
            SetRsslData(decodeSeries, series);
            dataDictionary.DecodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_NORMAL);
            dataDictionary.DecodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_NORMAL);
            Assert.True(false, "Calling DataDictionary.DecodeEnumTypeDictionary() multiple times - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.NotEqual("Failed to decode the enumerated types dictionary. Reason='FieldId 4 has duplicate Enum Table reference'", excp.Message);
        }

        series.ClearAndReturnToPool_All();
        decodeSeries.ClearAndReturnToPool_All();

        CheckEtaGlobalPoolSizes();
    }

    [Fact(Skip = "Obsolete")]
    public void TestDataDictionary_DictionaryUtility_FromEncodingOnly()
    {
        output.WriteLine("TestDataDictionary_DictionaryUtility_FromEncodingOnly(): Test to retrieve DataDictionary from the FieldList from encoding only.");

        FieldList fieldList = new();

        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        try
        {
            DictionaryUtility.ExtractDataDictionary(fieldList);
            Assert.True(false, "DictionaryUtility.dataDictionary(FieldList) - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='Failed to extract DataDictionary from the passed in FieldList', Error Code='-22'",
                excp.ToString());
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_CloneDataDictionary()
    {
        output.WriteLine("TestDataDictionary_CloneDataDictionary(): Test to clone DataDictonary from another DataDictionary instance.");

        try
        {
            Rdm.DataDictionary dataDictionary = new(m_GlobalDataDictionary);
            AssertEqualDataDictionary(m_GlobalEtaDataDictionary, dataDictionary, true);
        }
        catch (OmmException excp)
        {
            Assert.True(false, $"DataDictionary.DataDictionary(DataDictionary) failed to clone dictionary - exception not expected: {excp}");
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_CloneDataDictionary_From_Uninitialized()
    {
        output.WriteLine("TestDataDictionary_CloneDataDictionary_From_Uninitialized(): Test to clone DataDictonary from another uninitialized DataDictionary instance");

        Rdm.DataDictionary uninitializedDataDictionary = new();

        try
        {
            Rdm.DataDictionary dataDictionary = new(uninitializedDataDictionary);
            Assert.Empty(dataDictionary.Entries());
            Assert.Empty(dataDictionary.EnumTables());
            Assert.Equal("DataDictionary is not initialized", dataDictionary.ToString());
        }
        catch (OmmException excp)
        {
            Assert.True(false, $"DataDictionary.DataDictionary(DataDictionary) failed to clone from uninitialized DataDictionary - exception not expected: {excp}");
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_ToString()
    {
        output.WriteLine("TestDataDictionary_ToString(): Test to call toString() from an instance of DataDictionary loading from local file.");

        string toStringValue = m_GlobalDataDictionary.ToString();

        Assert.Contains("Data Dictionary Dump: MinFid=-32768 MaxFid=32767 NumEntries 15047", toStringValue);
        Assert.Contains("Version=\"4.20.29\"", toStringValue);
        Assert.Contains("RT_Version=\"4.20.29\"", toStringValue);
        Assert.Contains("DT_Version=\"17.81\"", toStringValue);
        Assert.Contains("Fid=1 'PROD_PERM' 'PERMISSION' Type=1 RippleTo=0 Len=5 EnumLen=0 RwfType=4 RwfLen=2", toStringValue);
        Assert.Contains("Fid=32761 'CD_CEILING' 'CD_CEILING' Type=1 RippleTo=0 Len=10 EnumLen=0 RwfType=4 RwfLen=5", toStringValue);
        Assert.Contains("value=1 display=\"ASE\" meaning=\"NYSE AMEX\"", toStringValue);
        Assert.Contains("(Referenced by Fid 14157)", toStringValue);
        Assert.Contains("value=1 display=\"AUT  \" meaning=\"Authorised\"", toStringValue);

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_ExtractDictionaryType()
    {
        output.WriteLine("TestDataDictionary_ExtractDictionaryType(): Test to extract dictionary type from dictionary payload's summary data.");

        uint dictionaryType = 5;

        Series series = manager.GetOmmSeries();

        ElementList summaryData = new();
        summaryData.AddBuffer("Version", new EmaBuffer(Encoding.ASCII.GetBytes("40.20")));
        summaryData.AddUInt("Type", dictionaryType);
        summaryData.AddInt("DictionaryId", 5);
        summaryData.Complete();

        series.SummaryData(summaryData).TotalCountHint(0).AddEntry(new ElementList());

        Assert.Equal((int)dictionaryType, new Rdm.DataDictionary().ExtractDictionaryType(series));

        series.ClearAndReturnToPool_All();
        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_EnumTypeDef_Encode_With_Fragmentation()
    {
        output.WriteLine("TestDataDictionary_EnumTypeDef_encode_with_fragmentation(): Test to encode and decode enumerated types payload with fragmentation size.\n");

        int fragmentationSize = 12800;
        int currentCount = 0;
        Rdm.DataDictionary dataDictionary = new();

        Series series = manager.GetOmmSeries();
        int result;

        try
        {
            while (true)
            {
                result = m_GlobalDataDictionary.EncodeEnumTypeDictionary(series, currentCount, EmaRdm.DICTIONARY_VERBOSE, fragmentationSize);

                try
                {
                    dataDictionary.DecodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_VERBOSE);
                }
                catch (OmmException excp)
                {
                    Assert.True(false,
                        $"DataDictionary.DecodeEnumTypeDictionary() failed to decode enumerated type dictionary - exception not expected: {excp}");
                }

                if (result != m_GlobalDataDictionary.EnumTables().Count)
                {
                    if (dataDictionary.EnumTables().Count != result)
                    {
                        Assert.True(false, "DataDictionary.EnumTables().Count must be equal to the index of current table count of multi-part payload");
                    }
                }

                if (series.HasTotalCountHint)
                {
                    Assert.Equal(series.TotalCountHint(), m_GlobalDataDictionary.EnumTables().Count);
                }

                currentCount = result;

                if (result == m_GlobalDataDictionary.EnumTables().Count)
                    break;
            }

            Assert.Equal(dataDictionary.EnumTables().Count, m_GlobalDataDictionary.EnumTables().Count);

        }
        catch (OmmException excp)
        {
            Assert.True(false, $"DataDictionary.EncodeEnumTypeDictionary(fragmentationSize) failed to encode enumerated type dictionary - exception not expected: {excp}");
        }

        series.ClearAndReturnToPool_All();

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDictionaryEntry_ToString()
    {
        output.WriteLine("TestDictionaryEntry_ToString(): Test to call toString() from an instance of DictionaryEntry");

        DictionaryEntry dictionaryEntry = m_GlobalDataDictionary.Entry(14154);

        StringBuilder expected = new StringBuilder(256);
        expected.Append("Fid=14154 'OPEN_GRADE' 'OPEN GRADE' Type=6 RippleTo=0 Len=3 EnumLen=5 RwfType=14 RwfLen=1");
        expected.AppendLine().AppendLine()
            .AppendLine("Enum Type Table:")
            .AppendLine("(Referenced by Fid 14145)")
            .AppendLine("(Referenced by Fid 14146)")
            .AppendLine("(Referenced by Fid 14147)")
            .AppendLine("(Referenced by Fid 14148)")
            .AppendLine("(Referenced by Fid 14149)")
            .AppendLine("(Referenced by Fid 14150)")
            .AppendLine("(Referenced by Fid 14151)")
            .AppendLine("(Referenced by Fid 14152)")
            .AppendLine("(Referenced by Fid 14153)")
            .AppendLine("(Referenced by Fid 14154)")
            .AppendLine("value=0 display=\"     \" meaning=\"Undefined\"")
            .AppendLine("value=1 display=\"    0\" meaning=\"Valid\"")
            .AppendLine("value=2 display=\"    1\" meaning=\"Suspect\"");

        Assert.Equal(expected.ToString(), dictionaryEntry.ToString());
        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDictionaryEntry_HasEnumTable_GetEnumTable()
    {
        output.WriteLine("TestDictionaryEntry_HasEnumTable_GetEnumTable(): Test to check and get EnumTable from DictionaryEntry.");

        DictionaryEntry dictionaryEntryFid1 = m_GlobalDataDictionary.Entry(1);

        Assert.False(dictionaryEntryFid1.HasEnumTypeTable);

        try
        {
            dictionaryEntryFid1.GetEnumTypeTable();
            Assert.True(false, "Calling DictionaryEntry.GetEnumTypeTable() for non existing EnumTypeTable - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The EnumTypeTable does not exist for the Field ID 1', Error Code='-4048'",
                excp.ToString());
        }

        DictionaryEntry dictionaryEntryFid4 = m_GlobalDataDictionary.Entry(4);

        Assert.True(dictionaryEntryFid4.HasEnumTypeTable);

        try
        {
            dictionaryEntryFid4.GetEnumTypeTable();
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.True(false, $"Calling DictionaryEntry.GetEnumTypeTable() for an existing EnumTypeTable - exception not expected: {excp}");
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDictionaryEntry_HasEnumType_GetEnumType()
    {
        output.WriteLine("TestDictionaryEntry_HasEnumType_GetEnumType(): Test to check and get EnumType from DitionaryEntry.");

        DictionaryEntry dictionaryEntryFid1 = m_GlobalDataDictionary.Entry(1);

        Assert.False(dictionaryEntryFid1.HasEnumType(5555));

        try
        {
            dictionaryEntryFid1.EnumType(5555);
            Assert.True(false, "Calling DictionaryEntry.enumType() for non existing EnumType - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The enum value 5555 for the Field ID 1 does not exist in enumerated type definitions', Error Code='-22'",
                excp.ToString());
        }

        DictionaryEntry dictionaryEntryFid4 = m_GlobalDataDictionary.Entry(4);

        Assert.True(dictionaryEntryFid4.HasEnumType(5));

        try
        {
            dictionaryEntryFid4.EnumType(5);
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.True(false, $"Calling DictionaryEntry.EnumType() for an existing EnumType - exception not expected: {excp}");
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_HasEntry_GetEntry_by_FieldID()
    {
        output.WriteLine("TestDataDictionary_HasEntry_GetEntry_by_FieldID(): Test to check and get DictionaryEntry by field ID from DataDictionary.");

        Assert.False(m_GlobalDataDictionary.HasEntry(-555));

        try
        {
            m_GlobalDataDictionary.Entry(-555);
            Assert.True(false, "Calling DictionaryUtility.Entry(fid) from non existing DictionaryEntry - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The Field ID -555 does not exist in the field dictionary', Error Code='-22'",
                excp.ToString());
        }

        Assert.True(m_GlobalDataDictionary.HasEntry(3));

        try
        {
            DictionaryEntry dictionaryEntry = m_GlobalDataDictionary.Entry(3);
            Assert.Equal("DSPLY_NAME", dictionaryEntry.Acronym);
        }
        catch (OmmException excp)
        {
            Assert.True(false, $"Calling DictionaryUtility.Entry(fid) from an existing DictionaryEntry - exception not expected: {excp}");
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_HasEntry_GetEntry_by_FieldName()
    {
        output.WriteLine("TestDataDictionary_HasEntry_GetEntry_by_FieldName(): Test to check and get DictionaryEntry by field name from DataDictionary.");

        Assert.False(m_GlobalDataDictionary.HasEntry("UNKNOWN_FID"), "Check from non existing DictionaryEntry");

        try
        {
            m_GlobalDataDictionary.Entry("UNKNOWN_FID");
            Assert.True(false, "Calling DictionaryUtility.Entry(name) from non existing DictionaryEntry - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The Field name UNKNOWN_FID does not exist in the field dictionary', Error Code='-22'",
                excp.ToString());
        }

        Assert.True(m_GlobalDataDictionary.HasEntry("DSPLY_NAME"));

        try
        {
            DictionaryEntry dictionaryEntry = m_GlobalDataDictionary.Entry("DSPLY_NAME");
            Assert.Equal(3, dictionaryEntry.FieldId);
        }
        catch (OmmException excp)
        {
            Assert.True(false, $"Calling DictionaryUtility.Entry(fid) from an existing DictionaryEntry - exception not expected: {excp}");
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_HasEnumType_GetEnumType()
    {
        output.WriteLine("TestDataDictionary_HasEnumType_GetEnumType(): Test to check and get EnumType from DataDictionary.");

        Assert.False(m_GlobalDataDictionary.HasEnumType(4, 5555), "Check from non existing EnumType");

        try
        {
            m_GlobalDataDictionary.EnumType(4, 5555);
            Assert.True(false, "Calling DictionaryUtility.enumType(fid, enumValue) from non existing EnumType - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The enum value 5555 for the Field ID 4 does not exist in enumerated type definitions', Error Code='-22'",
                excp.ToString());
        }

        Assert.True(m_GlobalDataDictionary.HasEnumType(4, 15), "Check from existing DictionaryEntry");

        try
        {
            m_GlobalDataDictionary.EnumType(4, 15);
        }
        catch (OmmException excp)
        {
            Assert.True(false, $"Calling DictionaryUtility.EnumType(fid, enumvalue) from an existing EnumType - exception not expected: {excp}");
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_EntryShouldReturnSameEntry()
    {
        output.WriteLine("TestDataDictionary_EntryShouldReturnSameEntry(): Test Entry(x) and Entry(y) calls return same internal reference, Entry(int), Entry(int) should interference each other");

        DictionaryEntry entry11 = m_GlobalDataDictionary.Entry(11);
        string entry11AcronymBefore = entry11.Acronym;

        DictionaryEntry entry12 = m_GlobalDataDictionary.Entry(12);
        Assert.Equal(entry11, entry12);
        Assert.NotEqual(entry11AcronymBefore, entry11.Acronym);
    }

    [Fact]
    public void TestDataDictionary_EntryShouldReturnDifferentDictionaryEntry()
    {
        output.WriteLine("TestDataDictionary_EntryShouldReturnDifferentDictionaryEntry(): Test Entry(x, xHolder) and Entry(y, yHolder) calls return different out reference, calls are not interference each other");

        DictionaryEntry entry11 = new();
        m_GlobalDataDictionary.Entry(11, entry11);
        string entry11AcronymBefore = entry11.Acronym;

        DictionaryEntry entry12 = new();
        m_GlobalDataDictionary.Entry(12, entry12);
        Assert.NotEqual(entry11, entry12);
        Assert.Equal(entry11AcronymBefore, entry11.Acronym);

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_EntryWithNullArgumentShouldRaiseOMMInvalidUsage()
    {
        output.WriteLine("TestDataDictionary_EntryWithNullArgumentShouldRaiseOMMInvalidUsage(): Test Entry(x, xHolder) should raise exception in case xHolder is internal var and is not managed by the user");

        bool entryCallFailed = false;
        try
        {
            m_GlobalDataDictionary.Entry(11, null!);
        }
        catch (OmmInvalidUsageException excp)
        {
            entryCallFailed = true;
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, excp.ErrorCode);
        }
        finally
        {
            Assert.True(entryCallFailed, "OmmInvalidUsageException was expected");
            CheckEtaGlobalPoolSizes();
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_EntryWithNonFactoryDictionaryEntryShouldRaiseOMMInvalidUsage()
    {
        output.WriteLine("TestDataDictionary_EntryWithNonFactoryDictionaryEntryShouldRaiseOMMInvalidUsage(): Test Entry(x, xHolder) should raise exception in case xHolder is internal var and is not managed by the user");

        DictionaryEntry entry11 = m_GlobalDataDictionary.Entry(11);

        try
        {
            m_GlobalDataDictionary.Entry(11, entry11);
            Assert.True(false, "OmmInvalidUsageException exception is expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_USAGE, ((OmmInvalidUsageException)excp).ErrorCode);
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_EntryShouldRaiseExceptionForNonExistingId()
    {
        output.WriteLine("TestDataDictionary_entryShouldRaiseExceptionForNonexistingId(): Test to check and get DictionaryEntry by non-existing field ID from DataDictionary.");

        Assert.False(m_GlobalDataDictionary.HasEntry(-555));

        try
        {
            DictionaryEntry entry = new();
            m_GlobalDataDictionary.Entry(-555, entry);
            Assert.True(false, "Calling DictionaryUtility.Entry(fid, entryDst) for non existing DictionaryEntry - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal("Exception Type='OmmInvalidUsageException', Text='The Field ID -555 does not exist in the field dictionary', Error Code='-22'",
                excp.ToString());
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_EntryShouldRaiseExceptionForNonExistingName()
    {
        output.WriteLine("TestDataDictionary_EntryShouldRaiseExceptionForNonExistingId(): Test to check and get DictionaryEntry by non-existing field name from DataDictionary.");

        Assert.False(m_GlobalDataDictionary.HasEntry("UNKNOWN_FID"), "Check for non existing UNKNOWN_FID DictionaryEntry");

        try
        {
            DictionaryEntry entry = new();
            m_GlobalDataDictionary.Entry("UNKNOWN_FID", entry);
            Assert.True(false, "Calling DictionaryUtility.Entry(name) from non existing DictionaryEntry - exception expected");
        }
        catch (OmmException excp)
        {
            Assert.Equal(OmmException.ExceptionType.OmmInvalidUsageException, excp.Type);
            Assert.Equal(
                "Exception Type='OmmInvalidUsageException', Text='The Field name UNKNOWN_FID does not exist in the field dictionary', Error Code='-22'",
                excp.ToString());
        }

        CheckEtaGlobalPoolSizes();
    }

    [Fact]
    public void TestDataDictionary_NonAsciiEnumDisplay()
    {
        output.WriteLine("TestDataDictionary_NonAsciiEnumDisplay(): Test to clone DataDictonary from another DataDictionary instance.");

        try
        {
            Rdm.DataDictionary dataDictionary = new(m_GlobalDataDictionary);

            var dictEntry = dataDictionary.Entry(54);
            var enumType = dictEntry.GetEnumType(70);
            string result = enumType.Display.ToString();

            // U+004B
            Assert.Equal(75, result.ToCharArray()[0]);

            // U+0067
            Assert.Equal(103, result.ToCharArray()[1]);

            // U+0035
            Assert.Equal(53, result.ToCharArray()[2]);

            // U+0033
            Assert.Equal(51, result.ToCharArray()[3]);

            // U+00BD
            Assert.Equal(189, result.ToCharArray()[4]);

            dictEntry = dataDictionary.Entry(14);
            enumType = dictEntry.GetEnumType(1);
            result = enumType.Display.ToString();

            // U+00DE
            Assert.Equal(222, result.ToCharArray()[0]);

            dictEntry = dataDictionary.Entry(14);
            enumType = dictEntry.GetEnumType(2);
            result = enumType.Display.ToString();

            // U+00FE
            Assert.Equal(254, result.ToCharArray()[0]);

            dictEntry = dataDictionary.Entry(270);
            enumType = dictEntry.GetEnumType(2);
            result = enumType.Display.ToString();

            // U+0020
            Assert.Equal(32, result.ToCharArray()[0]);
            // U+00DE
            Assert.Equal(222, result.ToCharArray()[1]);

            dictEntry = dataDictionary.Entry(270);
            enumType = dictEntry.GetEnumType(3);
            result = enumType.Display.ToString();

            // U+0020
            Assert.Equal(32, result.ToCharArray()[0]);
            // U+00FE
            Assert.Equal(254, result.ToCharArray()[1]);

            dictEntry = dataDictionary.Entry(270);
            enumType = dictEntry.GetEnumType(26);
            result = enumType.Display.ToString();

            // U+0042
            Assert.Equal(66, result.ToCharArray()[0]);
            // U+00DE
            Assert.Equal(222, result.ToCharArray()[1]);

            dictEntry = dataDictionary.Entry(270);
            enumType = dictEntry.GetEnumType(27);
            result = enumType.Display.ToString();

            // U+0042
            Assert.Equal(66, result.ToCharArray()[0]);
            // U+00FE
            Assert.Equal(254, result.ToCharArray()[1]);

            dictEntry = dataDictionary.Entry(270);
            enumType = dictEntry.GetEnumType(61);
            result = enumType.Display.ToString();

            // U+004D
            Assert.Equal(77, result.ToCharArray()[0]);
            // U+00DE
            Assert.Equal(222, result.ToCharArray()[1]);

            dictEntry = dataDictionary.Entry(270);
            enumType = dictEntry.GetEnumType(62);
            result = enumType.Display.ToString();

            // U+004D
            Assert.Equal(77, result.ToCharArray()[0]);
            // U+00FE
            Assert.Equal(254, result.ToCharArray()[1]);
        }
        catch (OmmException excp)
        {
            Assert.True(false, $"DataDictionary.DataDictionary(DataDictionary) failed to clone dictionary - exception not expected: {excp}");
        }

        CheckEtaGlobalPoolSizes();
    }

    private void CheckEtaGlobalPoolSizes()
    {
        var pool = EtaObjectGlobalPool.Instance;
        Assert.Equal(EtaObjectGlobalPool.INITIAL_POOL_SIZE, pool.m_etaBufferPool.Count);
        Assert.Equal(EtaObjectGlobalPool.INITIAL_POOL_SIZE, pool.m_etaEncodeIteratorPool.Count);
        foreach (var keyVal in pool.m_etaByteBufferBySizePool)
        {
            Assert.Equal(EtaObjectGlobalPool.INITIAL_POOL_SIZE, keyVal.Value.Count);
        }
    }

    #endregion
}
