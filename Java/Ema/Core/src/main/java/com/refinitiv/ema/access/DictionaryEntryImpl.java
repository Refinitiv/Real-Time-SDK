/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.rdm.DictionaryEntry;
import com.refinitiv.ema.rdm.EnumType;
import com.refinitiv.ema.rdm.EnumTypeTable;

import com.refinitiv.eta.valueadd.common.VaNode;

class DictionaryEntryImpl extends VaNode implements DictionaryEntry
{
	private com.refinitiv.eta.codec.DictionaryEntry		rsslDictionaryEntry;
	private DataDictionaryImpl 									dataDictionaryImpl;
	private OmmInvalidUsageExceptionImpl 						ommIUExcept;
	private EnumTypeTableImpl									enumTypeTableImpl;
	private EnumTypeImpl										enumTypeImpl;
	private StringBuilder 										toStringValue;
	private boolean												isManagedByUser;

	DictionaryEntryImpl() {
		this(false);
	}

	DictionaryEntryImpl(boolean isManagedByUser) {
		this.isManagedByUser = isManagedByUser;
	}
	
	DictionaryEntryImpl dictionaryEntry(DataDictionaryImpl dataDictionary, com.refinitiv.eta.codec.DictionaryEntry dictionaryEntry)
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

	/**
	 * Detects if DictionaryEntry was created by user
	 *
	 * @see {@link EmaFactory#createDictionaryEntry()}
	 * @return true when DictionaryEntry instance was created by user, false - instance was created by API.
	 */
	boolean isManagedByUser() {
		return isManagedByUser;
	}
}
