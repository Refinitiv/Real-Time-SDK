/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using LSEG.Ema.Access;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;

namespace LSEG.Ema.Rdm;

/// <summary>
/// This class houses all known fields loaded from an RDM field dictionary and their
/// corresponding enum types loaded from an enum type dictionary.
/// </summary>
/// <remarks>
/// The dictionary also saves general information about the dictionary itself.<br/>
/// This is found in the "!tag" comments of the file or in the summary data of
/// dictionaries encoded via the official domain model.<br/>
/// The data dictionary must be loaded prior to using the methods to access dictionary
/// entries.<br/>
/// </remarks>
/// <seealso cref="DictionaryEntry"/>
/// <seealso cref="Rdm.EnumType"/>
public sealed class DataDictionary
{
    #region Public members

    /// <summary>
    /// Clears DataDictionary.
    /// </summary>
    /// <remarks>
    /// This method is used to clear the existing dictionary information.
    /// </remarks>
    public void Clear()
    {
        m_DictionaryLock.Enter();
        try
        {
            ClearFlags();

            ClearDictionaryEntryList();

            ClearEnumTypeTableList();

            m_FieldNametoIdMap?.Clear();

            m_rsslDataDictionary?.Clear();
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// The lowest fieldId present in the dictionary.
    /// </summary>
    public int MinFid
    {
        get
        {
            m_DictionaryLock.Enter();
            try
            {
                return m_rsslDataDictionary.MinFid;
            }
            finally
            {
                m_DictionaryLock.Exit();
            }
        }
    }

    /// <summary>
    /// The highest fieldId present in the dictionary.
    /// </summary>
    public int MaxFid
    {
        get
        {
            m_DictionaryLock.Enter();
            try
            {
                return m_rsslDataDictionary.MaxFid;
            }
            finally
            {
                m_DictionaryLock.Exit();
            }
        }
    }

    /// <summary>
    /// The list of DictionaryEntry of this DataDictionary.
    /// </summary>
    /// <returns>A List containing all of the <see cref="DictionaryEntry"/> contained in the dictionary.</returns>
    public List<DictionaryEntry> Entries()
    {
        m_DictionaryLock.Enter();
        try
        {
            if (m_DictionaryEntryList == null)
            {
                m_DictionaryEntryList = new(m_rsslDataDictionary.NumberOfEntries);
            }

            if (m_DictionaryEntryList.Count() != m_rsslDataDictionary.NumberOfEntries)
            {
                ClearDictionaryEntryList();
            }
            else
            {
                return m_DictionaryEntryList;
            }

            if (m_LoadedFieldDictionary)
            {
                Eta.Codec.IDictionaryEntry dictionaryEntry;

                for (int fieldId = m_rsslDataDictionary.MinFid; fieldId <= m_rsslDataDictionary.MaxFid; fieldId++)
                {
                    dictionaryEntry = m_rsslDataDictionary.Entry(fieldId);

                    if (dictionaryEntry != null)
                    {
                        m_DictionaryEntryList.Add(GetDictionaryEntry(this, dictionaryEntry));
                    }
                }
            }

            return m_DictionaryEntryList;
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// The list of EnumTypeTable of this DataDictionary.
    /// </summary>
    /// <returns>the list of EnumTypeTable</returns>
    public List<EnumTypeTable> EnumTables()
    {
        m_DictionaryLock.Enter();
        try
        {
            if (m_EnumTypeTableList == null)
            {
                m_EnumTypeTableList = new(m_rsslDataDictionary.EnumTableCount);
            }

            if (m_EnumTypeTableList.Count != m_rsslDataDictionary.EnumTableCount)
            {
                ClearEnumTypeTableList();
            }
            else
            {
                return m_EnumTypeTableList;
            }

            if (m_LoadedEnumTypeDef)
            {
                Eta.Codec.IEnumTypeTable enumTypeTable;

                for (int index = 0; index < m_rsslDataDictionary.EnumTableCount; index++)
                {
                    enumTypeTable = m_rsslDataDictionary.EnumTables[index];

                    if (enumTypeTable != null)
                    {
                        m_EnumTypeTableList.Add(GetEnumTypeTable(enumTypeTable));
                    }
                }
            }
            return m_EnumTypeTableList;
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// DictionaryId Tag. All dictionaries loaded using this object will have this tag
    /// matched if found.
    /// </summary>
    /// <value>the dictionaryId</value>
    public int DictionaryId
    {
        get
        {
            m_DictionaryLock.Enter();
            try
            {
                return m_rsslDataDictionary.InfoDictionaryId;
            }
            finally
            {
                m_DictionaryLock.Exit();
            }
        }
    }

    /// <summary>
    /// Field Version Tag.
    /// </summary>
    /// <value>the fieldVersion</value>
    public string FieldVersion
    {
        get
        {
            m_DictionaryLock.Enter();
            try
            {
                return m_rsslDataDictionary.InfoFieldVersion.Data() != null
                    ? m_rsslDataDictionary.InfoFieldVersion.ToString()
                    : string.Empty;
            }
            finally
            {
                m_DictionaryLock.Exit();
            }
        }
    }

    /// <summary>
    /// Enum RT_Version Tag.
    /// </summary>
    /// <value>the enumRecordTemplateVersion</value>
    public string EnumRecordTemplateVersion
    {
        get
        {
            m_DictionaryLock.Enter();
            try
            {
                return m_rsslDataDictionary.InfoEnumRTVersion.Data() != null
                    ? m_rsslDataDictionary.InfoEnumRTVersion.ToString()
                    : string.Empty;
            }
            finally
            {
                m_DictionaryLock.Exit();
            }
        }
    }

    /// <summary>
    /// Enum DT_Version Tag.
    /// </summary>
    /// <value>the enumDisplayTemplateVersion</value>
    public string EnumDisplayTemplateVersion
    {
        get
        {
            m_DictionaryLock.Enter();
            try
            {
                return m_rsslDataDictionary.InfoEnumDTVersion.Data() != null
                    ? m_rsslDataDictionary.InfoEnumDTVersion.ToString()
                    : string.Empty;
            }
            finally
            {
                m_DictionaryLock.Exit();
            }
        }
    }

    /// <summary>
    /// Field Filename Tag.
    /// </summary>
    /// <value>the fieldFilename</value>
    public string FieldFilename
    {
        get
        {
            m_DictionaryLock.Enter();
            try
            {
                return m_rsslDataDictionary.InfoFieldFilename.Data() != null
                    ? m_rsslDataDictionary.InfoFieldFilename.ToString()
                    : string.Empty;
            }
            finally
            {
                m_DictionaryLock.Exit();
            }
        }
    }

    /// <summary>
    /// Field Description Tag.
    /// </summary>
    /// <value>the fieldDescription</value>
    public string FieldDescription
    {
        get
        {
            m_DictionaryLock.Enter();
            try
            {
                return m_rsslDataDictionary.InfoFieldDesc.Data() != null
                    ? m_rsslDataDictionary.InfoFieldDesc.ToString()
                    : string.Empty;
            }
            finally
            {
                m_DictionaryLock.Exit();
            }
        }
    }

    /// <summary>
    /// Field Build Tag.
    /// </summary>
    /// <value>the fieldBuild</value>
    public string FieldBuild
    {
        get
        {
            m_DictionaryLock.Enter();
            try
            {
                return m_rsslDataDictionary.InfoFieldBuild.Data() != null
                    ? m_rsslDataDictionary.InfoFieldBuild.ToString()
                    : string.Empty;
            }
            finally
            {
                m_DictionaryLock.Exit();
            }
        }
    }

    /// <summary>
    /// Field Date Tag.
    /// </summary>
    /// <value>the fieldDate</value>
    public string FieldDate
    {
        get
        {
            m_DictionaryLock.Enter();
            try
            {
                return m_rsslDataDictionary.InfoFieldDate.Data() != null
                    ? m_rsslDataDictionary.InfoFieldDate.ToString()
                    : string.Empty;
            }
            finally
            {
                m_DictionaryLock.Exit();
            }
        }
    }

    /// <summary>
    /// Enum Filename Tag.
    /// </summary>
    /// <value>the enumFilename</value>
    public string EnumFilename
    {
        get
        {
            m_DictionaryLock.Enter();
            try
            {
                return m_rsslDataDictionary.InfoEnumFilename.Data() != null
                    ? m_rsslDataDictionary.InfoEnumFilename.ToString()
                    : string.Empty;
            }
            finally
            {
                m_DictionaryLock.Exit();
            }
        }
    }

    /// <summary>
    /// Enum Description Tag.
    /// </summary>
    /// <value>the enumDescription</value>
    public string EnumDescription
    {
        get
        {
            m_DictionaryLock.Enter();
            try
            {
                return m_rsslDataDictionary.InfoEnumDesc.Data() != null
                    ? m_rsslDataDictionary.InfoEnumDesc.ToString()
                    : string.Empty;
            }
            finally
            {
                m_DictionaryLock.Exit();
            }
        }
    }

    /// <summary>
    /// Enum Date Tag.
    /// </summary>
    /// <value>the enumDate</value>
    public string EnumDate
    {
        get
        {
            m_DictionaryLock.Enter();
            try
            {
                return m_rsslDataDictionary.InfoEnumDate.Data() != null
                    ? m_rsslDataDictionary.InfoEnumDate.ToString()
                    : string.Empty;
            }
            finally
            {
                m_DictionaryLock.Exit();
            }
        }
    }

    /// <summary>
    /// Adds information from a field dictionary file to the data dictionary object.
    /// </summary>
    /// <remarks>
    /// Subsequent calls to this method may be made to the same
    /// <see cref="DataDictionary"/> to load additional dictionaries (provided the
    /// fields do not conflict).
    /// </remarks>
    /// <param name="filename">specifies a field dictionary file</param>
    /// <exception cref="OmmInvalidUsageException">Thrown if this method fails to load specified file name from <code>filename</code>.</exception>
    public void LoadFieldDictionary(string filename)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (m_OwnRsslDataDictionary)
            {
                if (m_rsslDataDictionary.LoadFieldDictionary(filename, out m_RsslError) < 0)
                {
                    string errText = ErrorString().Append("Unable to load field dictionary from file named ")
                            .Append(filename).Append(ILoggerClient.CR)
                            .Append("Current working directory ")
                            .Append(System.IO.Directory.GetCurrentDirectory())
                            .Append(ILoggerClient.CR)
                            .Append("Reason='")
                            .Append(m_RsslError.Text)
                            .Append('\'').ToString();
                    throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.FAILURE);
                }
                else
                {
                    m_LoadedFieldDictionary = true;
                }
            }
            else
            {
                throw new OmmInvalidUsageException(QUERYING_ONLY_ERROR_TEXT, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// Adds information from an enumerated types dictionary file to the data dictionary object.
    /// </summary>
    /// <remarks>
    /// Subsequent calls to this method may be made to the same <see cref="DataDictionary"/> to 
    /// load additional dictionaries (provided that there are no duplicate table references for any field).
    /// </remarks>
    /// <param name="filename">specifies an enumerated types dictionary file</param>
    /// <exception cref="OmmInvalidUsageException">Thrown if this method fails to load specified file name from <code>filename</code>.</exception>
    public void LoadEnumTypeDictionary(string filename)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (m_OwnRsslDataDictionary)
            {
                if (m_rsslDataDictionary.LoadEnumTypeDictionary(filename, out m_RsslError) < 0)
                {
                    string errText = ErrorString().Append("Unable to load enumerated type definition from file named ")
                        .Append(filename).Append(ILoggerClient.CR)
                        .Append("Current working directory ")
                        .Append(System.IO.Directory.GetCurrentDirectory())
                        .Append(ILoggerClient.CR)
                        .Append("Reason='")
                        .Append(m_RsslError.Text)
                        .Append('\'').ToString();
                    throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.FAILURE);
                }
                else
                {
                    m_LoadedEnumTypeDef = true;
                }
            }
            else
            {
                throw new OmmInvalidUsageException(QUERYING_ONLY_ERROR_TEXT, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// Encode the field dictionary information into a data payload according the domain model, using the field information from the entries
    /// present in this dictionary.
    /// </summary>
    /// <remarks>
    /// This method supports building the encoded data in one full part.
    /// </remarks>
    /// <param name="series">Series to be used for encoding dictionary information into.</param>
    /// <param name="verbosity">The desired verbosity to encode. The verbosity values are <br/>
    /// <see cref="EmaRdm.DICTIONARY_MINIMAL"/>, <see cref="EmaRdm.DICTIONARY_NORMAL"/>, <see cref="EmaRdm.DICTIONARY_VERBOSE"/>.</param>
    /// <exception cref="OmmInvalidUsageException">Thrown if this method fails to encode field dictionary information.</exception>
    /// <seealso cref="EmaRdm"/>
    public void EncodeFieldDictionary(Access.Series series, long verbosity)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (!m_LoadedFieldDictionary)
            {
                throw new OmmInvalidUsageException("The field dictionary information was not loaded",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (series == null)
            {
                string errText = ErrorString().Append("Passed in series parameter is null").ToString();
                throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }

            series.Clear();

            int fieldDictionarySize = m_rsslDataDictionary.NumberOfEntries > 0
                ? (m_rsslDataDictionary.NumberOfEntries * DEFAULT_DICTIONARY_ENTRY_SIZE)
                : Data.ENCODE_RSSL_BUFFER_INIT_SIZE;

            series.GetEncoder().AcquireEncodeIterator(fieldDictionarySize);

            series.m_bodyBuffer = series.GetEncoder().m_encodeIterator!.Buffer();

            m_RsslInt.Value(m_rsslDataDictionary.MinFid);
            CodecReturnCode ret;

            while ((ret = m_rsslDataDictionary.EncodeFieldDictionary(series.GetEncoder().m_encodeIterator,
                m_RsslInt, (int)verbosity, out m_RsslError))
                == CodecReturnCode.DICT_PART_ENCODED)
            {
                Utilities.Reallocate(series.GetEncoder().m_encodeIterator!,
                    series.GetEncoder().m_encodeIterator!.Buffer().Capacity * 2);
                m_RsslInt.Value(m_rsslDataDictionary.MinFid);
            }

            if (ret != CodecReturnCode.SUCCESS)
            {
                series.Clear();
                string errText = ErrorString().Append("Failed to encode the field dictionary information")
                    .Append(ILoggerClient.CR)
                    .Append("Reason='")
                    .Append(m_RsslError.Text)
                    .Append('\'').ToString();

                throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INVALID_DATA);
            }

            series.GetEncoder().m_containerComplete = true;
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// Encode the field dictionary information into a data payload
    /// according the domain model, using the field information from the entries
    /// present in this dictionary.
    /// </summary>
    /// <remarks>
    /// This method supports building the encoded data in multiple parts according to the
    /// fragmentation size.
    /// </remarks>
    /// <param name="series">Series to be used for encoding dictionary information into.</param>
    /// <param name="currentFid">tracks which fields have been encoded in case of multi-part encoding.</param>
    /// <param name="verbosity">The desired verbosity to encode. The verbosity values are <br/>
    /// <see cref="EmaRdm.DICTIONARY_MINIMAL"/>, <see cref="EmaRdm.DICTIONARY_NORMAL"/>, <see cref="EmaRdm.DICTIONARY_VERBOSE"/>
    /// </param>
    /// <param name="fragmentationSize">The fragmentation size in number of bytes.</param>
    /// <returns>
    /// Field ID that have been encoded or equal to <see cref="MaxFid"/> to indicate final part
    /// or single complete payload.
    /// </returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if this method fails to encode field dictionary information.</exception>
    /// <seealso cref="EmaRdm"/>
    public int EncodeFieldDictionary(Access.Series series, int currentFid, long verbosity, int fragmentationSize)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (series == null)
            {
                throw new OmmInvalidUsageException(ErrorString().Append("Passed in series parameter is null").ToString(),
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }

            if (!m_LoadedFieldDictionary)
            {
                throw new OmmInvalidUsageException("The field dictionary information was not loaded",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            series.Clear();

            int fieldDictionarySize = fragmentationSize > 0 ? fragmentationSize : DEFAULT_FRAGMENTATION_SIZE;

            series.GetEncoder().AcquireEncodeIterator(fieldDictionarySize);

            series.m_bodyBuffer = series.GetEncoder().m_encodeIterator!.Buffer();

            m_RsslInt.Value(currentFid);

            CodecReturnCode ret;

            ret = m_rsslDataDictionary.EncodeFieldDictionary(series.GetEncoder().m_encodeIterator, m_RsslInt, (int)verbosity, out m_RsslError);

            if (ret == CodecReturnCode.SUCCESS)
            {
                series.Complete();
                return m_rsslDataDictionary.MaxFid;
            }
            if (ret == CodecReturnCode.DICT_PART_ENCODED)
            {
                series.Complete();
                return (int)m_RsslInt.ToLong();
            }

            string errText = ErrorString().Append("Failed to encode the field dictionary information. Reason='")
                .Append(ret.GetAsString()).Append('\'').ToString();

            throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INVALID_DATA);
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// Decode the field dictionary information contained in a data payload according to
    /// the domain model.
    /// </summary>
    /// <remarks>
    /// This method may be called multiple times on the same dictionary, to load
    /// information from dictionaries that have been encoded in multiple parts.
    /// </remarks>
    /// <param name="series">Series to be used for decoding dictionary information from.</param>
    /// <param name="verbosity">The desired verbosity to encode. The verbosity values are <br/>
    /// <see cref="EmaRdm.DICTIONARY_MINIMAL"/>, <see cref="EmaRdm.DICTIONARY_NORMAL"/>, <see cref="EmaRdm.DICTIONARY_VERBOSE"/>.</param>
    /// <exception cref="OmmInvalidUsageException">Thrown if this method fails to decode field dictionary information.</exception>
    /// <seealso cref="EmaRdm"/>
    public void DecodeFieldDictionary(Access.Series series, long verbosity)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (m_OwnRsslDataDictionary)
            {
                if (series == null)
                {
                    string errText = ErrorString().Append("Passed in series parameter is null").ToString();
                    throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                }

                Eta.Codec.Buffer encodedBuffer = series.m_bodyBuffer!;

                Eta.Codec.DecodeIterator decodeIterator = DecodeIterator();

                CodecReturnCode ret = decodeIterator.SetBufferAndRWFVersion(encodedBuffer, Eta.Codec.Codec.MajorVersion(),
                        Eta.Codec.Codec.MinorVersion());

                if (ret != CodecReturnCode.SUCCESS)
                {
                    string errText = ErrorString().Append("Failed to setBufferAndRWFVersion on rssl decode iterator. Reason='")
                            .Append(ret.GetAsString())
                            .Append('\'').ToString();
                    throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
                }

                ret = m_rsslDataDictionary.DecodeFieldDictionary(decodeIterator, (int)verbosity, out m_RsslError);

                if (ret < CodecReturnCode.SUCCESS)
                {
                    string errText = ErrorString()
                        .Append("Failed to decode the field dictionary information. Reason='")
                        .Append(m_RsslError.Text)
                        .Append('\'').ToString();
                    throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INVALID_DATA);
                }

                m_LoadedFieldDictionary = true;
            }
            else
            {
                throw new OmmInvalidUsageException(QUERYING_ONLY_ERROR_TEXT, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// Encode the enumerated types dictionary according the domain model, using the
    /// information from the tables and referencing fields present in this dictionary.
    /// </summary>
    /// <remarks>
    /// Note: This method will use the type Ascii for the DISPLAY array.
    /// </remarks>
    /// <param name="series">Series to be used for encoding enumerated types dictionary into.</param>
    /// <param name="verbosity">The desired verbosity to encode. The verbosity values are <br/>
    /// <see cref="EmaRdm.DICTIONARY_MINIMAL"/>, <see cref="EmaRdm.DICTIONARY_NORMAL"/>, <see cref="EmaRdm.DICTIONARY_VERBOSE"/>.</param>
    /// <exception cref="OmmInvalidUsageException">Thrown if this method fails to encode enumerated types dictionary.</exception>
    /// <see cref="Access.Series"/>
    /// <seealso cref="EmaRdm"/>
    public void EncodeEnumTypeDictionary(Access.Series series, long verbosity)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (!m_LoadedEnumTypeDef)
            {
                throw new OmmInvalidUsageException("The enumerated types dictionary was not loaded",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (series == null)
            {
                string errText = ErrorString().Append("Passed in series parameter is null").ToString();
                throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }

            series.Clear();

            int enumTypeDictionarySize = m_rsslDataDictionary.EnumTableCount > 0
                ? (m_rsslDataDictionary.EnumTableCount * DEFAULT_ENUM_TABLE_ENTRY_SIZE)
                : Data.ENCODE_RSSL_BUFFER_INIT_SIZE;

            series.GetEncoder().AcquireEncodeIterator(enumTypeDictionarySize);

            series.m_bodyBuffer = series.GetEncoder().m_encodeIterator!.Buffer();

            CodecReturnCode ret;

            while ((ret = m_rsslDataDictionary.EncodeEnumTypeDictionary(series.GetEncoder().m_encodeIterator,
                (int)verbosity, m_RsslError))
                == CodecReturnCode.DICT_PART_ENCODED)
            {
                Utilities.Reallocate(series.GetEncoder().m_encodeIterator!, series.GetEncoder().m_encodeIterator!.Buffer().Capacity * 2);
            }

            if (ret != CodecReturnCode.SUCCESS)
            {
                series.Clear();
                string errText = ErrorString().Append("Failed to encode the enumerated type definition")
                    .Append(ILoggerClient.CR)
                    .Append("Reason='")
                    .Append(m_RsslError.Text)
                    .Append('\'').ToString();

                throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
            }

            series.GetEncoder().m_containerComplete = true;
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// Encode the enumerated types dictionary according the domain model, using the
    /// information from the tables and referencing fields present in this dictionary.
    /// </summary>
    /// <remarks>
    /// This method supports building the encoded data in multiple parts according to the
    /// fragmentation size.<br/>
    /// Note: This method will use the type Ascii for the DISPLAY array.
    /// </remarks>
    /// <param name="series">Series to be used for encoding enumerated types dictionary into.</param>
    /// <param name="currentEnumTableEntry">Tracks the index of enumerated type table Entry have been encoded.
    ///  		Must be initialized to 0 on the first call and is updated with each successfully encoded part.</param>
    /// <param name="verbosity">The desired verbosity to encode. The verbosity values are <br/>
    /// <see cref="EmaRdm.DICTIONARY_MINIMAL"/>, <see cref="EmaRdm.DICTIONARY_NORMAL"/>, <see cref="EmaRdm.DICTIONARY_VERBOSE"/>.</param>
    /// <param name="fragmentationSize">The fragmentation size in number of bytes.</param>
    /// <returns>the index of enumerated type table Entry that have been encoded or equal to the size
    /// of <see cref="EnumTables()"/> to indicate final part or single complete payload.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if this method fails to encode enumerated types dictionary.</exception>
    /// <seealso cref="Access.Series"/>
    /// <seealso cref="EmaRdm"/>
    public int EncodeEnumTypeDictionary(Access.Series series, int currentEnumTableEntry, long verbosity, int fragmentationSize)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (series == null)
            {
                string nullErrText = ErrorString().Append("Passed in series parameter is null").ToString();
                throw new OmmInvalidUsageException(nullErrText, OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }

            if (!m_LoadedEnumTypeDef)
            {
                throw new OmmInvalidUsageException("The enumerated types dictionary was not loaded",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            series.Clear();

            int enumTypeDictionarySize = fragmentationSize > 0 ? fragmentationSize : DEFAULT_FRAGMENTATION_SIZE;

            series.GetEncoder().AcquireEncodeIterator(enumTypeDictionarySize);

            series.m_bodyBuffer = series.GetEncoder().m_encodeIterator!.Buffer();

            m_RsslInt.Value(currentEnumTableEntry);

            CodecReturnCode ret = m_rsslDataDictionary.EncodeEnumTypeDictionaryAsMultiPart(series.GetEncoder().m_encodeIterator, m_RsslInt,
                (int)verbosity, out m_RsslError);

            if (ret == CodecReturnCode.DICT_PART_ENCODED)
            {
                series.GetEncoder().m_containerComplete = true;
                return (int)m_RsslInt.ToLong();
            }
            else if (ret == CodecReturnCode.SUCCESS)
            {
                series.GetEncoder().m_containerComplete = true;
                return EnumTables().Count;
            }

            series.Clear();
            string errText = ErrorString().Append("Failed to encode the enumerated type definition")
                .Append(ILoggerClient.CR)
                .Append("Reason='")
                .Append(m_RsslError.Text)
                .Append('\'').ToString();

            throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INVALID_DATA);
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// Decode the enumerated types information contained in an encoded enum
    /// types dictionary according to the domain model.
    /// </summary>
    /// <remarks>
    /// This method may be called multiple times on the same dictionary, to load
    /// information from dictionaries that have been encoded in multiple parts.
    /// </remarks>
    /// <param name="series">Series to be used for decoding enumerated types information from.</param>
    /// <param name="verbosity">The desired verbosity to encode. The verbosity values are <br/>
    /// <see cref="EmaRdm.DICTIONARY_MINIMAL"/>, <see cref="EmaRdm.DICTIONARY_NORMAL"/>, <see cref="EmaRdm.DICTIONARY_VERBOSE"/>.</param>
    /// <exception cref="OmmInvalidUsageException">This method if fails to decode enumerated types dictionary.</exception>
    /// <seealso cref="Access.Series"/>
    /// <seealso cref="EmaRdm"/>
    public void DecodeEnumTypeDictionary(Access.Series series, long verbosity)
    {
        m_DictionaryLock.Enter();
        try
        {

            if (m_OwnRsslDataDictionary)
            {
                if (series == null)
                {
                    string errText = ErrorString().Append("Passed in series parameter is null").ToString();
                    throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
                }

                Eta.Codec.Buffer encodedBuffer = series.m_bodyBuffer!;

                Eta.Codec.DecodeIterator decodeIterator = DecodeIterator();

                CodecReturnCode ret = decodeIterator.SetBufferAndRWFVersion(encodedBuffer,
                    Eta.Codec.Codec.MajorVersion(), Eta.Codec.Codec.MinorVersion());

                if (ret != CodecReturnCode.SUCCESS)
                {
                    string errText = ErrorString().Append("Failed to setBufferAndRWFVersion on rssl decode iterator. Reason='")
                            .Append(ret.GetAsString())
                            .Append('\'').ToString();
                    throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
                }

                ret = m_rsslDataDictionary.DecodeEnumTypeDictionary(decodeIterator, (int)verbosity, out m_RsslError);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    string errText = ErrorString().Append("Failed to decode the enumerated types dictionary. Reason='")
                            .Append(m_RsslError.Text)
                            .Append('\'').ToString();
                    throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INVALID_DATA);
                }

                m_LoadedEnumTypeDef = true;
            }
            else
            {
                throw new OmmInvalidUsageException(QUERYING_ONLY_ERROR_TEXT, OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// Extract dictionary type from the encoded payload of a EMA message where the domain
    /// type is DICTIONARY.
    /// </summary>
    /// <param name="series">Series to be used for extracting dictionary type.</param>
    /// <returns>The dictionary type defined in EmaRdm.</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if the dictionary type is not available.</exception>
    /// <seealso cref="Series"/>
    /// <seealso cref="Rdm.EmaRdm"/>
    public int ExtractDictionaryType(Access.Series series)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (series == null)
            {
                string errText = ErrorString().Append("Passed in series parameter is null").ToString();
                throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }

            Eta.Codec.Buffer? encodedBuffer = series.m_bodyBuffer;

            if (encodedBuffer is null)
            {
                encodedBuffer = series.Encoder!.m_encodeIterator!.Buffer();
            }

            Eta.Codec.DecodeIterator decodeIterator = DecodeIterator();

            CodecReturnCode ret = decodeIterator.SetBufferAndRWFVersion(encodedBuffer,
                Eta.Codec.Codec.MajorVersion(), Eta.Codec.Codec.MinorVersion());

            if (ret != CodecReturnCode.SUCCESS)
            {
                string errText = ErrorString()
                    .Append("Failed to setBufferAndRWFVersion on rssl decode iterator. Reason='")
                    .Append(ret.GetAsString())
                    .Append('\'').ToString();
                throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INTERNAL_ERROR);
            }

            Eta.Codec.Int dictionaryType = new();

            if (m_rsslDataDictionary.ExtractDictionaryType(decodeIterator, dictionaryType, out m_RsslError) < CodecReturnCode.SUCCESS)
            {
                string errText = ErrorString().Append("Failed to extract dictionary type. Reason='")
                        .Append(m_RsslError.Text)
                        .Append('\'').ToString();
                throw new OmmInvalidUsageException(errText, OmmInvalidUsageException.ErrorCodes.INVALID_DATA);
            }

            return (int)dictionaryType.ToLong();
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// Check whether the DictionaryEntry exists
    /// </summary>
    /// <param name="fieldName">the field name to check the dictionary Entry</param>
    /// <returns><c>true</c> if the DictionaryEntry exists otherwise <c>false</c></returns>
    public bool HasEntry(string fieldName)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (!m_LoadedFieldDictionary)
            {
                return false;
            }

            Dictionary<string, int>? nameToIdMap = FieldNameToIdMap();

            return (nameToIdMap != null
                && nameToIdMap.ContainsKey(fieldName));
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// Returns the Entry in the dictionary corresponding to the given field name, if the Entry exists.
    /// </summary>
    /// <param name="fieldName">the field name to get the dictionary Entry for</param>
    /// <returns>the dictionary Entry if it exists</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if the Entry does not exist.</exception>
    public DictionaryEntry Entry(string fieldName)
    {
        GetEntry(fieldName, m_DictionaryEntryImpl);
        return m_DictionaryEntryImpl;
    }

    /// <summary>
    /// Returns the Entry in the dictionary corresponding to the given fieldId, if the Entry exists.
    /// </summary>
    /// <remarks>
    /// Same like <see cref="DataDictionary.Entry(string)"/> note, entryDst should be created first and managed by the user
    /// Otherwise Dictionary is owned by API and method will throw <see cref="OmmInvalidUsageException"/>
    /// with <see cref="OmmInvalidUsageException.ErrorCodes.INVALID_USAGE"/> error code.
    /// </remarks>
    /// <param name="fieldName">the field name to get the dictionary Entry for</param>
    /// <param name="entryDst">out parameter, dictionary Entry.</param>
    /// <exception cref="OmmInvalidUsageException">Thrown if the Entry does not exist or entryDst is null or is owned by API.</exception>
    public void Entry(string fieldName, DictionaryEntry entryDst)
    {
        if (entryDst == null)
        {
            throw new OmmInvalidUsageException("DictionaryEntry entryDst parameter is null",
                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }
        if (!entryDst.m_IsManagedByUser)
        {
            throw new OmmInvalidUsageException("DictionaryEntry entryDst parameter should be created by EmaFactory. call",
                OmmInvalidUsageException.ErrorCodes.INVALID_USAGE);
        }
        GetEntry(fieldName, entryDst);
    }

    /// <summary>
    /// Check whether the DictionaryEntry exists
    /// </summary>
    /// <param name="fieldId">the fieldId to check the dictionary Entry</param>
    /// <returns><c>true</c> if the DictionaryEntry exists otherwise <c>false</c></returns>
    public bool HasEntry(int fieldId)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (!m_LoadedFieldDictionary)
            {
                return false;
            }

            return m_rsslDataDictionary.Entry(fieldId) != null;
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// Returns the Entry in the dictionary corresponding to the given fieldId, if the
    /// Entry exists.
    /// </summary>
    /// <param name="fieldId">the fieldId to get the dictionary Entry for</param>
    /// <returns>the dictionary Entry if it exists</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if the Entry does not exist.</exception>
    public DictionaryEntry Entry(int fieldId)
    {
        GetEntry(fieldId, m_DictionaryEntryImpl);
        return m_DictionaryEntryImpl;
    }

    /// <summary>
    /// Returns the Entry in the dictionary corresponding to the given fieldId, if the
    /// Entry exists.
    /// </summary>
    /// <remarks>
    /// Note: entryDst should be created first and managed by the user.<br/>
    /// Otherwise Dictionary is owned by API and method will throw <see cref="OmmInvalidUsageException"/> with <see cref="OmmInvalidUsageException.ErrorCodes.INVALID_USAGE"/> error code
    /// </remarks>
    /// <param name="fieldId">the fieldId to get the dictionary Entry for</param>
    /// <param name="entryDst">out parameter, dictionary Entry.</param>
    /// <exception cref="OmmInvalidUsageException">Thrown if the Entry does not exist or entryDst is null or is owned by API.</exception>
    public void Entry(int fieldId, DictionaryEntry entryDst)
    {
        if (entryDst == null)
        {
            throw new OmmInvalidUsageException("DictionaryEntry entryDst parameter is null",
                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }
        if (!entryDst.m_IsManagedByUser)
        {
            throw new OmmInvalidUsageException("DictionaryEntry entryDst parameter should be created with DictionaryEntry(true) constructor",
                OmmInvalidUsageException.ErrorCodes.INVALID_USAGE);
        }
        GetEntry(fieldId, entryDst);
    }

    /// <summary>
    /// Check whether the EnumEntry exists
    /// </summary>
    /// <param name="fieldId">the fieldId to check the enumerated type</param>
    /// <param name="val">the value of the enumerated type to check</param>
    /// <returns>the enumerated type if it exists</returns>
    public bool HasEnumType(int fieldId, int val)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (!m_LoadedFieldDictionary && !m_LoadedEnumTypeDef)
            {
                return false;
            }

            Eta.Codec.IDictionaryEntry dictionaryEntry;

            dictionaryEntry = m_rsslDataDictionary.Entry(fieldId);

            if (dictionaryEntry != null)
            {
                m_RsslEnumValue.Value(val);
                return m_rsslDataDictionary.EntryEnumType(dictionaryEntry, m_RsslEnumValue) != null ? true : false;
            }

            return false;
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// Returns the corresponding enumerated type in the dictionary Entry's
    /// table, if the type exists.
    /// </summary>
    /// <param name="fieldId">the fieldId to get the enumerated type from</param>
    /// <param name="val">the value of the enumerated type to get</param>
    /// <returns>the enumerated type if it exists</returns>
    /// <exception cref="OmmInvalidUsageException">Thrown if the Entry does not exist.</exception>
    public EnumType EnumType(int fieldId, int val)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (!m_LoadedEnumTypeDef)
            {
                throw new OmmInvalidUsageException("The enumerated types dictionary was not loaded",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            Eta.Codec.IDictionaryEntry dictionaryEntry;

            dictionaryEntry = m_rsslDataDictionary.Entry(fieldId);

            if (dictionaryEntry != null)
            {
                m_RsslEnumValue.Value(val);

                Eta.Codec.IEnumType enumType = m_rsslDataDictionary.EntryEnumType(dictionaryEntry, m_RsslEnumValue);

                if (enumType != null)
                {
                    return m_EnumTypeImpl.SetEnumType(enumType);
                }
            }

            throw new OmmInvalidUsageException($"The enum value {val} for the Field ID {fieldId} does not exist in enumerated type definitions",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    /// <summary>
    /// Check whether the FieldDictionary has been loaded or not.
    /// </summary>
    /// <value><c>true</c> if FieldDictionary has been loaded, otherwise <c>false</c>.</value>
    public bool IsFieldDictionaryLoaded
    {
        get => m_LoadedFieldDictionary;
    }

    /// <summary>
    /// Check whether the EnumTypeDef has been loaded or not.
    /// </summary>
    /// <value><c>true</c> if EnumTypeDef has been loaded, otherwise <c>false</c>.</value>
    public bool IsEnumTypeDefLoaded
    {
        get => m_LoadedEnumTypeDef;
    }

    /// <summary>
    /// Convert information contained in the data dictionary to a string.
    /// </summary>
    /// <value>the string representation of this <see cref="DataDictionary"/></value>
    /// <returns>string containing all information contained in the data dictionary.<br/>
    /// NOTE: This may be a very large string.</returns>
    public override string ToString()
    {
        m_DictionaryLock.Enter();
        try
        {
            if (m_rsslDataDictionary.ToString() == null
                || (!m_LoadedFieldDictionary && !m_LoadedEnumTypeDef))
            {
                return "DataDictionary is not initialized";
            }

            return m_rsslDataDictionary.ToString();
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    #endregion

    #region Constructors

    /// <summary>
    /// Create a new instance of DataDictionary
    /// </summary>
    public DataDictionary()
        :this(true)
    {
    }

#pragma warning disable CS8618
    internal DataDictionary(bool ownDataDictionary)
#pragma warning restore CS8618
    {
        m_OwnRsslDataDictionary = ownDataDictionary;

        if (m_OwnRsslDataDictionary)
        {
            m_rsslDataDictionary = new Eta.Codec.DataDictionary();
        }

        ClearFlags();
    }

    /// <summary>
    /// Creates a deep copy of the <paramref name="other"/> dictionary.
    /// </summary>
    /// <param name="other">DataDictionary that is copied from</param>
    public DataDictionary(DataDictionary other)
    {
        m_OwnRsslDataDictionary = true;

        m_rsslDataDictionary = new Eta.Codec.DataDictionary();

        ClearFlags();

        if (!other.m_LoadedFieldDictionary && !other.m_LoadedEnumTypeDef)
        {
            return;
        }

        Access.Series series = new Access.Series();

        if (other.m_LoadedFieldDictionary)
        {
            other.EncodeFieldDictionary(series, EmaRdm.DICTIONARY_VERBOSE);

            DecodeFieldDictionary(series, EmaRdm.DICTIONARY_VERBOSE);

            series.Clear();
        }

        if (other.m_LoadedEnumTypeDef)
        {
            other.EncodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_VERBOSE);

            DecodeEnumTypeDictionary(series, EmaRdm.DICTIONARY_VERBOSE);

            series.Clear();
        }
    }

    #endregion

    #region Implementation details

    private Eta.Codec.DataDictionary m_rsslDataDictionary;
    private bool m_LoadedFieldDictionary;
    private bool m_LoadedEnumTypeDef;
    private List<DictionaryEntry>? m_DictionaryEntryList;
    private List<EnumTypeTable>? m_EnumTypeTableList;
    private StringBuilder? m_ErrorString;
    private Eta.Codec.DecodeIterator? m_RsslDecodeIterator;
    private bool m_OwnRsslDataDictionary;
    private Dictionary<string, int>? m_FieldNametoIdMap;

    private DictionaryEntry m_DictionaryEntryImpl = new DictionaryEntry(false);
    private EnumType m_EnumTypeImpl = new EnumType();

    private Eta.Codec.Int m_RsslInt = new();
    private Eta.Codec.Enum m_RsslEnumValue = new();
    private Eta.Codec.CodecError m_RsslError = new();
    private LinkedList<DictionaryEntry> m_DictionaryEntryPool = new();
    private LinkedList<EnumTypeTable> m_EnumTypeTablePool = new LinkedList<EnumTypeTable>();

    private MonitorWriteLocker m_DictionaryLock = new MonitorWriteLocker(new object());

    private const int DEFAULT_DICTIONARY_ENTRY_SIZE = 40;
    private const int DEFAULT_ENUM_TABLE_ENTRY_SIZE = 1024;
    private const int DEFAULT_FRAGMENTATION_SIZE = 12800;
    private const string QUERYING_ONLY_ERROR_TEXT = "This DataDictionary instance is used for query data dictionary information only";


    private void GetEntry(string fieldName, DictionaryEntry entryDst)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (!m_LoadedFieldDictionary)
            {
                throw new OmmInvalidUsageException("The field dictionary information was not loaded",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            if (!HasEntry(fieldName))
            {
                throw new OmmInvalidUsageException($"The Field name {fieldName} does not exist in the field dictionary",
                    OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
            }

            GetEntry(FieldNameToIdMap()![fieldName], entryDst);
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    private void GetEntry(int fieldId, DictionaryEntry entryDst)
    {
        m_DictionaryLock.Enter();
        try
        {
            if (!m_LoadedFieldDictionary)
            {
                throw new OmmInvalidUsageException("The field dictionary information was not loaded",
                    OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            }

            Eta.Codec.IDictionaryEntry dictionaryEntry = m_rsslDataDictionary.Entry(fieldId);

            if (dictionaryEntry != null)
            {
                entryDst.UpdateDictionaryEntry(this, dictionaryEntry);
                return;
            }

            throw new OmmInvalidUsageException($"The Field ID {fieldId} does not exist in the field dictionary",
                OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    private StringBuilder ErrorString()
    {
        if (m_ErrorString == null)
        {
            m_ErrorString = new StringBuilder(255);
        }
        else
        {
            m_ErrorString.Clear();
        }

        return m_ErrorString;
    }

    void ClearFlags()
    {
        m_LoadedFieldDictionary = false;
        m_LoadedEnumTypeDef = false;
    }

    internal void rsslDataDictionary(Eta.Codec.DataDictionary dataDictionary)
    {
        m_DictionaryLock.Enter();
        try
        {
            ClearDictionaryEntryList();

            ClearEnumTypeTableList();

            m_FieldNametoIdMap?.Clear();

            if (!m_OwnRsslDataDictionary)
            {
                m_LoadedFieldDictionary = true;
                m_LoadedEnumTypeDef = true;
                m_rsslDataDictionary = dataDictionary;
            }
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    internal Eta.Codec.DataDictionary rsslDataDictionary()
    {
        return m_rsslDataDictionary;
    }

    private void ClearDictionaryEntryList()
    {
        m_DictionaryLock.Enter();
        try
        {
            if (m_DictionaryEntryList != null && m_DictionaryEntryList.Count != 0)
            {
                for (int index = 0; index < m_DictionaryEntryList.Count; index++)
                {
                    m_DictionaryEntryList[index].ReturnToPool(m_DictionaryEntryPool);
                }

                m_DictionaryEntryList.Clear();
            }
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    private void ClearEnumTypeTableList()
    {
        m_DictionaryLock.Enter();
        try
        {
            if (m_EnumTypeTableList != null && m_EnumTypeTableList.Count != 0)
            {
                for (int index = 0; index < m_EnumTypeTableList.Count; index++)
                {
                    m_EnumTypeTableList[index].Clear().ReturnToPool(m_EnumTypeTablePool);
                }

                m_EnumTypeTableList.Clear();
            }
        }
        finally
        {
            m_DictionaryLock.Exit();
        }
    }

    private Dictionary<string, int>? FieldNameToIdMap()
    {
        if (m_LoadedFieldDictionary)
        {
            m_FieldNametoIdMap ??= new(m_rsslDataDictionary!.NumberOfEntries);

            if (m_FieldNametoIdMap.Count == 0)
            {
                Eta.Codec.IDictionaryEntry dictionaryEntry;

                for (int fieldId = m_rsslDataDictionary.MinFid; fieldId <= m_rsslDataDictionary.MaxFid; fieldId++)
                {
                    dictionaryEntry = m_rsslDataDictionary.Entry(fieldId);

                    if (dictionaryEntry != null
                        && dictionaryEntry.GetAcronym().Data() != null)
                    {
                        m_FieldNametoIdMap[dictionaryEntry.GetAcronym().ToString()] = dictionaryEntry.GetFid();
                    }
                }
            }
        }

        return m_FieldNametoIdMap;
    }

    private DictionaryEntry GetDictionaryEntry(DataDictionary dataDictionary, IDictionaryEntry dictionaryEntry)
    {
        DictionaryEntry? dictionaryEntryImpl = m_DictionaryEntryPool.First?.Value;

        if (dictionaryEntryImpl == null)
        {
            // pool is empty, create new entry
            dictionaryEntryImpl = new DictionaryEntry();
        }
        else
        {
            // pool wasn't empty, remove entry from the pool
            m_DictionaryEntryPool.RemoveFirst();
        }

        dictionaryEntryImpl.UpdateDictionaryEntry(dataDictionary, dictionaryEntry);

        return dictionaryEntryImpl;
    }

    private EnumTypeTable GetEnumTypeTable(IEnumTypeTable enumTypeTable)
    {
        EnumTypeTable? enumTypeTableImpl = m_EnumTypeTablePool.First?.Value;

        if (enumTypeTableImpl == null)
        {
            // pool is empty, create new entry
            enumTypeTableImpl = new EnumTypeTable();
        }
        else
        {
            // pool wasn't empty, remove entry from the pool
            m_EnumTypeTablePool.RemoveFirst();
            enumTypeTableImpl.Clear();
        }

        enumTypeTableImpl.rsslEnumTypeTable = enumTypeTable;

        return enumTypeTableImpl;
    }

    private Eta.Codec.DecodeIterator DecodeIterator()
    {
        if (m_RsslDecodeIterator == null)
        {
            m_RsslDecodeIterator = new();
        }

        m_RsslDecodeIterator.Clear();

        return m_RsslDecodeIterator;
    }

    #endregion
}
