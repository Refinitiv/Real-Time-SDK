///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2023 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.Iterator;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.OmmError.ErrorCode;

class OmmArrayIterImpl implements Iterator<OmmArrayEntry>
{
	public com.refinitiv.eta.codec.Array				_rsslArray;				// Pulled from EmaObjectManager
	protected com.refinitiv.eta.codec.DecodeIterator 	_rsslDecodeIter;		// Pulled from EmaObjectManager
	OmmArrayEntryImpl 									_ommArrayEntry = null;	// Pulled from EmaObjectManager
	OmmArrayImpl _ommArrayImpl;		// Uses the ElementListImpl of the parent that created this object
	int _errorCode = 0;

	public OmmArrayIterImpl(OmmArrayImpl ommArrayImpl)
	{
		_ommArrayImpl = ommArrayImpl;
	}
	
	// Resets the iterImpl
	public void clear()
	{
		if (_ommArrayEntry != null)
		{
			_ommArrayEntry.returnToPool();
			_ommArrayEntry = null;
		}
		if (_rsslArray != null)
		{
			_ommArrayImpl._objManager._rsslArrayPool.add(_rsslArray);
			_rsslArray = null;
		}
		if (_rsslDecodeIter != null)
		{
			_ommArrayImpl._objManager._etaDecodeIteratorPool.add(_rsslDecodeIter);
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
		if (_ommArrayEntry == null)
		{
			_ommArrayEntry = ommArrayEntryInstance();
			_rsslArray = rsslArrayInstance();
			_rsslDecodeIter = decodeIteratorInstance();
			
			retCode = _rsslDecodeIter.setBufferAndRWFVersion(_ommArrayImpl._rsslBuffer, _ommArrayImpl._rsslMajVer, _ommArrayImpl._rsslMinVer);
			if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS != retCode)
			{
				_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
				return true;
			}
			retCode = _rsslArray.decode(_rsslDecodeIter);
		}
		
		if ((retCode = _ommArrayEntry._rsslArrayEntry
				.decode(_rsslDecodeIter)) != com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
			case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS :
			load = _ommArrayImpl.dataInstance(_ommArrayEntry._load, _rsslArray.primitiveType());
				int decodeRetVal = load.decode(_ommArrayEntry._rsslArrayEntry.encodedData(), _rsslDecodeIter);
				if(decodeRetVal == com.refinitiv.eta.codec.CodecReturnCodes.INVALID_ARGUMENT ||
						decodeRetVal ==	com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA)
				{
					load = _ommArrayImpl.dataInstance(load, DataTypes.ERROR);
					load.decode(_ommArrayEntry._rsslArrayEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				}
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA :
				load = _ommArrayImpl.dataInstance(_ommArrayEntry._load, DataTypes.ERROR);
				load.decode(_ommArrayEntry._rsslArrayEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				load = _ommArrayImpl.dataInstance(_ommArrayEntry._load, DataTypes.ERROR);
				load.decode(_ommArrayEntry._rsslArrayEntry.encodedData(),ErrorCode.UNSUPPORTED_DATA_TYPE);
				break;
			default :
				load = _ommArrayImpl.dataInstance(_ommArrayEntry._load, DataTypes.ERROR);
				load.decode(_ommArrayEntry._rsslArrayEntry.encodedData(),ErrorCode.UNKNOWN_ERROR);
				break;
			}

			_ommArrayEntry._load = load;
			return true;
		}
		else
		{
			_ommArrayEntry.returnToPool();
			_ommArrayImpl._objManager._rsslArrayPool.add(_rsslArray);
			_ommArrayImpl._objManager._etaDecodeIteratorPool.add(_rsslDecodeIter);
			return false;
		}
	}

	@Override
	// Only returns the current iter we are on. Does NOT iterate, as we require hasNext() called first.
	public OmmArrayEntry next() {
		return _ommArrayEntry;
	}

	private OmmArrayEntryImpl ommArrayEntryInstance()
	{
		OmmArrayEntryImpl retData = (OmmArrayEntryImpl)_ommArrayImpl._objManager._arrayEntryPool.poll();
        if(retData == null)
        {
        	retData = new OmmArrayEntryImpl();
        	_ommArrayImpl._objManager._elementEntryPool.updatePool(retData);
        }
        else
        	retData._rsslArrayEntry.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.Array rsslArrayInstance()
	{
		com.refinitiv.eta.codec.Array retData = null;
		if (_ommArrayImpl._objManager._rsslArrayPool.size() > 0)
		{
			retData = _ommArrayImpl._objManager._rsslArrayPool.remove(_ommArrayImpl._objManager._rsslArrayPool.size()-1);
		}
        if (retData == null)
        {
        	retData = com.refinitiv.eta.codec.CodecFactory.createArray();
        }
        else
        	retData.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.DecodeIterator decodeIteratorInstance()
	{
		com.refinitiv.eta.codec.DecodeIterator retData = null;
		if (_ommArrayImpl._objManager._etaDecodeIteratorPool.size() > 0)
		{
			retData = _ommArrayImpl._objManager._etaDecodeIteratorPool.remove(_ommArrayImpl._objManager._etaDecodeIteratorPool.size()-1);
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