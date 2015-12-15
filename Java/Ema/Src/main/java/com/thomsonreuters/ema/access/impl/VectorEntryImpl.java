///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.VectorEntry;

public class VectorEntryImpl extends EntryImpl implements VectorEntry
{
	private final static String SET_STRING 				= "Set";
	private final static String UPDATE_STRING 			= "Update";
	private final static String CLEAR_STRING 			= "Clear";
	private final static String INSERT_STRING 			= "Insert";
	private final static String DELETE_STRING 			= "Delete";
	private final static String DEFAULTACTION_STRING 	= "Unknown VectorAction value ";
	
	private ByteBuffer _permData;
	private com.thomsonreuters.upa.codec.VectorEntry	_rsslVectorEntry;

	VectorEntryImpl(VectorImpl vector, com.thomsonreuters.upa.codec.VectorEntry rsslVectorEntry, DataImpl load)
	{
		super(vector, load);
		_rsslVectorEntry = rsslVectorEntry;
	}
	
	@Override
	public String vectorActionAsString()
	{
		switch (action())
		{
			case VectorAction.SET:
				return SET_STRING;
			case VectorAction.UPDATE:
				return UPDATE_STRING;
			case VectorAction.CLEAR:
				return CLEAR_STRING;
			case VectorAction.DELETE:
				return DELETE_STRING;
			case VectorAction.INSERT:
				return INSERT_STRING;
			default:
				return DEFAULTACTION_STRING + action();
		}
	}

	@Override
	public boolean hasPermissionData()
	{
		return _rsslVectorEntry.checkHasPermData();
	}

	@Override
	public int action()
	{
		return _rsslVectorEntry.action();
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw oommIUExcept().message("Attempt to permissionData() while it is NOT set.");
		
		GlobalPool.releaseByteBuffer(_permData);
		_permData = DataImpl.asByteBuffer(_rsslVectorEntry.permData());
		
		return _permData;
	}

	@Override
	public long position()
	{
		return _rsslVectorEntry.index();
	}
	
	@Override
	public String toString()
	{
		_toString.setLength(0);
		_toString.append("VectorEntry ")
				.append(" action=\"").append(vectorActionAsString()).append("\"")
				.append(" index=\"").append(position());
				
		if (hasPermissionData())
		{
			_toString.append("\" permissionData=\"").append(permissionData()).append("\"");
			Utilities.asHexString(_toString, permissionData()).append("\"");
		}
		
		_toString.append("\" dataType=\"").append(DataType.asString(_load.dataType())).append("\"\n");
		_toString.append(_load.toString(1));
		Utilities.addIndent(_toString, 0).append("VectorEntryEnd\n");

		return _toString.toString();
	}
}