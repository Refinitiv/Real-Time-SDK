///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2018. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;


class FilterEntryImpl extends EntryImpl implements FilterEntry
{
	private final static String SET_STRING 				= "Set";
	private final static String UPDATE_STRING 			= "Update";
	private final static String CLEAR_STRING 			= "Clear";
	private final static String DEFAULTACTION_STRING 	= "Unknown FilterAction value ";
	
	private ByteBuffer _permData;
	private EmaObjectManager _objManager;
	protected com.thomsonreuters.upa.codec.FilterEntry _rsslFilterEntry;
	
	FilterEntryImpl()
	{
		_rsslFilterEntry = com.thomsonreuters.upa.codec.CodecFactory.createFilterEntry();
	}
	
	FilterEntryImpl(com.thomsonreuters.upa.codec.FilterEntry rsslFilterEntry, DataImpl load, EmaObjectManager objManager)
	{
		super(load);
		_rsslFilterEntry = rsslFilterEntry;
		_objManager = objManager;
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
		
		_permData = Utilities.copyFromPool( _rsslFilterEntry.permData(), _permData, _objManager);
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
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry reqMsg(int filterId, int action, ReqMsg value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry refreshMsg(int filterId, int action, RefreshMsg value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry refreshMsg(int filterId, int action, RefreshMsg value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry statusMsg(int filterId, int action, StatusMsg value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry statusMsg(int filterId, int action, StatusMsg value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry updateMsg(int filterId, int action, UpdateMsg value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry updateMsg(int filterId, int action, UpdateMsg value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry postMsg(int filterId, int action, PostMsg value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry postMsg(int filterId, int action, PostMsg value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry ackMsg(int filterId, int action, AckMsg value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry ackMsg(int filterId, int action, AckMsg value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry genericMsg(int filterId, int action, GenericMsg value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry genericMsg(int filterId, int action, GenericMsg value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry fieldList(int filterId, int action, FieldList value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry fieldList(int filterId, int action, FieldList value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry elementList(int filterId, int action, ElementList value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry elementList(int filterId, int action, ElementList value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry map(int filterId, int action, Map value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry map(int filterId, int action, Map value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry vector(int filterId, int action, Vector value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry vector(int filterId, int action, Vector value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry series(int filterId, int action, Series value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry series(int filterId, int action, Series value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry filterList(int filterId, int action, FilterList value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry filterList(int filterId, int action, FilterList value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry opaque(int filterId, int action, OmmOpaque value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry opaque(int filterId, int action, OmmOpaque value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry xml(int filterId, int action, OmmXml value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry xml(int filterId, int action, OmmXml value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	@Override
	public FilterEntry ansiPage(int filterId, int action, OmmAnsiPage value)
	{
		return entryValue(filterId, action, (DataImpl) value, null);
	}

	@Override
	public FilterEntry ansiPage(int filterId, int action, OmmAnsiPage value, ByteBuffer permissionData)
	{
		return entryValue(filterId, action, (DataImpl) value, permissionData);
	}

	private FilterEntry entryValue(int filterId, int action, DataImpl value, ByteBuffer permissionData)
	{
		if (filterId < 0 || filterId > 255)
			throw ommOORExcept().message("position is out of range [0 - 255].");
		if (action < 0 || action > 15)
			throw ommOORExcept().message("action is out of range [0 - 15].");
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");

		_rsslFilterEntry.id(filterId);
		_rsslFilterEntry.action(action);
		_rsslFilterEntry.applyHasContainerType();
		_rsslFilterEntry.containerType(Utilities.toRsslDataType(value.dataType()));
		
		Utilities.copy(value.encodedData(), _rsslFilterEntry.encodedData());

		if (permissionData != null)
		{
			Utilities.copy(permissionData, _rsslFilterEntry.permData());
			_rsslFilterEntry.applyHasPermData();
		}

		return this;
	}
	
	private FilterEntry entryValue(int filterId, int action, ByteBuffer permissionData)
	{
		if (filterId < 0 || filterId > 255)
			throw ommOORExcept().message("position is out of range [0 - 255].");
		if (action < 0 || action > 15)
			throw ommOORExcept().message("action is out of range [0 - 15].");

		_rsslFilterEntry.id(filterId);
		_rsslFilterEntry.action(action);
		_rsslFilterEntry.applyHasContainerType();
		_rsslFilterEntry.containerType(com.thomsonreuters.upa.codec.DataTypes.NO_DATA);
		
		if (permissionData != null)
		{
			Utilities.copy(permissionData, _rsslFilterEntry.permData());
			_rsslFilterEntry.applyHasPermData();
		}

		return this;
	}

	@Override
	public FilterEntry noData(int filterId, int action) 
	{
		entryValue(filterId, action, null);
		return this;
	}

	@Override
	public FilterEntry noData(int filterId, int action, ByteBuffer permissionData)
	{
		entryValue(filterId, action, permissionData);
		return this;
	}
}
