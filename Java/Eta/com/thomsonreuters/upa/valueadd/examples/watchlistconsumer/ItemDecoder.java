package com.thomsonreuters.upa.valueadd.examples.watchlistconsumer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.DictionaryEntry;
import com.thomsonreuters.upa.codec.EnumType;
import com.thomsonreuters.upa.codec.FieldEntry;
import com.thomsonreuters.upa.codec.FieldList;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.LocalFieldSetDefDb;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.Real;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;


public class ItemDecoder 
{	
    private final String FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
    private final String ENUM_TABLE_FILE_NAME = "enumtype.def";     	
	private DataDictionary dictionary;
    DecodeIterator dIter = CodecFactory.createDecodeIterator();		
	boolean fieldDictionaryLoadedFromFile;
	boolean enumTypeDictionaryLoadedFromFile;
    protected FieldList fieldList = CodecFactory.createFieldList();
    protected FieldEntry fieldEntry = CodecFactory.createFieldEntry();    
    private Map map = CodecFactory.createMap();
    private MapEntry mapEntry = CodecFactory.createMapEntry();
    private Buffer mapKey = CodecFactory.createBuffer();
    private LocalFieldSetDefDb localFieldSetDefDb = CodecFactory.createLocalFieldSetDefDb();
    private Error error;       
    private UInt fidUIntValue = CodecFactory.createUInt();
    private Int fidIntValue = CodecFactory.createInt();
    private Real fidRealValue = CodecFactory.createReal();
    private com.thomsonreuters.upa.codec.Enum fidEnumValue = CodecFactory.createEnum();
    private com.thomsonreuters.upa.codec.Date fidDateValue = CodecFactory.createDate();
    private Time fidTimeValue = CodecFactory.createTime();
    private DateTime fidDateTimeValue = CodecFactory.createDateTime();
    private com.thomsonreuters.upa.codec.Float fidFloatValue = CodecFactory.createFloat();
    private com.thomsonreuters.upa.codec.Double fidDoubleValue = CodecFactory.createDouble();
    private Qos fidQosValue = CodecFactory.createQos();
    private State fidStateValue = CodecFactory.createState();    
    StringBuilder displayStr = new StringBuilder();

	/* Initializes the item decoder. Loads dictionaries from their files if the files
	 * are found in the application's folder. */
	public void init()
	{
	    dictionary = CodecFactory.createDataDictionary();
	        
		fieldDictionaryLoadedFromFile = false;
		enumTypeDictionaryLoadedFromFile = false;
	    error = TransportFactory.createError();
		loadDictionary();
	}
	
	void loadDictionary()
    {
        dictionary.clear();
        if (dictionary.loadFieldDictionary(FIELD_DICTIONARY_FILE_NAME, error) < 0)
        {
            System.out.println("Unable to load field dictionary.  Will attempt to download from provider.\n\tText: "
                    + error.text());
        }
        else
        {
            fieldDictionaryLoadedFromFile = true;
        }

        if (dictionary.loadEnumTypeDictionary(ENUM_TABLE_FILE_NAME, error) < 0)
        {
            System.out.println("Unable to load enum dictionary.  Will attempt to download from provider.\n\tText: "
                        + error.text());
        }
        else
        {
            enumTypeDictionaryLoadedFromFile = true;
        }
    }	
	
	public DataDictionary getDictionary() 
	{
		return dictionary;
	}

	/* Decodes the payload of a message according to its domain. */
	int decodeDataBody(ReactorChannel channel, Msg msg)
	{
		switch (msg.domainType())
		{
			case DomainTypes.MARKET_PRICE:
				return decodeMarketPriceDataBody(channel, msg);

			case  DomainTypes.MARKET_BY_ORDER:
				return decodeMarketByOrderDataBody(channel, msg);

			case  DomainTypes.MARKET_BY_PRICE: 
				return decodeMarketByPriceDataBody(channel, msg);

			case DomainTypes.YIELD_CURVE:
				return decodeYieldCurveDataBody(channel, msg);
				
			case  DomainTypes.SYMBOL_LIST:
				return decodeSymbolListDataBody(channel, msg);

			default:
				System.out.println("Received message with unhandled domain %u\n\n" + 
						msg.domainType());
				return CodecReturnCodes.FAILURE;
		}
	}
	
	/* Decodes a MarketPrice payload.
	 * MarketPrice is represented as an RsslFieldList, where each field contains data about the item. */
	int decodeMarketPriceDataBody(ReactorChannel channel, Msg msg)
	{
		if (!fieldDictionaryLoadedFromFile|| !enumTypeDictionaryLoadedFromFile)
		{
			System.out.println("  (Dictionary not loaded).\n");
			return CodecReturnCodes.FAILURE;
		}

		if (msg.containerType() != DataTypes.FIELD_LIST)
		{
			System.out.println("  Incorrect container type: " + msg.containerType());
			return CodecReturnCodes.FAILURE;
		}

		displayStr.setLength(0);
		dIter.clear();
		dIter.setBufferAndRWFVersion(msg.encodedDataBody(), channel.majorVersion(), channel.minorVersion());
				
	    int ret = fieldList.decode(dIter, null);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	System.out.println("DecodeFieldList() failed with return code: " + ret);
	    	return ret;
	    }

	    // decode each field entry in list
	    while ((ret = fieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
	    {
	    	if (ret != CodecReturnCodes.SUCCESS)
	    	{
	    		System.out.println("decodeFieldEntry() failed with return code: " + ret);
	    		return ret;
	    	}

	        ret = decodeFieldEntry(fieldEntry, dIter, dictionary, displayStr);
	        if (ret != CodecReturnCodes.SUCCESS)
	        {
	        	System.out.println("decodeFieldEntry() failed");
	        	return ret;
	        }
	        displayStr.append("\n");
	    }
	    System.out.println(displayStr.toString());

	    return CodecReturnCodes.SUCCESS;
	}		
		
	/* Decodes a MarketByOrder payload.
	 * MarketByOrder is represented as an RsslMap, where each entry represents an order.
	 * Each entry contains an RsslFieldList which contains data about that order. */
	int decodeMarketByOrderDataBody(ReactorChannel channel, Msg msg)
	{
		if (msg.containerType() != DataTypes.MAP)
		{
			System.out.println("  Incorrect container type: " + msg.containerType());
			return CodecReturnCodes.FAILURE;
		}

		displayStr.setLength(0);
		dIter.clear();
		dIter.setBufferAndRWFVersion(msg.encodedDataBody(),
				channel.majorVersion(), channel.minorVersion());
		int ret;

        if ((ret = map.decode(dIter)) != CodecReturnCodes.SUCCESS)
        {
            System.out.println("Map.Decode() failed with return code: " + ret);
            return ret;
        }

        //decode set definition database
        if (map.checkHasSetDefs())
        {
            /*
             * decode set definition - should be field set definition
             */
            /*
             * this needs to be passed in when we decode each field list
             */
            localFieldSetDefDb.clear();
            ret = localFieldSetDefDb.decode(dIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeLocalFieldSetDefDb() failed: <" + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }
        }

        if (map.checkHasSummaryData())
        {
            ret = decodeSummaryData(dIter, dictionary, displayStr);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        ret = decodeMap(dIter, dictionary, displayStr);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;
        System.out.println(displayStr.toString());
        return CodecReturnCodes.SUCCESS;		
	}
	
	/* Decodes a MarketByPrice payload.
	 * MarketByPrice is represented as an RsslMap, where each entry represents a price point.
	 * Each entry contains an RsslFieldList which contains data about that price point. */
	int decodeMarketByPriceDataBody(ReactorChannel channel, Msg msg)
	{

		if (msg.containerType() != DataTypes.MAP)
		{
			System.out.println("  Incorrect container type: " + msg.containerType());
			return CodecReturnCodes.FAILURE;
		}

		displayStr.setLength(0);
		dIter.clear();
		dIter.setBufferAndRWFVersion(msg.encodedDataBody(),
				channel.majorVersion(), channel.minorVersion());
			
		
        int ret = map.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeMap() failed with return code: " + ret);
            return ret;
        }

        if (map.checkHasSetDefs())
        {
            localFieldSetDefDb.clear();
            ret = localFieldSetDefDb.decode(dIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeLocalFieldSetDefDb() failed");
                return ret;
            }
        }
        if (map.checkHasSummaryData())
        {
            ret = decodeSummaryData(dIter, dictionary, displayStr);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        ret = decodeMap(dIter, dictionary, displayStr);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        System.out.println(displayStr.toString());
        return CodecReturnCodes.SUCCESS;


	}	
	
	/* Decodes a YieldCurve payload.
	 * YieldCurve is represented as an RsslFieldList, where each field contains data about the item. */
	int decodeYieldCurveDataBody(ReactorChannel channel, Msg msg)
	{
		if (!fieldDictionaryLoadedFromFile|| !enumTypeDictionaryLoadedFromFile)
		{
			System.out.println("  (Dictionary not loaded).\n");
			return CodecReturnCodes.FAILURE;
		}

		if (msg.containerType() != DataTypes.FIELD_LIST)
		{
			System.out.println("  Incorrect container type: " + msg.containerType());
			return CodecReturnCodes.FAILURE;
		}
		
	    int ret = fieldList.decode(dIter, null);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	System.out.println("DecodeFieldList() failed with return code: " + ret);
	    	return ret;
	    }
	    // decode each field entry in list
	    while ((ret = fieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
	    {
	    	if (ret != CodecReturnCodes.SUCCESS)
	    	{
	    		System.out.println("decodeFieldEntry() failed with return code: " + ret);
	    		return ret;
	    	}

	    	ret = decodeFieldEntry(fieldEntry, dIter, dictionary, displayStr);
	    	if (ret != CodecReturnCodes.SUCCESS)
	    	{
	    		System.out.println("decodeFieldEntry() failed");
	    		return ret;
	    	}
	    	displayStr.append("\n");
	    }
	    System.out.println(displayStr.toString());

	    return CodecReturnCodes.SUCCESS;
	}		
		
	/* Decodes a SymbolList payload.
	 * SymbolList is represented as an RsslMap, where each entry represents an item.
	 * The entries are indexed by the item's name. */
	int decodeSymbolListDataBody(ReactorChannel channel, Msg msg)
	{


		if (msg.containerType() != DataTypes.MAP)
		{
			System.out.println("  Incorrect container type: " + msg.containerType());
			return CodecReturnCodes.FAILURE;
		}
		
		
		displayStr.setLength(0);
		dIter.clear();
		dIter.setBufferAndRWFVersion(msg.encodedDataBody(),
				channel.majorVersion(), channel.minorVersion());
			
		
        int ret = map.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeMap() failed with return code: " + ret);
            return ret;
        }

        if (map.checkHasSetDefs())
        {
            localFieldSetDefDb.clear();
            ret = localFieldSetDefDb.decode(dIter);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeLocalFieldSetDefDb() failed");
                return ret;
            }
        }
        if (map.checkHasSummaryData())
        {
            ret = decodeSummaryData(dIter, dictionary, displayStr);
            if (ret != CodecReturnCodes.SUCCESS)
                return ret;
        }

        ret = decodeMap(dIter, dictionary, displayStr);
        if (ret != CodecReturnCodes.SUCCESS)
            return ret;

        System.out.println(displayStr.toString());
        return CodecReturnCodes.SUCCESS;

	}
		


	
	private int decodeMap(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
    {
		String actionString;
		int ret;
	        /* decode the map */
	        while ((ret = mapEntry.decode(dIter, mapKey)) != CodecReturnCodes.END_OF_CONTAINER)
	        {
	            if (ret != CodecReturnCodes.SUCCESS)
	            {
	                System.out.println("DecodeMapEntry() failed with return code: " + ret);
	                return ret;
	            }

	            
	            //convert the action to a string for display purposes
	            switch (mapEntry.action())
	            {
	                case MapEntryActions.UPDATE:
	                    actionString = "UPDATE";
	                    break;
	                case MapEntryActions.ADD:
	                    actionString = "ADD";
	                    break;
	                case MapEntryActions.DELETE:
	                    actionString = "DELETE";
	                    break;
	                default:
	                    actionString = "Unknown";

	            }
	            //print out the key
	            if (mapKey.length() > 0)
	            {
	                fieldValue.append("ORDER ID: " + mapKey.toString() + "\nACTION: "
	                            + actionString + "\n");
	            }

	            //there is not any payload in delete actions
	            if (mapEntry.action() != MapEntryActions.DELETE)
	            {
	                ret = decodeFieldList(dIter, dictionary, fieldValue);
	                if (ret != CodecReturnCodes.SUCCESS)
	                    return ret;
	            }
	        }
	        return CodecReturnCodes.SUCCESS;
    }   
	
	private int decodeFieldList(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
	{
		//decode field list
		int ret = fieldList.decode(dIter, localFieldSetDefDb);
		if (ret != CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeFieldList() failed: <" + CodecReturnCodes.toString(ret) + ">");
			return ret;
		}

		//decode each field entry in list
		while ((ret = fieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret != CodecReturnCodes.SUCCESS)
			{
				System.out.println("DecodeFieldEntry() failed: <" + CodecReturnCodes.toString(ret) + ">");
				return ret;
			}

			ret = decodeFieldEntry(fieldEntry, dIter, dictionary, fieldValue);
			if (ret != CodecReturnCodes.SUCCESS)
			{
				System.out.println("decodeFieldEntry() failed");
				return ret;
			}
			fieldValue.append("\n");
		}

		return CodecReturnCodes.SUCCESS;
	}

/*
 * This is used by all market price domain handlers to output field lists.
 * 
 * Decodes the field entry data and prints out the field entry data with
 * help of the dictionary. Returns success if decoding succeeds or failure
 * if decoding fails.
 */
	protected int decodeFieldEntry(FieldEntry fEntry, DecodeIterator dIter,
        DataDictionary dictionary, StringBuilder fieldValue)
	{
    // get dictionary entry
		DictionaryEntry dictionaryEntry = dictionary.entry(fEntry.fieldId());

    // return if no entry found
    if (dictionaryEntry == null)
    {
        fieldValue.append("\tFid " + fEntry.fieldId() + " not found in dictionary");
        return CodecReturnCodes.SUCCESS;
    }

    // print out fid name
    fieldValue.append("\t" + fEntry.fieldId() + "/" + dictionaryEntry.acronym().toString() + ": ");

    // decode and print out fid value
    int dataType = dictionaryEntry.rwfType();
    int ret = 0;
    switch (dataType)
    {
        case DataTypes.UINT:
            ret = fidUIntValue.decode(dIter);
            if (ret == CodecReturnCodes.SUCCESS)
            {
                fieldValue.append(fidUIntValue.toLong());
            }
            else if (ret != CodecReturnCodes.BLANK_DATA)
            {
                System.out.println("DecodeUInt() failed: <" + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }
            break;
        case DataTypes.INT:
            ret = fidIntValue.decode(dIter);
            if (ret == CodecReturnCodes.SUCCESS)
            {
                fieldValue.append(fidIntValue.toLong());
            }
            else if (ret != CodecReturnCodes.BLANK_DATA)
            {
                System.out.println("DecodeInt() failed: <" + CodecReturnCodes.toString(ret) + ">");

                return ret;
            }
            break;
        case DataTypes.FLOAT:
            ret = fidFloatValue.decode(dIter);
            if (ret == CodecReturnCodes.SUCCESS)
            {
                fieldValue.append(fidFloatValue.toFloat());
            }
            else if (ret != CodecReturnCodes.BLANK_DATA)
            {
                System.out.println("DecodeFloat() failed: <" + CodecReturnCodes.toString(ret) + ">");

                return ret;
            }
            break;
        case DataTypes.DOUBLE:
            ret = fidDoubleValue.decode(dIter);
            if (ret == CodecReturnCodes.SUCCESS)
            {
                fieldValue.append(fidDoubleValue.toDouble());
            }
            else if (ret != CodecReturnCodes.BLANK_DATA)
            {
                System.out.println("DecodeDouble() failed: <" + CodecReturnCodes.toString(ret) + ">");

                return ret;
            }
            break;
        case DataTypes.REAL:
            ret = fidRealValue.decode(dIter);
            if (ret == CodecReturnCodes.SUCCESS)
            {
                fieldValue.append(fidRealValue.toDouble());
            }
            else if (ret != CodecReturnCodes.BLANK_DATA)
            {
                System.out.println("DecodeReal() failed: <" + CodecReturnCodes.toString(ret) + ">");

                return ret;
            }
            break;
        case DataTypes.ENUM:
            ret = fidEnumValue.decode(dIter);
            if (ret == CodecReturnCodes.SUCCESS)
            {
                EnumType enumType = dictionary.entryEnumType(dictionaryEntry,
                                                             fidEnumValue);

                if (enumType == null)
                {
                    fieldValue.append(fidEnumValue.toInt());
                }
                else
                {
                    fieldValue.append(enumType.display().toString() + "(" +
                            fidEnumValue.toInt() + ")");
                }
            }
            else if (ret != CodecReturnCodes.BLANK_DATA)
            {
                System.out.println("DecodeEnum() failed: <" + CodecReturnCodes.toString(ret) + ">");

                return ret;
            }
            break;
        case DataTypes.DATE:
            ret = fidDateValue.decode(dIter);
            if (ret == CodecReturnCodes.SUCCESS)
            {
                fieldValue.append(fidDateValue.toString());

            }
            else if (ret != CodecReturnCodes.BLANK_DATA)
            {
                System.out.println("DecodeDate() failed: <" +
                        CodecReturnCodes.toString(ret) + ">");

                return ret;
            }
            break;
        case DataTypes.TIME:
            ret = fidTimeValue.decode(dIter);
            if (ret == CodecReturnCodes.SUCCESS)
            {
                fieldValue.append(fidTimeValue.toString());
            }
            else if (ret != CodecReturnCodes.BLANK_DATA)
            {
                System.out.println("DecodeTime() failed: <" +
                        CodecReturnCodes.toString(ret) + ">");

                return ret;
            }
            break;
        case DataTypes.DATETIME:
            ret = fidDateTimeValue.decode(dIter);
            if (ret == CodecReturnCodes.SUCCESS)
            {
                fieldValue.append(fidDateTimeValue.toString());
            }
            else if (ret != CodecReturnCodes.BLANK_DATA)
            {
                System.out.println("DecodeDateTime() failed: <" + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }
            break;
        case DataTypes.QOS:
            ret = fidQosValue.decode(dIter);
            if (ret == CodecReturnCodes.SUCCESS)
            {
                fieldValue.append(fidQosValue.toString());
            }
            else if (ret != CodecReturnCodes.BLANK_DATA)
            {
                System.out.println("DecodeQos() failed: <" + CodecReturnCodes.toString(ret) + ">");

                return ret;
            }
            break;
        case DataTypes.STATE:
            ret = fidStateValue.decode(dIter);
            if (ret == CodecReturnCodes.SUCCESS)
            {
                fieldValue.append(fidStateValue.toString());
            }
            else if (ret != CodecReturnCodes.BLANK_DATA)
            {
                System.out.println("DecodeState() failed: <" + CodecReturnCodes.toString(ret) + ">");

                return ret;
            }
            break;
        // For an example of array decoding, see
        // FieldListCodec.exampleDecode()
        case DataTypes.ARRAY:
            break;
        case DataTypes.BUFFER:
        case DataTypes.ASCII_STRING:
        case DataTypes.UTF8_STRING:
        case DataTypes.RMTES_STRING:
            if (fEntry.encodedData().length() > 0)
            {
                fieldValue.append(fEntry.encodedData().toString());
            }
            else
            {
                ret = CodecReturnCodes.BLANK_DATA;
            }
            break;
        default:
            fieldValue.append("Unsupported data type (" + DataTypes.toString(dataType) + ")");
            break;
    }
    if (ret == CodecReturnCodes.BLANK_DATA)
    {
        fieldValue.append("<blank data>");
    }

    	return CodecReturnCodes.SUCCESS;
	}
	
    private int decodeSummaryData(DecodeIterator dIter, DataDictionary dictionary, StringBuilder fieldValue)
    {
       int ret = fieldList.decode(dIter, localFieldSetDefDb);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeFieldList failed: <" + CodecReturnCodes.toString(ret) + ">");
            return ret;
        }

        fieldValue.append("SUMMARY DATA\n");
        //decode each field entry in list
        while ((ret = fieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeFieldEntry failed: <" + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }

            ret = decodeFieldEntry(fieldEntry, dIter, dictionary, fieldValue);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("decodeFieldEntry() failed");
                return ret;
            }
            fieldValue.append("\n");
        }

        return CodecReturnCodes.SUCCESS;
    }
	
}
