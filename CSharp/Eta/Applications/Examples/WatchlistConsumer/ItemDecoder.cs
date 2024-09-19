/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Eta.ValueAdd.WatchlistConsumer;

using System;
using System.Text;

using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

internal class ItemDecoder
{
    private const string FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
    private const string ENUM_TABLE_FILE_NAME = "enumtype.def";
    private readonly DecodeIterator m_DIter = new();
    protected FieldList m_FieldList = new();
    protected FieldEntry m_FieldEntry = new();
    private readonly Map m_Map = new();
    private readonly MapEntry m_MapEntry = new();
    private readonly Eta.Codec.Buffer m_MapKey = new();
    private readonly LocalFieldSetDefDb m_LocalFieldSetDefDb = new();
    private CodecError m_Error = new();
    private readonly UInt m_FidUIntValue = new();
    private readonly Int m_FidIntValue = new();
    private readonly Real m_FidRealValue = new();
    private readonly Eta.Codec.Enum m_FidEnumValue = new();
    private readonly Date m_FidDateValue = new();
    private readonly Time m_FidTimeValue = new();
    private readonly Eta.Codec.DateTime m_FidDateTimeValue = new();
    private readonly Float m_FidFloatValue = new();
    private readonly Eta.Codec.Double m_FidDoubleValue = new();
    private readonly Qos m_FidQosValue = new();
    private readonly State m_FidStateValue = new();
    private readonly StringBuilder m_DisplayStr = new();
    private int m_IndentCount;
    private static readonly string[] m_Indents = new[] { "", "    ", "        ", "            " };
    private readonly Vector m_IdVectorValue = new();
    private readonly Eta.Codec.Array m_FidArrayValue = new();
    private readonly FieldList m_EmbeddedFieldList = new();
    private readonly FieldEntry m_EmbeddedFieldEntry = new();
    private readonly VectorEntry m_VectorEntry = new();
    private readonly ArrayEntry m_ArrayEntry = new();
    private readonly Eta.Codec.Buffer m_idBufferValue = new();

    private bool IsDictionaryLoaded =>
        (FieldDictionaryLoadedFromFile && EnumTypeDictionaryLoadedFromFile) ||
                    (FieldDictionaryDownloadedFromNetwork && EnumTypeDictionaryDownloadedFromNetwork);

    public bool FieldDictionaryDownloadedFromNetwork { get; set; }

    public DataDictionary Dictionary { get; set; } = new();
    public bool FieldDictionaryLoadedFromFile { get; set; }
    public bool EnumTypeDictionaryLoadedFromFile { get; set; }
    public bool EnumTypeDictionaryDownloadedFromNetwork { get; set; }

    /// <summary>
    /// Initializes the item decoder. Loads dictionaries from their files if the files are found in the application's folder.
    /// </summary>
    public void Init() =>
        LoadDictionary();

    private void LoadDictionary()
    {
        Dictionary.Clear();
        if (Dictionary.LoadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, out m_Error) < 0)
        {
            Console.WriteLine("Unable to load field dictionary.  Will attempt to download from provider.\n\tText: "
                    + m_Error.Text);
        }
        else
        {
            FieldDictionaryLoadedFromFile = true;
        }

        if (Dictionary.LoadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, out m_Error) < 0)
        {
            Console.WriteLine("Unable to load enum dictionary.  Will attempt to download from provider.\n\tText: "
                        + m_Error.Text);
        }
        else
        {
            EnumTypeDictionaryLoadedFromFile = true;
        }
    }

    /// <summary>
    /// Decodes the payload of a message according to its domain.
    /// </summary>
    /// <param name="channel">Reactor channel.</param>
    /// <param name="msg">Message.</param>
    /// <returns>Code.</returns>
    internal CodecReturnCode DecodeDataBody(ReactorChannel channel, IMsg msg)
    {
        if (msg.ContainerType == DataTypes.NO_DATA)
        {
            return CodecReturnCode.SUCCESS;
        }

        switch (msg.DomainType)
        {
            case (int)DomainType.MARKET_PRICE:
                return DecodeMarketPriceDataBody(channel, msg);

            case (int)DomainType.MARKET_BY_ORDER:
                return DecodeMarketByOrderDataBody(channel, msg);

            case (int)DomainType.MARKET_BY_PRICE:
                return DecodeMarketByPriceDataBody(channel, msg);

            case (int)DomainType.YIELD_CURVE:
                return DecodeYieldCurveDataBody(channel, msg);

            case (int)DomainType.SYMBOL_LIST:
                return DecodeSymbolListDataBody(channel, msg);

            default:
                Console.WriteLine($"Received message with unhandled domain {msg.DomainType}\n\n");
                return CodecReturnCode.FAILURE;
        }
    }

    /// <summary>
    /// Decodes a MarketPrice payload. MarketPrice is represented as an RsslFieldList, where each field contains data about the item.
    /// </summary>
    private CodecReturnCode DecodeMarketPriceDataBody(ReactorChannel channel, IMsg msg)
    {
        if (!IsDictionaryLoaded)
        {
            Console.WriteLine("  (Dictionary not loaded).\n");
            return CodecReturnCode.FAILURE;
        }

        if (msg.ContainerType != DataTypes.FIELD_LIST)
        {
            Console.WriteLine("  Incorrect container type: " + msg.ContainerType);
            return CodecReturnCode.FAILURE;
        }

        m_DisplayStr.Length = 0;
        m_DIter.Clear();
        m_DIter.SetBufferAndRWFVersion(msg.EncodedDataBody, channel.MajorVersion, channel.MinorVersion);

        CodecReturnCode ret = m_FieldList.Decode(m_DIter, null);
        if (ret != CodecReturnCode.SUCCESS)
        {
            Console.WriteLine("FieldList.Decode() failed with return code: " + ret);
            return ret;
        }

        // decode each field entry in list
        while ((ret = m_FieldEntry.Decode(m_DIter)) != CodecReturnCode.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("FieldEntry.Decode() failed with return code: " + ret);
                return ret;
            }

            ret = DecodeFieldEntry(m_FieldEntry, m_DIter, Dictionary, m_DisplayStr);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("DecodeFieldEntry() failed");
                return ret;
            }
            m_DisplayStr.Append("\n");
        }
        Console.WriteLine(m_DisplayStr.ToString());

        return CodecReturnCode.SUCCESS;
    }

    /// <summary>
    /// Decodes a MarketByOrder payload.
    /// MarketByOrder is represented as an RsslMap, where each entry represents an order.
    /// Each entry contains an RsslFieldList which contains data about that order.
    /// </summary>
    /// <param name="channel">Reactor channel.</param>
    /// <param name="msg">Message</param>
    private CodecReturnCode DecodeMarketByOrderDataBody(ReactorChannel channel, IMsg msg)
    {
        if (msg.ContainerType != DataTypes.MAP)
        {
            Console.WriteLine("  Incorrect container type: " + msg.ContainerType);
            return CodecReturnCode.FAILURE;
        }

        m_DisplayStr.Length = 0;
        m_DIter.Clear();
        m_DIter.SetBufferAndRWFVersion(msg.EncodedDataBody,
                channel.MajorVersion, channel.MinorVersion);
        CodecReturnCode ret;

        if ((ret = m_Map.Decode(m_DIter)) != CodecReturnCode.SUCCESS)
        {
            Console.WriteLine("Map.Decode() failed with return code: " + ret);
            return ret;
        }

        //decode set definition database
        if (m_Map.CheckHasSetDefs())
        {
            /*
             * decode set definition - should be field set definition
             */
            /*
             * this needs to be passed in when we decode each field list
             */
            m_LocalFieldSetDefDb.Clear();
            ret = m_LocalFieldSetDefDb.Decode(m_DIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("LocalFieldSetDefDb.Decode() failed: <" + ret.GetAsString() + ">");
                return ret;
            }
        }

        if (m_Map.CheckHasSummaryData())
        {
            ret = DecodeSummaryData(m_DIter, Dictionary, m_DisplayStr);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
        }

        ret = DecodeMap(m_DIter, Dictionary, m_DisplayStr);
        if (ret != CodecReturnCode.SUCCESS)
            return ret;
        Console.WriteLine(m_DisplayStr.ToString());
        return CodecReturnCode.SUCCESS;
    }

    /// <summary>
    /// Decodes a MarketByPrice payload.
    /// MarketByPrice is represented as an RsslMap, where each entry represents a price point.
    /// Each entry contains an RsslFieldList which contains data about that price point.
    /// </summary>
    /// <param name="channel">Reactor channel.</param>
    /// <param name="msg">Message</param>
    /// <returns></returns>
    private CodecReturnCode DecodeMarketByPriceDataBody(ReactorChannel channel, IMsg msg)
    {
        if (msg.ContainerType != DataTypes.MAP)
        {
            Console.WriteLine("  Incorrect container type: " + msg.ContainerType);
            return CodecReturnCode.FAILURE;
        }

        m_DisplayStr.Length = 0;
        m_DIter.Clear();
        m_DIter.SetBufferAndRWFVersion(msg.EncodedDataBody,
                channel.MajorVersion, channel.MinorVersion);

        CodecReturnCode ret = m_Map.Decode(m_DIter);
        if (ret != CodecReturnCode.SUCCESS)
        {
            Console.WriteLine("Map.Decode() failed with return code: " + ret);
            return ret;
        }

        if (m_Map.CheckHasSetDefs())
        {
            m_LocalFieldSetDefDb.Clear();
            ret = m_LocalFieldSetDefDb.Decode(m_DIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("LocalFieldSetDefDb.Decode() failed");
                return ret;
            }
        }
        if (m_Map.CheckHasSummaryData())
        {
            ret = DecodeSummaryData(m_DIter, Dictionary, m_DisplayStr);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
        }

        ret = DecodeMap(m_DIter, Dictionary, m_DisplayStr);
        if (ret != CodecReturnCode.SUCCESS)
            return ret;

        Console.WriteLine(m_DisplayStr.ToString());
        return CodecReturnCode.SUCCESS;
    }

    /// <summary>
    /// Decodes a YieldCurve payload.
    /// YieldCurve is represented as an RsslFieldList, where each field contains data about the item.
    /// </summary>
    /// <param name="channel">Reactor channel.</param>
    /// <param name="msg">Message</param>
    private CodecReturnCode DecodeYieldCurveDataBody(ReactorChannel channel, IMsg msg)
    {
        if (!IsDictionaryLoaded)
        {
            Console.WriteLine("  (Dictionary not loaded).\n");
            return CodecReturnCode.FAILURE;
        }

        if (msg.ContainerType != DataTypes.FIELD_LIST)
        {
            Console.WriteLine("  Incorrect container type: " + msg.ContainerType);
            return CodecReturnCode.FAILURE;
        }

        m_DisplayStr.Length = 0;
        m_DIter.Clear();
        m_DIter.SetBufferAndRWFVersion(msg.EncodedDataBody, channel.MajorVersion, channel.MinorVersion);

        CodecReturnCode ret = DecodeFieldListForYieldCurve(m_DIter, Dictionary, m_FieldList, m_FieldEntry);

        if (ret != CodecReturnCode.SUCCESS)
        {
            Console.WriteLine("DecodeFieldListForYieldCurve() failed with return code: " + ret);
            return ret;
        }
        return CodecReturnCode.SUCCESS;
    }

    /// <summary>
    /// Decodes a SymbolList payload.
    /// SymbolList is represented as an RsslMap, where each entry represents an item.
    /// The entries are indexed by the item's name.
    /// </summary>
    /// <param name="channel">Reactor channel.</param>
    /// <param name="msg">Message</param>
    /// <returns></returns>
    private CodecReturnCode DecodeSymbolListDataBody(ReactorChannel channel, IMsg msg)
    {
        if (msg.ContainerType != DataTypes.MAP)
        {
            Console.WriteLine("  Incorrect container type: " + msg.ContainerType);
            return CodecReturnCode.FAILURE;
        }

        m_DIter.Clear();
        m_DIter.SetBufferAndRWFVersion(msg.EncodedDataBody,
                channel.MajorVersion, channel.MinorVersion);

        CodecReturnCode ret = m_Map.Decode(m_DIter);
        if (ret != CodecReturnCode.SUCCESS)
        {
            Console.WriteLine("Map.Decode() failed with return code: " + ret);
            return ret;
        }

        if (m_Map.CheckHasSetDefs())
        {
            m_LocalFieldSetDefDb.Clear();
            ret = m_LocalFieldSetDefDb.Decode(m_DIter);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("LocalFieldSetDefDb.Decode() failed");
                return ret;
            }
        }
        if (m_Map.CheckHasSummaryData())
        {
            ret = DecodeSummaryData(m_DIter, Dictionary, m_DisplayStr);
            if (ret != CodecReturnCode.SUCCESS)
                return ret;
        }

        while ((ret = m_MapEntry.Decode(m_DIter, m_MapKey)) != CodecReturnCode.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("MapEntry.Decode() failed: < " + ret.GetAsString() + ">");
                return ret;
            }
            Console.WriteLine(m_MapKey.ToString() + "\t" +
                        MapEntryActionToString(m_MapEntry.Action));
        }
        return CodecReturnCode.SUCCESS;
    }

    private CodecReturnCode DecodeMap(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
    {
        string actionstring;
        CodecReturnCode ret;
        /* decode the map */
        while ((ret = m_MapEntry.Decode(dIter, m_MapKey)) != CodecReturnCode.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("MapEntry.Decode() failed with return code: " + ret);
                return ret;
            }

            //convert the action to a string for display purposes
            actionstring = m_MapEntry.Action switch
            {
                MapEntryActions.UPDATE => "UPDATE",
                MapEntryActions.ADD => "ADD",
                MapEntryActions.DELETE => "DELETE",
                _ => "Unknown",
            };
            //print out the key
            if (m_MapKey.Length > 0)
            {
                fieldValue.Append("ORDER ID: " + m_MapKey.ToString() + "\nACTION: "
                        + actionstring + "\n");
            }

            //there is not any payload in delete actions
            if (m_MapEntry.Action != MapEntryActions.DELETE)
            {
                ret = DecodeFieldList(dIter, dictionary, fieldValue);
                if (ret != CodecReturnCode.SUCCESS)
                    return ret;
            }
        }
        return CodecReturnCode.SUCCESS;
    }

    private CodecReturnCode DecodeFieldListForYieldCurve(DecodeIterator dIter, DataDictionary dictionary, FieldList localFieldList, FieldEntry localFieldEntry)
    {
        localFieldList.Clear();

        CodecReturnCode ret = localFieldList.Decode(dIter, m_LocalFieldSetDefDb);
        if (ret < CodecReturnCode.SUCCESS)
        {
            Console.WriteLine("LocalFieldList.Decode() failed with return code: " + ret);
            return ret;
        }

        localFieldEntry.Clear();

        m_IndentCount++;

        // decode each field entry in list
        while ((ret = localFieldEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
        {
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("LocalFieldEntry.Decode() failed with return code: " + ret);
                return ret;
            }
            // get dictionary entry
            IDictionaryEntry dictionaryEntry = dictionary.Entry(localFieldEntry.FieldId);

            // return if no entry found
            if (dictionaryEntry == null)
            {
                Console.WriteLine("\tFid " + localFieldEntry.FieldId
                        + " not found in dictionary");
                Console.WriteLine(localFieldEntry.EncodedData.ToHexString());
                return CodecReturnCode.SUCCESS;
            }

            // print out fid name
            Console.Write(m_Indents[m_IndentCount] + dictionaryEntry.GetAcronym().ToString());
            for (int i = 0; i < 40 - m_Indents[m_IndentCount].Length - dictionaryEntry.GetAcronym().Length; i++)
            {
                Console.Write(" ");
            }

            // decode and print out fid value
            int dataType = dictionaryEntry.GetRwfType();

            switch (dataType)
            {
                case DataTypes.VECTOR:
                    ret = DecodeVector(dIter, dictionary);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine("DecodeVector inside FieldList failed");
                        return ret;
                    }
                    break;

                case DataTypes.ARRAY:
                    ret = DecodeArray(dIter);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine("DecodeArray inside FieldList failed");
                        return ret;
                    }
                    break;

                default:
                    ret = DecodePrimitive(dIter, dataType, false);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        Console.WriteLine("DecodePrimitive inside FieldList failed");
                        return ret;
                    }
                    break;
            }
        }
        m_IndentCount--;
        return CodecReturnCode.SUCCESS;
    }

    private CodecReturnCode DecodeVector(DecodeIterator dIter, DataDictionary dictionary)
    {
        CodecReturnCode ret = m_IdVectorValue.Decode(dIter);
        if (ret < CodecReturnCode.SUCCESS)
        {
            Console.WriteLine("FidVectorValue.Decode() failed: <" + ret.GetAsString() + ">");
            return ret;
        }
        if (ret == CodecReturnCode.NO_DATA)
        {
            Console.WriteLine("<no data>");
            return CodecReturnCode.SUCCESS;
        }
        if (m_IdVectorValue.CheckHasSummaryData())
        {
            Console.WriteLine();
            // fieldList inside summaryData within vector
            ret = DecodeFieldListForYieldCurve(dIter, dictionary, m_EmbeddedFieldList, m_EmbeddedFieldEntry);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("DecodeSummaryData failed: <" + ret.GetAsString() + ">");
                return ret;
            }
        }
        // If the vector flags indicate that set definition content is present,
        // decode the set def db
        if (m_IdVectorValue.CheckHasSetDefs())
        {
            if (m_IdVectorValue.ContainerType == DataTypes.FIELD_LIST)
            {
                m_LocalFieldSetDefDb.Clear();
                ret = m_LocalFieldSetDefDb.Decode(dIter);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    Console.WriteLine("LocalFieldSetDefDb.Decode() failed: <" + ret.GetAsString() + ">");
                    return ret;
                }
            }
        }

        m_IndentCount++;
        Console.WriteLine();

        m_VectorEntry.Clear();
        while ((ret = m_VectorEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
        {
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.Write("Error {0} ({1:D}) encountered with DecodeVectorEntry.  Error Text: {2}\n",
                                ret.GetAsString(), ret, ret.GetAsInfo());
                return ret;
            }

            Console.WriteLine(m_Indents[m_IndentCount] + "INDEX: " + m_VectorEntry.Index);
            Console.Write(m_Indents[m_IndentCount] + "ACTION: ");
            switch (m_VectorEntry.Action)
            {
                case VectorEntryActions.UPDATE:
                    Console.WriteLine("UPDATE_ENTRY");
                    break;

                case VectorEntryActions.SET:
                    Console.WriteLine("SET_ENTRY");
                    break;

                case VectorEntryActions.CLEAR:
                    Console.WriteLine("CLEAR_ENTRY");
                    break;

                case VectorEntryActions.INSERT:
                    Console.WriteLine("INSERT_ENTRY");
                    break;

                case VectorEntryActions.DELETE:
                    Console.WriteLine("DELETE_ENTRY");
                    break;

                default:
                    Console.WriteLine("UNKNOWN");
                    break;
            }

            /* Continue decoding vector entries. */
            switch (m_IdVectorValue.ContainerType)
            {
                case DataTypes.FIELD_LIST:
                    // fieldList inside vectorEntry within vector
                    ret = DecodeFieldListForYieldCurve(dIter, dictionary, m_EmbeddedFieldList, m_EmbeddedFieldEntry);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        Console.Write("Error {0} ({1:D}) encountered with decoding FieldList within Vector: {2}\n",
                                        ret.GetAsString(), ret, ret.GetAsInfo());
                        return ret;
                    }
                    break;

                case DataTypes.ARRAY:
                    ret = DecodeArray(dIter);
                    if (ret < CodecReturnCode.SUCCESS)
                    {
                        Console.Write("Error {0} ({1:D}) encountered with decoding ARRAY within Vector: {2}\n",
                                        ret.GetAsString(), ret, ret.GetAsInfo());
                        return ret;
                    }
                    break;

                default:
                    Console.WriteLine("Error: Vector contained unhandled containerType " + m_IdVectorValue.ContainerType);
                    break;
            }
        }
        m_IndentCount--;
        return CodecReturnCode.SUCCESS;
    }

    private CodecReturnCode DecodePrimitive(DecodeIterator dIter, int dataType, bool isArray)
    {
        CodecReturnCode ret = 0;

        switch (dataType)
        {
            case DataTypes.INT:
                ret = m_FidIntValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    Console.Write(m_FidIntValue.ToLong());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("Int.Decode() failed: <" + ret.GetAsString() + ">");
                    return ret;
                }
                break;

            case DataTypes.REAL:
                ret = m_FidRealValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    Console.Write(m_FidRealValue.ToDouble());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("Real.Decode() failed: <" + ret.GetAsString() + ">");
                    return ret;
                }
                break;

            case DataTypes.DATE:
                ret = m_FidDateValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    Console.Write(m_FidDateValue.ToString());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("Date.Decode() failed: <" + ret.GetAsString() + ">");
                    return ret;
                }
                break;

            case DataTypes.TIME:
                ret = m_FidTimeValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    Console.Write(m_FidTimeValue.ToString());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("Time.Decode() failed: <" + ret.GetAsString() + ">");
                    return ret;
                }
                break;

            case DataTypes.DATETIME:
                ret = m_FidDateTimeValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    Console.Write(m_FidDateTimeValue.ToString());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("DateTime.Decode() failed: <" + ret.GetAsString() + ">");
                    return ret;
                }
                break;

            case DataTypes.ARRAY:
                ret = DecodeArray(dIter);
                if (ret < CodecReturnCode.SUCCESS)
                {
                    Console.Write("Error {0} ({1:D}) encountered with decoding ARRAY was primitive: {2}\n",
                                            ret.GetAsString(), ret, ret.GetAsInfo());
                    return ret;
                }
                break;

            case DataTypes.BUFFER:
            case DataTypes.ASCII_STRING:
            case DataTypes.UTF8_STRING:
            case DataTypes.RMTES_STRING:
                ret = m_idBufferValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    if (isArray)
                        Console.Write("\"");
                    Console.Write(m_idBufferValue.ToString());
                    if (isArray)
                        Console.Write("\"");
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("Buffer.Decode() failed: <" + ret.GetAsString() + ">");
                    return ret;
                }
                break;

            default:
                Console.Write("Unsupported data type (" + DataTypes.ToString(dataType) + ")");
                break;
        }
        if (ret == CodecReturnCode.BLANK_DATA)
        {
            Console.Write("<blank data>");
        }

        if (!isArray)
            Console.Write("\n");

        return CodecReturnCode.SUCCESS;
    }

    private CodecReturnCode DecodeArray(DecodeIterator dIter)
    {
        bool firstArrayEntry = true;

        Console.Write("{ ");

        m_FidArrayValue.Clear();
        CodecReturnCode ret = m_FidArrayValue.Decode(dIter);
        if (ret < CodecReturnCode.SUCCESS)
        {
            Console.WriteLine("Array.Decode() failed: <" + ret.GetAsString() + ">");
            return ret;
        }

        int dataType = m_FidArrayValue.PrimitiveType;

        m_ArrayEntry.Clear();
        while ((ret = m_ArrayEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
        {
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.Write("Error {0} ({1:D}) encountered with DecodeArrayEntry.  Error Text: {2}\n",
                                ret.GetAsString(), ret, ret.GetAsInfo());
                return ret;
            }

            if (firstArrayEntry)
                firstArrayEntry = false;
            else
                Console.Write(", ");
            ret = DecodePrimitive(dIter, dataType, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                Console.Write("Error {0} ({1:D}) encountered with DecodeArrayEntryPrimitives.  Error Text: {2}\n",
                                ret.GetAsString(), ret, ret.GetAsInfo());
                return ret;
            }
        }

        Console.Write(" }\n");

        return CodecReturnCode.SUCCESS;
    }

    private CodecReturnCode DecodeFieldList(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
    {
        //decode field list
        CodecReturnCode ret = m_FieldList.Decode(dIter, m_LocalFieldSetDefDb);
        if (ret != CodecReturnCode.SUCCESS)
        {
            Console.WriteLine("FieldList.Decode() failed: <" + ret.GetAsString() + ">");
            return ret;
        }

        //decode each field entry in list
        while ((ret = m_FieldEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("FieldEntry.Decode() failed: <" + ret.GetAsString() + ">");
                return ret;
            }

            ret = DecodeFieldEntry(m_FieldEntry, dIter, dictionary, fieldValue);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("Entry.Field() failed");
                return ret;
            }
            fieldValue.Append("\n");
        }

        return CodecReturnCode.SUCCESS;
    }

    /// <summary>
    /// This is used by all market price domain handlers to output field lists.
    /// Decodes the field entry data and prints out the field entry data with help of the dictionary. Returns success if decoding succeeds or failure if decoding fails.
    /// </summary>
    /// <param name="fEntry">Field entry</param>
    /// <param name="dIter">Decode iterator</param>
    /// <param name="dictionary">Data dictionary</param>
    /// <param name="fieldValue"></param>
    protected CodecReturnCode DecodeFieldEntry(FieldEntry fEntry, DecodeIterator dIter,
        DataDictionary dictionary, StringBuilder fieldValue)
    {
        // get dictionary entry
        IDictionaryEntry dictionaryEntry = dictionary.Entry(fEntry.FieldId);

        // return if no entry found
        if (dictionaryEntry == null)
        {
            fieldValue.Append("\tFid " + fEntry.FieldId + " not found in dictionary");
            return CodecReturnCode.SUCCESS;
        }

        // print out fid name
        fieldValue.Append("\t" + fEntry.FieldId + "/" + dictionaryEntry.GetAcronym().ToString() + ": ");

        // decode and print out fid value
        int dataType = dictionaryEntry.GetRwfType();
        CodecReturnCode ret = 0;
        switch (dataType)
        {
            case DataTypes.UINT:
                ret = m_FidUIntValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    fieldValue.Append(m_FidUIntValue.ToLong());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("UInt.Decode() failed: <" + ret.GetAsString() + ">");
                    return ret;
                }
                break;

            case DataTypes.INT:
                ret = m_FidIntValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    fieldValue.Append(m_FidIntValue.ToLong());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("Int.Decode() failed: <" + ret.GetAsString() + ">");

                    return ret;
                }
                break;

            case DataTypes.FLOAT:
                ret = m_FidFloatValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    fieldValue.Append(m_FidFloatValue.ToFloat());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("Float.Decode() failed: <" + ret.GetAsString() + ">");

                    return ret;
                }
                break;

            case DataTypes.DOUBLE:
                ret = m_FidDoubleValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    fieldValue.Append(m_FidDoubleValue.ToDouble());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("Double.Decode() failed: <" + ret.GetAsString() + ">");

                    return ret;
                }
                break;

            case DataTypes.REAL:
                ret = m_FidRealValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    fieldValue.Append(m_FidRealValue.ToDouble());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("Real.Decode() failed: <" + ret.GetAsString() + ">");

                    return ret;
                }
                break;

            case DataTypes.ENUM:
                ret = m_FidEnumValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    IEnumType enumType = dictionary.EntryEnumType(dictionaryEntry,
                                                                 m_FidEnumValue);

                    if (enumType == null)
                    {
                        fieldValue.Append(m_FidEnumValue.ToInt());
                    }
                    else
                    {
                        fieldValue.Append(enumType.Display.ToString() + "(" +
                                m_FidEnumValue.ToInt() + ")");
                    }
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("Enum.Decode() failed: <" + ret.GetAsString() + ">");

                    return ret;
                }
                break;

            case DataTypes.DATE:
                ret = m_FidDateValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    fieldValue.Append(m_FidDateValue.ToString());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("Date.Decode() failed: <" +
                            ret.GetAsString() + ">");

                    return ret;
                }
                break;

            case DataTypes.TIME:
                ret = m_FidTimeValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    fieldValue.Append(m_FidTimeValue.ToString());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("Time.Decode() failed: <" +
                            ret.GetAsString() + ">");

                    return ret;
                }
                break;

            case DataTypes.DATETIME:
                ret = m_FidDateTimeValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    fieldValue.Append(m_FidDateTimeValue.ToString());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("DateTime.Decode() failed: <" + ret.GetAsString() + ">");
                    return ret;
                }
                break;

            case DataTypes.QOS:
                ret = m_FidQosValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    fieldValue.Append(m_FidQosValue.ToString());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("Qos.Decode() failed: <" + ret.GetAsString() + ">");

                    return ret;
                }
                break;

            case DataTypes.STATE:
                ret = m_FidStateValue.Decode(dIter);
                if (ret == CodecReturnCode.SUCCESS)
                {
                    fieldValue.Append(m_FidStateValue.ToString());
                }
                else if (ret != CodecReturnCode.BLANK_DATA)
                {
                    Console.WriteLine("State.Decode() failed: <" + ret.GetAsString() + ">");

                    return ret;
                }
                break;
            // For an example of array decoding, see
            // FieldListCodec.ExampleDecode()
            case DataTypes.ARRAY:
                break;

            case DataTypes.BUFFER:
            case DataTypes.ASCII_STRING:
            case DataTypes.UTF8_STRING:
            case DataTypes.RMTES_STRING:
                if (fEntry.EncodedData.Length > 0)
                {
                    fieldValue.Append(fEntry.EncodedData.ToString());
                }
                else
                {
                    ret = CodecReturnCode.BLANK_DATA;
                }
                break;

            default:
                fieldValue.Append("Unsupported data type (" + DataTypes.ToString(dataType) + ")");
                break;
        }
        if (ret == CodecReturnCode.BLANK_DATA)
        {
            fieldValue.Append("<blank data>");
        }

        return CodecReturnCode.SUCCESS;
    }

    private CodecReturnCode DecodeSummaryData(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
    {
        CodecReturnCode ret = m_FieldList.Decode(dIter, m_LocalFieldSetDefDb);
        if (ret != CodecReturnCode.SUCCESS)
        {
            Console.WriteLine("FieldList.Decode() failed: <" + ret.GetAsString() + ">");
            return ret;
        }

        fieldValue.Append("SUMMARY DATA\n");
        //decode each field entry in list
        while ((ret = m_FieldEntry.Decode(dIter)) != CodecReturnCode.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("FieldEntry.Decode() failed: <" + ret.GetAsString() + ">");
                return ret;
            }

            ret = DecodeFieldEntry(m_FieldEntry, dIter, dictionary, fieldValue);
            if (ret != CodecReturnCode.SUCCESS)
            {
                Console.WriteLine("FieldEntry.Decode() failed");
                return ret;
            }
            fieldValue.Append("\n");
        }

        return CodecReturnCode.SUCCESS;
    }

    private static string MapEntryActionToString(MapEntryActions mapEntryAction)
    {
        switch (mapEntryAction)
        {
            case MapEntryActions.UPDATE:
                return "UPDATE";

            case MapEntryActions.ADD:
                return "ADD";

            case MapEntryActions.DELETE:
                return "DELETE";

            default:
                return "Unknown Map Entry Action";
        }
    }
}
