///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.Attrib;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.ema.access.Payload;
import com.thomsonreuters.upa.codec.AckMsgFlags;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.GenericMsgFlags;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.PostMsgFlags;
import com.thomsonreuters.upa.codec.RefreshMsgFlags;
import com.thomsonreuters.upa.codec.StatusMsgFlags;
import com.thomsonreuters.upa.codec.UpdateMsgFlags;
import com.thomsonreuters.upa.rdm.DomainTypes;

class MsgImpl extends DataImpl implements Msg
{
	private OmmOutOfRangeExceptionImpl	_ommOORExcept;
	private OmmInvalidUsageExceptionImpl	_ommIUExcept;
	private OmmUnsupportedDomainTypeExceptionImpl _ommUDTExcept;
	protected int _dataType;
	protected ByteBuffer	_permissionData;
	protected ByteBuffer	_itemGroup;
	protected PayloadAttribSummaryImpl	_payloadAttrib = new PayloadAttribSummaryImpl();
	protected DataImpl 	_attribDecoded = noDataInstance();
	protected DataImpl 	_payloadDecoded =  noDataInstance();
	protected com.thomsonreuters.upa.codec.DataDictionary _rsslDictionary;
	protected com.thomsonreuters.upa.codec.Msg _rsslMsg; 
	protected com.thomsonreuters.upa.codec.Msg _rsslNestedMsg = CodecFactory.createMsg();
	protected String _serviceName;
	protected boolean _serviceNameSet;
	protected com.thomsonreuters.upa.codec.EncodeIterator _rsslEncodeIter;
	protected boolean _encodeComplete;
	protected int  _errorCode = ErrorCode.NO_ERROR;
	protected StringBuilder _errorString;
	
	MsgImpl(int dataType, boolean decoding)
	{
		_dataType = dataType;
		if (!decoding)
		{
			_rsslMsg = CodecFactory.createMsg(); 
			_rsslMsg.msgClass(Utilities.toRsslMsgClass[_dataType]);
			_rsslMsg.domainType(DomainTypes.MARKET_PRICE);
			_rsslMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.NO_DATA);
			
			_encodeComplete = false;
			_rsslEncodeIter = com.thomsonreuters.upa.codec.CodecFactory.createEncodeIterator() ;
			_rsslBuffer = CodecFactory.createBuffer();
			_rsslBuffer.data(ByteBuffer.allocate(CollectionDataImpl.ENCODE_RSSL_BUFFER_INIT_SIZE));
		}
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
			throw ommIUExcept().message(temp);
		}
		
		com.thomsonreuters.upa.codec.Buffer nameBuffer = _rsslMsg.msgKey().name();
		if (nameBuffer.length() == 0)
			return DataImpl.EMPTY_STRING;
		else
			return nameBuffer.toString();
	}

	@Override
	public int nameType()
	{
		if (!hasNameType())
		{
			String temp = "Attempt to nameType() while it is NOT set." ;
			throw ommIUExcept().message(temp);
		}
		
		return _rsslMsg.msgKey().nameType();
	}

	@Override
	public int serviceId()
	{
		if (!hasServiceId())
		{
			String temp = "Attempt to serviceId() while it is NOT set." ;
			throw ommIUExcept().message(temp);
		}
		
		return _rsslMsg.msgKey().serviceId();
	}

	@Override
	public int id()
	{
		if (!hasId())
		{
			String temp = "Attempt to id() while it is NOT set." ;
			throw ommIUExcept().message(temp);
		}
		
		return _rsslMsg.msgKey().identifier();
	}

	@Override
	public long filter()
	{
		if (!hasFilter())
		{
			String temp = "Attempt to filter() while it is NOT set." ;
			throw ommIUExcept().message(temp);
		}

		return _rsslMsg.msgKey().filter();
	}

	@Override
	public ByteBuffer extendedHeader()
	{
		if (!hasExtendedHeader())
		{
			String temp = "Attempt to extendedHeader() while it is NOT set." ;
			throw ommIUExcept().message(temp);
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
		return _serviceNameSet;
	}
	
	public String serviceName()
	{
		if (!_serviceNameSet)
		{
			String temp = "Attempt to serviceName() while it is NOT set." ;
			throw ommIUExcept().message(temp);
		}
		
		return _serviceName;
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
	
	void msgServiceName(String serviceName)
	{
		if (serviceName == null)
			throw ommIUExcept().message("Passed in serviceName is null");
		
		if (hasServiceId())
			throw ommIUExcept().message("Attempt to set serviceName while service id is already set.");
				
		switch (_dataType)
		{
		case DataTypes.REFRESH_MSG:
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.UPDATE_MSG:
			((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.STATUS_MSG:
			((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.REQ_MSG:
			break;
		case DataTypes.GENERIC_MSG:
			((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.POST_MSG:
			((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.ACK_MSG:
			((com.thomsonreuters.upa.codec.AckMsg)_rsslMsg).applyHasMsgKey();
			break;
		default:
			throw ommIUExcept().message("Msg encoding failed.");
		}
		
		_serviceName = serviceName;
		_serviceNameSet = true;
	}

	void service(String serviceName)
	{
		_serviceName = serviceName;
		_serviceNameSet = true;
	}
	
	OmmOutOfRangeExceptionImpl ommOORExcept()
	{
		if (_ommOORExcept == null)
			_ommOORExcept = new OmmOutOfRangeExceptionImpl();
		
		return _ommOORExcept;
	}
	
	OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();
		
		return _ommIUExcept;
	}
	
	OmmUnsupportedDomainTypeExceptionImpl ommUDTExcept()
	{
		if (_ommUDTExcept == null)
			_ommUDTExcept = new OmmUnsupportedDomainTypeExceptionImpl();
		
		return _ommUDTExcept;
	}
	
	StringBuilder errorString()
	{
		if (_errorString == null)
			_errorString = new StringBuilder();
		else
			_errorString.setLength(0);
		
		return _errorString;
	}
	
	//encoding impl
	void msgStreamId(int streamId)
	{
		if (streamId < -2147483648 || streamId > 2147483647)
			throw ommOORExcept().message("Passed in streamId is out of range. [(-2147483648) - 2147483647]");
				
		_rsslMsg.streamId(streamId);
	}

	void msgDomainType(int domainType)
	{
		if (domainType < 0 || domainType > 255)
			throw ommUDTExcept().message(domainType,"Passed in domainType is out of range. [0 - 255]");
		
		_rsslMsg.domainType(domainType);
	}

	void msgName(String name)
	{
		if (name == null)
			throw ommIUExcept().message("Passed in name is null");
		
		switch (_dataType)
		{
		case DataTypes.REFRESH_MSG:
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.UPDATE_MSG:
			((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.STATUS_MSG:
			((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.GENERIC_MSG:
			((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.POST_MSG:
			((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.ACK_MSG:
			((com.thomsonreuters.upa.codec.AckMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.REQ_MSG:
			break;
		default:
			return;
		}
		
		_rsslMsg.msgKey().applyHasName();
		_rsslMsg.msgKey().name().data(name);
	}

	void msgNameType(int nameType)
	{
		if (nameType < 0 || nameType > 255)
			throw ommOORExcept().message("Passed in nameType is out of range. [0 - 255]");
		
		switch (_dataType)
		{
		case DataTypes.REFRESH_MSG:
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.UPDATE_MSG:
			((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.STATUS_MSG:
			((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.GENERIC_MSG:
			((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.POST_MSG:
			((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.ACK_MSG:
			((com.thomsonreuters.upa.codec.AckMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.REQ_MSG:
			break;
		default:
			return;
		}
		
		_rsslMsg.msgKey().applyHasNameType();
		_rsslMsg.msgKey().nameType(nameType);
	}

	void msgServiceId(int serviceId)
	{
		if (serviceId < 0 || serviceId > 65535)
			throw ommOORExcept().message("Passed in serviceId is out of range. [0 - 65535]");
		
		switch (_dataType)
		{
		case DataTypes.REFRESH_MSG:
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.UPDATE_MSG:
			((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.STATUS_MSG:
			((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.GENERIC_MSG:
			((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.POST_MSG:
			((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.ACK_MSG:
			((com.thomsonreuters.upa.codec.AckMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.REQ_MSG:
			break;
		default:
			return;
		}
		
		_rsslMsg.msgKey().applyHasServiceId();
		_rsslMsg.msgKey().serviceId(serviceId);
	}

	void msgId(int id)
	{
		if (id < -2147483648 || id > 2147483647)
			throw ommOORExcept().message("Passed in id is out of range. [(-2147483648) - 2147483647]");
		
		switch (_dataType)
		{
		case DataTypes.REFRESH_MSG:
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.UPDATE_MSG:
			((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.STATUS_MSG:
			((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.GENERIC_MSG:
			((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.POST_MSG:
			((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.ACK_MSG:
			((com.thomsonreuters.upa.codec.AckMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.REQ_MSG:
			break;
		default:
			return;
		}
		
		_rsslMsg.msgKey().applyHasIdentifier();
		_rsslMsg.msgKey().identifier(id);
	}

	void msgFilter(long filter)
	{
		if (filter < 0 || filter > 4294967295L)
			throw ommOORExcept().message("Passed in filter is out of range. [0 - 4294967295]");

		switch (_dataType)
		{
		case DataTypes.REFRESH_MSG:
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.UPDATE_MSG:
			((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.STATUS_MSG:
			((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.GENERIC_MSG:
			((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.POST_MSG:
			((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.ACK_MSG:
			((com.thomsonreuters.upa.codec.AckMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.REQ_MSG:
			break;
		default:
			return;
		}
		
		_rsslMsg.msgKey().applyHasFilter();
		_rsslMsg.msgKey().filter(filter);
	}

   void  msgSeqNum(long seqNum)
   {
	   if (seqNum < 0 || seqNum > 4294967295L)
			throw ommOORExcept().message("Passed in seqNum is out of range. [0 - 4294967295]");
		
	   switch (_dataType)
       {
	       case DataTypes.POST_MSG :
	   		((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).applyHasSeqNum();
	   		((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).seqNum(seqNum);
	          break;
           case DataTypes.REFRESH_MSG :
        		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyHasSeqNum();
        		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).seqNum(seqNum);
               break;
           case DataTypes.GENERIC_MSG :
               ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).applyHasSeqNum();
       			((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).seqNum(seqNum);
               break;
           case DataTypes.UPDATE_MSG :
       		((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).applyHasSeqNum();
       		((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).seqNum(seqNum);
              break;  
           case DataTypes.ACK_MSG :
        		((com.thomsonreuters.upa.codec.AckMsg)_rsslMsg).applyHasSeqNum();
        		((com.thomsonreuters.upa.codec.AckMsg)_rsslMsg).seqNum(seqNum);
               break;
           case DataTypes.STATUS_MSG :
           case DataTypes.REQ_MSG :
           default :
        	   return;
       }
   }
   
   void  msgPartNum(int partNum)
   {
	   if (partNum < 0 || partNum > 32767)
			throw ommOORExcept().message("Passed in partNum is out of range. [0 - 32767]");
	   
	   switch (_dataType)
       {
           case DataTypes.REFRESH_MSG :
        		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyHasPartNum();
        		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).partNum(partNum);
               break;
           case DataTypes.GENERIC_MSG :
               ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).applyHasPartNum();
       			((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).partNum(partNum);
               break;
           case DataTypes.POST_MSG :
        		((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).applyHasPartNum();
        		((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).partNum(partNum);
               break;
           case DataTypes.UPDATE_MSG :
           case DataTypes.STATUS_MSG :
           case DataTypes.ACK_MSG :
           case DataTypes.REQ_MSG :
           default :
               return;
       }
   }
   
   void  msgPublisherId(long userId, long userAddress)
   {
	   if (userId < 0 || userId > 4294967295L || userAddress < 0 || userAddress > 4294967295L)
			throw ommOORExcept().message("Passed in userId or userAddress is out of range. [0 - 4294967295]");
		
	   switch (_dataType)
       {
       	   case DataTypes.POST_MSG :
       		((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).postUserInfo().userId(userId);
    		((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).postUserInfo().userAddr(userAddress);
       		   break;
	       case DataTypes.UPDATE_MSG :
	   		((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).applyHasPostUserInfo();
	   		((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).postUserInfo().userId(userId);
	   		((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).postUserInfo().userAddr(userAddress);
	          break;
           case DataTypes.REFRESH_MSG :
        		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyHasPostUserInfo();
        		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).postUserInfo().userId(userId);
        		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).postUserInfo().userAddr(userAddress);
               break;
           case DataTypes.STATUS_MSG :
               ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyHasPostUserInfo();
       			((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).postUserInfo().userId(userId);
       			((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).postUserInfo().userAddr(userAddress);
               break;
           case DataTypes.GENERIC_MSG :
           case DataTypes.ACK_MSG :
           case DataTypes.REQ_MSG :
           default :
               break;
       }
   }
	
   void  msgPermissionData(ByteBuffer permissionData)
   {
	   if (permissionData == null)
			throw ommIUExcept().message("Passed in permissionData is null");
	   
	   switch (_dataType)
       {
       	   case DataTypes.POST_MSG :
       		((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).applyHasPermData();
       		Utilities.copy(permissionData, ((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).permData());
       		   break;
	       case DataTypes.UPDATE_MSG :
	    		((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).applyHasPermData();
	    		Utilities.copy(permissionData, ((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).permData());
	          break;
           case DataTypes.REFRESH_MSG :
        		((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyHasPermData();
        		Utilities.copy(permissionData, ((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).permData());
               break;
           case DataTypes.STATUS_MSG :
        		((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyHasPermData();
        		Utilities.copy(permissionData, ((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).permData());
               break;
           case DataTypes.GENERIC_MSG :
        		((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).applyHasPermData();
        		Utilities.copy(permissionData, ((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).permData());
        		break;
           case DataTypes.ACK_MSG :
           case DataTypes.REQ_MSG :
           default :
               break;
       }
   }
   
	void msgAttrib(ComplexType attrib)
	{
		if (attrib == null)
			throw ommIUExcept().message("Passed in payload is null");
		
		switch (_dataType)
		{
		case DataTypes.REFRESH_MSG:
			((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.UPDATE_MSG:
			((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.STATUS_MSG:
			((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.GENERIC_MSG:
			((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.POST_MSG:
			((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.ACK_MSG:
			((com.thomsonreuters.upa.codec.AckMsg)_rsslMsg).applyHasMsgKey();
			break;
		case DataTypes.REQ_MSG:
			break;
		default:
			return;
		}
		
		MsgKey msgKey = _rsslMsg.msgKey();
		msgKey.applyHasAttrib();
		msgKey.attribContainerType(Utilities.toRsslDataType(attrib.dataType()));
		Utilities.copy(((DataImpl)attrib).encodedData(),  msgKey.encodedAttrib());
	}

	void msgPayload(ComplexType payload)
	{
		if (payload == null)
			throw ommIUExcept().message("Passed in payload is null");
		
		_rsslMsg.containerType(Utilities.toRsslDataType(payload.dataType()));
		Utilities.copy(((DataImpl)payload).encodedData(),  _rsslMsg.encodedDataBody());
	}

	void msgExtendedHeader(ByteBuffer buffer)
	{
		if (buffer == null)
			throw ommIUExcept().message("Passed in buffer is null");
		
	    switch (_dataType)
        {
            case DataTypes.UPDATE_MSG :
            	((com.thomsonreuters.upa.codec.UpdateMsg)_rsslMsg).applyHasExtendedHdr();
                break;
            case DataTypes.REFRESH_MSG :
            	((com.thomsonreuters.upa.codec.RefreshMsg)_rsslMsg).applyHasExtendedHdr();
                break;
            case DataTypes.STATUS_MSG :
            	((com.thomsonreuters.upa.codec.StatusMsg)_rsslMsg).applyHasExtendedHdr();
                break;
            case DataTypes.GENERIC_MSG :
            	((com.thomsonreuters.upa.codec.GenericMsg)_rsslMsg).applyHasExtendedHdr();
                break;
            case DataTypes.REQ_MSG :
            	((com.thomsonreuters.upa.codec.RequestMsg)_rsslMsg).applyHasExtendedHdr();
                break;
            case DataTypes.POST_MSG :
            	((com.thomsonreuters.upa.codec.PostMsg)_rsslMsg).applyHasExtendedHdr();
                break;
            case DataTypes.ACK_MSG :
            	((com.thomsonreuters.upa.codec.AckMsg)_rsslMsg).applyHasExtendedHdr();
                break;
    		default:
    			return;
        }        
	    
		Utilities.copy(buffer, _rsslMsg.extendedHeader());
	}
	
	boolean msgHasServiceId()
	{
		return (_rsslMsg.msgKey() != null && _rsslMsg.msgKey().checkHasServiceId()) ? true : false;
	}
	
	void msgClear()
	{
		_serviceNameSet = false;
		_serviceName = null;
		
		if (_rsslMsg != null)
		{
			_rsslMsg.clear(); 
			_rsslMsg.msgClass(Utilities.toRsslMsgClass[_dataType]);
			_rsslMsg.domainType(DomainTypes.MARKET_PRICE);
			_rsslMsg.containerType(com.thomsonreuters.upa.codec.DataTypes.NO_DATA);
		}
		
		_encodeComplete = false;
		
		_rsslEncodeIter.clear();
		ByteBuffer data = _rsslBuffer.data();
		if (data != null)
		{
			data.clear();
			_rsslBuffer.data(data);
		}
		else
			_rsslBuffer.clear();
	}
	
	Buffer encodedData() 
	{
		if (_encodeComplete)
			return _rsslBuffer; 
		
		int ret = _rsslEncodeIter.setBufferAndRWFVersion(_rsslBuffer, _rsslMajVer, _rsslMinVer);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to setBufferAndRWFVersion on rssl encode iterator. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText);
	    }
	    
		ret = _rsslMsg.encode(_rsslEncodeIter);
		 while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
		  {
		    _rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
		    _rsslEncodeIter.realignBuffer(_rsslBuffer);
		    ret = _rsslMsg.encode(_rsslEncodeIter);
		  }
		    
		 if (ret != CodecReturnCodes.SUCCESS)
		    {
			 	String errText = errorString().append("Failed to ")
						.append("rsslMsg.encode()")
						.append(" while encoding rsslMsg. Reason='")
						.append(CodecReturnCodes.toString(ret))
						.append("'").toString();
			 	throw ommIUExcept().message(errText);
		    }
		 
		return _rsslBuffer;
	}
}