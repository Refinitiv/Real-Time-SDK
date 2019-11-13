///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import com.thomsonreuters.ema.rdm.DictionaryEntry;
import com.thomsonreuters.ema.rdm.EnumType;
import com.thomsonreuters.ema.rdm.EnumTypeTable;

import com.thomsonreuters.upa.valueadd.common.VaNode;

class DictionaryEntryImpl extends VaNode implements DictionaryEntry
{
	private com.thomsonreuters.upa.codec.DictionaryEntry		rsslDictionaryEntry;
	private DataDictionaryImpl 									dataDictionaryImpl;
	private OmmInvalidUsageExceptionImpl 						ommIUExcept;
	private EnumTypeTableImpl									enumTypeTableImpl;
	private EnumTypeImpl										enumTypeImpl;
	private StringBuilder 										toStringValue;
	
	DictionaryEntryImpl() {
	}
	
	DictionaryEntryImpl dictionaryEntry(DataDictionaryImpl dataDictionary, com.thomsonreuters.upa.codec.DictionaryEntry dictionaryEntry)
	{
		rsslDictionaryEntry = dictionaryEntry;
		dataDictionaryImpl = dataDictionary;
		
		return this;
	}
	
	@Override
	public String acronym() {
		return rsslDictionaryEntry.acronym().data() != null ? rsslDictionaryEntry.acronym().toString() : "";
	}

	@Override
	public String ddeAcronym() {
		return rsslDictionaryEntry.ddeAcronym().data() != null ? rsslDictionaryEntry.ddeAcronym().toString() : "";
	}

	@Override
	public int fid() {
		return rsslDictionaryEntry.fid();
	}

	@Override
	public int rippleToField() {
		return rsslDictionaryEntry.rippleToField();
	}

	@Override
	public int fieldType() {
		return rsslDictionaryEntry.fieldType();
	}

	@Override
	public int length() {
		return rsslDictionaryEntry.length();
	}

	@Override
	public int enumLength() {
		return rsslDictionaryEntry.enumLength();
	}

	@Override
	public int rwfType() {
		return rsslDictionaryEntry.rwfType();
	}

	@Override
	public int rwfLength() {
		return rsslDictionaryEntry.rwfLength();
	}

	@Override
	public boolean hasEnumType(int value) {
		return dataDictionaryImpl.hasEnumType( rsslDictionaryEntry.fid(), value);
	}

	@Override
	public EnumType enumType(int value) {
		if ( hasEnumType(value) == false )
		{
			throw ommIUExcept().message("The enum value " + value + " for the Field ID " + rsslDictionaryEntry.fid() +
					" does not exist in enumerated type definitions", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT );
		}
		
		if ( enumTypeImpl == null )
		{
			enumTypeImpl = new EnumTypeImpl();
		}
		
		return enumTypeImpl.enumType(((EnumTypeImpl)dataDictionaryImpl.enumType(rsslDictionaryEntry.fid(), value)).enumType());
	}

	@Override
	public boolean hasEnumTypeTable() {
		return rsslDictionaryEntry.enumTypeTable() != null ? true : false;
	}

	@Override
	public EnumTypeTable enumTypeTable() {
		if( hasEnumTypeTable() == false)
		{
			throw ommIUExcept().message("The EnumTypeTable does not exist for the Field ID " + rsslDictionaryEntry.fid()
			, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}
		
		return  enumTypeTableImpl();
	}
	
	@Override
	public String toString() {
		
		if ( toStringValue == null )
		{
			toStringValue = new StringBuilder(512);
		}
		else
		{
			toStringValue.setLength(0);
		}

		toStringValue.append("Fid=").append(fid()).append(" '").append(acronym()).
			append("' '").append(ddeAcronym()).
			append("' Type=").append(fieldType()).
			append(" RippleTo=").append(rippleToField()).append(" Len=").append(length()).
			append(" EnumLen=").append(enumLength()).
			append(" RwfType=").append(rwfType()).append(" RwfLen=").append(rwfLength());

		if ( hasEnumTypeTable() )
		{
			toStringValue.append("\n\nEnum Type Table:\n");

			toStringValue.append(enumTypeTableImpl());
		}
		
		return toStringValue.toString();
	}
		
	private EnumTypeTableImpl enumTypeTableImpl()
	{
		if ( enumTypeTableImpl == null )
		{
			enumTypeTableImpl = new EnumTypeTableImpl();
		}
		
		return enumTypeTableImpl.enumTypeTable(rsslDictionaryEntry.enumTypeTable());
	}
	
	private OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (ommIUExcept == null)
			ommIUExcept = new OmmInvalidUsageExceptionImpl();

		return ommIUExcept;
	}
}
