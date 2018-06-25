///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.thomsonreuters.ema.access.ConfigManager.ConfigAttributes;
import com.thomsonreuters.ema.access.ConfigManager.ConfigElement;
import com.thomsonreuters.ema.access.DirectoryServiceStore.ServiceIdInteger;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.ema.rdm.EmaRdm;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.State;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBase;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service.ServiceGroup;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service.ServiceState;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEventTypes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;

class OmmIProviderImpl extends OmmServerBaseImpl implements OmmProvider, DirectoryServiceStoreClient
{
	private OmmIProviderActiveConfig _activeConfig = null;
	
	private OmmIProviderDirectoryStore _ommIProviderDirectoryStore;
	private boolean _storeUserSubmitted;
	private DirectoryMsg	_fanoutDirectoryMsg;
	protected EmaObjectManager _objManager = new EmaObjectManager();
	private ItemWatchList	_itemWatchList;
	private static final long MIN_LONG_VALUE = 1;
    private static final long MAX_LONG_VALUE = Long.MAX_VALUE;
    
	private static long _longId = Integer.MAX_VALUE;
	
	OmmIProviderImpl(OmmProviderConfig config, OmmProviderClient ommProviderClient, Object closure)
	{
		super(ommProviderClient, closure);
		
		_activeConfig = new OmmIProviderActiveConfig();
		
		_activeConfig.dictionaryAdminControl = ((OmmIProviderConfigImpl)config).adminControlDictionary();
		
		_activeConfig.directoryAdminControl = ((OmmIProviderConfigImpl)config).adminControlDirectory();
		
		_ommIProviderDirectoryStore = new OmmIProviderDirectoryStore(_objManager, this, _activeConfig);
		
		_ommIProviderDirectoryStore.setClient(this);
		
		super.initialize(_activeConfig, (OmmIProviderConfigImpl)config);
		
		_itemWatchList = new ItemWatchList(_itemCallbackClient);
		
		_storeUserSubmitted = _activeConfig.directoryAdminControl == OmmIProviderConfig.AdminControl.API_CONTROL ? true : false;
		
		_fanoutDirectoryMsg = DirectoryMsgFactory.createMsg();
	}
	
	OmmIProviderImpl(OmmProviderConfig config, OmmProviderClient ommProviderClient, OmmProviderErrorClient providerErrorClient, Object closure)
	{
		super(ommProviderClient, providerErrorClient, closure);
		
		_activeConfig = new OmmIProviderActiveConfig();
		
		_activeConfig.dictionaryAdminControl = ((OmmIProviderConfigImpl)config).adminControlDictionary();
		
		_activeConfig.directoryAdminControl = ((OmmIProviderConfigImpl)config).adminControlDirectory();
		
		_ommIProviderDirectoryStore = new OmmIProviderDirectoryStore(_objManager, this, _activeConfig);	
		
		_ommIProviderDirectoryStore.setClient(this);
		
		super.initialize(_activeConfig, (OmmIProviderConfigImpl)config);
		
		_itemWatchList = new ItemWatchList(_itemCallbackClient);
		
		_storeUserSubmitted = _activeConfig.directoryAdminControl == OmmIProviderConfig.AdminControl.API_CONTROL ? true : false;
		
		_fanoutDirectoryMsg = DirectoryMsgFactory.createMsg();
	}

	@Override
	public String providerName()
	{
		return _activeConfig.instanceName;
	}

	@Override
	public int providerRole()
	{
		return OmmProviderConfig.ProviderRole.INTERACTIVE;
	}
	
	@Override
	void readCustomConfig(EmaConfigServerImpl config)
	{
	    int maxInt = Integer.MAX_VALUE;
	
		_activeConfig.directoryAdminControl = ((OmmIProviderConfigImpl)config).adminControlDirectory();

		_ommIProviderDirectoryStore.loadConfigDirectory(config);
		
		ConfigAttributes iProviderAttributes = getAttributes(config);
		
		if(iProviderAttributes != null)
		{
			ConfigElement element = (ConfigElement)iProviderAttributes.getElement(ConfigManager.IProviderRefreshFirstRequired);
			
			if (element != null)
			{
				_activeConfig.refreshFirstRequired = element.intLongValue() > 0 ? true : false;
			}
			
			element = (ConfigElement)iProviderAttributes.getElement(ConfigManager.IProviderAcceptMessageWithoutAcceptingRequests);
			
			if (element != null)
			{
				_activeConfig.acceptMessageWithoutAcceptingRequests = element.intLongValue() > 0 ? true : false;
			}
			
			element = (ConfigElement)iProviderAttributes.getElement(ConfigManager.IProviderAcceptDirMessageWithoutMinFilters);
			
			if (element != null)
			{
				_activeConfig.acceptDirMessageWithoutMinFilters = element.intLongValue() > 0 ? true : false;
			}
			
			element = (ConfigElement)iProviderAttributes.getElement(ConfigManager.IProviderAcceptMessageWithoutBeingLogin);
			
			if (element != null)
			{
				_activeConfig.acceptMessageWithoutBeingLogin = element.intLongValue() > 0 ? true : false;
			}
			
			element = (ConfigElement)iProviderAttributes.getElement(ConfigManager.IProviderAcceptMessageSameKeyButDiffStream);
			
			if (element != null)
			{
				_activeConfig.acceptMessageSameKeyButDiffStream = element.intLongValue() > 0 ? true : false;
			}
			
			element = (ConfigElement)iProviderAttributes.getElement(ConfigManager.IProviderAcceptMessageThatChangesService);
			
			if (element != null)
			{
				_activeConfig.acceptMessageThatChangesService = element.intLongValue() > 0 ? true : false;
			}
			
			element = (ConfigElement)iProviderAttributes.getElement(ConfigManager.IProviderAcceptMessageWithoutQosInRange);
			
			if (element != null)
			{
				_activeConfig.acceptMessageWithoutQosInRange = element.intLongValue() > 0 ? true : false;
			}
			
			element = (ConfigElement)iProviderAttributes.getElement(ConfigManager.DictionaryFieldDictFragmentSize);
			
			if (element != null)
			{
	            if ( element.intLongValue()  > maxInt )
	                _activeConfig.maxFieldDictFragmentSize = maxInt;
	            else
	                _activeConfig.maxFieldDictFragmentSize = element.intLongValue() < 0 ? OmmIProviderActiveConfig.DEFAULT_FIELD_DICT_FRAGMENT_SIZE : element.intLongValue();
			}
			else
			{
			    _activeConfig.maxFieldDictFragmentSize = OmmIProviderActiveConfig.DEFAULT_FIELD_DICT_FRAGMENT_SIZE;
			}
			
	        element = (ConfigElement)iProviderAttributes.getElement(ConfigManager.DictionaryEnumTypeFragmentSize);
	        
            if (element != null)
            {
                if ( element.intLongValue()  > maxInt )
                    _activeConfig.maxEnumTypeFragmentSize = maxInt;
                else
                    _activeConfig.maxEnumTypeFragmentSize = element.intLongValue() < 0 ? OmmIProviderActiveConfig.DEFAULT_ENUM_TYPE_FRAGMENT_SIZE : element.intLongValue();
            }
            else
            {
                _activeConfig.maxEnumTypeFragmentSize = OmmIProviderActiveConfig.DEFAULT_ENUM_TYPE_FRAGMENT_SIZE;
            }
            
            element = (ConfigElement)iProviderAttributes.getElement(ConfigManager.RequestTimeout);
            
            if (element != null)
            {
            	 if ( element.intLongValue()  > maxInt )
                     _activeConfig.requestTimeout = maxInt;
                 else
                     _activeConfig.requestTimeout = element.intLongValue() < 0 ? OmmIProviderActiveConfig.DEFAULT_REQUEST_TIMEOUT : element.intLongValue();
            }
                 
		}

		// TODO: add handling for programmatic configuration
	}

	@Override
	public long registerClient(ReqMsg reqMsg, OmmProviderClient client)
	{
		return  registerClient(reqMsg, client, null);
	}

	@Override
	public long registerClient(ReqMsg reqMsg, OmmProviderClient client, Object closure)
	{
		userLock().lock();
		
		if ( reqMsg.domainType() != EmaRdm.MMT_DICTIONARY )
		{
			StringBuilder temp = strBuilder();
			temp.append("OMM Interactive provider supports registering DICTIONARY domain type only.");
			userLock().unlock();
			handleInvalidUsage(temp.toString());
			return 0;
		}
		
		if ( serverChannelHandler().clientSessionMap().size() == 0 )
		{
			StringBuilder temp = strBuilder();
			temp.append("There is no active client session available for registering.");
			userLock().unlock();
			handleInvalidUsage(temp.toString());
			return 0;
		}
		
		long handle = _itemCallbackClient.registerClient(reqMsg, client, closure, 0);
		
		userLock().unlock();
		
		return handle;
	}
	
	@Override
	public void reissue(ReqMsg reqMsg, long handle) 
	{
		userLock().lock();
		
		ReqMsgImpl reqMsgImpl = (ReqMsgImpl)reqMsg;
		
		if ( reqMsgImpl.domainTypeSet() && reqMsgImpl.domainType() != EmaRdm.MMT_DICTIONARY )
		{
			userLock().unlock();
			StringBuilder temp = strBuilder();
			temp.append("OMM Interactive provider supports reissuing DICTIONARY domain type only.");
			handleInvalidUsage(temp.toString());
			return;
		}
		
		_itemCallbackClient.reissue(reqMsg, handle);
		
		userLock().unlock();
	}

	@Override
	public void submit(GenericMsg genericMsg, long handle)
	{
		userLock().lock();
		
		GenericMsgImpl genericMsgImpl = (GenericMsgImpl)genericMsg;
		
		ItemInfo itemInfo = getItemInfo(handle);
		
		if( ( itemInfo == null ) )
		{
			userLock().unlock();
			StringBuilder temp = strBuilder();
			temp.append("Attempt to submit GenericMsg with non existent Handle = ")
			.append(handle).append(".");
			handleInvalidUsage(temp.toString());
			return;
		}
		
		if ( itemInfo.domainType() == EmaRdm.MMT_DICTIONARY )
		{
			userLock().unlock();
			StringBuilder temp = strBuilder();
			temp.append("Attempt to submit GenericMsg with Dictionary domain while this is not supported.")
			.append(handle).append(".");
			handleInvalidUsage(temp.toString());
			return;
		}
		
		genericMsgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
		if (genericMsgImpl._rsslMsg.domainType() == 0)
			genericMsgImpl._rsslMsg.domainType(itemInfo.domainType());
		
		if( !submit(genericMsgImpl, itemInfo.clientSession()) )
		{
			return;
		}
		
		userLock().unlock();
	}

	@Override
	public void submit(RefreshMsg refreshMsg, long handle)
	{
		userLock().lock();
		
		RefreshMsgImpl refreshMsgImpl = (RefreshMsgImpl)refreshMsg;
		com.thomsonreuters.upa.codec.RefreshMsg rsslRefreshMsg = (com.thomsonreuters.upa.codec.RefreshMsg )refreshMsgImpl._rsslMsg;
		
		ItemInfo itemInfo = getItemInfo(handle);
		
		if( ( itemInfo == null ) && (handle != 0 ) )
		{
			userLock().unlock();
			StringBuilder temp = strBuilder();
			temp.append("Attempt to submit RefreshMsg with non existent Handle = ")
			.append(handle).append(".");
			handleInvalidUsage(temp.toString());
			return;
		}
		
		ClientSession clientSession = null;
		
		if ( refreshMsgImpl.domainType() == EmaRdm.MMT_LOGIN )
		{
			if( handle == 0 )
			{
				StringBuilder text = strBuilder();
				text.append("Fanout login message for item handle = ");
				
				if (!submit(refreshMsgImpl, _loginHandler.getItemInfoList(), text, false) )
				{
					return;
				}
				
				userLock().unlock();
				return;
			}
			else
			{
				clientSession = itemInfo.clientSession();
				refreshMsgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
			
				if ( ( rsslRefreshMsg.state().streamState() == StreamStates.OPEN) && ( rsslRefreshMsg.state().dataState() == DataStates.OK) )
				{
					itemInfo.clientSession().isLogin(true);
				}
			}
		}
		else if ( refreshMsgImpl.domainType() == EmaRdm.MMT_DIRECTORY )
		{
			if ( refreshMsgImpl._rsslMsg.containerType() != com.thomsonreuters.upa.codec.DataTypes.MAP )
			{
				userLock().unlock();
				handleInvalidUsage(strBuilder().append("Attempt to submit RefreshMsg with directory domain using container with wrong data type. Expected container data type is Map. Passed in is ")
						.append(DataType.asString(refreshMsgImpl.payload().dataType())).toString());
				return;
			}
			
			if ( !_ommIProviderDirectoryStore.decodeSourceDirectory(refreshMsgImpl._rsslMsg, strBuilder()) )
			{
				userLock().unlock();
				handleInvalidUsage(_strBuilder.toString());
				return;
			}
			
			clientSession = handle != 0 ? itemInfo.clientSession() : null;
			
			if ( !_ommIProviderDirectoryStore.submitSourceDirectory(clientSession, refreshMsgImpl._rsslMsg, strBuilder(), _storeUserSubmitted) )
			{
				userLock().unlock();
				StringBuilder text = new StringBuilder();
				text.append("Attempt to submit invalid directory domain message.").append(OmmLoggerClient.CR)
				.append("Reason = ").append(_strBuilder);
				handleInvalidUsage(text.toString());
				return;
			}
			
			if( handle == 0 )
			{
				StringBuilder text = strBuilder();
				text.append("Fanout directory message for item handle = ");
				
				if (!submit(refreshMsgImpl, _directoryHandler.getItemInfoList(), text, true) )
				{
					return;
				}
				
				userLock().unlock();
				return;
			}
			else
			{
				clientSession = itemInfo.clientSession();
				refreshMsgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
			}
		}
		else if (refreshMsgImpl.domainType() == EmaRdm.MMT_DICTIONARY)
		{
			if ( refreshMsgImpl._rsslMsg.containerType() != com.thomsonreuters.upa.codec.DataTypes.SERIES )
			{
				userLock().unlock();
				handleInvalidUsage(strBuilder().append("Attempt to submit RefreshMsg with dictionary domain using container with wrong data type. Expected container data type is Series. Passed in is ")
						.append(DataType.asString(refreshMsgImpl.payload().dataType())).toString());
				return;
			}
			
			if ( refreshMsgImpl.hasServiceName() )
			{
				if ( encodeServiceIdFromName(refreshMsgImpl.serviceName(), refreshMsgImpl._rsslMsg) )
				{
					refreshMsgImpl._rsslMsg.flags( refreshMsgImpl._rsslMsg.flags() | com.thomsonreuters.upa.codec.RefreshMsgFlags.HAS_MSG_KEY );
				}
				else
				{
					return;
				}
			} 
			else if ( refreshMsgImpl.hasServiceId() )
			{
				if ( validateServiceId(refreshMsgImpl.serviceId(), refreshMsgImpl._rsslMsg ) == false )
				{
					return;
				}
			}
			
			if( handle == 0 )
			{
				StringBuilder text = strBuilder();
				text.append("Fanout dictionary message for item handle = ");
				
				if (!submit(refreshMsgImpl, _dictionaryHandler.getItemInfoList(), text, false) )
				{
					return;
				}
				
				userLock().unlock();
				return;
			}
			else
			{
				clientSession = itemInfo.clientSession();
				refreshMsgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
			}
		}
		else
		{
			if( handle == 0 )
			{
				userLock().unlock();
				StringBuilder temp = strBuilder();
				temp.append("Attempt to fanout RefreshMsg with domain type ")
				.append(Utilities.rdmDomainAsString(refreshMsgImpl.domainType())).append(".");
				handleInvalidUsage(temp.toString());
				return;
			}
			
			if (loggerClient().isTraceEnabled())
        	{
				StringBuilder text = strBuilder();
				text.append("Received RefreshMsg with domain type ")
				.append(Utilities.rdmDomainAsString(refreshMsgImpl.domainType()))
				.append("; handle = ").append(handle).append(", user assigned streamId = ")
				.append(refreshMsgImpl.streamId()).append(".");
				
				loggerClient().trace(formatLogMessage(instanceName(),text.toString(), Severity.TRACE));
        	}
			
			if ( refreshMsgImpl.hasServiceName() )
			{
				if ( encodeServiceIdFromName(refreshMsgImpl.serviceName(), refreshMsgImpl._rsslMsg) )
				{
					refreshMsgImpl._rsslMsg.flags( refreshMsgImpl._rsslMsg.flags() | com.thomsonreuters.upa.codec.RefreshMsgFlags.HAS_MSG_KEY );
				}
				else
				{
					return;
				}
			}
			else if ( refreshMsgImpl.hasServiceId() )
			{
				if ( validateServiceId(refreshMsgImpl.serviceId(), refreshMsgImpl._rsslMsg ) == false )
				{
					return;
				}
			}
			
			clientSession = itemInfo.clientSession();
			refreshMsgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
			
			handleItemGroup( itemInfo, rsslRefreshMsg.groupId(), rsslRefreshMsg.state() );
			
			if ( itemInfo.isPrivateStream() )
			{
		        ((com.thomsonreuters.upa.codec.RefreshMsg)refreshMsgImpl._rsslMsg).applyPrivateStream();
			}
		}
		
		if( !submit(refreshMsgImpl, clientSession) )
		{
			return;
		}
		
		itemInfo.setSentRefresh();
		
		handleItemInfo(rsslRefreshMsg.domainType(), handle, rsslRefreshMsg.state(), rsslRefreshMsg.checkRefreshComplete());
		
		userLock().unlock();
	}
	
	@Override
	public void submit(UpdateMsg updateMsg, long handle)
	{
		userLock().lock();
		
		UpdateMsgImpl updateMsgImpl = (UpdateMsgImpl)updateMsg;
		
		ItemInfo itemInfo = getItemInfo(handle);
		
		if( itemInfo == null && handle != 0 )
		{
			userLock().unlock();
			StringBuilder temp = strBuilder();
			temp.append("Attempt to submit UpdateMsg with non existent Handle = ")
			.append(handle).append(".");
			handleInvalidUsage(temp.toString());
			return;
		}
		
		ClientSession clientSession = null;
		
		if ( updateMsgImpl.domainType() == EmaRdm.MMT_LOGIN )
		{
			userLock().unlock();
			StringBuilder temp = strBuilder();
			temp.append("Attempt to submit UpdateMsg with login domain while this is not supported.");
			handleInvalidUsage(temp.toString());
			return;
		}
		else if ( updateMsgImpl.domainType() == EmaRdm.MMT_DIRECTORY )
		{
			if(_activeConfig.refreshFirstRequired)
			{
				if(itemInfo != null  && !itemInfo.isSentRefresh())
				{
					userLock().unlock();
					StringBuilder temp = strBuilder();
					temp.append("Attempt to submit UpdateMsg with SourceDirectory while RefreshMsg was not submitted on this stream yet. Handle = ");
					temp.append(itemInfo.handle().value());
					handleInvalidUsage(temp.toString());
					return;
				}
			}
			
			if ( updateMsgImpl._rsslMsg.containerType() != com.thomsonreuters.upa.codec.DataTypes.MAP )
			{
				userLock().unlock();
				handleInvalidUsage(strBuilder().append("Attempt to submit UpdateMsg with directory domain using container with wrong data type. Expected container data type is Map. Passed in is ")
						.append(DataType.asString(updateMsgImpl.payload().dataType())).toString());
				return;
			}
			
			if ( !_ommIProviderDirectoryStore.decodeSourceDirectory(updateMsgImpl._rsslMsg, strBuilder()) )
			{
				userLock().unlock();
				handleInvalidUsage(_strBuilder.toString());
				return;
			}
			
			clientSession = handle != 0 ? itemInfo.clientSession() : null;
			
			if ( !_ommIProviderDirectoryStore.submitSourceDirectory(clientSession, updateMsgImpl._rsslMsg, strBuilder(), _storeUserSubmitted ) )
			{
				userLock().unlock();
				StringBuilder text = new StringBuilder();
				text.append("Attempt to submit invalid directory domain message.").append(OmmLoggerClient.CR)
				.append("Reason = ").append(_strBuilder);
				handleInvalidUsage(text.toString());
				return;
			}
			
			if( handle == 0 )
			{
				StringBuilder text = strBuilder();
				text.append("Fanout directory message for item handle = ");
				
				if (!submit(updateMsgImpl, _directoryHandler.getItemInfoList(), text, true) )
				{
					return;
				}
				
				userLock().unlock();
				return;
			}
			else
			{
				clientSession = itemInfo.clientSession();
				updateMsgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
			}
		}
		else if (updateMsgImpl.domainType() == EmaRdm.MMT_DICTIONARY)
		{
			userLock().unlock();
			StringBuilder temp = strBuilder();
			temp.append("Attempt to submit UpdateMsg with dictionary domain while this is not supported.");
			handleInvalidUsage(temp.toString());
			return;
		}
		else
		{
			if( handle == 0 )
			{
				userLock().unlock();
				StringBuilder temp = strBuilder();
				temp.append("Attempt to fanout UpdateMsg with domain type ")
				.append(Utilities.rdmDomainAsString(updateMsgImpl.domainType())).append(".");
				handleInvalidUsage(temp.toString());
				return;
			}
			
			if (loggerClient().isTraceEnabled())
        	{
				StringBuilder text = strBuilder();
				text.append("Received UpdateMsg with domain type ")
				.append(Utilities.rdmDomainAsString(updateMsgImpl.domainType()))
				.append("; handle = ").append(handle).append(", user assigned streamId = ")
				.append(updateMsgImpl.streamId()).append(".");
				
				loggerClient().trace(formatLogMessage(instanceName(),text.toString(), Severity.TRACE));
        	}
			
			if ( updateMsgImpl.hasServiceName() )
			{
				if ( encodeServiceIdFromName(updateMsgImpl.serviceName(), updateMsgImpl._rsslMsg) )
				{
					updateMsgImpl._rsslMsg.flags( updateMsgImpl._rsslMsg.flags() | com.thomsonreuters.upa.codec.UpdateMsgFlags.HAS_MSG_KEY );
				}
				else
				{
					return;
				}
			}
			else if ( updateMsgImpl.hasServiceId() )
			{
				if ( validateServiceId(updateMsgImpl.serviceId(), updateMsgImpl._rsslMsg ) == false )
				{
					return;
				}
			}
			
			if(_activeConfig.refreshFirstRequired && !itemInfo.isSentRefresh())
			{
				userLock().unlock();
				StringBuilder temp = strBuilder();
				temp.append("Attempt to submit UpdateMsg while RefreshMsg was not submitted on this stream yet. Handle = ");
				temp.append(itemInfo.handle().value());
				handleInvalidUsage(temp.toString());
				return;
			}
			
			clientSession = itemInfo.clientSession();
			updateMsgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
		}
		
		if( !submit(updateMsgImpl, clientSession) )
		{
			return;
		}
		
		userLock().unlock();
	}

	@Override
	public void submit(StatusMsg statusMsg, long handle)
	{
		userLock().lock();
		
		StatusMsgImpl statusMsgImpl = (StatusMsgImpl)statusMsg;
		
		ItemInfo itemInfo = getItemInfo(handle);
		
		if( itemInfo == null && handle != 0 )
		{
			userLock().unlock();
			StringBuilder temp = strBuilder();
			temp.append("Attempt to submit StatusMsg with non existent Handle = ")
			.append(handle).append(".");
			handleInvalidUsage(temp.toString());
			return;
		}
		
		ClientSession clientSession = null;
		
		if ( statusMsgImpl.domainType() == EmaRdm.MMT_LOGIN )
		{
			if( handle == 0 )
			{
				StringBuilder text = strBuilder();
				text.append("Fanout login message for item handle = ");
				
				if (!submit(statusMsgImpl, _loginHandler.getItemInfoList(), text, false) )
				{
					return;
				}
				
				userLock().unlock();
				return;
			}
			else
			{
				clientSession = itemInfo.clientSession();
				statusMsgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
			}
		}
		else if ( statusMsgImpl.domainType() == EmaRdm.MMT_DIRECTORY )
		{
			if( handle == 0 )
			{
				StringBuilder text = strBuilder();
				text.append("Fanout directory message for item handle = ");
				
				if (!submit(statusMsgImpl, _directoryHandler.getItemInfoList(), text, false) )
				{
					return;
				}
				
				userLock().unlock();
				return;
			}
			else
			{
				clientSession = itemInfo.clientSession();
				statusMsgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
			}
		}
		else if ( statusMsgImpl.domainType() == EmaRdm.MMT_DICTIONARY )
		{
			if ( statusMsgImpl.hasServiceName() )
			{
				if ( encodeServiceIdFromName(statusMsgImpl.serviceName(), statusMsgImpl._rsslMsg) )
				{
					statusMsgImpl._rsslMsg.flags( statusMsgImpl._rsslMsg.flags() | com.thomsonreuters.upa.codec.StatusMsgFlags.HAS_MSG_KEY );
				}
				else
				{
					return;
				}
			}
			else if ( statusMsgImpl.hasServiceId() )
			{
				if ( validateServiceId(statusMsgImpl.serviceId(), statusMsgImpl._rsslMsg ) == false )
				{
					return;
				}
			}
			
			if( handle == 0 )
			{
				StringBuilder text = strBuilder();
				text.append("Fanout dictionary message for item handle = ");
				
				if (!submit(statusMsgImpl, _dictionaryHandler.getItemInfoList(), text, false) )
				{
					return;
				}
				
				userLock().unlock();
				return;
			}
			else
			{
				clientSession = itemInfo.clientSession();
				statusMsgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
			}
		}
		else
		{
			if( handle == 0 )
			{
				userLock().unlock();
				StringBuilder temp = strBuilder();
				temp.append("Attempt to fanout StatusMsg with domain type ")
				.append(Utilities.rdmDomainAsString(statusMsgImpl.domainType())).append(".");
				handleInvalidUsage(temp.toString());
				return;
			}
			
			if (loggerClient().isTraceEnabled())
        	{
				StringBuilder text = strBuilder();
				text.append("Received StatusMsg with domain type ")
				.append(Utilities.rdmDomainAsString(statusMsgImpl.domainType()))
				.append("; handle = ").append(handle).append(", user assigned streamId = ")
				.append(statusMsgImpl.streamId()).append(".");
				
				loggerClient().trace(formatLogMessage(instanceName(),text.toString(), Severity.TRACE));
        	}
			
			if ( statusMsgImpl.hasServiceName() )
			{
				if ( encodeServiceIdFromName(statusMsgImpl.serviceName(), statusMsgImpl._rsslMsg) )
				{
					statusMsgImpl._rsslMsg.flags( statusMsgImpl._rsslMsg.flags() | com.thomsonreuters.upa.codec.StatusMsgFlags.HAS_MSG_KEY );
				}
				else
				{
					return;
				}
			}
			else if ( statusMsgImpl.hasServiceId() )
			{
				if ( validateServiceId(statusMsgImpl.serviceId(), statusMsgImpl._rsslMsg ) == false )
				{
					return;
				}
			}
			
			clientSession = itemInfo.clientSession();
			statusMsgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
			
			com.thomsonreuters.upa.codec.StatusMsg rsslStatusMsg =  (com.thomsonreuters.upa.codec.StatusMsg)statusMsgImpl._rsslMsg;
			
			if ( rsslStatusMsg.checkHasGroupId() )
			{
				handleItemGroup( itemInfo, rsslStatusMsg.groupId(), rsslStatusMsg.state() );
			}
			
			if ( itemInfo.isPrivateStream() )
			{
		        ((com.thomsonreuters.upa.codec.StatusMsg)statusMsgImpl._rsslMsg).applyPrivateStream();
			}
		}
		
		if( !submit(statusMsgImpl, clientSession) )
		{
			return;
		}
		
		handleItemInfo(statusMsgImpl.domainType(), handle, ((com.thomsonreuters.upa.codec.StatusMsg )statusMsgImpl._rsslMsg).state(), false);
		
		userLock().unlock();
	}
	
	void handleItemInfo(int domainType, long handle, State state, boolean refreshComplete )
	{
		if  ( ( state.streamState() == StreamStates.CLOSED ) || ( state.streamState() == StreamStates.CLOSED_RECOVER ) || 
				( state.streamState() == StreamStates.REDIRECTED ) ||  ( state.streamState() == StreamStates.NON_STREAMING &&  refreshComplete ) )
		{
			ItemInfo itemInfo = getItemInfo(handle);
			
			if( itemInfo != null )
			{
				switch(domainType)
				{
					case EmaRdm.MMT_LOGIN:
					{
						_serverChannelHandler.closeChannel(itemInfo.clientSession().channel());
						_itemWatchList.processCloseLogin(itemInfo.clientSession());
					}
					break;
					case EmaRdm.MMT_DIRECTORY:
					{
						_directoryHandler.getItemInfoList().remove(itemInfo);
						removeItemInfo(itemInfo, false);
					}
					break;
					case EmaRdm.MMT_DICTIONARY:
					{
						_dictionaryHandler.getItemInfoList().remove(itemInfo);
						removeItemInfo(itemInfo, false);
					}
					break;
					default:
					{
						removeItemInfo(itemInfo, true);
					}
					break;
				}
			}
		}
	}

	boolean submit(MsgImpl msgImpl, List<ItemInfo> itemInfoList, StringBuilder text, boolean applyDirectoryFilter)
	{
		ItemInfo itemInfo;
		
		for( int index = 0; index < itemInfoList.size(); index++ )
		{
			itemInfo = itemInfoList.get(index);
			
			if (loggerClient().isTraceEnabled())
        	{
				text.append(itemInfo.handle().value()).append(", client handle = ")
				.append(itemInfo.clientSession().clientHandle().value()).append(".");
				
				loggerClient().error(formatLogMessage(instanceName() , _strBuilder.toString(), Severity.TRACE));
        	}
			
			msgImpl._rsslMsg.streamId((int)itemInfo.streamId().value());
			
			if( applyDirectoryFilter )
			{
				MsgBase rdmMsgBase = null;
				DirectoryMsg directoryMsg = _ommIProviderDirectoryStore.getSubmittedDirectoryMsg();
				
				switch (directoryMsg.rdmMsgType())
				{
					case REFRESH:
					{
						DirectoryRefresh directoryRefresh = (DirectoryRefresh)directoryMsg;
						
						DirectoryRefresh fanoutDirectoryRefresh = (DirectoryRefresh)_fanoutDirectoryMsg;
						fanoutDirectoryRefresh.clear();
						fanoutDirectoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
						
						
						long filter = DirectoryServiceStore.encodeDirectoryMsg(directoryRefresh.serviceList() , fanoutDirectoryRefresh.serviceList(),
								itemInfo.msgKey().filter(), false, itemInfo.msgKey().checkHasServiceId(), itemInfo.msgKey().serviceId());
						
						fanoutDirectoryRefresh.filter(filter);
						fanoutDirectoryRefresh.streamId((int)itemInfo.streamId().value());
						fanoutDirectoryRefresh.state(directoryRefresh.state());
						
						if ( directoryRefresh.checkClearCache() )
						{
							fanoutDirectoryRefresh.applyClearCache();
						}
						
						if ( directoryRefresh.checkHasSequenceNumber())
						{
							fanoutDirectoryRefresh.applyHasSequenceNumber();
							fanoutDirectoryRefresh.sequenceNumber(directoryRefresh.sequenceNumber());
						}
						
						if ( directoryRefresh.checkHasServiceId())
						{
							fanoutDirectoryRefresh.applyHasServiceId();
							fanoutDirectoryRefresh.serviceId(directoryRefresh.serviceId());
						}
						
						if ( directoryRefresh.checkSolicited())
						{
							fanoutDirectoryRefresh.applySolicited();
						}
						
						rdmMsgBase = fanoutDirectoryRefresh;					
					}
						break;
					case UPDATE:
					{
						if (!_storeUserSubmitted && _activeConfig.refreshFirstRequired)
						{
							if (!itemInfo.isSentRefresh())
							{
								if (loggerClient().isWarnEnabled())
								{
									strBuilder().append("Skip sending source directory update message for handle ")
									.append(itemInfo.handle().value()).append(", client handle ")
									.append(itemInfo.clientSession().clientHandle().value()).append(" as refresh message is required first.");
							
									loggerClient().error(formatLogMessage(instanceName() , _strBuilder.toString(), Severity.WARNING));
								}

								continue;
							}
						}
						
						DirectoryUpdate directoryUpdate = (DirectoryUpdate)directoryMsg;
						
						DirectoryUpdate fanoutDirectoryUpdate = (DirectoryUpdate)_fanoutDirectoryMsg;
						fanoutDirectoryUpdate.clear();
						fanoutDirectoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
						
						long filter = DirectoryServiceStore.encodeDirectoryMsg(directoryUpdate.serviceList() , fanoutDirectoryUpdate.serviceList(),
								itemInfo.msgKey().filter(), false, itemInfo.msgKey().checkHasServiceId(), itemInfo.msgKey().serviceId());
						
						fanoutDirectoryUpdate.streamId((int)itemInfo.streamId().value());
					
						if ( directoryUpdate.checkHasFilter() && ( filter != 0 ) )
						{
							fanoutDirectoryUpdate.applyHasFilter();
							fanoutDirectoryUpdate.filter(filter);
						}
						
						if ( directoryUpdate.checkHasSequenceNumber())
						{
							fanoutDirectoryUpdate.applyHasSequenceNumber();
							fanoutDirectoryUpdate.sequenceNumber(directoryUpdate.sequenceNumber());
						}
						
						if ( directoryUpdate.checkHasServiceId())
						{
							fanoutDirectoryUpdate.applyHasServiceId();
							fanoutDirectoryUpdate.serviceId(directoryUpdate.serviceId());
						}
						
						rdmMsgBase = fanoutDirectoryUpdate;
					}
						break;
				default:
					break;
				}
				
				if ( rdmMsgBase != null )
				{
					_rsslErrorInfo.clear();
					
					int ret = itemInfo.clientSession().channel().submit(rdmMsgBase, _rsslSubmitOptions, _rsslErrorInfo);
					
					if (ReactorReturnCodes.SUCCESS > ret )
				    {			
						if (loggerClient().isErrorEnabled())
			        	{
							com.thomsonreuters.upa.transport.Error error = _rsslErrorInfo.error();
							
				        	strBuilder().append("Internal error: rsslChannel.submit() failed in OmmProviderImpl.submit(")
				        		.append(DataType.asString(msgImpl.dataType())).append(")").append(OmmLoggerClient.CR)
				        		.append("Client handle ").append(itemInfo.clientSession().clientHandle().value()).append(OmmLoggerClient.CR)
				    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
				    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
				    			.append("Error Location ").append(_rsslErrorInfo.location()).append(OmmLoggerClient.CR)
				    			.append("Error Text ").append(error.text());
				        	
				        	loggerClient().error(formatLogMessage(instanceName() , _strBuilder.toString(), Severity.ERROR));
			        	}
						
						userLock().unlock();
						strBuilder().append("Failed to submit ")
							.append(DataType.asString(msgImpl.dataType())).append(". Reason: ")
							.append(ReactorReturnCodes.toString(ret))
							.append(". Error text: ")
							.append(_rsslErrorInfo.error().text());
						
						handleInvalidUsage(_strBuilder.toString());
						return false;
				    }
				}
			}
			else
			{
				if (!_storeUserSubmitted && _activeConfig.refreshFirstRequired && ( msgImpl._rsslMsg.msgClass() == MsgClasses.UPDATE))
				{
					if (!itemInfo.isSentRefresh() )
					{
						if (loggerClient().isWarnEnabled())
						{
							strBuilder().append("Skip sending update message for handle ")
							.append(itemInfo.handle().value()).append(", client handle ")
							.append(itemInfo.clientSession().clientHandle().value()).append(" as refresh message is required first.");
					
							loggerClient().error(formatLogMessage(instanceName() , _strBuilder.toString(), Severity.WARNING));
						}

						continue;
					}
				}
				
				if( !submit(msgImpl, itemInfo.clientSession()) )
				{
					return false;
				}
			}
			
			switch (msgImpl._rsslMsg.msgClass())
			{
			case MsgClasses.REFRESH:
				itemInfo.setSentRefresh();
				com.thomsonreuters.upa.codec.RefreshMsg refreshMsg =  (com.thomsonreuters.upa.codec.RefreshMsg)msgImpl._rsslMsg;
				handleItemInfo(msgImpl._rsslMsg.domainType(), itemInfo.handle().value(), refreshMsg.state(), refreshMsg.checkRefreshComplete());
				break;
			case MsgClasses.STATUS:
				handleItemInfo(msgImpl._rsslMsg.domainType(),  itemInfo.handle().value(),((com.thomsonreuters.upa.codec.StatusMsg)msgImpl._rsslMsg).state(), false);
				break;
			default:
				break;
			}
		}
		
		return true;
	}
	
	boolean submit(MsgImpl msgImpl, ClientSession clientSession)
	{
		_rsslErrorInfo.clear();
		int ret;
		if (ReactorReturnCodes.SUCCESS > (ret = clientSession.channel().submit(msgImpl._rsslMsg, _rsslSubmitOptions, _rsslErrorInfo)))
	    {			
			if (loggerClient().isErrorEnabled())
        	{
				com.thomsonreuters.upa.transport.Error error = _rsslErrorInfo.error();
				
	        	strBuilder().append("Internal error: rsslChannel.submit() failed in OmmProviderImpl.submit(")
	        		.append(DataType.asString(msgImpl.dataType())).append(")").append(OmmLoggerClient.CR)
	        		.append("Client handle ").append(clientSession.clientHandle().value()).append(OmmLoggerClient.CR)
	    			.append("Error Id ").append(error.errorId()).append(OmmLoggerClient.CR)
	    			.append("Internal sysError ").append(error.sysError()).append(OmmLoggerClient.CR)
	    			.append("Error Location ").append(_rsslErrorInfo.location()).append(OmmLoggerClient.CR)
	    			.append("Error Text ").append(error.text());
	        	
	        	loggerClient().error(formatLogMessage(instanceName() , _strBuilder.toString(), Severity.ERROR));
        	}
			
			userLock().unlock();
			strBuilder().append("Failed to submit ")
				.append(DataType.asString(msgImpl.dataType())).append(". Reason: ")
				.append(ReactorReturnCodes.toString(ret))
				.append(". Error text: ")
				.append(_rsslErrorInfo.error().text());
			
			handleInvalidUsage(_strBuilder.toString());
			return false;
	    }
		
		return true;
	}
	
	void handleItemGroup(ItemInfo itemInfo, Buffer groupId, State state)
	{
		if ( ( groupId.length() < 2 ) || ( groupId.data().get(0) == 0 && groupId.data().get(1) == 0 ) 
				|| !itemInfo.msgKey().checkHasServiceId() )
		{
			return;
		}
		
		if( itemInfo.hasItemGroup() )
		{
			if ( !groupId.equals(itemInfo.itemGroup() ) )
			{
				updateItemGroup(itemInfo, groupId);
				itemInfo.itemGroup(groupId);
			}
		}
		else
		{
			itemInfo.itemGroup(groupId);
			addItemGroup(itemInfo, groupId);
		}
	}
	
	boolean encodeServiceIdFromName(String serviceName, com.thomsonreuters.upa.codec.Msg rsslMsg)
	{	
		ServiceIdInteger serviceId = _ommIProviderDirectoryStore.serviceId(serviceName);
		
		if ( serviceId == null )
		{
			userLock().unlock();
			strBuilder().append("Attempt to submit ").append(DataType.asString(Utilities.toEmaMsgClass[rsslMsg.msgClass()])).
			append(" with service name of ").append(serviceName).append(" that was not included in the SourceDirectory. Dropping this ").
			append(DataType.asString(Utilities.toEmaMsgClass[rsslMsg.msgClass()])).append(".");
			handleInvalidUsage(_strBuilder.toString());
			return false;
		}
		else if ( serviceId.value() > 65535)
		{
			userLock().unlock();
			strBuilder().append("Attempt to submit ").append(DataType.asString(Utilities.toEmaMsgClass[rsslMsg.msgClass()])).
			append(" with service name of ").append(serviceName).append(" whose matching service id of ").append(serviceId.value()).
			append(" is out of range. Dropping this ").append(DataType.asString(Utilities.toEmaMsgClass[rsslMsg.msgClass()])).append(".");
			handleInvalidUsage(_strBuilder.toString());
			return false;
		}
		
		rsslMsg.msgKey().serviceId(serviceId.value());
		rsslMsg.msgKey().applyHasServiceId();
				
		return true;
	}
	
	boolean validateServiceId(int serviceId, com.thomsonreuters.upa.codec.Msg rsslMsg)
	{	
		String serviceName = _ommIProviderDirectoryStore.serviceName(serviceId);
		
		if ( serviceName == null )
		{
			userLock().unlock();
			strBuilder().append("Attempt to submit ").append(DataType.asString(Utilities.toEmaMsgClass[rsslMsg.msgClass()])).
			append(" with service Id of ").append(serviceId).append(" that was not included in the SourceDirectory. Dropping this ").
			append(DataType.asString(Utilities.toEmaMsgClass[rsslMsg.msgClass()])).append(".");
			handleInvalidUsage(_strBuilder.toString());
			return false;
		}
		else if ( serviceId > 65535)
		{
			userLock().unlock();
			strBuilder().append("Attempt to submit ").append(DataType.asString(Utilities.toEmaMsgClass[rsslMsg.msgClass()])).
			append(" with service Id of ").append(serviceId).append(" is out of range. Dropping this ").
			append(DataType.asString(Utilities.toEmaMsgClass[rsslMsg.msgClass()])).append(".");
			handleInvalidUsage(_strBuilder.toString());
			return false;
		}
		
		return true;
	}

	@Override
	public void submit(AckMsg ackMsg, long handle)
	{
		throw new UnsupportedOperationException("Calling the OmmIProviderImpl.submit(AckMsg ackMsg, long handle) method is not support in this release.");	
	}

	@Override
	public long dispatch()
	{
		return super.dispatch();
	}

	@Override
	public long dispatch(long timeOut)
	{
		return super.dispatch(timeOut);
	}

	@Override
	public void unregister(long handle)
	{
		_itemCallbackClient.unregister(handle);
	}

	@Override
	public void uninitialize()
	{
		super.uninitialize();
	}

	@Override
	Logger createLoggerClient()
	{
		return LoggerFactory.getLogger(OmmIProviderImpl.class);
	}

	@Override
	ConfigAttributes getAttributes(EmaConfigServerImpl config)
	{
		return config.xmlConfig().getIProviderAttributes(_activeConfig.configuredName);
	}

	@Override
	Object getAttributeValue(EmaConfigServerImpl config, int AttributeKey)
	{
		return config.xmlConfig().getIProviderAttributeValue(_activeConfig.configuredName, AttributeKey);
	}

	@Override
	public String formatLogMessage(String clientName, String text, int level)
	{
		return strBuilder().append("loggerMsg\n").append("    ClientName: ").append(clientName).append("\n")
        .append("    Severity: ").append(OmmLoggerClient.loggerSeverityAsString(level)).append("\n")
        .append("    Text:    ").append(text).append("\n").append("loggerMsgEnd\n\n").toString();
	}

	@Override
	OmmProvider provider()
	{
		return this;
	}

	@Override
	public String instanceName()
	{
		return _activeConfig.instanceName;
	}

	@Override
	DirectoryServiceStore directoryServiceStore()
	{
		return _ommIProviderDirectoryStore;
	}

	@Override
	public void onServiceDelete(ClientSession clientSession,int serviceId)
	{
		if ( clientSession != null )
		{
			removeServiceId(clientSession, serviceId);
			_itemWatchList.processServiceDelete(clientSession, serviceId);
		}
		else
		{
			if ( _directoryHandler != null )
			{
				List<ItemInfo> itemInfoList = _directoryHandler.getItemInfoList();
					
				for(int itemInfoIndex = 0; itemInfoIndex < itemInfoList.size(); itemInfoIndex++ )
				{
					clientSession = itemInfoList.get(itemInfoIndex).clientSession();
					
					removeServiceId(clientSession, serviceId);
					_itemWatchList.processServiceDelete(clientSession, serviceId);
				}
			}
		}
	}

	@Override
	public void onServiceStateChange(ClientSession clientSession,int serviceId, ServiceState serviceState)
	{
		if ( serviceState.checkHasStatus() )
		{
			if( serviceState.status().streamState() == StreamStates.CLOSED_RECOVER)
			{
				if ( clientSession != null )
				{
					removeServiceId(clientSession, serviceId);
				}
				else
				{
					if ( _directoryHandler != null )
					{
						List<ItemInfo> itemInfoList = _directoryHandler.getItemInfoList();
						
						for(int itemInfoIndex = 0; itemInfoIndex < itemInfoList.size(); itemInfoIndex++ )
						{
							removeServiceId(itemInfoList.get(itemInfoIndex).clientSession(), serviceId);
						}
					}
				}
			}
		}
	}

	@Override
	public void onServiceGroupChange(ClientSession clientSession,int serviceId, List<ServiceGroup> serviceGroupList)
	{
		ServiceGroup serviceGroup;
		
		for(int index = 0; index < serviceGroupList.size(); index++ )
		{
			serviceGroup = serviceGroupList.get(index);
			
			if( serviceGroup.checkHasMergedToGroup() )
			{
				if ( clientSession != null )
				{
					mergeToGroupId(clientSession, serviceId, serviceGroup.group(), serviceGroup.mergedToGroup());
				}
				else
				{
					if ( _directoryHandler != null )
					{
						List<ItemInfo> itemInfoList = _directoryHandler.getItemInfoList();
						
						for(int itemInfoIndex = 0; itemInfoIndex < itemInfoList.size(); itemInfoIndex++ )
						{
							mergeToGroupId(itemInfoList.get(itemInfoIndex).clientSession(), serviceId, serviceGroup.group(), serviceGroup.mergedToGroup());
						}
					}
				}
			}
			
			if ( serviceGroup.checkHasStatus() )
			{
				if ( serviceGroup.status().streamState() == StreamStates.CLOSED_RECOVER )
				{
					if ( clientSession != null )
					{
						removeGroupId(clientSession, serviceId, serviceGroup.group());
					}
					else
					{
						if ( _directoryHandler != null )
						{
							List<ItemInfo> itemInfoList = _directoryHandler.getItemInfoList();
							for(int itemInfoIndex = 0; itemInfoIndex < itemInfoList.size(); itemInfoIndex++ )
							{
								removeGroupId(itemInfoList.get(itemInfoIndex).clientSession(), serviceId, serviceGroup.group());
							}
						}
					}
				}
			}
		}
	}

	@Override
	public EmaObjectManager objManager() {
		return _objManager;
	}

	@Override
	public int implType() {
		return OmmCommonImpl.ImplementationType.IPROVIDER;
	}

	@Override
	public long nextLongId() {
		
		long id = _longId;
		
		while( getItemInfo(id) != null && itemCallbackClient().getItem(id) != null )
		{
			if ( _longId == MAX_LONG_VALUE )
			{
				_longId = MIN_LONG_VALUE;
			}
			else
			{
				id = ++_longId;
			}
		}
		
		++_longId;
		
		return id;
	}
	
	ItemWatchList itemWatchList() {
		return _itemWatchList;
	}
	
	int requestTimeout() {
		return _activeConfig.requestTimeout;
	}
	
	@Override
	void processChannelEvent( ReactorChannelEvent reactorChannelEvent)
	{
		switch ( reactorChannelEvent.eventType() )
		{
		case ReactorChannelEventTypes.CHANNEL_DOWN:
		case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
			
			if ( _itemWatchList != null )
				_itemWatchList.processChannelEvent(reactorChannelEvent);
			
			break;
		default:
			break;
		}
	}
}
