///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.ArrayList;
import java.util.List;

import com.thomsonreuters.ema.access.ConfigReader.XMLConfigReader;

import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.ema.access.ProgrammaticConfigure.InstanceEntryFlag;
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
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginAttrib;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequestFlags;

abstract class  EmaConfigBaseImpl
{	
	protected ConfigErrorTracker 				_errorTracker;
	protected XMLConfigReader					_xmlConfig;
	protected ProgrammaticConfigure				_programmaticConfigure;
	
	private OmmInvalidConfigurationExceptionImpl _oommICExcept;
	private OmmInvalidUsageExceptionImpl		_ommIUExcept;
	
	protected StringBuilder 						_configStrBuilder;
	protected String							 _configSessionName = null;
	
	EmaConfigBaseImpl()
	{
		_errorTracker = new ConfigErrorTracker();
	}
	
	ConfigErrorTracker errorTracker()
	{
		return _errorTracker;
	}
	
	XMLConfigReader xmlConfig()
	{
		return _xmlConfig;
	}
	
	protected OmmInvalidConfigurationExceptionImpl oommICExcept()
	{
		if (_oommICExcept == null)
			_oommICExcept = new OmmInvalidConfigurationExceptionImpl();
		
		return _oommICExcept;
	}
	
	protected OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();
		
		return _ommIUExcept;
	}
	
	
	protected StringBuilder configStrBuilder()
	{
		if (_configStrBuilder == null)
			_configStrBuilder = new StringBuilder();
		else
			_configStrBuilder.setLength(0);
		
		return _configStrBuilder;
	}
	
	ProgrammaticConfigure programmaticConfigure()
	{
		return _programmaticConfigure;
	}
	
	String directoryName(String instanceName)
	{
		return null;
	}
	
	abstract void configInt(Data config);
	abstract String configuredName();
	abstract int operationModel();
	abstract void readConfiguration(String path);
}

abstract class EmaConfigImpl extends EmaConfigBaseImpl
{
	private LoginRequest						_rsslLoginReq = (LoginRequest) LoginMsgFactory.createMsg();
	private DirectoryRequest					_rsslDirectoryReq; 
	private DictionaryRequest					_rsslFidDictReq;
	private DictionaryRequest					_rsslEnumDictReq;
	
	private DirectoryRefresh 					_rsslDirectoryRefresh;
	
	private DecodeIterator    					_rsslDecIter;
	private String              				_hostnameSetViaFunctionCall;
	private String              				_portSetViaFunctionCall;
	private String								_fidDictReqServiceName;
	private String								_enumDictReqServiceName;
	private boolean 							_fidDictReqServiceIdSet;
	private boolean 							_enumDictReqServiceIdSet;
    protected List<Integer> channelOrChannelSet = new ArrayList<Integer>();
    
    private static String 						_defaultAppName = "ema";
    private HttpChannelConfig 				    _tunnelingChannelCfg;
    
    private Buffer								_clientId = CodecFactory.createBuffer();
    private Buffer								_tokenServiceUrl = CodecFactory.createBuffer();
    private Buffer								_serviceDiscoveryUrl = CodecFactory.createBuffer();

	EmaConfigImpl()
	{
		super();
		
		clearInt();

		readConfiguration(null);
	}
	
	EmaConfigImpl(String path)
	{
		super();

		clearInt();
		readConfiguration(path);
	}

	void readConfiguration(String path)
	{
		_xmlConfig = (XMLConfigReader) ConfigReader.createXMLConfigReader(this);
		_xmlConfig.loadFile(path);
	}
	
	protected void clearInt()
	{
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
		
		_rsslLoginReq.clear();
		_rsslLoginReq.rdmMsgType(LoginMsgType.REQUEST);
		_rsslLoginReq.initDefaultRequest(1);
		
		_rsslLoginReq.attrib().applicationName().data(_defaultAppName);

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
		
		if (_programmaticConfigure != null)
			_programmaticConfigure.clear();
	}
	
	protected void usernameInt(String username)
	{
		if(username == null || username.length() == 0)
		{
			configStrBuilder().append("EmaConfigImpl:UserName input String cannot be blank.");
			throw ( ommIUExcept().message( _configStrBuilder.toString(), OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT));
		}
			
		_rsslLoginReq.userName().data(username);
	}
	
	protected void passwordInt(String password)
	{
		_rsslLoginReq.applyHasPassword();
		_rsslLoginReq.password().data(password);
	}

	protected void positionInt(String position)
	{
		_rsslLoginReq.applyHasAttrib();
		_rsslLoginReq.attrib().position().data(position);
		_rsslLoginReq.attrib().applyHasPosition();
	}

	protected void applicationIdInt(String applicationId)
	{
		_rsslLoginReq.applyHasAttrib();
		_rsslLoginReq.attrib().applicationId().data(applicationId);
		_rsslLoginReq.attrib().applyHasApplicationId();
	}
	
	protected void clientIdInt(String clientId)
	{
		_clientId.data(clientId);
	}
	
	protected void tokenServiceUrlInt(String tokenServiceUrl)
	{
		_tokenServiceUrl.data(tokenServiceUrl);
	}
	
	protected void serviceDiscoveryUrlInt(String serviceDiscoveryUrl)
	{
		_serviceDiscoveryUrl.data(serviceDiscoveryUrl);
	}
	
	protected void applicationNameInt(String applicationName)
	{
		_rsslLoginReq.applyHasAttrib();
		_rsslLoginReq.attrib().applicationName().data(applicationName);
		_rsslLoginReq.attrib().applyHasApplicationName();
	}
	
	protected void instanceIdInt(String instanceId )
	{
		_rsslLoginReq.applyHasInstanceId();
		_rsslLoginReq.instanceId().data(instanceId);
	}
	
	protected void hostInt(String host, String defaultService)
	{
		int index = host.indexOf(':');
	    if (index == -1) 
		{
			_portSetViaFunctionCall = defaultService;

			if (host.length() > 0)
				_hostnameSetViaFunctionCall = host;
			else
				_hostnameSetViaFunctionCall = ActiveConfig.DEFAULT_HOST_NAME;
		}
	    else if (index == 0)
		{ 
			_hostnameSetViaFunctionCall = ActiveConfig.DEFAULT_HOST_NAME;

			if (host.length() > 1)
				_portSetViaFunctionCall = host.substring(1, host.length());
			else
				_portSetViaFunctionCall =defaultService;
		}
		else
		{
			_hostnameSetViaFunctionCall = host.substring(0, index);
			if (host.length() > (index + 1))
				_portSetViaFunctionCall = host.substring(index + 1, host.length());
			else
				_portSetViaFunctionCall = defaultService;
		}
	}
	
	protected void addAdminMsgInt(ReqMsg reqMsg)
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
		}
		
		if (CodecReturnCodes.SUCCESS != ret)
	    {
			errorTracker().append("Failed to convert rssl msg to RDM msg in addAdminMsg(ReqMsg reqMsg). Domain type='")
			.append(rsslRequestMsg.domainType())
			.append( "'. ")
			.create(Severity.ERROR);
	    }
	}
		
	protected void addAdminMsgInt(RefreshMsg refreshMsg)
	{	
		com.thomsonreuters.upa.codec.RefreshMsg rsslRefreshMsg = ((RefreshMsgImpl)refreshMsg).rsslMsg();
		
		switch( rsslRefreshMsg.domainType() )
		{
			case com.thomsonreuters.upa.rdm.DomainTypes.LOGIN :
				break;
			case com.thomsonreuters.upa.rdm.DomainTypes.DICTIONARY :
				break;
			case com.thomsonreuters.upa.rdm.DomainTypes.SOURCE :
				if ( rsslRefreshMsg.streamId() > 0 )
				{
					errorTracker().append("Refresh passed into addAdminMsg(RefreshMsg refreshMsg) contains unhandled stream id. StreamId='")
					.append(rsslRefreshMsg.streamId())
					.append( "'. ")
					.create(Severity.ERROR);
				}
				else
				{
					setDirectoryRefresh( rsslRefreshMsg );
				}
				break;
			default:
				errorTracker().append("Refresh message passed into addAdminMsg(RefreshMsg refreshMsg) contains unhandled domain type. Domain type='")
				.append(rsslRefreshMsg.domainType())
				.append( "'. ")
				.create(Severity.ERROR);
				break;
		}
	}
	
	abstract String channelName(String instanceName);
	

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
	      flags |= LoginRequestFlags.HAS_USERNAME_TYPE;
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
	                flags |= LoginRequestFlags.HAS_INSTANCE_ID;
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
	                _rsslLoginReq.applyHasRole();
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
	            else if (elementEntry.name().equals(ElementNames.AUTHN_TOKEN))
	            {
	                if (elementEntry.dataType() != DataTypes.BUFFER && elementEntry.dataType() != DataTypes.ASCII_STRING )
	                    return CodecReturnCodes.FAILURE;
	                
	                _rsslLoginReq.userName().data(elementEntry.encodedData().data(), elementEntry.encodedData().position(), elementEntry.encodedData().length());
	            }
	            else if (elementEntry.name().equals(ElementNames.AUTHN_EXTENDED))
	            {
	                if (elementEntry.dataType() != DataTypes.BUFFER && elementEntry.dataType() != DataTypes.ASCII_STRING)
	                    return CodecReturnCodes.FAILURE;
	                flags |= LoginRequestFlags.HAS_AUTHENTICATION_EXTENDED;
	                _rsslLoginReq.authenticationExtended().data(elementEntry.encodedData().data(), elementEntry.encodedData().position(), elementEntry.encodedData().length());
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
	
	void configInt(Data config)
	{
		if (config.dataType() == DataTypes.MAP)
		{
			if (_programmaticConfigure == null)
			{
				_programmaticConfigure = new ProgrammaticConfigure((Map)(config), _errorTracker);
			}
			else
			{
				_programmaticConfigure.addConfigure((Map)(config));
			}
		}
		else
		{
			_errorTracker.append("Invalid Data type='")
			.append(DataTypes.toString(config.dataType())).append("' for Programmatic Configure.").create(Severity.ERROR);
		}
	}
	
	int setDirectoryRefresh(com.thomsonreuters.upa.codec.RefreshMsg rsslRefreshMsg)
	{		
		if ( _rsslDirectoryRefresh == null )
			_rsslDirectoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
		else
			_rsslDirectoryRefresh.clear();
		
		_rsslDirectoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
		
		if (rsslRefreshMsg.msgClass() != MsgClasses.REFRESH)
            return CodecReturnCodes.FAILURE;

		// TODO: encode directory message to _rsslDirectoryRefresh
		
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
		
		if (dictName.equals(DictionaryCallbackClient.DICTIONARY_RWFFID))
		{
			 if (_rsslFidDictReq == null)
			 {
				 _rsslFidDictReq = (DictionaryRequest)DictionaryMsgFactory.createMsg();
				 _rsslFidDictReq.rdmMsgType(DictionaryMsgType.REQUEST);
			 }
			 if (decodeDictionaryReqMsg(_rsslFidDictReq, rsslRequestMsg) < 0)
			 {
				 _rsslFidDictReq = null;
				 String temp = "Failed to convert rssl dictionary msg to RDM msg. Message ignored.";
				 errorTracker().append(temp).create(Severity.WARNING);
					
				return CodecReturnCodes.FAILURE;
			 }
			 
			 if ( rsslRequestMsg.checkStreaming())
				 _rsslFidDictReq.applyStreaming();
			 
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
			 if (decodeDictionaryReqMsg(_rsslEnumDictReq, rsslRequestMsg) < 0)
			 {
				 _rsslEnumDictReq = null;
				 String temp = "Failed to convert rssl dictionary msg to RDM msg. Message ignored.";
				 errorTracker().append(temp).create(Severity.WARNING);
					
				return CodecReturnCodes.FAILURE;
			 }
			 
			 if ( rsslRequestMsg.checkStreaming())
				 _rsslEnumDictReq.applyStreaming();
			 
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
	
	DirectoryRefresh directoryRefresh()
	{
		return _rsslDirectoryRefresh;
	}
	
	String fidDictReqServiceName()
	{
		return _fidDictReqServiceName;
	}
	
	String enumDictReqServiceName()
	{
		return _enumDictReqServiceName;
	}
	
	String userSpecifiedHostname() 
	{
		return _hostnameSetViaFunctionCall;
	}
	
	String userSpecifiedPort()
	{
		return _portSetViaFunctionCall;
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
	
	public String getUserSpecifiedHostname() 
	{
		return _hostnameSetViaFunctionCall;
	}

	public String getUserSpecifiedPort() 
	{
		return _portSetViaFunctionCall; 
	}
	
	public HttpChannelConfig tunnelingChannelCfg()
	{
		if (_tunnelingChannelCfg == null)
			_tunnelingChannelCfg = new EncryptedChannelConfig() ;
		
		return _tunnelingChannelCfg;
	}
	
	Buffer clientId()
	{
		return _clientId;
	}
	
	Buffer tokenServiceUrl()
	{
		return _tokenServiceUrl;
	}
	
	Buffer serviceDiscoveryUrl()
	{
		return _serviceDiscoveryUrl;
	}
}

abstract class EmaConfigServerImpl extends EmaConfigBaseImpl
{	
	private DecodeIterator    					_rsslDecIter;
	private String              				_portSetViaFunctionCall;
	
	EmaConfigServerImpl()
	{
		super();
		readConfiguration(null);
	}

	EmaConfigServerImpl(String path)
	{
		super();
		readConfiguration(path);
	}
	
	void readConfiguration(String path) 
	{
		_xmlConfig = (XMLConfigReader) ConfigReader.createXMLConfigReader(this);
		_xmlConfig.loadFile(path);
	}
	
	protected void addAdminMsgInt(RefreshMsg refreshMsg)
	{	
		com.thomsonreuters.upa.codec.RefreshMsg rsslRefreshMsg = ((RefreshMsgImpl)refreshMsg).rsslMsg();
		
		switch( rsslRefreshMsg.domainType() )
		{
			case com.thomsonreuters.upa.rdm.DomainTypes.DICTIONARY :
				// TODO : add implementation to decode and store dictionary refresh message
				break;
			case com.thomsonreuters.upa.rdm.DomainTypes.SOURCE :
				// TODO: add implementation to decode and store source directory refresh message
				break;
			default:
				errorTracker().append("Refresh message passed into addAdminMsg(RefreshMsg refreshMsg) contains unhandled domain type. Domain type='")
				.append(rsslRefreshMsg.domainType())
				.append( "'. ")
				.create(Severity.ERROR);
				break;
		}
	}
	
	protected void portInt(String port, String defaultService)
	{
		if ( ( port == null ) || ( port.isEmpty() ) )
		{
			_portSetViaFunctionCall = defaultService;
		}
		
		_portSetViaFunctionCall = port;
	}
	
	public String getUserSpecifiedPort() 
	{
		return _portSetViaFunctionCall; 
	}
	
	void configInt(Data config)
	{
		if (config.dataType() == DataTypes.MAP)
		{
			if (_programmaticConfigure == null)
			{
				_programmaticConfigure = new ProgrammaticConfigure((Map)(config), _errorTracker);
			}
			else
			{
				_programmaticConfigure.addConfigure((Map)(config));
			}
		}
		else
		{
			_errorTracker.append("Invalid Data type='")
			.append(DataTypes.toString(config.dataType())).append("' for Programmatic Configure.").create(Severity.ERROR);
		}
	}
	
	DecodeIterator rsslDecodeIterator()
	{
		if (_rsslDecIter == null)
			_rsslDecIter = CodecFactory.createDecodeIterator();
		else 
			_rsslDecIter.clear();
		
		return _rsslDecIter;
	}
	
	String serverName(String instanceName)
	{
		String serverName = null;

		if ( _programmaticConfigure != null )
		{
			serverName = _programmaticConfigure.activeEntryNames(instanceName, InstanceEntryFlag.SERVER_FLAG);
			if (serverName != null)
				return serverName;
		}
	
		return (String) _xmlConfig.getIProviderAttributeValue(instanceName, ConfigManager.IProviderServerName);
	}
	
	protected void clearInt()
	{
		if (_programmaticConfigure != null)
			_programmaticConfigure.clear();
	}
}
