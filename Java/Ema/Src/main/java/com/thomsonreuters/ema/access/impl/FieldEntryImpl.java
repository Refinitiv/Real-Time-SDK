///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.FieldEntry;

class FieldEntryImpl extends EntryImpl implements FieldEntry
{
	private com.thomsonreuters.upa.codec.FieldEntry	_rsslFieldEntry;
	private com.thomsonreuters.upa.codec.DictionaryEntry _rsslDictionaryEntry;
	
	FieldEntryImpl(FieldListImpl fieldList, com.thomsonreuters.upa.codec.FieldEntry rsslFieldEntry)
	{
		super(fieldList, null);
		
		_rsslFieldEntry = rsslFieldEntry;
	}
	
	FieldEntryImpl(FieldListImpl fieldList, com.thomsonreuters.upa.codec.FieldEntry rsslFieldEntry,
				   com.thomsonreuters.upa.codec.DictionaryEntry rsslDictionaryEntry, DataImpl load)
	{
		super(fieldList, load);
		
		_rsslFieldEntry = rsslFieldEntry;
		_rsslDictionaryEntry = rsslDictionaryEntry;
	}
	
	@Override
	public int fieldId()
	{
		return _rsslFieldEntry.fieldId();
	}

	@Override
	public String name()
	{
		return ((_rsslDictionaryEntry == null) ? ComplexTypeImpl.EMPTY_STRING : (_rsslDictionaryEntry.acronym().toString()));
	}

	@Override
	public int rippleTo(int fieldId)
	{
		if (fieldId == 0)
			fieldId = _rsslFieldEntry.fieldId();
		
		com.thomsonreuters.upa.codec.DictionaryEntry rsslDictEntry = ((FieldListImpl)_data)._rsslDictionary.entry(fieldId);
		
		return ((rsslDictEntry == null) ? 0 : rsslDictEntry.rippleToField());
	}
	
	@Override
	public String toString()
	{
		_toString.setLength(0);
		_toString.append("FieldEntry ")
				.append(" fid=\"").append(fieldId()).append("\"")
				.append(" name=\"").append(name()).append("\"")
				.append(" dataType=\"").append(DataType.asString(_load.dataType()));

		if (_load.dataType() <= DataTypes.ARRAY)
		{
			_toString.append("\"\n").append(_load.toString(1));
			Utilities.addIndent(_toString, 0).append("FieldEntryEnd\n");
		}
		else
			_toString.append("\" value=\"").append(_load.toString()).append("\"\n");
	
		return _toString.toString();
	}
	
	void load(	com.thomsonreuters.upa.codec.DictionaryEntry rsslDictionaryEntry, DataImpl load)
	{
		_rsslDictionaryEntry = rsslDictionaryEntry;
		_load = load;
	}
}