///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2023 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.Iterator;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.OmmError.ErrorCode;

class FieldListIterImpl implements Iterator<FieldEntry>
{
	public com.refinitiv.eta.codec.FieldList			_rsslFieldList;		// Pulled from EmaObjectManager
	protected com.refinitiv.eta.codec.DecodeIterator 	_rsslDecodeIter;	// Pulled from EmaObjectManager
	FieldEntryImpl 										_fieldEntry = null;	// Pulled from EmaObjectManager
	FieldListImpl _fieldListImpl;			// Uses the FieldListImpl of the parent that created this object

	public FieldListIterImpl(FieldListImpl fieldListImpl)
	{
		this._fieldListImpl = fieldListImpl;
	}
	
	// Resets the iterImpl
	public void clear()
	{
		if (_fieldEntry != null)
		{
			_fieldEntry.returnToPool();
			_fieldEntry = null;
		}
		if (_rsslFieldList != null)
		{
			_fieldListImpl._objManager._rsslFieldListPool.add(_rsslFieldList);
			_rsslFieldList = null;
		}
		if (_rsslDecodeIter != null)
		{
			_fieldListImpl._objManager._etaDecodeIteratorPool.add(_rsslDecodeIter);
			_rsslFieldList = null;
		}
	}
	
	@Override
	// Decodes to next iter, required before calling next(). next() just gets current iter that we are on.
	public boolean hasNext() {
		return decodeNext();
	}
	
	public boolean decodeNext()
	{
		DataImpl load;
		com.refinitiv.eta.codec.DictionaryEntry rsslDictionaryEntry = null;
		
		int retCode;

		// First entry to decode
		if (_fieldEntry == null)
		{
			_fieldEntry = fieldEntryInstance();
			_rsslFieldList = rsslFieldListInstance();
			_rsslDecodeIter = decodeIteratorInstance();
			
			retCode = _rsslDecodeIter.setBufferAndRWFVersion(_fieldListImpl._rsslBuffer, _fieldListImpl._rsslMajVer, _fieldListImpl._rsslMinVer);
			
			retCode = _rsslFieldList.decode(_rsslDecodeIter, _fieldListImpl._rsslLocalFLSetDefDb);
		}
		
		if ((retCode  = _fieldEntry._rsslFieldEntry.decode(_rsslDecodeIter)) != com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
			case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS :
				rsslDictionaryEntry = _fieldListImpl._rsslDictionary.entry(_fieldEntry._rsslFieldEntry.fieldId());
				if (rsslDictionaryEntry == null)
				{
					load = _fieldListImpl.dataInstance(_fieldEntry._load, DataTypes.ERROR);
					load.decode(_fieldEntry._rsslFieldEntry.encodedData(),ErrorCode.FIELD_ID_NOT_FOUND);
				}
				else			
				{			
					int dType = _fieldListImpl.dataType(rsslDictionaryEntry.rwfType(), _fieldListImpl._rsslMajVer, _fieldListImpl._rsslMinVer, _fieldEntry._rsslFieldEntry.encodedData());
					load = _fieldListImpl.dataInstance(_fieldEntry._load, dType);
					if ( dType < DataType.DataTypes.FIELD_LIST || dType == DataType.DataTypes.ANSI_PAGE ) {
						int decodeRetVal = load.decode(_fieldEntry._rsslFieldEntry.encodedData(), _rsslDecodeIter);
						if(decodeRetVal == com.refinitiv.eta.codec.CodecReturnCodes.INVALID_ARGUMENT ||
								decodeRetVal ==	com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA){
							load = _fieldListImpl.dataInstance(load, DataTypes.ERROR);
							load.decode(_fieldEntry._rsslFieldEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
						}
					} else
						load.decode(_fieldEntry._rsslFieldEntry.encodedData(), _fieldListImpl._rsslMajVer, _fieldListImpl._rsslMinVer, _fieldListImpl._rsslDictionary, _fieldListImpl._rsslLocalFLSetDefDb);
				}
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA :
				load = _fieldListImpl.dataInstance(_fieldEntry._load, DataTypes.ERROR);
				load.decode(_fieldEntry._rsslFieldEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				load = _fieldListImpl.dataInstance(_fieldEntry._load, DataTypes.ERROR);
				load.decode(_fieldEntry._rsslFieldEntry.encodedData(),ErrorCode.UNSUPPORTED_DATA_TYPE);
				break;
			default :
				load = _fieldListImpl.dataInstance(_fieldEntry._load, DataTypes.ERROR);
				load.decode(_fieldEntry._rsslFieldEntry.encodedData(),ErrorCode.UNKNOWN_ERROR);
				break;
			}

			_fieldEntry.entryValue(_fieldListImpl, rsslDictionaryEntry, load);
			return true;
		}
		else
		{
			return false;
		}
	}

	@Override
	// Only returns the current iter we are on. Does NOT iterate, as we require hasNext() called first.
	public FieldEntry next() {
		return _fieldEntry;
	}

	private FieldEntryImpl fieldEntryInstance()
	{
		FieldEntryImpl retData = (FieldEntryImpl)_fieldListImpl._objManager._fieldEntryPool.poll();
        if(retData == null)
        {
        	retData = new FieldEntryImpl(com.refinitiv.eta.codec.CodecFactory.createFieldEntry(), _fieldListImpl.noDataInstance());
        	_fieldListImpl._objManager._fieldEntryPool.updatePool(retData);
        }
        else
        	retData._rsslFieldEntry.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.FieldList rsslFieldListInstance()
	{
		com.refinitiv.eta.codec.FieldList retData = null;
		if (_fieldListImpl._objManager._rsslFieldListPool.size() > 0)
		{
			retData = _fieldListImpl._objManager._rsslFieldListPool.remove(_fieldListImpl._objManager._rsslFieldListPool.size()-1);
		}
        if (retData == null)
        {
        	retData = com.refinitiv.eta.codec.CodecFactory.createFieldList();
        }
        else
        	retData.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.DecodeIterator decodeIteratorInstance()
	{
		com.refinitiv.eta.codec.DecodeIterator retData = null;
		if (_fieldListImpl._objManager._etaDecodeIteratorPool.size() > 0)
		{
			retData = _fieldListImpl._objManager._etaDecodeIteratorPool.remove(_fieldListImpl._objManager._etaDecodeIteratorPool.size()-1);
		}
        if (retData == null)
        {
        	retData = com.refinitiv.eta.codec.CodecFactory.createDecodeIterator();
        }
        else
        	retData.clear();
        
        return retData;
	}
	
	@Override
	public void remove()
	{
		throw new UnsupportedOperationException("EMA collection iterator doesn't support this operation.");
	}
	
}