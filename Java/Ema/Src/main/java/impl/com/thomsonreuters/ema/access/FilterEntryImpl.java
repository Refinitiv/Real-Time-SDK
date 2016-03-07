///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.FilterEntry;
import com.thomsonreuters.upa.codec.CodecFactory;

class FilterEntryImpl extends EntryImpl implements FilterEntry
{
	private final static String SET_STRING 				= "Set";
	private final static String UPDATE_STRING 			= "Update";
	private final static String CLEAR_STRING 			= "Clear";
	private final static String DEFAULTACTION_STRING 	= "Unknown FilterAction value ";
	
	private ByteBuffer _permData;
	private com.thomsonreuters.upa.codec.FilterEntry _rsslFilterEntry;
	
	FilterEntryImpl()
	{
		_rsslFilterEntry = CodecFactory.createFilterEntry();
	}
	
	FilterEntryImpl(com.thomsonreuters.upa.codec.FilterEntry rsslFilterEntry, DataImpl load)
	{
		super(load);
		_rsslFilterEntry = rsslFilterEntry;
	}
	
	@Override
	public String filterActionAsString()
	{
		switch (action())
		{
			case FilterAction.SET:
				return SET_STRING;
			case FilterAction.UPDATE:
				return UPDATE_STRING;
			case FilterAction.CLEAR:
				return CLEAR_STRING;
			default:
				return DEFAULTACTION_STRING + action();
		}
	}

	@Override
	public boolean hasPermissionData()
	{
		return _rsslFilterEntry.checkHasPermData();
	}

	@Override
	public int action()
	{
		return _rsslFilterEntry.action();
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw ommIUExcept().message("Attempt to permissionData() while it is NOT set.");
		
		_permData = Utilities.copyFromPool( _rsslFilterEntry.permData(), _permData);
		return _permData;
	}

	@Override
	public int filterId()
	{
		return _rsslFilterEntry.id();
	}
	
	@Override
	public String toString()
	{
		_toString.setLength(0);
		_toString.append("FilterEntry ")
				.append(" action=\"").append(filterActionAsString()).append("\"")
				.append(" filterId=\"").append(filterId());

		if (hasPermissionData())
		{
			_toString.append("\" permissionData=\"").append(permissionData()).append("\"");
			Utilities.asHexString(_toString, permissionData()).append("\"");
		}
		
		_toString.append("\" dataType=\"").append(DataType.asString(_load.dataType())).append("\"\n");
		_toString.append(_load.toString(1));
		Utilities.addIndent(_toString, 0).append("FilterEntryEnd\n");

		return _toString.toString();
	}

	@Override
	public FilterEntry reqMsg(int filterId, int action, ReqMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry reqMsg(int filterId, int action, ReqMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry refreshMsg(int filterId, int action, RefreshMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry refreshMsg(int filterId, int action, RefreshMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry statusMsg(int filterId, int action, StatusMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry statusMsg(int filterId, int action, StatusMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry updateMsg(int filterId, int action, UpdateMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry updateMsg(int filterId, int action, UpdateMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry postMsg(int filterId, int action, PostMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry postMsg(int filterId, int action, PostMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry ackMsg(int filterId, int action, AckMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry ackMsg(int filterId, int action, AckMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry genericMsg(int filterId, int action, GenericMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry genericMsg(int filterId, int action, GenericMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry fieldList(int filterId, int action, FieldList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry fieldList(int filterId, int action, FieldList value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry elementList(int filterId, int action, ElementList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry elementList(int filterId, int action, ElementList value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry map(int filterId, int action, Map value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry map(int filterId, int action, Map value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry vector(int filterId, int action, Vector value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry vector(int filterId, int action, Vector value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry series(int filterId, int action, Series value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry series(int filterId, int action, Series value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry filterList(int filterId, int action, FilterList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry filterList(int filterId, int action, FilterList value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry opaque(int filterId, int action, OmmOpaque value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry opaque(int filterId, int action, OmmOpaque value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry xml(int filterId, int action, OmmXml value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry xml(int filterId, int action, OmmXml value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry ansiPage(int filterId, int action, OmmAnsiPage value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterEntry ansiPage(int filterId, int action, OmmAnsiPage value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}
}