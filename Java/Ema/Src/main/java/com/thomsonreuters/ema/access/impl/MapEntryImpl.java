///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.Key;
import com.thomsonreuters.ema.access.MapEntry;

public class MapEntryImpl extends EntryImpl implements MapEntry
{
	private final static String ADD_STRING 				= "Add";
	private final static String UPDATE_STRING 			= "Update";
	private final static String DELETE_STRING 			= "Delete";
	private final static String DEFAULTACTION_STRING 	= "Unknown MapAction value ";
	
	private ByteBuffer _permData;
	private com.thomsonreuters.upa.codec.MapEntry	_rsslMapEntry;
	private DataImpl _mapEntryKey;

	MapEntryImpl(MapImpl map, com.thomsonreuters.upa.codec.MapEntry rsslMapEntry, DataImpl mapEntryKey, DataImpl load)
	{
		super(map, load);
		_rsslMapEntry = rsslMapEntry;
		_mapEntryKey = mapEntryKey;
	}
	
	@Override
	public String mapActionAsString()
	{
		switch (action())
		{
			case MapAction.ADD:
				return ADD_STRING;
			case MapAction.UPDATE:
				return UPDATE_STRING;
			case MapAction.DELETE:
				return DELETE_STRING;
			default:
				return DEFAULTACTION_STRING + action();
		}
	}

	@Override
	public boolean hasPermissionData()
	{
		return _rsslMapEntry.checkHasPermData();
	}

	@Override
	public Key key()
	{
		return ((MapImpl)_data).key(_mapEntryKey);
	}

	@Override
	public int action()
	{
		return _rsslMapEntry.action();
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw oommIUExcept().message("Attempt to permissionData() while it is NOT set.");
		
		GlobalPool.releaseByteBuffer(_permData);
		_permData = DataImpl.asByteBuffer(_rsslMapEntry.permData());
		
		return _permData;
	}
	
	@Override
	public String toString()
	{
		_toString.setLength(0);
		_toString.append("MapEntry ")
				.append(" action=\"").append(mapActionAsString()).append("\"")
				.append(" value=\"").append(key().toString());
				
		if (hasPermissionData())
		{
			_toString.append("\" permissionData=\"").append(permissionData()).append("\"");
			Utilities.asHexString(_toString, permissionData()).append("\"");
		}
		
		_toString.append("\" dataType=\"").append(DataType.asString(_load.dataType())).append("\"\n");
		_toString.append(_load.toString(1));
		Utilities.addIndent(_toString, 0).append("MapEntryEnd\n");

		return _toString.toString();
	}
	
	void load(DataImpl load, DataImpl mapEntryKey)
	{
		_load = load;
		_mapEntryKey = mapEntryKey;
	}
}