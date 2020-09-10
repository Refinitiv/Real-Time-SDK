/// *|-----------------------------------------------------------------------------
// *| This source code is provided under the Apache 2.0 license --
// *| and is provided AS IS with no warranty or guarantee of fit for purpose. --
// *| See the project's LICENSE.md for details. --
// *| Copyright (C) 2019 Refinitiv. All rights reserved. --
/// *|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

import com.rtsdk.ema.access.OmmLoggerClient.Severity;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataDictionary;
import com.rtsdk.eta.codec.DataStates;
import com.rtsdk.eta.codec.EncodeIterator;
import com.rtsdk.eta.codec.RefreshMsgFlags;
import com.rtsdk.eta.codec.MsgKeyFlags;
import com.rtsdk.eta.codec.RequestMsg;
import com.rtsdk.eta.codec.RequestMsgFlags;
import com.rtsdk.eta.codec.StateCodes;
import com.rtsdk.eta.codec.StatusMsgFlags;
import com.rtsdk.eta.codec.StreamStates;
import com.rtsdk.eta.rdm.Dictionary;
import com.rtsdk.eta.transport.Error;
import com.rtsdk.eta.transport.TransportBuffer;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsg;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.rtsdk.eta.valueadd.domainrep.rdm.dictionary.DictionaryStatus;
import com.rtsdk.eta.valueadd.reactor.RDMDictionaryMsgCallback;
import com.rtsdk.eta.valueadd.reactor.RDMDictionaryMsgEvent;
import com.rtsdk.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.rtsdk.eta.valueadd.reactor.ReactorChannel;
import com.rtsdk.eta.valueadd.reactor.ReactorErrorInfo;
import com.rtsdk.eta.valueadd.reactor.ReactorFactory;
import com.rtsdk.eta.valueadd.reactor.ReactorReturnCodes;

class DictionaryPayload
{
	enum DictionaryType
	{
		FIELD_DICTIONARY,
		ENUM_TYPE
	}
	
	DictionaryPayload(DataDictionary dataDictionary, DictionaryType dictionaryType)
	{
		_dataDictionary = dataDictionary;
		_dictionaryType = dictionaryType;
	}
	
	DataDictionary dictionary()
	{
		return _dataDictionary;
	}
	
	DictionaryType dictionaryType()
	{
		return _dictionaryType;
	}
	
	private DataDictionary _dataDictionary;
	private DictionaryType _dictionaryType;
}

class DictionaryHandler implements RDMDictionaryMsgCallback
{
    private static final int INIT_DICTIONARY_STATUS_MSG_SIZE = 256;
    private static final String CLIENT_NAME = "DictionaryHandler";
    
    private enum DictionaryRejectEnum
    {
        DICTIONARY_NOT_LOADED, 
        DICTIONARY_ENCODING_FAILED,
        USER_IS_NOT_LOGGED_IN,
        DICTIONARY_NAME_NOT_FOUND,
        SERVICE_ID_NOT_FOUND,
        DICTIONARY_INVALID_MESSAGE,
    	DICTIONARY_UNHANDLED_MESSAGE;
    }

    protected OmmServerBaseImpl _ommServerBaseImpl;
    private EncodeIterator _encodeIter = CodecFactory.createEncodeIterator();
    private DictionaryRefresh _dictionaryRefresh = (DictionaryRefresh)DictionaryMsgFactory.createMsg();
    private DictionaryStatus _dictionaryStatus = (DictionaryStatus)DictionaryMsgFactory.createMsg();
    
    private int _maxFieldDictFragmentSize;
    private int _maxEnumTypeFragmentSize;
    
    private boolean _apiAdminControl;

    ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();
    
    private	HashMap<String, DictionaryPayload>           _dictionaryInfoHash = new HashMap<>();
    private HashMap<LongObject, DataDictionary>			 _serviceDictionaryByIdHash = new HashMap<>();
    private StringBuilder								 _dictionaryNameAndServiceId = new StringBuilder();
    private ArrayList<ItemInfo> 						 _itemInfoList = new ArrayList<>();
    
    private LongObject                                   _streamId = new LongObject();
    private LongObject                                   _serviceId = new LongObject();

    DictionaryHandler(OmmServerBaseImpl ommServerBaseImpl)
    {
        _ommServerBaseImpl = ommServerBaseImpl;
        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryStatus.rdmMsgType(DictionaryMsgType.STATUS);
    }
    
    ArrayList<ItemInfo> getItemInfoList()
	{
		return _itemInfoList;
	}

    public void initialize()
    {
        _apiAdminControl = _ommServerBaseImpl.activeConfig().dictionaryAdminControl() == OmmIProviderConfig.AdminControl.API_CONTROL ? true : false;
        
        if (_apiAdminControl)
        {
            _maxFieldDictFragmentSize = ((OmmIProviderActiveConfig)_ommServerBaseImpl.activeConfig()).maxFieldDictFragmentSize;
            _maxEnumTypeFragmentSize = ((OmmIProviderActiveConfig)_ommServerBaseImpl.activeConfig()).maxEnumTypeFragmentSize;
            loadDictionaryFromFile(_errorInfo.error());
        }
    }

    public void loadDictionaryFromFile(Error error)
    {
    	Iterator<ServiceDictionaryConfig> iterator =  _ommServerBaseImpl.activeConfig().getServiceDictionaryConfigCollection().iterator();
    	
    	DataDictionary           dictionary = null;
    	ServiceDictionaryConfig  serviceDictionaryConfig;
    	DictionaryConfig         dictionaryConfig;
    	StringBuilder fieldNameAndServiceId = new StringBuilder();
    	StringBuilder enumTypeAndServiceId = new StringBuilder();
    	boolean existingFieldName;
    	boolean existingEnumName;
    	
    	while(iterator.hasNext())
    	{
    		serviceDictionaryConfig = iterator.next();
    		int serviceId = serviceDictionaryConfig.serviceId;
    		
    		List<DictionaryConfig> dictionaryConfigList = serviceDictionaryConfig.dictionaryProvidedList;
    		
    		for(int i = 0; i < dictionaryConfigList.size(); i++)
    		{
    			dictionaryConfig = dictionaryConfigList.get(i);
    			
    			fieldNameAndServiceId.setLength(0);
    			fieldNameAndServiceId.append(dictionaryConfig.rdmFieldDictionaryItemName).append(serviceId);
    			
    			enumTypeAndServiceId.setLength(0);
    			enumTypeAndServiceId.append(dictionaryConfig.enumTypeDefItemName).append(serviceId);
    		
    			existingFieldName = _dictionaryInfoHash.get(fieldNameAndServiceId.toString()) != null ? true : false;
    			existingEnumName = _dictionaryInfoHash.get(enumTypeAndServiceId.toString()) != null ? true: false;
    			
    			if( existingFieldName && existingEnumName )
    			{
    				_dictionaryInfoHash.remove(fieldNameAndServiceId.toString());
    				_dictionaryInfoHash.remove(enumTypeAndServiceId.toString());
    				
    				continue;
    			}
    			
    			dictionary = CodecFactory.createDataDictionary();
    			
    			 if (dictionary.loadFieldDictionary(dictionaryConfig.rdmfieldDictionaryFileName, error) < ReactorReturnCodes.SUCCESS)
    		     {
    				 StringBuilder temp = _ommServerBaseImpl.strBuilder();
		                temp.append("DictionaryHandler.loadDictionaryFromFile() failed while initializing DictionaryHandler.")
		                	.append(OmmLoggerClient.CR).append("Unable to load RDMFieldDictionary file named ")
		                	.append(dictionaryConfig.rdmfieldDictionaryFileName)
		                	.append(OmmLoggerClient.CR).append("Error Text: ").append(error.text());
    				
    				 if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
    		         {
    					 temp.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
    					 
    		             _ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
    		         }
    				 
    				 throw (_ommServerBaseImpl.ommIUExcept().message(temp.toString(), error.errorId()));
    		     }
    			 
    			 if (dictionary.loadEnumTypeDictionary(dictionaryConfig.enumtypeDefFileName, error) < 0)
    		     {
    				 StringBuilder temp = _ommServerBaseImpl.strBuilder();
		                temp.append("DictionaryHandler.loadDictionaryFromFile() failed while initializing DictionaryHandler.")
		                	.append(OmmLoggerClient.CR).append("Unable to load enumtype.def file named ")
		                	.append(dictionaryConfig.enumtypeDefFileName)
		                	.append(OmmLoggerClient.CR).append("Error Text: ").append(error.text());
    					 
    				 if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
        		     {
    					 temp.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
    					 
    		             _ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
    		         }
    				 
    				 throw (_ommServerBaseImpl.ommIUExcept().message(temp.toString(), error.errorId()));
    		     }
    			 
    			 if ( !existingFieldName )
    			 {
    				 _dictionaryInfoHash.put(fieldNameAndServiceId.toString(), new DictionaryPayload(dictionary, DictionaryPayload.DictionaryType.FIELD_DICTIONARY));
    			 }
    			 
    			 if ( !existingEnumName )
    			 {
    				 _dictionaryInfoHash.put(enumTypeAndServiceId.toString(), new DictionaryPayload(dictionary, DictionaryPayload.DictionaryType.ENUM_TYPE));
    			 }
    		}
    		
    		_serviceDictionaryByIdHash.put(new LongObject().value(serviceId), dictionary);
    	}
    }
    
    DataDictionary getDictionaryByServiceId(int serviceId)
    {
    	_serviceId.value(serviceId);
    	
    	return _serviceDictionaryByIdHash.get(_serviceId);
    }

    @Override
    public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
    {
    	_ommServerBaseImpl.eventReceived();
        ClientSession clientSession = (ClientSession)event.reactorChannel().userSpecObj();
        ReactorChannel rsslReactorChannel = event.reactorChannel();
        DictionaryMsg dictionaryMsg = event.rdmDictionaryMsg();

        if ( dictionaryMsg == null )
		{
			sendRequestReject(rsslReactorChannel, event.rdmDictionaryMsg(), DictionaryRejectEnum.DICTIONARY_INVALID_MESSAGE, _errorInfo, false);
			
			if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
        	{
				StringBuilder temp = _ommServerBaseImpl.strBuilder();
				temp.append("Dictionary message rejected - Invalid dictionary domain message.")
				.append(OmmLoggerClient.CR).append("Stream Id ").append(event.msg().streamId())
				.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
				.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
				
				_ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
	        			temp.toString(), Severity.ERROR));
        	}
			
			return ReactorCallbackReturnCodes.SUCCESS;
		}
        
        if ( !_ommServerBaseImpl.activeConfig().acceptMessageWithoutBeingLogin && !clientSession.isLogin() )
        {
        	sendRequestReject(rsslReactorChannel, event.rdmDictionaryMsg(), DictionaryRejectEnum.USER_IS_NOT_LOGGED_IN, _errorInfo, true);
			
			return ReactorCallbackReturnCodes.SUCCESS;
        }

        switch (dictionaryMsg.rdmMsgType())
        {
            case REQUEST:
            {
            	 if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
                 {
                     StringBuilder temp = _ommServerBaseImpl.strBuilder();
                     temp.append("Received dictionary request message.")
                     	.append(OmmLoggerClient.CR).append("Stream Id ").append(dictionaryMsg.streamId())
                     	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                     	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
                     
                     _ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE));
                 }
                
                _streamId.value(event.msg().streamId());
				
				ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
                
				if( itemInfo == null )
				{
					itemInfo = ServerPool.getItemInfo();
					itemInfo.clientSession(clientSession);
					itemInfo.setRequestMsg((RequestMsg)event.msg());
					clientSession.addItemInfo(itemInfo);
					_ommServerBaseImpl.addItemInfo(clientSession, itemInfo);
					_itemInfoList.add(itemInfo);
					
					 if( _apiAdminControl == false )
		             {
						 	ReqMsgImpl reqMsg = _ommServerBaseImpl.reqMsg();
						 
						 	int flags = event.msg().msgKey().flags();
							
						 	if ( (flags & MsgKeyFlags.HAS_SERVICE_ID) == MsgKeyFlags.HAS_SERVICE_ID )
							{
								reqMsg.decode(event.msg(), rsslReactorChannel.majorVersion(),
			                			rsslReactorChannel.minorVersion(), getDictionaryByServiceId(event.msg().msgKey().serviceId()) );
								
								String serviceName = _ommServerBaseImpl.directoryServiceStore().serviceName(event.msg().msgKey().serviceId());
								
								if (serviceName != null)
								{
									flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
							
									reqMsg._rsslMsg.msgKey().flags(flags);
							
									reqMsg.msgServiceName(serviceName);
							
									reqMsg._rsslMsg.msgKey().flags( flags | MsgKeyFlags.HAS_SERVICE_ID);
								}
								else
								{						
									sendRequestReject(event.reactorChannel(), event.rdmDictionaryMsg(), DictionaryRejectEnum.SERVICE_ID_NOT_FOUND , _errorInfo, true );
									
									_itemInfoList.remove(itemInfo);
									_ommServerBaseImpl.removeItemInfo(itemInfo, false);
									
									return ReactorCallbackReturnCodes.SUCCESS;
								}
			                	
								_ommServerBaseImpl.ommProviderEvent()._clientHandle = clientSession.clientHandle();
								_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
								_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
								_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
								_ommServerBaseImpl.ommProviderEvent()._channel = event.reactorChannel();
		
								_ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
								_ommServerBaseImpl.ommProviderClient().onReqMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
							}
							else
							{
								reqMsg.decode(event.msg(), rsslReactorChannel.majorVersion(),
			                			rsslReactorChannel.minorVersion(), null );
							}
		             }
					 else
					 {
						 if ( sendDictionaryResponse(rsslReactorChannel, event.rdmDictionaryMsg(), _errorInfo) == false )
						 {
							 _itemInfoList.remove(itemInfo);
							_ommServerBaseImpl.removeItemInfo(itemInfo, false);
						 }
					 }
				}
				else
				{
					itemInfo.setRequestMsg((RequestMsg)event.msg());
					
					 if( _apiAdminControl == false )
		             {
						 	ReqMsgImpl reqMsg = _ommServerBaseImpl.reqMsg();
						 	
						 	int flags = event.msg().msgKey().flags();
						 	
						 	if ( (flags & MsgKeyFlags.HAS_SERVICE_ID) == MsgKeyFlags.HAS_SERVICE_ID )
							{
								reqMsg.decode(event.msg(), rsslReactorChannel.majorVersion(),
			                			rsslReactorChannel.minorVersion(), getDictionaryByServiceId(event.msg().msgKey().serviceId()) );
								
								String serviceName = _ommServerBaseImpl.directoryServiceStore().serviceName(event.msg().msgKey().serviceId());
								
								if (serviceName != null)
								{
									flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
							
									reqMsg._rsslMsg.msgKey().flags(flags);
							
									reqMsg.msgServiceName(serviceName);
							
									reqMsg._rsslMsg.msgKey().flags( flags | MsgKeyFlags.HAS_SERVICE_ID);
								}
								else
								{						
									sendRequestReject(event.reactorChannel(), event.rdmDictionaryMsg(), DictionaryRejectEnum.SERVICE_ID_NOT_FOUND , _errorInfo, true );
									
									_itemInfoList.remove(itemInfo);
									_ommServerBaseImpl.removeItemInfo(itemInfo, false);
									
									return ReactorCallbackReturnCodes.SUCCESS;
								}
							}
			                	
			                 _ommServerBaseImpl.ommProviderEvent()._clientHandle = clientSession.clientHandle();
			    			 _ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
			    			 _ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
							 _ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
							 _ommServerBaseImpl.ommProviderEvent()._channel = event.reactorChannel();
						
							 _ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
							 _ommServerBaseImpl.ommProviderClient().onReissue(reqMsg, _ommServerBaseImpl.ommProviderEvent());
		             }
					 else
					 {
						 if ( sendDictionaryResponse(rsslReactorChannel, event.rdmDictionaryMsg(), _errorInfo) == false )
						 {
							 _itemInfoList.remove(itemInfo);
							_ommServerBaseImpl.removeItemInfo(itemInfo, false);
						 }
					 }
				}
                
                break;
            }
            case CLOSE:
            {
            	if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
                {
                    StringBuilder temp = _ommServerBaseImpl.strBuilder();
                    temp.append("Received dictionary close message.")
                    	.append(OmmLoggerClient.CR).append("Stream Id ").append(dictionaryMsg.streamId())
                    	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                    	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
                    
                    _ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE));
                }
                
                _streamId.value(event.msg().streamId());
				
				ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
                
				if( itemInfo != null )
				{
					if( _apiAdminControl == false )
		            {
						RequestMsg rsslReqMsg = _ommServerBaseImpl.rsslRequestMsg();
						
						rsslReqMsg.applyNoRefresh();
						
						rsslReqMsg.streamId(event.msg().streamId());
						
						if( itemInfo.msgKey().checkHasName() )
						{
							rsslReqMsg.msgKey().applyHasName();
							rsslReqMsg.msgKey().name(itemInfo.msgKey().name());
						}
						
						if ( itemInfo.msgKey().checkHasNameType())
						{
							rsslReqMsg.msgKey().applyHasNameType();
							rsslReqMsg.msgKey().nameType(itemInfo.msgKey().nameType());
						}
						
						rsslReqMsg.domainType(event.msg().domainType());
						
						ReqMsgImpl reqMsg = _ommServerBaseImpl.reqMsg();
                			
						if( itemInfo.msgKey().checkHasServiceId())
						{
							rsslReqMsg.msgKey().applyHasServiceId();
							rsslReqMsg.msgKey().serviceId(itemInfo.msgKey().serviceId());
							
							reqMsg.decode(rsslReqMsg, rsslReactorChannel.majorVersion(),
									rsslReactorChannel.minorVersion(), null);
							
							String serviceName = _ommServerBaseImpl.directoryServiceStore().serviceName(itemInfo.msgKey().serviceId());
							
							int flags = reqMsg._rsslMsg.msgKey().flags();
							
							if (serviceName != null)
							{
								flags &= ~MsgKeyFlags.HAS_SERVICE_ID;
						
								reqMsg._rsslMsg.msgKey().flags(flags);
						
								reqMsg.msgServiceName(serviceName);
						
								reqMsg._rsslMsg.msgKey().flags( flags | MsgKeyFlags.HAS_SERVICE_ID);
							}
						}
						else
						{
							reqMsg.decode(rsslReqMsg, rsslReactorChannel.majorVersion(),
		                			rsslReactorChannel.minorVersion(), null);
						}
						
						int flags = reqMsg._rsslMsg.flags();
						flags &= ~RequestMsgFlags.STREAMING;
						rsslReqMsg.flags(flags);
	                	
						_ommServerBaseImpl.ommProviderEvent()._clientHandle = clientSession.clientHandle();
						_ommServerBaseImpl.ommProviderEvent()._closure = _ommServerBaseImpl.closure();
						_ommServerBaseImpl.ommProviderEvent()._ommProvider = _ommServerBaseImpl.provider();
						_ommServerBaseImpl.ommProviderEvent()._handle = itemInfo.handle();
						_ommServerBaseImpl.ommProviderEvent()._channel = event.reactorChannel();

						_ommServerBaseImpl.ommProviderClient().onAllMsg(reqMsg, _ommServerBaseImpl.ommProviderEvent());
						_ommServerBaseImpl.ommProviderClient().onClose(reqMsg, _ommServerBaseImpl.ommProviderEvent());
		            }
					
					_itemInfoList.remove(itemInfo);
					_ommServerBaseImpl.removeItemInfo(itemInfo, false);
				}
               
               break;
            }
            case  REFRESH:
            {
				if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
	        	{
					StringBuilder temp = _ommServerBaseImpl.strBuilder();
					temp.append("Received refresh message.")
					.append(OmmLoggerClient.CR).append("Stream Id ").append(event.msg().streamId())
                	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
					
					_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
		        			temp.toString(), Severity.TRACE));
	        	}
				
				DataDictionary dataDictionary = null;

				if ( (event.msg().flags() & RefreshMsgFlags.HAS_MSG_KEY)  != 0)
				{
					if ( event.msg().msgKey().checkHasServiceId() )
					{
						dataDictionary = _ommServerBaseImpl.dictionaryHandler().getDictionaryByServiceId(event.msg().msgKey().serviceId());
					}
				}
					
				_ommServerBaseImpl.itemCallbackClient().processIProviderMsgCallback(event, dataDictionary);
				
				break;
            }
			case STATUS:
			{
				if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
	        	{
					StringBuilder temp = _ommServerBaseImpl.strBuilder();
					temp.append("Received status message.")
					.append(OmmLoggerClient.CR).append("Stream Id ").append(event.msg().streamId())
                	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
					
					_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
		        			temp.toString(), Severity.TRACE));
	        	}
				
				DataDictionary dataDictionary = null;
				
				if ( (event.msg().flags() & StatusMsgFlags.HAS_MSG_KEY)  != 0)
				{
					if ( event.msg().msgKey().checkHasServiceId() )
					{
						dataDictionary = _ommServerBaseImpl.dictionaryHandler().getDictionaryByServiceId(event.msg().msgKey().serviceId());
					}
				}
					
				_ommServerBaseImpl.itemCallbackClient().processIProviderMsgCallback(event, dataDictionary);
						
				break;
			}
            default:
            {
            	StringBuilder temp = _ommServerBaseImpl.strBuilder();
            	temp.append("Rejected unhandled dictionary message type ").append(dictionaryMsg.rdmMsgType().toString());
            	
            	_streamId.value(event.msg().streamId());
    			
    			ItemInfo itemInfo = clientSession.getItemInfo(_streamId);
    			
    			if( itemInfo == null )
    			{
    				sendRequestReject(rsslReactorChannel, event.rdmDictionaryMsg(), DictionaryRejectEnum.DICTIONARY_UNHANDLED_MESSAGE, _errorInfo, false);
    			}
    			
    			if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
            	{
    				temp.append(OmmLoggerClient.CR).append("Stream Id ").append(dictionaryMsg.streamId())
                   	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                   	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
    				
    				_ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME,
    	        			temp.toString(), Severity.TRACE));
            	}
            }
        }
        
        return ReactorCallbackReturnCodes.SUCCESS;
    }
    
    private boolean sendDictionaryResponse(ReactorChannel reactorChannel, DictionaryMsg dictionaryRequest, ReactorErrorInfo error)
    {
    	_dictionaryNameAndServiceId.setLength(0);
    	_dictionaryNameAndServiceId.append(((DictionaryRequest)dictionaryRequest).dictionaryName().toString()).append(((DictionaryRequest)dictionaryRequest).serviceId());
    	
    	DictionaryPayload dictionaryPayload = _dictionaryInfoHash.get(_dictionaryNameAndServiceId.toString());
    	
    	if( dictionaryPayload == null )
    	{
    		sendRequestReject(reactorChannel, dictionaryRequest, DictionaryRejectEnum.DICTIONARY_NAME_NOT_FOUND, error, true);
    		return false;
    	}
    	else
    	{
    		if (dictionaryPayload.dictionaryType() == DictionaryPayload.DictionaryType.FIELD_DICTIONARY)
    		{
    			if ( sendFieldDictionaryResponse(reactorChannel, dictionaryRequest, dictionaryPayload.dictionary(), error) != CodecReturnCodes.SUCCESS )
    			{
    				sendRequestReject(reactorChannel, dictionaryRequest, DictionaryRejectEnum.DICTIONARY_ENCODING_FAILED, error, true);
    	    		return false;
    			}
    		}
    		else if (dictionaryPayload.dictionaryType() == DictionaryPayload.DictionaryType.ENUM_TYPE)
    		{
    			if ( sendEnumTypeDictionaryResponse(reactorChannel, dictionaryRequest, dictionaryPayload.dictionary(), error) != CodecReturnCodes.SUCCESS )
    			{
    				sendRequestReject(reactorChannel, dictionaryRequest, DictionaryRejectEnum.DICTIONARY_ENCODING_FAILED, error, true);
    	    		return false;
    			}
    		}
    	}
    	
    	return true;
    }

    private int sendFieldDictionaryResponse(ReactorChannel reactorChannel, DictionaryMsg dictionaryRequest, DataDictionary dataDictionary, ReactorErrorInfo error)
    {
    	ClientSession clientSession = (ClientSession)reactorChannel.userSpecObj();
    	
        _dictionaryRefresh.clear();
        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryRefresh.streamId(dictionaryRequest.streamId());
        _dictionaryRefresh.dictionaryType(Dictionary.Types.FIELD_DEFINITIONS);
        _dictionaryRefresh.dictionary(dataDictionary);
        _dictionaryRefresh.verbosity(((DictionaryRequest)dictionaryRequest).verbosity());
        _dictionaryRefresh.serviceId(((DictionaryRequest)dictionaryRequest).serviceId());
        _dictionaryRefresh.dictionaryName(((DictionaryRequest)dictionaryRequest).dictionaryName());
        _dictionaryRefresh.applySolicited();

        _dictionaryRefresh.state().streamState(StreamStates.OPEN);
        _dictionaryRefresh.state().dataState(DataStates.OK);
        _dictionaryRefresh.state().code(StateCodes.NONE);
        
        boolean firstPartMultiPartRefresh = true;
        int flags = _dictionaryRefresh.flags();

        while (true)
        {
        	if (firstPartMultiPartRefresh)
        	{
        		_dictionaryRefresh.applyClearCache();
        		firstPartMultiPartRefresh = false;
        		_dictionaryRefresh.startFid(dataDictionary.minFid());
        	}
        	else
        	{
        		_dictionaryRefresh.flags(flags);
        	}
        	
            TransportBuffer msgBuf = reactorChannel.getBuffer(_maxFieldDictFragmentSize, false, error);
            
            if (msgBuf == null)
            {
                if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
                {
                    StringBuilder temp = _ommServerBaseImpl.strBuilder();
                    temp.append("Internal error. Failed to get bufffer in DictionaryHandler.sendFieldDictionaryResponse()")
                    .append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                   	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName())
                   	.append(OmmLoggerClient.CR).append("Error Id ").append(error.error().errorId())
                   	.append(OmmLoggerClient.CR).append("Internal sysError ").append(error.error().sysError())
                   	.append(OmmLoggerClient.CR).append("Error Location ").append(error.location())
                    .append(OmmLoggerClient.CR).append("Error Text: ").append(error.error().text());

                    _ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
                }
                
                return CodecReturnCodes.FAILURE;
            }
            
            _dictionaryRefresh.state().text().data("Field Dictionary Refresh (starting fid " + _dictionaryRefresh.startFid() + ")");
            
            _encodeIter.clear();
            int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(), reactorChannel.minorVersion());
            if (ret != CodecReturnCodes.SUCCESS)
            {
                if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
                {
                	StringBuilder temp = _ommServerBaseImpl.strBuilder();
                    temp.append("Internal error. Failed to set encode iterator in DictionaryHandler.sendFieldDictionaryResponse()")
                    .append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                   	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());

                    _ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
                }
                
                return CodecReturnCodes.FAILURE;
            }
    
            ret = _dictionaryRefresh.encode(_encodeIter);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
                {
                	StringBuilder temp = _ommServerBaseImpl.strBuilder();
                	temp.append("Internal error. Failed to encode message in DictionaryHandler.sendFieldDictionaryResponse()")
                	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                   	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
                   
                    _ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
                }
                
                return CodecReturnCodes.FAILURE;
            }
    
            int retCode = reactorChannel.submit(msgBuf, _ommServerBaseImpl._rsslSubmitOptions, error);
            if (retCode < CodecReturnCodes.SUCCESS)
            {
                if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
                {
                    StringBuilder temp = _ommServerBaseImpl.strBuilder();
                    temp.append("Internal error. Failure to submit dictionary message in DictionaryHandler.sendFieldDictionaryResponse().")
                    .append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                   	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName())
                   	.append(OmmLoggerClient.CR).append("Error Id ").append(error.error().errorId())
                   	.append(OmmLoggerClient.CR).append("Internal sysError ").append(error.error().sysError())
                   	.append(OmmLoggerClient.CR).append("Error Location ").append(error.location())
                    .append(OmmLoggerClient.CR).append("Error Text: ").append(error.error().text());

                    _ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
                }
                
                return CodecReturnCodes.FAILURE;
            }
            
            if (ret == CodecReturnCodes.SUCCESS)
            {
                break;
            }
        }
        
        if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
        {
            StringBuilder temp = _ommServerBaseImpl.strBuilder();
            temp.append("Successfully sent field dictionary type.")
            .append(OmmLoggerClient.CR).append("Dictionary name ").append(((DictionaryRequest)dictionaryRequest).dictionaryName())
            .append(OmmLoggerClient.CR).append("Stream Id ").append(dictionaryRequest.streamId())
            .append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value());
            
            _ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE).toString());
        }
        
        return CodecReturnCodes.SUCCESS;
    }

    private int sendEnumTypeDictionaryResponse(ReactorChannel reactorChannel, DictionaryMsg dictionaryRequest,  DataDictionary dataDictionary, ReactorErrorInfo error)
    {
    	ClientSession clientSession = (ClientSession)reactorChannel.userSpecObj();
    	
        _dictionaryRefresh.clear();
        
        _dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
        _dictionaryRefresh.streamId(dictionaryRequest.streamId());
        _dictionaryRefresh.dictionaryType(Dictionary.Types.ENUM_TABLES);
        _dictionaryRefresh.dictionary(dataDictionary);
        _dictionaryRefresh.serviceId(((DictionaryRequest)dictionaryRequest).serviceId());
        _dictionaryRefresh.verbosity(((DictionaryRequest)dictionaryRequest).verbosity());
        _dictionaryRefresh.dictionaryName(((DictionaryRequest)dictionaryRequest).dictionaryName());
        _dictionaryRefresh.applySolicited();
        _dictionaryRefresh.applyRefreshComplete();

        _dictionaryRefresh.state().streamState(StreamStates.OPEN);
        _dictionaryRefresh.state().dataState(DataStates.OK);
        _dictionaryRefresh.state().code(StateCodes.NONE);
        
        boolean firstPartMultiPartRefresh = true;
        int flags =  _dictionaryRefresh.flags();

        while (true)
        {
        	if (firstPartMultiPartRefresh)
        	{
        		_dictionaryRefresh.applyClearCache();
        		firstPartMultiPartRefresh = false;
        	}
        	else
        	{
        		_dictionaryRefresh.flags(flags);
        	}
        	
            TransportBuffer msgBuf = reactorChannel.getBuffer(_maxEnumTypeFragmentSize, false, error);
            if (msgBuf == null)
            {
                if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
                {
                    StringBuilder temp = _ommServerBaseImpl.strBuilder();
                    temp.append("Internal error. Failed to get buffer in DictionaryHandler.sendEnumTypeDictionaryResponse()")
                    .append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                   	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName())
                   	.append(OmmLoggerClient.CR).append("Error Id ").append(error.error().errorId())
                   	.append(OmmLoggerClient.CR).append("Internal sysError ").append(error.error().sysError())
                   	.append(OmmLoggerClient.CR).append("Error Location ").append(error.location())
                    .append(OmmLoggerClient.CR).append("Error Text: ").append(error.error().text());
    
                    _ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
                }
                
                return CodecReturnCodes.FAILURE;
            }
    
            _encodeIter.clear();
            int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(), reactorChannel.minorVersion());
            if (ret < CodecReturnCodes.SUCCESS)
            {
                if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
                {
                	StringBuilder temp = _ommServerBaseImpl.strBuilder();
                    temp.append("Internal error. Failed to set encode iterator in DictionaryHandler.sendEnumTypeDictionaryResponse()")
                    .append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                   	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
    
                    _ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
                }
                
                return CodecReturnCodes.FAILURE;
            }
            
            _dictionaryRefresh.state().text().data("Enum Type Dictionary Refresh (starting enum table count " + _dictionaryRefresh.startEnumTableCount() + ")");
    
            ret = _dictionaryRefresh.encode(_encodeIter);
            if (ret < CodecReturnCodes.SUCCESS)
            {
                if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
                {
                	StringBuilder temp = _ommServerBaseImpl.strBuilder();
                	temp.append("Internal error. Failed to encode message in DictionaryHandler.sendEnumTypeDictionaryResponse()")
                	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
                   	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
    
                    _ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
                }
                
                return CodecReturnCodes.FAILURE;
            }
    
            if ( reactorChannel.submit(msgBuf, _ommServerBaseImpl._rsslSubmitOptions, error) < CodecReturnCodes.SUCCESS )
            {
                if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
                {
                    StringBuilder temp = _ommServerBaseImpl.strBuilder();
                    temp.append("Failed to submit dictionary message in DictionaryHandler.sendEnumTypeDictionaryResponse().")
                    .append(OmmLoggerClient.CR).append("Error Id ").append(error.error().errorId())
                    .append(OmmLoggerClient.CR).append("Internal sysError ").append(error.error().sysError())
                    .append(OmmLoggerClient.CR).append("Error Location ").append(error.location())
                    .append(OmmLoggerClient.CR).append("Error Text: ").append(_errorInfo.error().text());
    
                    _ommServerBaseImpl.loggerClient().error(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR));
                }
                
                return CodecReturnCodes.FAILURE;
            }
            
            if (ret == CodecReturnCodes.SUCCESS)
            {
                break;
            }
        }
        
        if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
        {
            StringBuilder temp = _ommServerBaseImpl.strBuilder();
            temp.append("Successfully sent enumeration dictionary type.")
            .append(OmmLoggerClient.CR).append("Dictionary name ").append(((DictionaryRequest)dictionaryRequest).dictionaryName())
            .append(OmmLoggerClient.CR).append("Stream Id ").append(dictionaryRequest.streamId())
            .append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value());
            
            _ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE).toString());
        }
        
        return CodecReturnCodes.SUCCESS;
    }

    private int sendRequestReject(ReactorChannel reactorChannel, DictionaryMsg dictionaryRequest, DictionaryRejectEnum reason, ReactorErrorInfo error, boolean traceMessage)
    {
    	int bufferSize = INIT_DICTIONARY_STATUS_MSG_SIZE;
    	
    	if ( reason == DictionaryRejectEnum.DICTIONARY_NAME_NOT_FOUND)
    	{
    		bufferSize += ((DictionaryRequest)dictionaryRequest).dictionaryName().length();
    	}
    	
        TransportBuffer msgBuf = reactorChannel.getBuffer(bufferSize, false, error);

        if (msgBuf != null)
        {
            int ret = encodeDictionaryRequestReject(reactorChannel, dictionaryRequest, reason, msgBuf, error, traceMessage);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                return ret;
            }

            return reactorChannel.submit(msgBuf, _ommServerBaseImpl._rsslSubmitOptions, error);
        }
        else
        {
        	 if (_ommServerBaseImpl.loggerClient().isTraceEnabled())
             {
                 StringBuilder temp = _ommServerBaseImpl.strBuilder();
                 temp.append("Internal error. Failed to get buffer in DictionaryHandler.sendRequestReject()")
                 .append(OmmLoggerClient.CR).append("Error Id ").append(error.error().errorId())
                 .append(OmmLoggerClient.CR).append("Internal sysError ").append(error.error().sysError())
                 .append(OmmLoggerClient.CR).append("Error Location ").append(error.location())
                 .append(OmmLoggerClient.CR).append("Error Text: ").append(_errorInfo.error().text());
                 
                 _ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.TRACE).toString());
             }
      
            return CodecReturnCodes.FAILURE;
        }
    }

    private int encodeDictionaryRequestReject(ReactorChannel reactorChannel, DictionaryMsg dictionaryRequest, DictionaryRejectEnum reason, TransportBuffer msgBuf,
            ReactorErrorInfo error, boolean traceMessage)
    {
    	ClientSession clientSession = (ClientSession)reactorChannel.userSpecObj();
    	
        _encodeIter.clear();
        _dictionaryStatus.clear();
        
        _dictionaryStatus.streamId(dictionaryRequest.streamId());
        _dictionaryStatus.applyHasState();
        _dictionaryStatus.state().dataState(DataStates.SUSPECT);
        _dictionaryStatus.state().code(StateCodes.ERROR);
        _dictionaryStatus.state().streamState(StreamStates.CLOSED_RECOVER);
        
        switch (reason)
        {
            case DICTIONARY_INVALID_MESSAGE:
                _dictionaryStatus.state().text().data("Dictionary message rejected - invalid dictionary domain message.");
                break;
            case DICTIONARY_NOT_LOADED:
                _dictionaryStatus.state().text().data("Dictionary request message rejected - dictionary is not loaded in provider.");
                break;
            case DICTIONARY_ENCODING_FAILED:
            	_dictionaryStatus.state().text().data("Dictionary request message rejected - failed to encode dictionary information.");
            	break;
            case USER_IS_NOT_LOGGED_IN:
            	_dictionaryStatus.state().text().data("Dictionary message rejected - there is no logged in user for this session.");
            	break;
            case DICTIONARY_UNHANDLED_MESSAGE:
            	_dictionaryStatus.state().text().data("Dictionary message rejected - unhandled dictionary message type.");
            	break;
            case DICTIONARY_NAME_NOT_FOUND:
            {
            	StringBuilder text = _ommServerBaseImpl.strBuilder();
            	text.append("Dictionary request message rejected - the reqesting dictionary name '")
            	.append(((DictionaryRequest)dictionaryRequest).dictionaryName()).append("' not found.");
            	_dictionaryStatus.state().text().data(text.toString());
            }
            	break;
            case SERVICE_ID_NOT_FOUND:
            {
            	StringBuilder text = _ommServerBaseImpl.strBuilder();
            	text.append("Dictionary request message rejected - the service Id = ")
            	.append(((DictionaryRequest)dictionaryRequest).serviceId())
            	.append("  does not exist in the source directory");
            	_dictionaryStatus.state().text().data(text.toString());
            }
                break;
            default:
            	return CodecReturnCodes.FAILURE;
        }

        if (traceMessage && _ommServerBaseImpl.loggerClient().isTraceEnabled())
        {
        	StringBuilder text = _ommServerBaseImpl.strBuilder();
        	text.append(_dictionaryStatus.state().text().toString())
        	.append(OmmLoggerClient.CR).append("Stream Id ").append(dictionaryRequest.streamId())
        	.append(OmmLoggerClient.CR).append("client handle ").append(clientSession.clientHandle().value())
        	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
        	
            _ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, text.toString(), Severity.TRACE).toString());
        }
       
        int ret = _encodeIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(), reactorChannel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            if (_ommServerBaseImpl.loggerClient().isErrorEnabled())
            {
            	StringBuilder temp = _ommServerBaseImpl.strBuilder();
                temp.append("Internal error. Failed to set encode iterator in DictionaryHandler.encodeDictionaryRequestReject()")
                .append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
               	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
                
                _ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR).toString());
            }
            
            return ret;
        }

        ret = _dictionaryStatus.encode(_encodeIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
        	StringBuilder temp = _ommServerBaseImpl.strBuilder();
        	temp.append("Internal error. Failed to encode status message in DictionaryHandler.encodeDictionaryRequestReject()")
        	.append(OmmLoggerClient.CR).append("Client handle ").append(clientSession.clientHandle().value())
           	.append(OmmLoggerClient.CR).append("Instance Name ").append(_ommServerBaseImpl.instanceName());
        	
        	 _ommServerBaseImpl.loggerClient().trace(_ommServerBaseImpl.formatLogMessage(CLIENT_NAME, temp.toString(), Severity.ERROR).toString());
        	
            return ret;
        }

        return CodecReturnCodes.SUCCESS;
    }
}
