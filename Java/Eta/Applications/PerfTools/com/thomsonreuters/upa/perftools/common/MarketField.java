package com.thomsonreuters.upa.perftools.common;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.FieldEntry;

/**
 * Represents one field in a fieldList (used by MarketPrice and MarketByOrder
 * messages). Stores the FieldID and dataType of the desired data using a
 * FieldEntry, as well as the value to use.
 */
public class MarketField
{
	private FieldEntry _fieldEntry;    // The market field entry.
	private Object _value;             // The market field value.
	
	public MarketField()
	{
		_fieldEntry = CodecFactory.createFieldEntry();
	}

	/** The market field entry. */
	public FieldEntry fieldEntry()
	{
		return _fieldEntry;
	}

	/** The market field value. */
	public Object value()
	{
		return _value;
	}

	/** The market field value. */
	public void value(Object value)
	{
		_value = value;
	}
}
