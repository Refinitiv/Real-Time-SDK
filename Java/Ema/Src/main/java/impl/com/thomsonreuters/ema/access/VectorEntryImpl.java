///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.VectorEntry;
import com.thomsonreuters.upa.codec.CodecFactory;

class VectorEntryImpl extends EntryImpl implements VectorEntry
{
	private final static String SET_STRING 				= "Set";
	private final static String UPDATE_STRING 			= "Update";
	private final static String CLEAR_STRING 			= "Clear";
	private final static String INSERT_STRING 			= "Insert";
	private final static String DELETE_STRING 			= "Delete";
	private final static String DEFAULTACTION_STRING 	= "Unknown VectorAction value ";
	
	private ByteBuffer _permData;
	private com.thomsonreuters.upa.codec.VectorEntry	_rsslVectorEntry;

	VectorEntryImpl()
	{
		_rsslVectorEntry = CodecFactory.createVectorEntry();
	}
	
	VectorEntryImpl(com.thomsonreuters.upa.codec.VectorEntry rsslVectorEntry, DataImpl load)
	{
		super(load);
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
			throw ommIUExcept().message("Attempt to permissionData() while it is NOT set.");
		
		_permData = Utilities.copyFromPool( _rsslVectorEntry.permData(), _permData);
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

	@Override
	public VectorEntry reqMsg(int position, int action, ReqMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry reqMsg(int position, int action, ReqMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry refreshMsg(int position, int action, RefreshMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry refreshMsg(int position, int action, RefreshMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry statusMsg(int position, int action, StatusMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry statusMsg(int position, int action, StatusMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry updateMsg(int position, int action, UpdateMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry updateMsg(int position, int action, UpdateMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry postMsg(int position, int action, PostMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry postMsg(int position, int action, PostMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry ackMsg(int position, int action, AckMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry ackMsg(int position, int action, AckMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry genericMsg(int position, int action, GenericMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry genericMsg(int position, int action, GenericMsg value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry fieldList(int position, int action, FieldList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry fieldList(int position, int action, FieldList value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry elementList(int position, int action, ElementList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry elementList(int position, int action, ElementList value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry map(int position, int action, Map value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry map(int position, int action, Map value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry vector(int position, int action, Vector value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry vector(int position, int action, Vector value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry series(int position, int action, Series value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry series(int position, int action, Series value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry filterList(int position, int action, FilterList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry filterList(int position, int action, FilterList value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry opaque(int position, int action, OmmOpaque value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry opaque(int position, int action, OmmOpaque value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry xml(int position, int action, OmmXml value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry xml(int position, int action, OmmXml value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry ansiPage(int position, int action, OmmAnsiPage value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public VectorEntry ansiPage(int filterId, int action, OmmAnsiPage value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}
}