///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.OmmConsumerConfig;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;

public class OmmConsumerConfigImpl implements OmmConsumerConfig
{
	private LoginRequest		_rsslLoginReq = (LoginRequest) LoginMsgFactory.createMsg();
	private DirectoryRequest	_rsslDirectoryReq; 
	private DictionaryRequest	_rsslFldDictReq;
	private DictionaryRequest	_rsslEnumDictReq;
	private DecodeIterator    	_rsslDecIter;
	private String              _hostnameSetViaFunctionCall;
	private String              _portSetViaFunctionCall;
	private int 				_operationModel;
	
	public OmmConsumerConfigImpl()
	{
		clear();
	}
	
	@Override
	public OmmConsumerConfig clear()
	{
		_operationModel = OperationModel.API_DISPATCH;
		
		_rsslLoginReq.rdmMsgType(LoginMsgType.REQUEST);
		_rsslLoginReq.initDefaultRequest(1);

		return this;
	}

	@Override
	public OmmConsumerConfig username(String username)
	{
		_rsslLoginReq.userName().data(username);
		return this;
	}

	@Override
	public OmmConsumerConfig password(String password)
	{
		_rsslLoginReq.applyHasPassword();
		_rsslLoginReq.password().data(password);
		return this;
	}

	@Override
	public OmmConsumerConfig position(String position)
	{
		_rsslLoginReq.applyHasAttrib();
		_rsslLoginReq.attrib().position().data(position);
		_rsslLoginReq.attrib().applyHasPosition();
		return this;
	}

	@Override
	public OmmConsumerConfig applicationId(String applicationId)
	{
		_rsslLoginReq.applyHasAttrib();
		_rsslLoginReq.attrib().applicationId().data(applicationId);
		_rsslLoginReq.attrib().applyHasApplicationId();
		return this;
	}
	
	OmmConsumerConfig applicationName(String applicationName)
	{
		_rsslLoginReq.applyHasAttrib();
		_rsslLoginReq.attrib().applicationName().data(applicationName);
		_rsslLoginReq.attrib().applyHasApplicationName();
		return this;
	}

	@Override
	public OmmConsumerConfig host(String host)
	{
		 int index = host.indexOf(':');
	    if (index == -1) 
		{
			_portSetViaFunctionCall = OmmConsumerActiveConfig.DEFAULT_SERVICE_NAME;

			if (host.length() > 0)
				_hostnameSetViaFunctionCall = host;
			else
				_hostnameSetViaFunctionCall = OmmConsumerActiveConfig.DEFAULT_HOST_NAME;
		}
	    else if (index == 0)
		{ 
			_hostnameSetViaFunctionCall = OmmConsumerActiveConfig.DEFAULT_HOST_NAME;

			if (host.length() > 1)
				_portSetViaFunctionCall = host.substring(1, host.length());
			else
				_portSetViaFunctionCall = OmmConsumerActiveConfig.DEFAULT_SERVICE_NAME;
		}
		else
		{
			_hostnameSetViaFunctionCall = host.substring(0, index);
			if (host.length() > (index + 1))
				_portSetViaFunctionCall = host.substring(index + 1, host.length());
			else
				_portSetViaFunctionCall = OmmConsumerActiveConfig.DEFAULT_SERVICE_NAME;
		}
		
		return this;
	}

	@Override
	public OmmConsumerConfig operationModel(int operationModel)
	{
		_operationModel = operationModel;
		return this;
	}

	@Override
	public OmmConsumerConfig consumerName(String consumerName)
	{
		// TODO Auto-generated method stub
		return this;
	}

	@Override
	public OmmConsumerConfig config(Data config)
	{
		// TODO Auto-generated method stub
		return this;
	}

	@Override
	public OmmConsumerConfig addAdminMsg(ReqMsg reqMsg)
	{
		RequestMsg rsslRequestMsg = ((ReqMsgImpl)reqMsg).rsslMsg();
		int ret;
		DecodeIterator dIter = rsslDecodeIterator();
		dIter.clear();
		dIter.setBufferAndRWFVersion(rsslRequestMsg.encodedMsgBuffer(), Codec.majorVersion(), Codec.minorVersion());
		
		switch (rsslRequestMsg.domainType())
		{
			case com.thomsonreuters.upa.rdm.DomainTypes.LOGIN :
				ret = _rsslLoginReq.decode(dIter, rsslRequestMsg);
				break;
			case com.thomsonreuters.upa.rdm.DomainTypes.DICTIONARY :
				ret = addDictionaryReqMsg(dIter, rsslRequestMsg);
				break;
			case com.thomsonreuters.upa.rdm.DomainTypes.SOURCE :
				if (_rsslDirectoryReq == null)
					_rsslDirectoryReq = (DirectoryRequest)DirectoryMsgFactory.createMsg();
				
				ret = _rsslDirectoryReq.decode(dIter, rsslRequestMsg);
				break;
			default :
				ret = CodecReturnCodes.FAILURE;
				//TODO logging error
				break;
		}
		
		if (CodecReturnCodes.SUCCESS != ret)
	    {
			//TODO logging error
	    }
		
		return this;
	}
	
	int addDictionaryReqMsg(DecodeIterator dIter, RequestMsg rsslRequestMsg)
	{
		//TODO check msg key attributes

		String rdmFldDictName = "RWFFld";
		String rdmEnumDictName = "RWFEnum";
		String dictName = rsslRequestMsg.msgKey().name().toString();
		int ret;
		
		if (dictName.equals(rdmFldDictName))
		{
			 if (_rsslFldDictReq == null)
				 _rsslFldDictReq = (DictionaryRequest)DictionaryMsgFactory.createMsg();
			 if ((ret = _rsslFldDictReq.decode(dIter, rsslRequestMsg)) < 0)
			 {
				//TODO add logger
				return CodecReturnCodes.FAILURE;
			 }
		}
		else if (dictName.equals(rdmEnumDictName))
		{
			 if (_rsslEnumDictReq == null)
				 _rsslEnumDictReq = (DictionaryRequest)DictionaryMsgFactory.createMsg();
			 if ((ret = _rsslEnumDictReq.decode(dIter, rsslRequestMsg)) < 0)
			 {
				//TODO add logger
				return CodecReturnCodes.FAILURE;
			 }
		}
		else
		{
			//TODO add logger
			return CodecReturnCodes.FAILURE;
		}
		
		return CodecReturnCodes.SUCCESS;
	}
	
	DecodeIterator rsslDecodeIterator()
	{
		if (_rsslDecIter == null)
			_rsslDecIter = CodecFactory.createDecodeIterator();
		
		return _rsslDecIter;
	}
	
	LoginRequest loginReq()
	{
		return _rsslLoginReq;
	}
	
	DirectoryRequest directoryReq()
	{
		return _rsslDirectoryReq;
	}
	
	DictionaryRequest rdmFldDictionaryReq()
	{
		return _rsslFldDictReq;
	}
	
	DictionaryRequest enumDefDictionaryReq()
	{
		return _rsslEnumDictReq;
	}
	
	String consumerName()
	{
		//TODO
		return "EmaConsumer1";
	}
	
	int operationModel()
	{
		return _operationModel;
	}
	
	String userSpecifiedHostname() 
	{
		return _hostnameSetViaFunctionCall;
	}
	
	String userSpecifiedPort()
	{
		return _portSetViaFunctionCall;
	}
	
	String channelName(String consumerName)
	{
		//TODO
		return "ChannelName";
	}
	
	String dictionaryName(String consumerName)
	{
		//TODO
		return "DictionaryName";
	}
}