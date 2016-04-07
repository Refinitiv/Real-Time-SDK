///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import com.thomsonreuters.ema.access.ConfigReader.XMLConfigReader;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginAttrib;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequestFlags;

class OmmConsumerConfigImpl implements OmmConsumerConfig
{
	private LoginRequest		_rsslLoginReq = (LoginRequest) LoginMsgFactory.createMsg();
	private DirectoryRequest	_rsslDirectoryReq; 
	private DictionaryRequest	_rsslFidDictReq;
	private DictionaryRequest	_rsslEnumDictReq;
	private DecodeIterator    	_rsslDecIter;
	private String              _hostnameSetViaFunctionCall;
	private String              _portSetViaFunctionCall;
	private int 				_operationModel;
	private StringBuilder _configStrBuilder;
	private String	_fidDictReqServiceName;
	private String	_enumDictReqServiceName;
	private boolean _fidDictReqServiceIdSet;
	private boolean _enumDictReqServiceIdSet;
	
	private ConfigErrorTracker _errorTracker;
	private XMLConfigReader	_xmlConfig;
	
	private OmmInvalidConfigurationExceptionImpl _oommICExcept;

	OmmConsumerConfigImpl()
	{
		clear();
		_errorTracker = new ConfigErrorTracker();
		readConfiguration();
	}
	
	public ConfigErrorTracker errorTracker() 
	{
		return _errorTracker;
	}
	
	XMLConfigReader xmlConfig()
	{
		return _xmlConfig;
	}
	
	void readConfiguration() 
	{
		_xmlConfig = (XMLConfigReader) ConfigReader.createXMLConfigReader(this);
		_xmlConfig.loadFile();
	}

	@Override
	public OmmConsumerConfig clear()
	{
		_operationModel = OperationModel.API_DISPATCH;
	
		try
		{
			_rsslLoginReq.rdmMsgType(LoginMsgType.REQUEST);
		}
		catch(Exception e) 
		{
			errorTracker().append("Failed to create login request, received exception: '")
			.append(e.getLocalizedMessage())
			.append( "'. ")
			.create(Severity.ERROR);
		}
		_rsslLoginReq.initDefaultRequest(1);
		
		_rsslLoginReq.rdmMsgType(LoginMsgType.REQUEST);
		_rsslLoginReq.initDefaultRequest(1);

		if ( _rsslDirectoryReq != null)
			_rsslDirectoryReq.clear();
		
		if ( _rsslFidDictReq != null)
		{
			_rsslFidDictReq.clear();
			_fidDictReqServiceIdSet = false;
			_fidDictReqServiceName = null;
		}
		if (_rsslEnumDictReq != null)
		{
			_rsslEnumDictReq.clear();
			_enumDictReqServiceIdSet = false;
			_enumDictReqServiceName = null;
		}
		
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
/*		if ( _pProgrammaticConfigure != null && _pProgrammaticConfigure.specifyConsumerName( consumerName ) == true )
		{ 
			return this;
		}*/

		String name = (String) _xmlConfig.getConsumerAttributeValue(consumerName,ConfigManager.ConsumerName);

		if ( name == null ) 
		{
			if ( consumerName.equals("EmaConsumer") )
			{
				boolean bFoundChild = xmlConfig().isConsumerChildAvailable();
				if( bFoundChild == false )
					return this;
			}

			configStrBuilder().append( "OmmConsumerConfigImpl::consumerName parameter [" )
									.append( consumerName )
									.append( "] is an non-existent consumer name" );
			oommICExcept().message(  _configStrBuilder.toString());
		}
 		else //if ( name != null ) 
		{
 			boolean bSetAttributeValue = _xmlConfig.setDefaultConsumer(name);
			if ( bSetAttributeValue == false )
			{
				xmlConfig().appendAttributeValue(ConfigManager.CONSUMER_GROUP, ConfigManager.DefaultConsumer,consumerName);
				xmlConfig().verifyAndGetDefaultConsumer();
			}
		}
	
		return this;
	}
	
	@Override
	public OmmConsumerConfig config(Data config)
	{
		if(config.dataType() == DataType.DataTypes.MAP)
		{
			/*if ( _pProgrammaticConfigure == null )
			{
				//_pProgrammaticConfigure = new ProgrammaticConfigure( (Map) config, _pEmaConfig.errors() );
				_pProgrammaticConfigure = new ProgrammaticConfigure( (Map) config, null );
			}
			else
			{
				_pProgrammaticConfigure.addConfigure( (Map) config  );
			}*/
		}
		else
		{
			errorTracker().append( "Invalid Data type='" )
						.append(DataType.asString(config.dataType()))
						.append("' for Programmatic Configure.")
						.create(Severity.ERROR);
		}
		return this;	
	}

	@Override
	public OmmConsumerConfig addAdminMsg(ReqMsg reqMsg)
	{
		RequestMsg rsslRequestMsg = ((ReqMsgImpl)reqMsg).rsslMsg();
		int ret = CodecReturnCodes.SUCCESS;
		switch (rsslRequestMsg.domainType())
		{
			case com.thomsonreuters.upa.rdm.DomainTypes.LOGIN :
				ret = setLoginRequest(rsslRequestMsg);
				break;
			case com.thomsonreuters.upa.rdm.DomainTypes.DICTIONARY :
				addDictionaryReqMsg(rsslRequestMsg, (reqMsg.hasServiceName() ? reqMsg.serviceName() : null));
				break;
			case com.thomsonreuters.upa.rdm.DomainTypes.SOURCE :
				if (_rsslDirectoryReq == null)
				{
					_rsslDirectoryReq = (DirectoryRequest)DirectoryMsgFactory.createMsg();
					_rsslDirectoryReq.rdmMsgType(DirectoryMsgType.REQUEST);
				}
				ret = setDirectoryRequest(rsslRequestMsg);
				break;
			default :
				errorTracker().append("Request message with unhandled domain passed into addAdminMsg((ReqMsg reqMsg). Domain type='")
				.append(rsslRequestMsg.domainType())
				.append( "'. ")
				.create(Severity.ERROR);
				return this;
		}
		
		if (CodecReturnCodes.SUCCESS != ret)
	    {
			errorTracker().append("Failed to convert rssl msg to RDM msg in addAdminMsg(ReqMsg reqMsg). Domain type='")
			.append(rsslRequestMsg.domainType())
			.append( "'. ")
			.create(Severity.ERROR);
	    }
		
		return this;
	}
	
	int setLoginRequest(RequestMsg rsslRequestMsg)
	{
		_rsslLoginReq.clear();
		_rsslLoginReq.rdmMsgType(LoginMsgType.REQUEST);
		int flags = _rsslLoginReq.flags();
		   
		if(!rsslRequestMsg.checkStreaming())
			return CodecReturnCodes.FAILURE;
	        
		if(rsslRequestMsg.checkNoRefresh())
		   flags |= LoginRequestFlags.NO_REFRESH;
		
		if(rsslRequestMsg.checkPause())
		  flags |= LoginRequestFlags.PAUSE_ALL;
    
	   _rsslLoginReq.streamId(rsslRequestMsg.streamId());
    
	   MsgKey msgKey = rsslRequestMsg.msgKey();
	   if (msgKey == null || !msgKey.checkHasName() || (msgKey.checkHasAttrib() && msgKey.attribContainerType() != DataTypes.ELEMENT_LIST))
         return CodecReturnCodes.FAILURE;

	   Buffer userName = msgKey.name();
	   _rsslLoginReq.userName().data(userName.data(), userName.position(), userName.length());
	        
	   if ( msgKey.checkHasNameType())
	   {
		  _rsslLoginReq.applyHasUserNameType();
		  _rsslLoginReq.userNameType(rsslRequestMsg.msgKey().nameType());
	   }
		
	  if (msgKey.checkHasAttrib() )
	  {
			DecodeIterator dIter = rsslDecodeIterator();
			com.thomsonreuters.upa.codec.ElementList elementList = CodecFactory.createElementList();
		    com.thomsonreuters.upa.codec.ElementEntry elementEntry = CodecFactory.createElementEntry();	
		    com.thomsonreuters.upa.codec.UInt  tmpUInt = CodecFactory.createUInt();
		    
		    dIter.setBufferAndRWFVersion(msgKey.encodedAttrib(), Codec.majorVersion(), Codec.minorVersion());
	        elementList.clear();

	        int ret = elementList.decode(dIter, null);
	        if (ret != CodecReturnCodes.SUCCESS)
	            return ret;

	        elementEntry.clear();
	        LoginAttrib attrib = _rsslLoginReq.attrib();
	        
	        while ((ret = elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
	        {
	            if (ret != CodecReturnCodes.SUCCESS)
	                return ret;

	            if (elementEntry.name().equals(ElementNames.ALLOW_SUSPECT_DATA))
	            {
	                if (elementEntry.dataType() != DataTypes.UINT)
	                    return CodecReturnCodes.FAILURE;

	                ret = tmpUInt.decode(dIter);
	                if (ret != CodecReturnCodes.SUCCESS)
	                    return ret;
	                flags |= LoginRequestFlags.HAS_ATTRIB;
	                attrib.applyHasAllowSuspectData();
	                attrib.allowSuspectData(tmpUInt.toLong());
	            }
	            else if (elementEntry.name().equals(ElementNames.APPID))
	            {
	                if (elementEntry.dataType() != DataTypes.ASCII_STRING)
	                    return CodecReturnCodes.FAILURE;
	                flags |= LoginRequestFlags.HAS_ATTRIB;
	                Buffer applicationId = elementEntry.encodedData();
	                attrib.applyHasApplicationId();
	                attrib.applicationId().data(applicationId.data(), applicationId.position(), applicationId.length());
	            }
	            else if (elementEntry.name().equals(ElementNames.APPNAME))
	            {
	                if (elementEntry.dataType() != DataTypes.ASCII_STRING)
	                    return CodecReturnCodes.FAILURE;
	                flags |= LoginRequestFlags.HAS_ATTRIB;
	                Buffer applicationName = elementEntry.encodedData();
	                attrib.applyHasApplicationName();
	                attrib.applicationName().data(applicationName.data(), applicationName.position(), applicationName.length());
	            }
	            else if (elementEntry.name().equals(ElementNames.POSITION))
	            {
	                if (elementEntry.dataType() != DataTypes.ASCII_STRING)
	                    return CodecReturnCodes.FAILURE;
	                
	                flags |= LoginRequestFlags.HAS_ATTRIB;
	                Buffer position = elementEntry.encodedData();
	                attrib.applyHasPosition();
	                attrib.position().data(position.data(), position.position(), position.length());
	            }
	            else if (elementEntry.name().equals(ElementNames.PASSWORD))
	            {
	                if (elementEntry.dataType() != DataTypes.ASCII_STRING)
	                    return CodecReturnCodes.FAILURE;
	                Buffer password = elementEntry.encodedData();
	                flags |= LoginRequestFlags.HAS_ATTRIB;
	                flags |= LoginRequestFlags.HAS_PASSWORD;
	                _rsslLoginReq.password().data(password.data(), password.position(), password.length());
	            }
	            else if (elementEntry.name().equals(ElementNames.INST_ID))
	            {
	                if (elementEntry.dataType() != DataTypes.ASCII_STRING)
	                    return CodecReturnCodes.FAILURE;
	                Buffer instanceId = elementEntry.encodedData();
	                flags |= LoginRequestFlags.HAS_ATTRIB;
	                _rsslLoginReq.instanceId().data(instanceId.data(), instanceId.position(), instanceId.length());
	            }
	            else if (elementEntry.name().equals(ElementNames.DOWNLOAD_CON_CONFIG))
	            {
	                if (elementEntry.dataType() != DataTypes.UINT)
	                    return CodecReturnCodes.FAILURE;
	                ret = tmpUInt.decode(dIter);
	                if (ret != CodecReturnCodes.SUCCESS)
	                    return ret;
	                
	                flags |= LoginRequestFlags.HAS_DOWNLOAD_CONN_CONFIG;
	                _rsslLoginReq.downloadConnectionConfig(tmpUInt.toLong());
	            }
	            else if (elementEntry.name().equals(ElementNames.PROV_PERM_EXP))
	            {
	                if (elementEntry.dataType() != DataTypes.UINT)
	                    return CodecReturnCodes.FAILURE;
	                ret = tmpUInt.decode(dIter);
	                if (ret != CodecReturnCodes.SUCCESS)
	                    return ret;
	                flags |= LoginRequestFlags.HAS_ATTRIB;
	                attrib.applyHasProvidePermissionExpressions();
	                attrib.providePermissionExpressions(tmpUInt.toLong());
	            }
	            else if (elementEntry.name().equals(ElementNames.PROV_PERM_PROF))
	            {
	                if (elementEntry.dataType() != DataTypes.UINT)
	                    return CodecReturnCodes.FAILURE;
	                ret = tmpUInt.decode(dIter);
	                if (ret != CodecReturnCodes.SUCCESS)
	                    return ret;
	                flags |= LoginRequestFlags.HAS_ATTRIB;
	                attrib.applyHasProvidePermissionProfile();
	                attrib.providePermissionProfile(tmpUInt.toLong());
	            }
	            else if (elementEntry.name().equals(ElementNames.SINGLE_OPEN))
	            {
	                if (elementEntry.dataType() != DataTypes.UINT)
	                    return CodecReturnCodes.FAILURE;
	                ret = tmpUInt.decode(dIter);
	                if (ret != CodecReturnCodes.SUCCESS)
	                    return ret;

	                flags |= LoginRequestFlags.HAS_ATTRIB;
	                attrib.applyHasSingleOpen();
	                attrib.singleOpen(tmpUInt.toLong());
	            }
	            else if (elementEntry.name().equals(ElementNames.ROLE))
	            {
	                if (elementEntry.dataType() != DataTypes.UINT)
	                    return CodecReturnCodes.FAILURE;
	                ret = tmpUInt.decode(dIter);
	                if (ret != CodecReturnCodes.SUCCESS)
	                    return ret;
	                
	                flags |= LoginRequestFlags.HAS_ROLE;
	                _rsslLoginReq.role(tmpUInt.toLong());
	            }
	            else if (elementEntry.name().equals(ElementNames.SUPPORT_PROVIDER_DICTIONARY_DOWNLOAD))
	            {
	                if (elementEntry.dataType() != DataTypes.UINT)
	                    return CodecReturnCodes.FAILURE;
	                ret = tmpUInt.decode(dIter);
	                if (ret != CodecReturnCodes.SUCCESS)
	                    return ret;
	                
	                flags |= LoginRequestFlags.HAS_ATTRIB;
	                attrib.applyHasProviderSupportDictionaryDownload();
	                attrib.supportProviderDictionaryDownload(tmpUInt.toLong());
	            }
        	}
		}
	     
	  _rsslLoginReq.flags(flags);
		
	  return CodecReturnCodes.SUCCESS;
	}
	
	int setDirectoryRequest(RequestMsg rsslRequestMsg)
	{
		_rsslDirectoryReq.clear();
		_rsslDirectoryReq.rdmMsgType(DirectoryMsgType.REQUEST);
		   
        if (rsslRequestMsg.msgClass() != MsgClasses.REQUEST)
            return CodecReturnCodes.FAILURE;


        if (rsslRequestMsg.checkStreaming())
        	_rsslDirectoryReq.applyStreaming();

        MsgKey msgKey = rsslRequestMsg.msgKey();
        if (msgKey == null || !msgKey.checkHasFilter())
            return CodecReturnCodes.FAILURE;

        if (msgKey.checkHasFilter())
        	_rsslDirectoryReq.filter(rsslRequestMsg.msgKey().filter());
        
        _rsslDirectoryReq.streamId(rsslRequestMsg.streamId());
        
        if (msgKey.checkHasServiceId())
        {
        	_rsslDirectoryReq.applyHasServiceId();
        	_rsslDirectoryReq.serviceId(msgKey.serviceId());
        }
	        
	    return CodecReturnCodes.SUCCESS;
	}
	
	int decodeDictionaryReqMsg(DictionaryRequest rdmDictionaryRequest, RequestMsg rsslRequestMsg)
	{
		rdmDictionaryRequest.clear();
        if (rsslRequestMsg.msgClass() != MsgClasses.REQUEST)
            return CodecReturnCodes.FAILURE;
        
        rdmDictionaryRequest.streamId(rsslRequestMsg.streamId());
        
        if (rsslRequestMsg.checkStreaming())
        	rsslRequestMsg.applyStreaming();
        
        MsgKey msgKey = rsslRequestMsg.msgKey();
        if (msgKey == null)
            return CodecReturnCodes.FAILURE;
        if (!msgKey.checkHasFilter())
            return CodecReturnCodes.FAILURE;
        if (msgKey.checkHasServiceId())
        	rdmDictionaryRequest.serviceId(msgKey.serviceId());
        if (!msgKey.checkHasName())
            return CodecReturnCodes.FAILURE;

        Buffer name = msgKey.name();
        rdmDictionaryRequest.dictionaryName().data(name.data(), name.position(), name.length());

        rdmDictionaryRequest.verbosity((int)msgKey.filter());
        
        return CodecReturnCodes.SUCCESS;
	}
	
	int addDictionaryReqMsg(RequestMsg rsslRequestMsg, String serviceName)
	{
		if (!rsslRequestMsg.msgKey().checkHasName())
		{
			String temp = "Received dicionary request message contains no dictionary name. Message ignored.";
			errorTracker().append(temp).create(Severity.WARNING);
			return CodecReturnCodes.FAILURE;
		}
		if ( !rsslRequestMsg.msgKey().checkHasServiceId() )
		{
			if ( serviceName == null)
			{
				String temp = "Received dicionary request message contains no serviceId or service name. Message ignored.";
				errorTracker().append(temp).create(Severity.WARNING);
				return CodecReturnCodes.FAILURE;
			}
		}
		else if (!rsslRequestMsg.msgKey().checkHasFilter() )
		{
			String temp = "Received dicionary request message contains no filter. Message ignored.";
			errorTracker().append(temp).create(Severity.WARNING);
			return CodecReturnCodes.FAILURE;
		}
		else if (rsslRequestMsg.checkNoRefresh())
		{
			String temp = "Received dicionary request message contains no_refresh flag. Message ignored.";
			errorTracker().append(temp).create(Severity.WARNING);
			return CodecReturnCodes.FAILURE;
		}

		String dictName = rsslRequestMsg.msgKey().name().toString();
		int ret = CodecReturnCodes.SUCCESS;
		
		if (dictName.equals(DictionaryCallbackClient.DICTIONARY_RWFFID))
		{
			 if (_rsslFidDictReq == null)
			 {
				 _rsslFidDictReq = (DictionaryRequest)DictionaryMsgFactory.createMsg();
				 _rsslFidDictReq.rdmMsgType(DictionaryMsgType.REQUEST);
			 }
			 if ((ret =decodeDictionaryReqMsg(_rsslFidDictReq, rsslRequestMsg)) < 0)
			 {
				 _rsslFidDictReq = null;
				 String temp = "Failed to convert rssl dictionary msg to RDM msg. Message ignored.";
				 errorTracker().append(temp).create(Severity.WARNING);
					
				return CodecReturnCodes.FAILURE;
			 }
			 
			 if (serviceName != null)
				 _fidDictReqServiceName = serviceName;
			 else
				_fidDictReqServiceIdSet =  true;
		}
		else if (dictName.equals(DictionaryCallbackClient.DICTIONARY_RWFENUM))
		{
			 if (_rsslEnumDictReq == null)
			 {
				 _rsslEnumDictReq = (DictionaryRequest)DictionaryMsgFactory.createMsg();
				 _rsslEnumDictReq.rdmMsgType(DictionaryMsgType.REQUEST);
			 }
			 if ((ret = decodeDictionaryReqMsg(_rsslEnumDictReq, rsslRequestMsg)) < 0)
			 {
				 _rsslEnumDictReq = null;
				 String temp = "Failed to convert rssl dictionary msg to RDM msg. Message ignored.";
				 errorTracker().append(temp).create(Severity.WARNING);
					
				return CodecReturnCodes.FAILURE;
			 }
			 
			 if (serviceName != null)
				 _enumDictReqServiceName = serviceName;
			 else
				_enumDictReqServiceIdSet =  true;
		}
		else
		{
			 String temp = "Received dicionary request message contains unrecognized dictionary name. Message ignored.";
			 errorTracker().append(temp).create(Severity.WARNING);
			return CodecReturnCodes.FAILURE;
		}
		
		return CodecReturnCodes.SUCCESS;
	}
	
	boolean validateDictRequest()
	{
		if (_rsslFidDictReq != null && _rsslEnumDictReq != null)
		{
			if ((_fidDictReqServiceIdSet && _enumDictReqServiceIdSet && _rsslFidDictReq.serviceId() != _rsslEnumDictReq.serviceId()) || 
					(_fidDictReqServiceName != null && !_fidDictReqServiceName.isEmpty() && 
					_enumDictReqServiceName != null && !_enumDictReqServiceName.isEmpty() &&
					!_fidDictReqServiceName.equals(_enumDictReqServiceName) ))
			{
				 _rsslFidDictReq = null;
				 _rsslEnumDictReq = null;
				 return false;
			}
		}
		
		return true;
	}
	
	DecodeIterator rsslDecodeIterator()
	{
		if (_rsslDecIter == null)
			_rsslDecIter = CodecFactory.createDecodeIterator();
		else 
			_rsslDecIter.clear();
		
		return _rsslDecIter;
	}
	
	StringBuilder configStrBuilder()
	{
		if (_configStrBuilder == null)
			_configStrBuilder = new StringBuilder();
		else
			_configStrBuilder.setLength(0);
		
		return _configStrBuilder;
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
		return _rsslFidDictReq;
	}
	
	DictionaryRequest enumDefDictionaryReq()
	{
		return _rsslEnumDictReq;
	}
	
	String fidDictReqServiceName()
	{
		return _fidDictReqServiceName;
	}
	
	String enumDictReqServiceName()
	{
		return _enumDictReqServiceName;
	}
	
	String consumerName()
	{
		String defaultConsumerName = null;

		/*if ( _pProgrammaticConfigure != null )
		{
			defaultConsumerName = _pProgrammaticConfigure.getDefaultConsumer();
 
			if( defaultConsumerName != null )
				return defaultConsumerName;	
		}*/
				
		defaultConsumerName = _xmlConfig.defaultConsumerName();

		// check if default consumer Name and the consumer name matched
		if ( defaultConsumerName != null )
		{
			String checkValue = (String) _xmlConfig.getConsumerAttributeValue(defaultConsumerName,ConfigManager.ConsumerName);

			if ( checkValue != null )
	            return defaultConsumerName;
			else
			{		
				errorTracker().append( "default consumer name [" )
							.append( defaultConsumerName )
							.append( "] is an non-existent consumer name; DefaultConsumer specification ignored" )
							.create(Severity.ERROR);
				return null;
			}
		}
		
		String firstConsumerName = (String) _xmlConfig.getFirstConsumer();
		if( firstConsumerName != null )
			return firstConsumerName;
		
	    return "EmaConsumer";	
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
		String channelName = null;

		/*if ( _pProgrammaticConfigure && _pProgrammaticConfigure->getActiveChannelName(consumerName, retVal ) )
		{
			return retVal;
		}*/

		channelName = (String) _xmlConfig.getConsumerAttributeValue(consumerName,ConfigManager.ConsumerChannelName);
		return channelName;
		//return "ChannelName";
	}
	
	String dictionaryName(String consumerName)
	{
		String dictionaryName = null;

		/*if ( _pProgrammaticConfigure && _pProgrammaticConfigure->getActiveDictionaryName(consumerName, retVal ) )
		{
			return retVal;
		}*/

		dictionaryName = (String) _xmlConfig.getConsumerAttributeValue(consumerName,ConfigManager.ConsumerDictionaryName);
		return dictionaryName;
	}
	
	OmmInvalidConfigurationExceptionImpl oommICExcept()
	{
		if (_oommICExcept == null)
			_oommICExcept = new OmmInvalidConfigurationExceptionImpl();
		
		return _oommICExcept;
	}
	
	public String getUserSpecifiedHostname() 
	{
		return _hostnameSetViaFunctionCall;
	}

	public String getUserSpecifiedPort() 
	{
		return _portSetViaFunctionCall; 
	}
}