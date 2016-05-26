///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;


class VectorEntryImpl extends EntryImpl implements VectorEntry
{
	private final static String SET_STRING 				= "Set";
	private final static String UPDATE_STRING 			= "Update";
	private final static String CLEAR_STRING 			= "Clear";
	private final static String INSERT_STRING 			= "Insert";
	private final static String DELETE_STRING 			= "Delete";
	private final static String DEFAULTACTION_STRING 	= "Unknown VectorAction value ";
	
	private ByteBuffer _permData;
	private EmaObjectManager _objManager;
	protected com.thomsonreuters.upa.codec.VectorEntry	_rsslVectorEntry;
	protected int _entryDataType;
	
	VectorEntryImpl()
	{
		_rsslVectorEntry = com.thomsonreuters.upa.codec.CodecFactory.createVectorEntry();
	}
	
	VectorEntryImpl(com.thomsonreuters.upa.codec.VectorEntry rsslVectorEntry, DataImpl load, EmaObjectManager objManager)
	{
		super(load);
		_rsslVectorEntry = rsslVectorEntry;
		_objManager = objManager;
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
		
		_permData = Utilities.copyFromPool( _rsslVectorEntry.permData(), _permData, _objManager);
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
	public VectorEntry reqMsg(long position, int action, ReqMsg value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry reqMsg(long position, int action, ReqMsg value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry refreshMsg(long position, int action, RefreshMsg value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry refreshMsg(long position, int action, RefreshMsg value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry statusMsg(long position, int action, StatusMsg value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry statusMsg(long position, int action, StatusMsg value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry updateMsg(long position, int action, UpdateMsg value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry updateMsg(long position, int action, UpdateMsg value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry postMsg(long position, int action, PostMsg value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry postMsg(long position, int action, PostMsg value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry ackMsg(long position, int action, AckMsg value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry ackMsg(long position, int action, AckMsg value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry genericMsg(long position, int action, GenericMsg value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry genericMsg(long position, int action, GenericMsg value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry fieldList(long position, int action, FieldList value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry fieldList(long position, int action, FieldList value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry elementList(long position, int action, ElementList value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry elementList(long position, int action, ElementList value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry map(long position, int action, Map value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry map(long position, int action, Map value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry vector(long position, int action, Vector value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry vector(long position, int action, Vector value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry series(long position, int action, Series value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry series(long position, int action, Series value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry filterList(long position, int action, FilterList value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry filterList(long position, int action, FilterList value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry opaque(long position, int action, OmmOpaque value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry opaque(long position, int action, OmmOpaque value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry xml(long position, int action, OmmXml value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry xml(long position, int action, OmmXml value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	@Override
	public VectorEntry ansiPage(long position, int action, OmmAnsiPage value)
	{
		return entryValue(position, action, (DataImpl) value, null);
	}

	@Override
	public VectorEntry ansiPage(long position, int action, OmmAnsiPage value, ByteBuffer permissionData)
	{
		return entryValue(position, action, (DataImpl) value, permissionData);
	}

	private VectorEntry entryValue(long position, int action, DataImpl value, ByteBuffer permissionData)
	{
		if (position < 0 || position > 1073741823)
			throw ommOORExcept().message("position is out of range [0 - 1073741823].");
		if (action < 0 || action > 15)
			throw ommOORExcept().message("action is out of range [0 - 15].");
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");

		_rsslVectorEntry.index(position);
		_rsslVectorEntry.action(action);
		_entryDataType = Utilities.toRsslDataType(value.dataType());
		
		Utilities.copy(((DataImpl) value).encodedData(), _rsslVectorEntry.encodedData());
		if (permissionData != null)
		{
			Utilities.copy(permissionData, _rsslVectorEntry.permData());
			_rsslVectorEntry.applyHasPermData();
		}

		return this;
	}
}