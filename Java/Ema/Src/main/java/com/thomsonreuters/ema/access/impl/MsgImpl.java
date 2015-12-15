///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.Attrib;
import com.thomsonreuters.ema.access.ComplexType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.ema.access.Payload;
import com.thomsonreuters.upa.codec.AckMsgFlags;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.GenericMsgFlags;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.PostMsgFlags;
import com.thomsonreuters.upa.codec.RefreshMsgFlags;
import com.thomsonreuters.upa.codec.StatusMsgFlags;
import com.thomsonreuters.upa.codec.UpdateMsgFlags;
import com.thomsonreuters.upa.rdm.DomainTypes;

class MsgImpl extends DataImpl implements Msg
{
	protected int 							   			_dataType;
	protected ByteBuffer 								_permissionData;
	protected ByteBuffer 								_itemGroup;
	protected PayloadAttribSummaryImpl 					_payloadAttrib = new PayloadAttribSummaryImpl();
	protected DataImpl 									_attribDecoded = new NoDataImpl();
	protected DataImpl 									_payloadDecoded =  new NoDataImpl();
	protected boolean 									_serviceNameSet;
	protected int 										_errorCode = ErrorCode.NO_ERROR;
	protected com.thomsonreuters.upa.codec.DataDictionary _rsslDictionary;
	protected com.thomsonreuters.upa.codec.Msg 	 		  _rsslMsg; 
	protected com.thomsonreuters.upa.codec.Msg 			  _rsslNestedMsg = CodecFactory.createMsg();
	protected com.thomsonreuters.upa.codec.Buffer 		  _rsslServiceNameBuffer = CodecFactory.createBuffer();
	
	protected com.thomsonreuters.upa.codec.Msg _rsslMsgEncoded; 
	protected boolean _initialEncoding = true;
	
	
	MsgImpl(int dataType)
	{
		_dataType = dataType;
	}
	
	@Override
	public boolean hasMsgKey()
	{
		switch (_dataType)
        {
            case DataTypes.UPDATE_MSG :
                return ((_rsslMsg.flags() & UpdateMsgFlags.HAS_MSG_KEY) > 0 ? true : false);
            case DataTypes.REFRESH_MSG :
                return ((_rsslMsg.flags() & RefreshMsgFlags.HAS_MSG_KEY) > 0 ? true : false);
            case DataTypes.STATUS_MSG :
                return ((_rsslMsg.flags() & StatusMsgFlags.HAS_MSG_KEY) > 0 ? true : false);
            case DataTypes.GENERIC_MSG :
                return ((_rsslMsg.flags() & GenericMsgFlags.HAS_MSG_KEY) > 0 ? true : false);
            case DataTypes.POST_MSG :
                return ((_rsslMsg.flags() & PostMsgFlags.HAS_MSG_KEY) > 0 ? true : false);
            case DataTypes.ACK_MSG :
                return ((_rsslMsg.flags() & AckMsgFlags.HAS_MSG_KEY) > 0 ? true : false);
            default :
                return false;
        }
	}

	@Override
	public boolean hasName()
	{
		return (_rsslMsg.msgKey() != null && _rsslMsg.msgKey().checkHasName()) ? true : false; 
	}

	@Override
	public boolean hasNameType()
	{
		return (_rsslMsg.msgKey() != null && _rsslMsg.msgKey().checkHasNameType()) ? true : false;
	}

	@Override
	public boolean hasServiceId()
	{
		return (_rsslMsg.msgKey() != null && _rsslMsg.msgKey().checkHasServiceId()) ? true : false;
	}

	@Override
	public boolean hasId()
	{
		return (_rsslMsg.msgKey() != null && _rsslMsg.msgKey().checkHasIdentifier()) ? true : false;
	}

	@Override
	public boolean hasFilter()
	{
		return (_rsslMsg.msgKey() != null && _rsslMsg.msgKey().checkHasFilter()) ? true : false;
	}

	@Override
	public boolean hasExtendedHeader()
	{
		return (_rsslMsg.extendedHeader() != null) ? true : false; 
	}

	@Override
	public int streamId()
	{
		return _rsslMsg.streamId();
	}

	@Override
	public int domainType()
	{
		return _rsslMsg.domainType();
	}

	@Override
	public String name()
	{
		if (!hasName())
		{
			String temp = "Attempt to name() while it is NOT set." ;
			throw oommIUExcept().message(temp);
		}
		
		return _rsslMsg.msgKey().name().toString();
	}

	@Override
	public int nameType()
	{
		if (!hasNameType())
		{
			String temp = "Attempt to nameType() while it is NOT set." ;
			throw oommIUExcept().message(temp);
		}
		
		return _rsslMsg.msgKey().nameType();
	}

	@Override
	public int serviceId()
	{
		if (!hasServiceId())
		{
			String temp = "Attempt to serviceId() while it is NOT set." ;
			throw oommIUExcept().message(temp);
		}
		
		return _rsslMsg.msgKey().serviceId();
	}

	@Override
	public int id()
	{
		if (!hasId())
		{
			String temp = "Attempt to id() while it is NOT set." ;
			throw oommIUExcept().message(temp);
		}
		
		return _rsslMsg.msgKey().identifier();
	}

	@Override
	public long filter()
	{
		if (!hasFilter())
		{
			String temp = "Attempt to filter() while it is NOT set." ;
			throw oommIUExcept().message(temp);
		}

		return _rsslMsg.msgKey().filter();
	}

	@Override
	public ByteBuffer extendedHeader()
	{
		if (!hasExtendedHeader())
		{
			String temp = "Attempt to extendedHeader() while it is NOT set." ;
			throw oommIUExcept().message(temp);
		}
		
		return _rsslMsg.extendedHeader().data();
	}

	@Override
	public Attrib attrib()
	{
		_payloadAttrib.data(_attribDecoded);
		return (Attrib) _payloadAttrib;
	}

	@Override
	public Payload payload()
	{
		_payloadAttrib.data(_payloadDecoded);
		return (Payload) _payloadAttrib;
	}
	
	@Override
	public int dataType()
	{
		return _dataType;
	}

	public boolean hasServiceName()
	{
		return (_serviceNameSet ? true : false);
	}
	
	public String serviceName()
	{
		if (!_serviceNameSet)
		{
			String temp = "Attempt to serviceName() while it is NOT set." ;
			throw oommIUExcept().message(temp);
		}
		
		return _rsslServiceNameBuffer.toString();
	}
	
	void decodeAttribPayload()
	{
		com.thomsonreuters.upa.codec.MsgKey msgKey = _rsslMsg.msgKey();
		int dType;
		if (msgKey != null)
		{
			int rsslDType =  msgKey.checkHasAttrib() ? msgKey.attribContainerType()
							: com.thomsonreuters.upa.codec.DataTypes.NO_DATA;
			
			dType = dataType(rsslDType, _rsslMajVer, _rsslMinVer, msgKey.encodedAttrib());
			if (DataTypes.ERROR == dType)
			{
				_attribDecoded = dataInstance(_attribDecoded, DataTypes.ERROR);
				_attribDecoded.decode(_rsslMsg.msgKey().encodedAttrib(),
						_rsslMajVer, _rsslMinVer, null, ErrorCode.ITERATOR_SET_FAILURE);
			}
			else
			{
				_attribDecoded = dataInstance(_attribDecoded, dType);
				_attribDecoded.decode(_rsslMsg.msgKey().encodedAttrib(),
						_rsslMajVer, _rsslMinVer, _rsslDictionary, null);
			}
		}
		
		dType = dataType(_rsslMsg.containerType(), _rsslMajVer, _rsslMinVer, _rsslMsg.encodedDataBody());
		if (DataTypes.ERROR == dType)
		{
			_payloadDecoded = dataInstance(_payloadDecoded, DataTypes.ERROR);
			_payloadDecoded.decode(_rsslMsg.encodedDataBody(),
					_rsslMajVer, _rsslMinVer, null, ErrorCode.ITERATOR_SET_FAILURE);
		}
		else
		{
			_payloadDecoded = dataInstance(_payloadDecoded, dType);
			_payloadDecoded.decode(_rsslMsg.encodedDataBody(), _rsslMajVer, _rsslMinVer, _rsslDictionary, null);
		}
	}
	
	boolean hasAttrib()
	{
		return _rsslMsg.msgKey().checkHasAttrib();
	}
	
	boolean hasPayload()
	{
		return ((_rsslMsg.containerType() != com.thomsonreuters.upa.codec.DataTypes.NO_DATA) ? true : false);
	}
	
	DataImpl attribData()
	{
		return _attribDecoded;
	}

	DataImpl payloadData()
	{
		return _payloadDecoded;
	}
	
	//TODO
	void intialEncoding()
	{
		_initialEncoding = false;

		if (_rsslMsgEncoded == null)
			_rsslMsgEncoded = CodecFactory.createMsg(); 
		
		_rsslMsgEncoded.msgClass(Utilities.toRsslMsgClass[_dataType]);
		
		switch (_rsslMsgEncoded.msgClass())
		{
			case MsgClasses.REQUEST :
			{
				_rsslMsgEncoded.domainType(DomainTypes.MARKET_PRICE);
				((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).applyStreaming();
				_rsslMsgEncoded.containerType(com.thomsonreuters.upa.codec.DataTypes.NO_DATA);
				break;
			}	
			case MsgClasses.REFRESH :
			{
				break;
			}	
			case MsgClasses.UPDATE :
			{
				break;
			}	
			case MsgClasses.STATUS :
			{
				break;
			}	
			case MsgClasses.GENERIC :
			{
				break;
			}	
			case MsgClasses.POST :
			{
				break;
			}	
			case MsgClasses.ACK :
			{
				break;
			}	
			case MsgClasses.CLOSE :
			{
				break;
			}
			default:
				return;
		}
	}
	
	void msgStreamId(int streamId)
	{
		_rsslMsgEncoded.streamId(streamId);
	}

	void msgDomainType(int domainType)
	{
		_rsslMsgEncoded.domainType(domainType);
	}

	void msgName(String name)
	{
		_rsslMsgEncoded.msgKey().applyHasName();
		_rsslMsgEncoded.msgKey().name().data(name);
	}

	void msgNameType(int nameType)
	{
		_rsslMsgEncoded.msgKey().applyHasNameType();
		_rsslMsgEncoded.msgKey().nameType(nameType);
	}

	void msgServiceName(String serviceName)
	{
		_rsslServiceNameBuffer.data(serviceName);
		_serviceNameSet = true;
	}

	void serviceName(com.thomsonreuters.upa.codec.Buffer serviceName)
	{
		_rsslServiceNameBuffer.data(serviceName.toString());
		_serviceNameSet = true;
	}
		
	void msgServiceId(int serviceId)
	{
		_rsslMsgEncoded.msgKey().applyHasServiceId();
		_rsslMsgEncoded.msgKey().applyHasServiceId();
		_rsslMsgEncoded.msgKey().serviceId(serviceId);
	}

	void msgId(int id)
	{
		_rsslMsgEncoded.msgKey().applyHasIdentifier();
		_rsslMsgEncoded.msgKey().identifier(id);
	}

	void msgFilter(long filter)
	{
		_rsslMsgEncoded.msgKey().applyHasFilter();
		_rsslMsgEncoded.msgKey().filter(filter);
	}

	void msgState(int streamState, int dataState, int statusCode,
			String statusText)
	{

	}

	void msgSeqNum(long seqNum)
	{

	}

	void msgPartNum(int partNum)
	{

	}

	void msgItemGroup(ByteBuffer itemGroup)
	{

	}

	void msgPermissionData(ByteBuffer permissionData)
	{
	}

	void msgPublisherId(long userId, long userAddress)
	{

	}

	void msgAttrib(ComplexType data)
	{
		_rsslMsgEncoded.msgKey().applyHasAttrib();
		_rsslMsgEncoded.msgKey().attribContainerType(Utilities.toRsslDataType(data.dataType()));
		_rsslMsgEncoded.msgKey().encodedAttrib().data(((ComplexTypeImpl)data).dataBuffer());
	}

	void msgPayload(ComplexType data)
	{
		_rsslMsgEncoded.containerType(Utilities.toRsslDataType(data.dataType()));
		_rsslMsgEncoded.encodedDataBody().data(((ComplexTypeImpl)data).dataBuffer());
	}

	void msgExtendedHeader(ByteBuffer buffer)
	{
		_rsslMsgEncoded.extendedHeader().data(buffer);
	}

	void msgSolicited(boolean solicited)
	{
	}

	void msgDoNotCache(boolean doNotCache)
	{
	}

	void msgClearCache(boolean clearCache)
	{
	}

	void msgComplete(boolean complete)
	{
	}

	void refreshMsgClear()
	{
	}

	void reqMsgClear()
	{
		if (_rsslMsgEncoded == null)
			_rsslMsgEncoded = CodecFactory.createMsg(); 
		else
			_rsslMsgEncoded.clear();
		
		_rsslMsgEncoded.msgClass(MsgClasses.REQUEST);
		
		_rsslMsgEncoded.domainType(DomainTypes.MARKET_PRICE);
		((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsgEncoded).applyStreaming();
		_rsslMsgEncoded.containerType(com.thomsonreuters.upa.codec.DataTypes.NO_DATA);
	}
	
	void updateMsgClear()
	{
	}
	
	void statusMsgClear()
	{
	}
}