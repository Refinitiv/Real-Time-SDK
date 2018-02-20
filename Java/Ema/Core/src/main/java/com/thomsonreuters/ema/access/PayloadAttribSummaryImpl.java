///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import com.thomsonreuters.ema.access.DataType.DataTypes;

class PayloadAttribSummaryImpl implements Payload, Attrib, SummaryData
{
	private Data 						 _data;
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private StringBuilder 				 _errorString;

	@Override
	public int dataType()
	{
		return _data.dataType();
	}

	@Override
	public ComplexType data()
	{
		return (ComplexType)_data;
	}

	@Override
	public ReqMsg reqMsg()
	{
		if (_data.dataType() != DataTypes.REQ_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to reqMsg() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}

		return (ReqMsg)_data;
	}

	@Override
	public RefreshMsg refreshMsg()
	{
		if (_data.dataType() != DataTypes.REFRESH_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to refreshMsg() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}

		return (RefreshMsg)_data;
	}

	@Override
	public UpdateMsg updateMsg()
	{
		if (_data.dataType() != DataTypes.UPDATE_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to updateMsg() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}

		return (UpdateMsg)_data;
	}

	@Override
	public StatusMsg statusMsg()
	{
		if (_data.dataType() != DataTypes.STATUS_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to statusMsg() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}

		return (StatusMsg)_data;
	}

	@Override
	public PostMsg postMsg()
	{
		if (_data.dataType() != DataTypes.POST_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to postMsg() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}

		return (PostMsg)_data;
	}

	@Override
	public AckMsg ackMsg()
	{
		if (_data.dataType() != DataTypes.ACK_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ackMsg() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}

		return (AckMsg)_data;
	}

	@Override
	public GenericMsg genericMsg()
	{
		if (_data.dataType() != DataTypes.GENERIC_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to genericMsg() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}

		return (GenericMsg)_data;
	}

	@Override
	public FieldList fieldList()
	{
		if (_data.dataType() != DataTypes.FIELD_LIST)
		{
			StringBuilder error = errorString();
			error.append("Attempt to fieldList() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (FieldList)_data;
	}

	@Override
	public ElementList elementList()
	{
		if (_data.dataType() != DataTypes.ELEMENT_LIST)
		{
			StringBuilder error = errorString();
			error.append("Attempt to elementList() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (ElementList)_data;
	}

	@Override
	public Map map()
	{
		if (_data.dataType() != DataTypes.MAP)
		{
			StringBuilder error = errorString();
			error.append("Attempt to map() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (Map)_data;
	}

	@Override
	public Vector vector()
	{
		if (_data.dataType() != DataTypes.VECTOR)
		{
			StringBuilder error = errorString();
			error.append("Attempt to vector() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (Vector)_data;
	}

	@Override
	public Series series()
	{
		if (_data.dataType() != DataTypes.SERIES)
		{
			StringBuilder error = errorString();
			error.append("Attempt to series() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (Series)_data;
	}

	@Override
	public FilterList filterList()
	{
		if (_data.dataType() != DataTypes.FILTER_LIST)
		{
			StringBuilder error = errorString();
			error.append("Attempt to filterList() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (FilterList)_data;
	}

	@Override
	public OmmOpaque opaque()
	{
		if (_data.dataType() != DataTypes.OPAQUE)
		{
			StringBuilder error = errorString();
			error.append("Attempt to opaque() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (OmmOpaque)_data;
	}

	@Override
	public OmmXml xml()
	{
		if (_data.dataType() != DataTypes.XML)
		{
			StringBuilder error = errorString();
			error.append("Attempt to xml() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (OmmXml)_data;
	}

	@Override
	public OmmAnsiPage ansiPage()
	{
		if (_data.dataType() != DataTypes.ANSI_PAGE)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ansiPage() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (OmmAnsiPage)_data;
	}

	@Override
	public OmmError error()
	{
		if (_data.dataType() != DataTypes.ERROR)
		{
			StringBuilder error = errorString();
			error.append("Attempt to error() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (OmmError)_data;
	}

	void data(Data data)
	{
		_data = data;
	}

	OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();
		
		return _ommIUExcept;
	}
	
	StringBuilder errorString()
	{
		if (_errorString == null)
			_errorString = new StringBuilder();
		else
			_errorString.setLength(0);
			
		return _errorString;
	}
}