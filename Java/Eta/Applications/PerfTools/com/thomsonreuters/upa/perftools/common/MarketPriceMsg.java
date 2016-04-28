package com.thomsonreuters.upa.perftools.common;

/** Represents one MarketPrice message */
public class MarketPriceMsg
{	
	private MarketField[] _fieldEntries;   // List of fields.
	private int _fieldEntryCount;          // Number of fields in list.
	private int _estimatedContentLength;   // Estimated size of payload.
	private int _arrayCount;               // count for field entry array 
		
	public MarketPriceMsg(int count)
	{
		_arrayCount = count;
		_fieldEntries = new MarketField[_arrayCount];
	}

	/** List of fields. */
	public MarketField[] fieldEntries()
	{
		return _fieldEntries;
	}

	/** Number of fields in list. */
	public int fieldEntryCount()
	{
		return _fieldEntryCount;
	}

	/** Number of fields in list. */
	public void fieldEntryCount(int count)
	{
		_fieldEntryCount = count;
	}

	/** Estimated size of payload. */
	public int estimatedContentLength()
	{
		return _estimatedContentLength;
	}

	/** Estimated size of payload. */
	public void estimatedContentLength(int length)
	{
		_estimatedContentLength = length;
	}
}
