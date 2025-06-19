/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.util.Iterator;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.OmmError.ErrorCode;

class SeriesIterImpl implements Iterator<SeriesEntry>
{
	public com.refinitiv.eta.codec.Series				_rsslSeries;			// Pulled from EmaObjectManager
	protected com.refinitiv.eta.codec.DecodeIterator 	_rsslDecodeIter;		// Pulled from EmaObjectManager
	SeriesEntryImpl 									_seriesEntry = null;	// Pulled from EmaObjectManager
	SeriesImpl _seriesImpl;						// Uses the SeriesImpl of the parent that created this object
	int _errorCode = 0;

	public SeriesIterImpl(SeriesImpl seriesImpl)
	{
		this._seriesImpl = seriesImpl;
	}
	
	// Resets the iterImpl
	public void clear()
	{
		if (_seriesEntry != null)
		{
			_seriesEntry.returnToPool();
			_seriesEntry = null;
		}
		if (_rsslSeries != null)
		{
			_seriesImpl._objManager._rsslSeriesPool.add(_rsslSeries);
			_rsslSeries = null;
		}
		if (_rsslDecodeIter != null)
		{
			_seriesImpl._objManager._etaDecodeIteratorPool.add(_rsslDecodeIter);
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
		if (_seriesEntry == null)
		{
			_seriesEntry = seriesEntryInstance();
			_rsslSeries = rsslSeriesInstance();
			_rsslDecodeIter = decodeIteratorInstance();
			
			retCode = _rsslDecodeIter.setBufferAndRWFVersion(_seriesImpl._rsslBuffer, _seriesImpl._rsslMajVer, _seriesImpl._rsslMinVer);
			if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS != retCode)
			{
				_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
				return true;
			}
			
			retCode = _rsslSeries.decode(_rsslDecodeIter);
		}
		
		if ((retCode  = _seriesEntry._rsslSeriesEntry.decode(_rsslDecodeIter)) != com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
			case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS :
				int dType = _seriesImpl.dataType(_rsslSeries.containerType(), _seriesImpl._rsslMajVer, _seriesImpl._rsslMinVer, _seriesEntry._rsslSeriesEntry.encodedData());
				load = _seriesImpl.dataInstance(_seriesEntry._load, dType);
				load.decode(_seriesEntry._rsslSeriesEntry.encodedData(), _seriesImpl._rsslMajVer, _seriesImpl._rsslMinVer, _seriesImpl._rsslDictionary, _seriesImpl._rsslLocalSetDefDb);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA :
				load = _seriesImpl.dataInstance(_seriesEntry._load, DataTypes.ERROR);
				load.decode(_seriesEntry._rsslSeriesEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				load = _seriesImpl.dataInstance(_seriesEntry._load, DataTypes.ERROR);
				load.decode(_seriesEntry._rsslSeriesEntry.encodedData(),ErrorCode.UNSUPPORTED_DATA_TYPE);
				break;
			default :
				load = _seriesImpl.dataInstance(_seriesEntry._load, DataTypes.ERROR);
				load.decode(_seriesEntry._rsslSeriesEntry.encodedData(),ErrorCode.UNKNOWN_ERROR);
				break;
			}
		
			_seriesEntry._load = load;
			return true;
		}
		else
		{
			return false;
		}
	}

	@Override
	// Only returns the current iter we are on. Does NOT iterate, as we require hasNext() called first.
	public SeriesEntry next() {
		return _seriesEntry;
	}

	private SeriesEntryImpl seriesEntryInstance()
	{
		SeriesEntryImpl retData = (SeriesEntryImpl)_seriesImpl._objManager._seriesEntryPool.poll();
        if(retData == null)
        {
        	retData = new SeriesEntryImpl(com.refinitiv.eta.codec.CodecFactory.createSeriesEntry(), _seriesImpl.noDataInstance());
        	_seriesImpl._objManager._seriesEntryPool.updatePool(retData);
        }
        else
        	retData._rsslSeriesEntry.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.Series rsslSeriesInstance()
	{
		com.refinitiv.eta.codec.Series retData = null;
		retData = _seriesImpl._objManager._rsslSeriesPool.poll();
        if (retData == null)
        {
        	retData = com.refinitiv.eta.codec.CodecFactory.createSeries();
        }
        else
        	retData.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.DecodeIterator decodeIteratorInstance()
	{
		com.refinitiv.eta.codec.DecodeIterator retData = null;
		retData = _seriesImpl._objManager._etaDecodeIteratorPool.poll();
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