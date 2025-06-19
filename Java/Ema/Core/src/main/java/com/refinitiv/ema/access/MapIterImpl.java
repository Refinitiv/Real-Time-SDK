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
import com.refinitiv.eta.codec.MapEntryActions;

class MapIterImpl implements Iterator<MapEntry>
{
	public com.refinitiv.eta.codec.Map					_rsslMap;			// Pulled from EmaObjectManager
	protected com.refinitiv.eta.codec.DecodeIterator 	_rsslDecodeIter;	// Pulled from EmaObjectManager
	private com.refinitiv.eta.codec.DecodeIterator 		_keyDecodeIter;		// Pulled from EmaObjectManager
	MapEntryImpl 										_mapEntry = null;	// Pulled from EmaObjectManager
	MapImpl _mapImpl;				// Uses the MapImpl of the parent that created this object

	public MapIterImpl(MapImpl mapImpl)
	{
		this._mapImpl = mapImpl;
	}
	
	// Resets the iterImpl
	public void clear()
	{
		if (_mapEntry != null)
		{
			_mapEntry.returnToPool();
			_mapEntry = null;
		}
		if (_rsslMap != null)
		{
			_mapImpl._objManager._rsslMapPool.add(_rsslMap);
			_rsslMap = null;
		}
		if (_rsslDecodeIter != null)
		{
			_mapImpl._objManager._etaDecodeIteratorPool.add(_rsslDecodeIter);
			_rsslDecodeIter = null;
		}
		if (_keyDecodeIter != null)
		{
			_mapImpl._objManager._etaDecodeIteratorPool.add(_keyDecodeIter);
			_keyDecodeIter = null;
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
		DataImpl mapEntryKey;
		
		int retCode;

		// First entry to decode
		if (_mapEntry == null)
		{
			_mapEntry = mapEntryInstance();
			_rsslMap = rsslMapInstance();
			_rsslDecodeIter = decodeIteratorInstance();
			_keyDecodeIter = decodeIteratorInstance();
			
			retCode = _rsslDecodeIter.setBufferAndRWFVersion(_mapImpl._rsslBuffer, _mapImpl._rsslMajVer, _mapImpl._rsslMinVer);
			
			retCode = _rsslMap.decode(_rsslDecodeIter);
		}
		
		if ((retCode  = _mapEntry._rsslMapEntry.decode(_rsslDecodeIter, null)) != com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
				case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS :
					retCode = _keyDecodeIter.setBufferAndRWFVersion( _mapEntry._rsslMapEntry.encodedKey(), _mapImpl._rsslMajVer, _mapImpl._rsslMinVer);
					if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS != retCode)
					{
						load =  _mapImpl.dataInstance(_mapEntry._load, DataTypes.ERROR);
						load.decode(_mapImpl._rsslBuffer, _mapImpl._errorCode);
						
						mapEntryKey = _mapImpl.dataInstance(((DataImpl)_mapEntry._keyDataDecoded.data()), DataTypes.NO_DATA);
						
						return true;
					}
					
					mapEntryKey = _mapImpl.dataInstance(((DataImpl)_mapEntry._keyDataDecoded.data()), _rsslMap.keyPrimitiveType());
					mapEntryKey.decode(_mapEntry._rsslMapEntry.encodedKey(), _keyDecodeIter);
					
					int rsslContainerType = ( _mapEntry._rsslMapEntry.action() != MapEntryActions.DELETE)? _rsslMap.containerType() : com.refinitiv.eta.codec.DataTypes.NO_DATA;
					int dType = _mapImpl.dataType(rsslContainerType, _mapImpl._rsslMajVer, _mapImpl._rsslMinVer,  _mapEntry._rsslMapEntry.encodedData());
					
					load = _mapImpl.dataInstance(_mapEntry._load, dType);
					load.decode( _mapEntry._rsslMapEntry.encodedData(), _mapImpl._rsslMajVer, _mapImpl._rsslMinVer, _mapImpl._rsslDictionary, _mapImpl._rsslLocalSetDefDb);
					break;
				case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA :
					mapEntryKey = _mapImpl.dataInstance(((DataImpl)_mapEntry._keyDataDecoded.data()), DataTypes.NO_DATA);
					load = _mapImpl.dataInstance(_mapEntry._load, DataTypes.ERROR);
					load.decode( _mapEntry._rsslMapEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
					break;
				case com.refinitiv.eta.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
					mapEntryKey = _mapImpl.dataInstance(((DataImpl)_mapEntry._keyDataDecoded.data()), DataTypes.NO_DATA);
					load = _mapImpl.dataInstance(_mapEntry._load, DataTypes.ERROR);
					load.decode( _mapEntry._rsslMapEntry.encodedData(),ErrorCode.UNSUPPORTED_DATA_TYPE);
					break;
				default :
					mapEntryKey = _mapImpl.dataInstance(((DataImpl)_mapEntry._keyDataDecoded.data()), DataTypes.NO_DATA);
					load = _mapImpl.dataInstance(_mapEntry._load, DataTypes.ERROR);
					load.decode( _mapEntry._rsslMapEntry.encodedData(),ErrorCode.UNKNOWN_ERROR);
					break;
				}
			_mapEntry.entryValue(mapEntryKey, load);
			return true;
		}
		else
		{
			return false;
		}
	}

	@Override
	// Only returns the current iter we are on. Does NOT iterate, as we require hasNext() called first.
	public MapEntry next() {
		return _mapEntry;
	}

	private MapEntryImpl mapEntryInstance()
	{
		MapEntryImpl retData = (MapEntryImpl)_mapImpl._objManager._mapEntryPool.poll();
        if(retData == null)
        {
        	retData = new MapEntryImpl(com.refinitiv.eta.codec.CodecFactory.createMapEntry(), _mapImpl.noDataInstance(), _mapImpl.noDataInstance(), _mapImpl._objManager);
        	_mapImpl._objManager._mapEntryPool.updatePool(retData);
        }
        else
        	retData._rsslMapEntry.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.Map rsslMapInstance()
	{
		com.refinitiv.eta.codec.Map retData = null;
		retData = _mapImpl._objManager._rsslMapPool.poll();
        if (retData == null)
        {
        	retData = com.refinitiv.eta.codec.CodecFactory.createMap();
        }
        else
        	retData.clear();
        
        return retData;
	}
	
	private com.refinitiv.eta.codec.DecodeIterator decodeIteratorInstance()
	{
		com.refinitiv.eta.codec.DecodeIterator retData = null;
		retData= _mapImpl._objManager._etaDecodeIteratorPool.poll();
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