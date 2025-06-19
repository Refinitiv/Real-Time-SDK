/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.watchlistconsumer;

import com.refinitiv.eta.codec.Array;
import com.refinitiv.eta.codec.ArrayEntry;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DateTime;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.DictionaryEntry;
import com.refinitiv.eta.codec.EnumType;
import com.refinitiv.eta.codec.FieldEntry;
import com.refinitiv.eta.codec.FieldList;
import com.refinitiv.eta.codec.Int;
import com.refinitiv.eta.codec.LocalFieldSetDefDb;
import com.refinitiv.eta.codec.Map;
import com.refinitiv.eta.codec.MapEntry;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.Qos;
import com.refinitiv.eta.codec.Real;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.Time;
import com.refinitiv.eta.codec.UInt;
import com.refinitiv.eta.codec.Vector;
import com.refinitiv.eta.codec.VectorEntry;
import com.refinitiv.eta.codec.VectorEntryActions;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;


public class ItemDecoder 
{	
    private final String FIELD_DICTIONARY_FILE_NAME = "RDMFieldDictionary";
    private final String ENUM_TABLE_FILE_NAME = "enumtype.def";     	
	DataDictionary dictionary;
    DecodeIterator dIter = CodecFactory.createDecodeIterator();		
	boolean fieldDictionaryLoadedFromFile;
	boolean enumTypeDictionaryLoadedFromFile;
    boolean fieldDictionaryDownloadedFromNetwork;
    boolean enumTypeDictionaryDownloadedFromNetwork;
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
    private com.refinitiv.eta.codec.Enum fidEnumValue = CodecFactory.createEnum();
    private com.refinitiv.eta.codec.Date fidDateValue = CodecFactory.createDate();
    private Time fidTimeValue = CodecFactory.createTime();
    private DateTime fidDateTimeValue = CodecFactory.createDateTime();
    private com.refinitiv.eta.codec.Float fidFloatValue = CodecFactory.createFloat();
    private com.refinitiv.eta.codec.Double fidDoubleValue = CodecFactory.createDouble();
    private Qos fidQosValue = CodecFactory.createQos();
    private State fidStateValue = CodecFactory.createState();    
    StringBuilder displayStr = new StringBuilder();
	private int indentCount;
	private String[] indents = {"", "    ", "        ", "            "};
	private Vector fidVectorValue = CodecFactory.createVector();
	private Array fidArrayValue = CodecFactory.createArray();
	private FieldList embeddedFieldList = CodecFactory.createFieldList();
	private FieldEntry embeddedFieldEntry = CodecFactory.createFieldEntry();
	private VectorEntry vectorEntry = CodecFactory.createVectorEntry();
	private ArrayEntry arrayEntry = CodecFactory.createArrayEntry();
	private Buffer fidBufferValue = CodecFactory.createBuffer();

	/* Initializes the item decoder. Loads dictionaries from their files if the files
	 * are found in the application's folder. */
	public void init()
	{
	    dictionary = CodecFactory.createDataDictionary();
	        
		fieldDictionaryLoadedFromFile = false;
		enumTypeDictionaryLoadedFromFile = false;
		fieldDictionaryDownloadedFromNetwork = false;
		enumTypeDictionaryDownloadedFromNetwork = false;
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
		if(msg.containerType() == DataTypes.NO_DATA)
        {
        	return CodecReturnCodes.SUCCESS;
        }
		
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
        if (!isDictionaryLoaded())
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
		if (!isDictionaryLoaded())
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
		
		int ret = decodeFieldListForYieldCurve(dIter, dictionary, fieldList, fieldEntry);
		
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	System.out.println("DecodeFieldList() failed with return code: " + ret);
	    	return ret;
	    }
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

        while ((ret = mapEntry.decode(dIter, mapKey)) != CodecReturnCodes.END_OF_CONTAINER)
        {
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("DecodeMapEntry() failed: < " + CodecReturnCodes.toString(ret) + ">");
                return ret;
            }
            System.out.println(mapKey.toString() + "\t" +
                        mapEntryActionToString(mapEntry.action()));
        }        
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
	
	private int decodeFieldListForYieldCurve(DecodeIterator dIter, DataDictionary dictionary, FieldList localFieldList, FieldEntry localFieldEntry)
	{
		localFieldList.clear();
				
		int ret = localFieldList.decode(dIter, localFieldSetDefDb);
		if (ret < CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeFieldList() failed with return code: " + ret);
			return ret;
		}

		localFieldEntry.clear();

		indentCount++;
		
		// decode each field entry in list
		while ((ret = localFieldEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret < CodecReturnCodes.SUCCESS)
			{
				System.out.println("decodeFieldEntry() failed with return code: " + ret);
				return ret;
			}
			// get dictionary entry
			DictionaryEntry dictionaryEntry = dictionary.entry(localFieldEntry.fieldId());
			
			// return if no entry found
			if (dictionaryEntry == null)
			{
				System.out.println("\tFid " + localFieldEntry.fieldId()
						+ " not found in dictionary");
				System.out.println(localFieldEntry.encodedData().toHexString());
				return CodecReturnCodes.SUCCESS;
			}

			// print out fid name
			System.out.print(indents[indentCount] + dictionaryEntry.acronym().toString());
			for (int i = 0; i < 40 - indents[indentCount].length() - dictionaryEntry.acronym().length(); i++)
			{
				System.out.print(" ");
			}
			
			// decode and print out fid value
			int dataType = dictionaryEntry.rwfType();

			switch (dataType)
			{
				case DataTypes.VECTOR:
					ret = decodeVector(dIter, dictionary);
					if (ret < CodecReturnCodes.SUCCESS)
					{
						System.out.println("decodeVector inside FieldList failed");
						return ret;
					}
					break;
				case DataTypes.ARRAY:
					ret = decodeArray(dIter);
					if (ret < CodecReturnCodes.SUCCESS)
					{
						System.out.println("decodeArray inside FieldList failed");
						return ret;
					}	
					break;
				default:
					ret = decodePrimitive(dIter, dataType, false);
					if (ret < CodecReturnCodes.SUCCESS)
					{
						System.out.println("decodePrimitive inside FieldList failed");
						return ret;
					}	
					break;
			}
		}
		indentCount--;
		return CodecReturnCodes.SUCCESS;
	}

	private int decodeVector(DecodeIterator dIter, DataDictionary dictionary)
	{
		int ret = fidVectorValue.decode(dIter);
		if (ret < CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeReal() failed: <"	+ CodecReturnCodes.toString(ret) + ">");
			return ret;
		}
		if (ret == CodecReturnCodes.NO_DATA)
		{
			System.out.println("<no data>");
			return CodecReturnCodes.SUCCESS;
		}
		if (fidVectorValue.checkHasSummaryData())
		{
			System.out.println();
			// fieldList inside summaryData within vector			
			ret = decodeFieldListForYieldCurve(dIter, dictionary, embeddedFieldList, embeddedFieldEntry);
			if (ret < CodecReturnCodes.SUCCESS)
			{
				System.out.println("DecodeSummaryData failed: <" + CodecReturnCodes.toString(ret) + ">");
				return ret;
			}
		}
		// If the vector flags indicate that set definition content is present,
		// decode the set def db
		if (fidVectorValue.checkHasSetDefs())
		{
			if (fidVectorValue.containerType() == DataTypes.FIELD_LIST)
			{
				localFieldSetDefDb.clear();
				ret = localFieldSetDefDb.decode(dIter);
				if (ret < CodecReturnCodes.SUCCESS)
				{
					System.out.println("DecodeSetDefDb() failed: <"	+ CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
			}
		}
		
		indentCount++;
		System.out.println();
		
		vectorEntry.clear();
		while ((ret = vectorEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with DecodeVectorEntry.  Error Text: %s\n",
								CodecReturnCodes.toString(ret), ret, CodecReturnCodes.info(ret));
				return ret;
			}

			System.out.println(indents[indentCount] + "INDEX: " + vectorEntry.index());
			System.out.print(indents[indentCount] + "ACTION: ");
			switch (vectorEntry.action())
			{
				case VectorEntryActions.UPDATE:
					System.out.println("UPDATE_ENTRY");
					break;
				case VectorEntryActions.SET:
					System.out.println("SET_ENTRY");
					break;
				case VectorEntryActions.CLEAR:
					System.out.println("CLEAR_ENTRY");
					break;
				case VectorEntryActions.INSERT:
					System.out.println("INSERT_ENTRY");
					break;
				case VectorEntryActions.DELETE:
					System.out.println("DELETE_ENTRY");
					break;
				default:
					System.out.println("UNKNOWN");
					break;
			}

			/* Continue decoding vector entries. */
			switch (fidVectorValue.containerType())
			{
				case DataTypes.FIELD_LIST:
					// fieldList inside vectorEntry within vector
					ret = decodeFieldListForYieldCurve(dIter, dictionary, embeddedFieldList, embeddedFieldEntry);
					if (ret < CodecReturnCodes.SUCCESS)
					{
						System.out.printf("Error %s (%d) encountered with decoding FieldList within Vector: %s\n",
										CodecReturnCodes.toString(ret), ret, CodecReturnCodes.info(ret));
						return ret;
					}
					break;
				case DataTypes.ARRAY:
					ret = decodeArray(dIter);
					if (ret < CodecReturnCodes.SUCCESS)
					{
						System.out.printf("Error %s (%d) encountered with decoding ARRAY within Vector: %s\n",
										CodecReturnCodes.toString(ret), ret, CodecReturnCodes.info(ret));
						return ret;
					}
					break;
				default:
					System.out.println("Error: Vector contained unhandled containerType " + fidVectorValue.containerType());
					break;
			}

		}
		indentCount--;
		return CodecReturnCodes.SUCCESS;
	}

	private int decodePrimitive(DecodeIterator dIter, int dataType, boolean isArray)
	{
		int ret = 0;

		switch (dataType)
		{
			case DataTypes.INT:
				ret = fidIntValue.decode(dIter);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					System.out.print(fidIntValue.toLong());
				}
				else if (ret != CodecReturnCodes.BLANK_DATA)
				{
					System.out.println("DecodeInt() failed: <" + CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				break;
			case DataTypes.REAL:
				ret = fidRealValue.decode(dIter);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					System.out.print(fidRealValue.toDouble());
				}
				else if (ret != CodecReturnCodes.BLANK_DATA)
				{
					System.out.println("DecodeReal() failed: <" + CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				break;
			case DataTypes.DATE:
				ret = fidDateValue.decode(dIter);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					System.out.print(fidDateValue.toString());
	
				}
				else if (ret != CodecReturnCodes.BLANK_DATA)
				{
					System.out.println("DecodeDate() failed: <" + CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				break;
			case DataTypes.TIME:
				ret = fidTimeValue.decode(dIter);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					System.out.print(fidTimeValue.toString());
				}
				else if (ret != CodecReturnCodes.BLANK_DATA)
				{
					System.out.println("DecodeTime() failed: <" + CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				break;
			case DataTypes.DATETIME:
				ret = fidDateTimeValue.decode(dIter);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					System.out.print(fidDateTimeValue.toString());
				}
				else if (ret != CodecReturnCodes.BLANK_DATA)
				{
					System.out.println("DecodeDateTime() failed: <" + CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				break;
			case DataTypes.ARRAY:
				ret = decodeArray(dIter);
				if (ret < CodecReturnCodes.SUCCESS)
				{
					System.out.printf("Error %s (%d) encountered with decoding ARRAY was primitive: %s\n",
											CodecReturnCodes.toString(ret), ret, CodecReturnCodes.info(ret));
					return ret;
				}
				break;			
			case DataTypes.BUFFER:
			case DataTypes.ASCII_STRING:
			case DataTypes.UTF8_STRING:
			case DataTypes.RMTES_STRING:
				ret = fidBufferValue.decode(dIter);
				if (ret == CodecReturnCodes.SUCCESS)
				{
					if (isArray)
						System.out.print("\"");
					System.out.print(fidBufferValue.toString());
					if (isArray)
						System.out.print("\"");
				}
				else if (ret != CodecReturnCodes.BLANK_DATA)
				{
					System.out.println("DecodeString() failed: <" + CodecReturnCodes.toString(ret) + ">");
					return ret;
				}
				break;
			default:
				System.out.print("Unsupported data type (" + DataTypes.toString(dataType) + ")");
				break;
		}
		if (ret == CodecReturnCodes.BLANK_DATA)
		{
			System.out.print("<blank data>");
		}	

		if(!isArray)
			System.out.print("\n");
		
		return CodecReturnCodes.SUCCESS;
	}

	private int decodeArray(DecodeIterator dIter)
	{
		boolean firstArrayEntry = true;

		System.out.print("{ ");

		fidArrayValue.clear();
		int ret = fidArrayValue.decode(dIter);
		if (ret < CodecReturnCodes.SUCCESS)
		{
			System.out.println("DecodeArray() failed: <" + CodecReturnCodes.toString(ret) + ">");
			return ret;
		}
		
		int dataType = fidArrayValue.primitiveType();

		arrayEntry.clear();
		while ((ret = arrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with DecodeArrayEntry.  Error Text: %s\n",
								CodecReturnCodes.toString(ret), ret, CodecReturnCodes.info(ret));
				return ret;
			}

			if (firstArrayEntry)
				firstArrayEntry = false;
			else
				System.out.print(", ");
			ret = decodePrimitive(dIter, dataType, true);
			if (ret < CodecReturnCodes.SUCCESS)
			{
				System.out.printf("Error %s (%d) encountered with DecodeArrayEntryPrimitives.  Error Text: %s\n",
								CodecReturnCodes.toString(ret), ret, CodecReturnCodes.info(ret));
				return ret;
			}
		}

		System.out.print(" }\n");

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
    
    private boolean isDictionaryLoaded()
    {
        return ((fieldDictionaryLoadedFromFile && enumTypeDictionaryLoadedFromFile) ||
                (fieldDictionaryDownloadedFromNetwork && enumTypeDictionaryDownloadedFromNetwork));
    }
    
    private String mapEntryActionToString(int mapEntryAction)
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
