///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2023, 2025 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.Iterator;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.OmmError.ErrorCode;

class ElementListIterImpl implements Iterator<ElementEntry>
{
	public com.refinitiv.eta.codec.ElementList			_rsslElementList;		// Pulled from EmaObjectManager
	protected com.refinitiv.eta.codec.DecodeIterator 	_rsslDecodeIter;		// Pulled from EmaObjectManager
	ElementEntryImpl 									_elementEntry = null;	// Pulled from EmaObjectManager
	ElementListImpl _elementListImpl;		// Uses the ElementListImpl of the parent that created this object
	int _errorCode = 0;

	public ElementListIterImpl(ElementListImpl elementListImpl)
	{
		this._elementListImpl = elementListImpl;
	}
	
	// Resets the iterImpl
	public void clear()
	{
		if (_elementEntry != null)
		{
			_elementEntry.returnToPool();
			_elementEntry = null;
		}
		if (_rsslElementList != null)
		{
			_elementListImpl._objManager._rsslElementListPool.add(_rsslElementList);
			_rsslElementList = null;
		}
		if (_rsslDecodeIter != null)
		{
			_elementListImpl._objManager._etaDecodeIteratorPool.add(_rsslDecodeIter);
			_rsslDecodeIter = null;
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
		int retCode;
		
		// First entry to decode
		if (_elementEntry == null)
		{
			_elementEntry = elementEntryInstance();
			_rsslElementList = rsslElementListInstance();
			_rsslDecodeIter = decodeIteratorInstance();
			
			retCode = _rsslDecodeIter.setBufferAndRWFVersion(_elementListImpl._rsslBuffer, _elementListImpl._rsslMajVer, _elementListImpl._rsslMinVer);
			if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS != retCode)
			{
				_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
				return true;
			}
			retCode = _rsslElementList.decode(_rsslDecodeIter, _elementListImpl._rsslLocalELSetDefDb);
		}
		
		if ((retCode = _elementEntry._rsslElementEntry
				.decode(_rsslDecodeIter)) != com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch (retCode)
			{
			case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS:
				int dType = _elementListImpl.dataType(_elementEntry._rsslElementEntry.dataType(), _elementListImpl._rsslMajVer, _elementListImpl._rsslMinVer,
						_elementEntry._rsslElementEntry.encodedData());
				load = _elementListImpl.dataInstance(_elementEntry._load, dType);

				if ( dType < DataType.DataTypes.FIELD_LIST || dType == DataType.DataTypes.ANSI_PAGE )
				{
					int decodeRetVal = load.decode(_elementEntry._rsslElementEntry.encodedData(), _rsslDecodeIter);
					if(decodeRetVal == com.refinitiv.eta.codec.CodecReturnCodes.INVALID_ARGUMENT ||
							decodeRetVal ==	com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA){
						load = _elementListImpl.dataInstance(load, DataTypes.ERROR);
						load.decode(_elementEntry._rsslElementEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
					}
				}
				else
					load.decode(_elementEntry._rsslElementEntry.encodedData(), _elementListImpl._rsslMajVer, _elementListImpl._rsslMinVer, _elementListImpl._rsslDictionary, null);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA:
				load = _elementListImpl.dataInstance(_elementEntry._load, DataTypes.ERROR);
				load.decode(_elementEntry._rsslElementEntry.encodedData(), ErrorCode.INCOMPLETE_DATA);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE:
				load = _elementListImpl.dataInstance(_elementEntry._load, DataTypes.ERROR);
				load.decode(_elementEntry._rsslElementEntry.encodedData(), ErrorCode.UNSUPPORTED_DATA_TYPE);
				break;
			default:
				load = _elementListImpl.dataInstance(_elementEntry._load, DataTypes.ERROR);
				load.decode(_elementEntry._rsslElementEntry.encodedData(), ErrorCode.UNKNOWN_ERROR);
				break;
			}

			_elementEntry._load = load;
			return true;
		}
		else
		{
			return false;
		}
	}

	@Override
	// Only returns the current iter we are on. Does NOT iterate, as we require hasNext() called first.
	public ElementEntry next() {
		return _elementEntry;
	}

	private ElementEntryImpl elementEntryInstance()
	{
		ElementEntryImpl retData = (ElementEntryImpl)_elementListImpl._objManager._elementEntryPool.poll();
        if(retData == null)
        {
        	retData = new ElementEntryImpl(com.refinitiv.eta.codec.CodecFactory.createElementEntry(), _elementListImpl.noDataInstance());
        	_elementListImpl._objManager._elementEntryPool.updatePool(retData);
        }
        else
        	retData._rsslElementEntry.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.ElementList rsslElementListInstance()
	{
		com.refinitiv.eta.codec.ElementList retData = null;
		retData = _elementListImpl._objManager._rsslElementListPool.poll();
        if (retData == null)
        {
        	retData = com.refinitiv.eta.codec.CodecFactory.createElementList();
        }
        else
        	retData.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.DecodeIterator decodeIteratorInstance()
	{
		com.refinitiv.eta.codec.DecodeIterator retData = null;
		retData = _elementListImpl._objManager._etaDecodeIteratorPool.poll();
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