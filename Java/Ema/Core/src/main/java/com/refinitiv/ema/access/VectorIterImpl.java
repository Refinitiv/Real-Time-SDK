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
import com.refinitiv.eta.codec.VectorEntryActions;

class VectorIterImpl implements Iterator<VectorEntry>
{
	public com.refinitiv.eta.codec.Vector				_rsslVector; 			// Pulled from EmaObjectManager
	protected com.refinitiv.eta.codec.DecodeIterator 	_rsslDecodeIter;		// Pulled from EmaObjectManager
	VectorEntryImpl 									_vectorEntry = null;	// Pulled from EmaObjectManager
	VectorImpl _vectorImpl;		// Uses the VectorImpl of the parent that created this object
	int _errorCode = 0;
	
	public VectorIterImpl(VectorImpl vectorImpl)
	{
		this._vectorImpl = vectorImpl;
	}
	
	// Resets the iterImpl
	public void clear()
	{
		if (_vectorEntry != null)
		{
			_vectorEntry.returnToPool();
			_vectorEntry = null;
		}
		if (_rsslVector != null)
		{
			_vectorImpl._objManager._rsslVectorPool.add(_rsslVector);
			_rsslVector = null;
		}
		if (_rsslDecodeIter != null)
		{
			_vectorImpl._objManager._etaDecodeIteratorPool.add(_rsslDecodeIter);
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
		if (_vectorEntry == null)
		{
			_vectorEntry = vectorEntryInstance();
			_rsslVector = rsslVectorInstance();
			_rsslDecodeIter = decodeIteratorInstance();

			retCode = _rsslDecodeIter.setBufferAndRWFVersion(_vectorImpl._rsslBuffer, _vectorImpl._rsslMajVer, _vectorImpl._rsslMinVer);
			if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS != retCode)
			{
				_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
				return true;
			}
			
			retCode = _rsslVector.decode(_rsslDecodeIter);
		}
		
		if ((retCode  = _vectorEntry._rsslVectorEntry.decode(_rsslDecodeIter)) != com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
			case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS :
				int rsslContainerType = (_vectorEntry._rsslVectorEntry.action() != VectorEntryActions.DELETE && _vectorEntry._rsslVectorEntry.action() != VectorEntryActions.CLEAR)?
														_rsslVector.containerType() : com.refinitiv.eta.codec.DataTypes.NO_DATA;
				int dType = _vectorImpl.dataType(rsslContainerType, _vectorImpl._rsslMajVer, _vectorImpl._rsslMinVer, _vectorEntry._rsslVectorEntry.encodedData());
				load = _vectorImpl.dataInstance(_vectorEntry._load, dType);
				load.decode(_vectorEntry._rsslVectorEntry.encodedData(), _vectorImpl._rsslMajVer, _vectorImpl._rsslMinVer, _vectorImpl._rsslDictionary, _vectorImpl._rsslLocalSetDefDb);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA :
				load = _vectorImpl.dataInstance(_vectorEntry._load, DataTypes.ERROR);
				load.decode(_vectorEntry._rsslVectorEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				load = _vectorImpl.dataInstance(_vectorEntry._load, DataTypes.ERROR);
				load.decode(_vectorEntry._rsslVectorEntry.encodedData(),ErrorCode.UNSUPPORTED_DATA_TYPE);
				break;
			default :
				load = _vectorImpl.dataInstance(_vectorEntry._load, DataTypes.ERROR);
				load.decode(_vectorEntry._rsslVectorEntry.encodedData(),ErrorCode.UNKNOWN_ERROR);
				break;
			}

			_vectorEntry._load = load;
			return true;
		}
		else
		{
			return false;
		}
	}

	@Override
	// Only returns the current iter we are on. Does NOT iterate, as we require hasNext() called first.
	public VectorEntry next() {
		return _vectorEntry;
	}

	private VectorEntryImpl vectorEntryInstance()
	{
		VectorEntryImpl retData = (VectorEntryImpl)_vectorImpl._objManager._vectorEntryPool.poll();
        if(retData == null)
        {
        	retData = new VectorEntryImpl(com.refinitiv.eta.codec.CodecFactory.createVectorEntry(), _vectorImpl.noDataInstance(), _vectorImpl._objManager);
        	_vectorImpl._objManager._vectorEntryPool.updatePool(retData);
        }
        else
        	retData._rsslVectorEntry.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.Vector rsslVectorInstance()
	{
		com.refinitiv.eta.codec.Vector retData = null;
		if (_vectorImpl._objManager._rsslVectorPool.size() > 0)
		{
			retData = _vectorImpl._objManager._rsslVectorPool.remove(_vectorImpl._objManager._rsslVectorPool.size()-1);
		}
        if (retData == null)
        {
        	retData = com.refinitiv.eta.codec.CodecFactory.createVector();
        }
        else
        	retData.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.DecodeIterator decodeIteratorInstance()
	{
		com.refinitiv.eta.codec.DecodeIterator retData = null;
		if (_vectorImpl._objManager._etaDecodeIteratorPool.size() > 0)
		{
			retData = _vectorImpl._objManager._etaDecodeIteratorPool.remove(_vectorImpl._objManager._etaDecodeIteratorPool.size()-1);
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