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
import com.refinitiv.eta.codec.FilterEntryActions;

class FilterListIterImpl implements Iterator<FilterEntry>
{
	public com.refinitiv.eta.codec.FilterList			_rsslFilterList;		// Pulled from EmaObjectManager
	protected com.refinitiv.eta.codec.DecodeIterator 	_rsslDecodeIter;		// Pulled from EmaObjectManager
	FilterEntryImpl 									_filterEntry = null;	// Pulled from EmaObjectManager
	FilterListImpl _filterListImpl;			// Uses the FilterListImpl of the parent that created this object
	int _errorCode = 0;
	
	public FilterListIterImpl(FilterListImpl filterListImpl)
	{
		this._filterListImpl = filterListImpl;
	}
	
	// Resets the iterImpl
	public void clear()
	{
		if (_filterEntry != null)
		{
			_filterEntry.returnToPool();
			_filterEntry = null;
		}
		if (_rsslFilterList != null)
		{
			_filterListImpl._objManager._rsslFilterListPool.add(_rsslFilterList);
			_rsslFilterList = null;
		}
		if (_rsslDecodeIter != null)
		{
			_filterListImpl._objManager._etaDecodeIteratorPool.add(_rsslDecodeIter);
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
		if (_filterEntry == null)
		{
			_filterEntry = filterEntryInstance();
			_rsslFilterList = rsslFilterListInstance();
			_rsslDecodeIter = decodeIteratorInstance();
			
			retCode = _rsslDecodeIter.setBufferAndRWFVersion(_filterListImpl._rsslBuffer, _filterListImpl._rsslMajVer, _filterListImpl._rsslMinVer);
			if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS != retCode)
			{
				_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
				return true;
			}
			
			retCode = _rsslFilterList.decode(_rsslDecodeIter);
		}
		
		if ((retCode  = _filterEntry._rsslFilterEntry.decode(_rsslDecodeIter)) != com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
			case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS :
				int rsslContainerType = (_filterEntry._rsslFilterEntry.action() == FilterEntryActions.CLEAR) ?  com.refinitiv.eta.codec.DataTypes.NO_DATA : (_filterEntry._rsslFilterEntry.checkHasContainerType() ?
						_filterEntry._rsslFilterEntry.containerType() : _rsslFilterList.containerType());

				int dType = _filterListImpl.dataType(rsslContainerType, _filterListImpl._rsslMajVer, _filterListImpl._rsslMinVer, _filterEntry._rsslFilterEntry.encodedData());
				
				load = _filterListImpl.dataInstance(_filterEntry._load, dType);
				load.decode(_filterEntry._rsslFilterEntry.encodedData(), _filterListImpl._rsslMajVer, _filterListImpl._rsslMinVer, _filterListImpl._rsslDictionary, null);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA :
				load = _filterListImpl.dataInstance(_filterEntry._load, DataTypes.ERROR);
				load.decode(_filterEntry._rsslFilterEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				load = _filterListImpl.dataInstance(_filterEntry._load, DataTypes.ERROR);
				load.decode(_filterEntry._rsslFilterEntry.encodedData(),ErrorCode.UNSUPPORTED_DATA_TYPE);
				break;
			default :
				load = _filterListImpl.dataInstance(_filterEntry._load, DataTypes.ERROR);
				load.decode(_filterEntry._rsslFilterEntry.encodedData(),ErrorCode.UNKNOWN_ERROR);
				break;
			}
			
			_filterEntry._load = load;
			return true;
		}
		else
		{
			return false;
		}
	}

	@Override
	// Only returns the current iter we are on. Does NOT iterate, as we require hasNext() called first.
	public FilterEntry next() {
		return _filterEntry;
	}

	private FilterEntryImpl filterEntryInstance()
	{
		FilterEntryImpl retData = (FilterEntryImpl)_filterListImpl._objManager._filterEntryPool.poll();
        if(retData == null)
        {
        	retData = new FilterEntryImpl(com.refinitiv.eta.codec.CodecFactory.createFilterEntry(), _filterListImpl.noDataInstance(), _filterListImpl._objManager);
        	_filterListImpl._objManager._filterEntryPool.updatePool(retData);
        }
        else
        	retData._rsslFilterEntry.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.FilterList rsslFilterListInstance()
	{
		com.refinitiv.eta.codec.FilterList retData = null;
		if (_filterListImpl._objManager._rsslFilterListPool.size() > 0)
		{
			retData = _filterListImpl._objManager._rsslFilterListPool.remove(_filterListImpl._objManager._rsslFilterListPool.size()-1);
		}
        if (retData == null)
        {
        	retData = com.refinitiv.eta.codec.CodecFactory.createFilterList();
        }
        else
        	retData.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.DecodeIterator decodeIteratorInstance()
	{
		com.refinitiv.eta.codec.DecodeIterator retData = null;
		if (_filterListImpl._objManager._etaDecodeIteratorPool.size() > 0)
		{
			retData = _filterListImpl._objManager._etaDecodeIteratorPool.remove(_filterListImpl._objManager._etaDecodeIteratorPool.size()-1);
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