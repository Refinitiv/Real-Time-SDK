/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2016-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmProvider.h"
#include "OmmNiProviderImpl.h"
#include "OmmNiProviderConfigImpl.h"
#include "ExceptionTranslator.h"
#include "LoginCallbackClient.h"
#include "RefreshMsgImpl.h"
#include "ReqMsgImpl.h"
#include "UpdateMsgImpl.h"
#include "GenericMsgImpl.h"
#include "Utilities.h"
#include "EmaRdm.h"
#include "OmmQosDecoder.h"
#include "StreamId.h"
#include "DirectoryServiceStore.h"
#include "ChannelInfoImpl.h"
#include "OmmInvalidUsageException.h"
#include "PackedMsgImpl.h"
#include "StatusMsgImpl.h"
#include "BaseRoutingChannel.h"
#include "BaseRoutingSession.h"
#include "NiProviderRoutingChannel.h"
#include "NiProviderRoutingSession.h"

#include <limits.h>

#ifdef WIN32
#pragma warning( disable : 4355)
#endif

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

static UInt32 _getMsgEncodedSize(RsslMsg* pMsg)
{
	UInt32 msgSize = 128;
	const RsslMsgKey* pKey;

	msgSize += pMsg->msgBase.encDataBody.length;

	if ((pKey = rsslGetMsgKey(pMsg)))
	{
		if (pKey->flags & RSSL_MKF_HAS_NAME)
			msgSize += pKey->name.length;

		if (pKey->flags & RSSL_MKF_HAS_ATTRIB)
			msgSize += pKey->encAttrib.length;
	}

	return msgSize;
}

OmmNiProviderImpl::OmmNiProviderImpl( OmmProvider* ommProvider, const OmmNiProviderConfig& config ) :
	_activeConfig(),
	OmmProviderImpl( ommProvider ),
	OmmBaseImpl( _activeConfig ),
	_handleToStreamInfo(),
	_streamInfoList(),
	_bIsStreamIdZeroRefreshSubmitted( false ),
	_ommNiProviderDirectoryStore(*this, _activeConfig),
	_nextProviderStreamId(0),
	_reusedProviderStreamIds(),
	_activeChannel(0),
	_itemWatchList()
{
	_activeConfig.operationModel = config._pImpl->getOperationModel();

	_activeConfig.directoryAdminControl = config.getConfigImpl()->getAdminControlDirectory();

	_ommNiProviderDirectoryStore.setClient(this);

	_encodeBuffer = RSSL_INIT_BUFFER;
	_encodeBufferAllocatedLength = 0;
	rsslClearEncodeIterator(&_encodeIterator);

	initialize( config._pImpl );

	_rsslDirectoryMsgBuffer.length = 2048;
	_rsslDirectoryMsgBuffer.data = (char*)malloc(_rsslDirectoryMsgBuffer.length * sizeof(char));
	if (!_rsslDirectoryMsgBuffer.data)
	{
		handleMee("Failed to allocate memory in OmmNiProviderImpl::OmmNiProviderImpl()");
		return;
	}
	rsslClearRDMDirectoryMsg(&_rsslDirectoryMsg);

	_handleToStreamInfo.rehash( _activeConfig.itemCountHint );
}

OmmNiProviderImpl::OmmNiProviderImpl(OmmProvider* ommProvider, const OmmNiProviderConfig& config, OmmProviderClient& adminClient, void* adminClosure) :
	_activeConfig(),
	OmmProviderImpl(ommProvider),
	OmmBaseImpl(_activeConfig, adminClient, adminClosure),
	_handleToStreamInfo(),
	_streamInfoList(),
	_bIsStreamIdZeroRefreshSubmitted(false),
	_ommNiProviderDirectoryStore(*this, _activeConfig),
	_nextProviderStreamId(0),
	_reusedProviderStreamIds(),
	_activeChannel(0),
	_itemWatchList()
{
	_activeConfig.operationModel = config._pImpl->getOperationModel();

	_activeConfig.directoryAdminControl = config.getConfigImpl()->getAdminControlDirectory();

	_ommNiProviderDirectoryStore.setClient(this);

	_encodeBuffer = RSSL_INIT_BUFFER;
	_encodeBufferAllocatedLength = 0;
	rsslClearEncodeIterator(&_encodeIterator);

	initialize(config._pImpl);

	_rsslDirectoryMsgBuffer.length = 2048;
	_rsslDirectoryMsgBuffer.data = (char*)malloc(_rsslDirectoryMsgBuffer.length * sizeof(char));
	if (!_rsslDirectoryMsgBuffer.data)
	{
		handleMee("Failed to allocate memory in OmmNiProviderImpl::OmmNiProviderImpl()");
		return;
	}

	rsslClearRDMDirectoryMsg(&_rsslDirectoryMsg);


	_handleToStreamInfo.rehash(_activeConfig.itemCountHint);
}

OmmNiProviderImpl::OmmNiProviderImpl(OmmProvider* ommProvider, const OmmNiProviderConfig& config, OmmProviderErrorClient& client) :
	_activeConfig(),
	OmmProviderImpl(ommProvider),
	OmmBaseImpl( _activeConfig, client ),
	_handleToStreamInfo(),
	_streamInfoList(),
	_bIsStreamIdZeroRefreshSubmitted( false ),
	_ommNiProviderDirectoryStore(*this, _activeConfig),
	_nextProviderStreamId(0),
	_reusedProviderStreamIds(),
	_activeChannel(0),
	_itemWatchList()
{
	_activeConfig.operationModel = config._pImpl->getOperationModel();

	_activeConfig.directoryAdminControl = config.getConfigImpl()->getAdminControlDirectory();

	_ommNiProviderDirectoryStore.setClient(this);

	_encodeBuffer = RSSL_INIT_BUFFER;
	_encodeBufferAllocatedLength = 0;
	rsslClearEncodeIterator(&_encodeIterator);

	initialize(config._pImpl);

	_rsslDirectoryMsgBuffer.length = 2048;
	_rsslDirectoryMsgBuffer.data = (char*)malloc(_rsslDirectoryMsgBuffer.length * sizeof(char));
	if (!_rsslDirectoryMsgBuffer.data)
	{
		handleMee("Failed to allocate memory in OmmNiProviderImpl::OmmNiProviderImpl()");
		return;
	}

	rsslClearRDMDirectoryMsg(&_rsslDirectoryMsg);

	_handleToStreamInfo.rehash( _activeConfig.itemCountHint );
}

OmmNiProviderImpl::OmmNiProviderImpl(OmmProvider* ommProvider, const OmmNiProviderConfig& config, OmmProviderClient& adminClient, OmmProviderErrorClient& client, void* adminClosure) :
	_activeConfig(),
	OmmProviderImpl(ommProvider),
	OmmBaseImpl(_activeConfig, adminClient, client, adminClosure),
	_handleToStreamInfo(),
	_streamInfoList(),
	_bIsStreamIdZeroRefreshSubmitted(false),
	_ommNiProviderDirectoryStore(*this, _activeConfig),
	_nextProviderStreamId(0),
	_reusedProviderStreamIds(),
	_activeChannel(0),
	_itemWatchList()
{
	_activeConfig.operationModel = config._pImpl->getOperationModel();

	_activeConfig.directoryAdminControl = config.getConfigImpl()->getAdminControlDirectory();

	_ommNiProviderDirectoryStore.setClient(this);

	_encodeBuffer = RSSL_INIT_BUFFER;
	_encodeBufferAllocatedLength = 0;
	rsslClearEncodeIterator(&_encodeIterator);

	initialize(config._pImpl);

	_rsslDirectoryMsgBuffer.length = 2048;
	_rsslDirectoryMsgBuffer.data = (char*)malloc(_rsslDirectoryMsgBuffer.length * sizeof(char));
	if (!_rsslDirectoryMsgBuffer.data)
	{
		handleMee("Failed to allocate memory in OmmNiProviderImpl::OmmNiProviderImpl()");
		return;
	}

	rsslClearRDMDirectoryMsg(&_rsslDirectoryMsg);

	_handleToStreamInfo.rehash(_activeConfig.itemCountHint);
}

//only for unit test, internal use
OmmNiProviderImpl::OmmNiProviderImpl(const OmmNiProviderConfig& config, OmmProviderClient& adminClient) :
	_activeConfig(),
	OmmProviderImpl(0),
	OmmBaseImpl(_activeConfig, adminClient, (void*)0),
	_handleToStreamInfo(),
	_streamInfoList(),
	_bIsStreamIdZeroRefreshSubmitted(false),
	_ommNiProviderDirectoryStore(*this, _activeConfig),
	_nextProviderStreamId(0),
	_reusedProviderStreamIds(),
	_activeChannel(0),
	_itemWatchList()
{
	_activeConfig.operationModel = config._pImpl->getOperationModel();

	_activeConfig.directoryAdminControl = config.getConfigImpl()->getAdminControlDirectory();

	_ommNiProviderDirectoryStore.setClient(this);

	_encodeBuffer = RSSL_INIT_BUFFER;
	_encodeBufferAllocatedLength = 0;
	rsslClearEncodeIterator(&_encodeIterator);

	_rsslDirectoryMsgBuffer.length = 2048;
	_rsslDirectoryMsgBuffer.data = (char*)malloc(_rsslDirectoryMsgBuffer.length * sizeof(char));
	if (!_rsslDirectoryMsgBuffer.data)
	{
		handleMee("Failed to allocate memory in OmmNiProviderImpl::OmmNiProviderImpl()");
		return;
	}

	rsslClearRDMDirectoryMsg(&_rsslDirectoryMsg);

	initializeForTest(config._pImpl);
}

OmmNiProviderImpl::~OmmNiProviderImpl()
{
	if (_activeConfig.pDirectoryRefreshMsg)
		delete _activeConfig.pDirectoryRefreshMsg;

	free(_rsslDirectoryMsgBuffer.data);

	if (_encodeBuffer.data != NULL)
	{
		free(_encodeBuffer.data);
	}

	removeItems();

	while (_reusedProviderStreamIds.size() != 0)
	{
		StreamId* tmp(_reusedProviderStreamIds.pop_back());
		delete tmp;
	}

	OmmBaseImplMap<OmmBaseImpl>::acquireCleanupLock();

	OmmBaseImpl::uninitialize( false, false );

	OmmBaseImplMap<OmmBaseImpl>::releaseCleanupLock();
}

void OmmNiProviderImpl::removeItems()
{
	_bIsStreamIdZeroRefreshSubmitted = false;

	_handleToStreamInfo.clear();

	_ommNiProviderDirectoryStore.clearServiceNamePair();

	for ( UInt32 idx = 0; idx < _streamInfoList.size(); ++idx )
		if ( _streamInfoList[idx] )
			delete _streamInfoList[idx];

	_streamInfoList.clear();
}

void OmmNiProviderImpl::readCustomConfig( EmaConfigImpl* pConfigImpl )
{
	_activeConfig.pDirectoryRefreshMsg = pConfigImpl->getDirectoryRefreshMsg();

	try
	{
		if ((_activeConfig.directoryAdminControl == OmmNiProviderConfig::ApiControlEnum) && _activeConfig.pDirectoryRefreshMsg)
		{
			RsslMsg rsslMsg;
			rsslMsg.refreshMsg = *_activeConfig.pDirectoryRefreshMsg->get();

			EmaString text;
			Int32 retCode;
			if (_ommNiProviderDirectoryStore.decodeSourceDirectory(&rsslMsg.refreshMsg.msgBase.encDataBody, text, retCode) == false)
			{
				handleIue(text, retCode);
				return;
			}

			if (_ommNiProviderDirectoryStore.submitSourceDirectory(0, &rsslMsg, _rsslDirectoryMsg, _rsslDirectoryMsgBuffer, false) == false)
			{
				return;
			}
		}
		else
		{
			_ommNiProviderDirectoryStore.loadConfigDirectory(pConfigImpl);
		}
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory in OmmNiProviderImpl::readCustomConfig()");
	}

	EmaString instanceNodeName( pConfigImpl->getInstanceNodeName() );
	instanceNodeName.append( _activeConfig.configuredName ).append( "|" );

	UInt64 tmp = 0;
	if ( pConfigImpl->get<UInt64>( instanceNodeName + "RefreshFirstRequired", tmp ) )
		_activeConfig.refreshFirstRequired = ( tmp > 0 ? true : false );

	tmp = 0;
	if ( pConfigImpl->get<UInt64>( instanceNodeName + "MergeSourceDirectoryStreams", tmp ) )
		_activeConfig.mergeSourceDirectoryStreams = ( tmp > 0 ? true : false );

	tmp = 0;
	if ( pConfigImpl->get<UInt64>( instanceNodeName + "RecoverUserSubmitSourceDirectory", tmp ) )
		_activeConfig.recoverUserSubmitSourceDirectory = ( tmp > 0 ? true : false );

	tmp = 0;
	if ( pConfigImpl->get<UInt64>( instanceNodeName + "RemoveItemsOnDisconnect", tmp ) )
		_activeConfig.removeItemsOnDisconnect = ( tmp > 0 ? true : false );

	_activeConfig.directoryAdminControl = static_cast<OmmNiProviderConfigImpl*>( pConfigImpl )->getAdminControlDirectory();

	if ( ProgrammaticConfigure* ppc = pConfigImpl->getProgrammaticConfigure() )
	{
		ppc->retrieveCustomConfig( _activeConfig.configuredName, _activeConfig );
	}

	// If a routing session is configured, alwyas set recoverUserSubmitSourceDirectory to true.
	if (_activeConfig.routingSessionSet.size() != 0)
	{
		_activeConfig.recoverUserSubmitSourceDirectory = true;
	}
}

const EmaString& OmmNiProviderImpl::getInstanceName() const
{
	return OmmBaseImpl::getInstanceName();
}

OmmProviderConfig::ProviderRole OmmNiProviderImpl::getProviderRole() const
{
	return OmmProviderConfig::NonInteractiveEnum;
}

OmmProvider* OmmNiProviderImpl::getProvider() const
{
	return _pOmmProvider;
}

void OmmNiProviderImpl::loadDirectory()
{
	if ( _activeConfig.directoryAdminControl == OmmNiProviderConfig::UserControlEnum )
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "DirectoryAdminControl = UserControl" );
		return;
	}

	if ( !_activeConfig.pDirectoryRefreshMsg )
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Using SourceDirectory configuration from EmaConfig.xml file & config( const Data& )" );

		RsslBuffer rsslMsgBuffer;
		rsslMsgBuffer.length = 4096;
		rsslMsgBuffer.data = (char*) malloc( sizeof( char ) * rsslMsgBuffer.length );

		if ( !rsslMsgBuffer.data )
		{
			handleMee( "Failed to allocate memory in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		RsslEncIterator eIter;
		rsslClearEncodeIterator( &eIter );
		RsslRet retCode;
		if ( RSSL_RET_SUCCESS != (retCode = rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION )) )
		{
			free( rsslMsgBuffer.data );
			handleIue( "Internal error. Failed to set encode iterator version in OmmNiProviderImpl::loadDirectory()", retCode );
			return;
		}

		if ( RSSL_RET_SUCCESS != (retCode = rsslSetEncodeIteratorBuffer( &eIter, &rsslMsgBuffer )) )
		{
			free( rsslMsgBuffer.data );
			handleIue( "Internal error. Failed to set encode iterator buffer in OmmNiProviderImpl::loadDirectory()", retCode );
			return;
		}

		RsslRDMDirectoryRefresh directoryRefresh;
		rsslClearRDMDirectoryRefresh( &directoryRefresh );

		directoryRefresh.flags = RDM_DR_RFF_CLEAR_CACHE;
		directoryRefresh.state.text.data = (char*)"Refresh Complete";
		directoryRefresh.state.text.length = 16;

		if (!DirectoryServiceStore::encodeDirectoryRefreshMsg(_ommNiProviderDirectoryStore.getApiControlDirectory(), directoryRefresh))
		{
			DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
			handleMee("Failed to allocate memory in OmmNiProviderImpl::loadDirectory()");
			return;
		}

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );
		retCode = rsslEncodeRDMDirectoryMsg( &eIter, (RsslRDMDirectoryMsg*) &directoryRefresh, &rsslMsgBuffer.length, &rsslErrorInfo );

		while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
		{
			free( rsslMsgBuffer.data );

			rsslMsgBuffer.length += rsslMsgBuffer.length;
			rsslMsgBuffer.data = (char*) malloc( sizeof( char ) * rsslMsgBuffer.length );

			if (RSSL_RET_SUCCESS != rsslSetEncodeIteratorBuffer(&eIter, &rsslMsgBuffer))
			{
				if (!rsslMsgBuffer.data)
					DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);

				handleMee("Internal error. Failed to set encode iterator buffer in OmmNiProviderImpl::loadDirectory()");
				return;
			}

			if ( !rsslMsgBuffer.data )
			{
				DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::loadDirectory()" );
				return;
			}

			clearRsslErrorInfo( &rsslErrorInfo );
			retCode = rsslEncodeRDMDirectoryMsg( &eIter, (RsslRDMDirectoryMsg*) &directoryRefresh, &rsslMsgBuffer.length, &rsslErrorInfo );
		}

		if ( retCode != RSSL_RET_SUCCESS )
		{
			DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);

			EmaString temp( "Internal error: failed to encode RsslRDMDirectoryMsg in OmmNiProviderImpl::loadDirectory()" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

			handleIue( temp, retCode );
			return;
		}

		try
		{
			_activeConfig.pDirectoryRefreshMsg = new AdminRefreshMsg( 0 );
		}
		catch ( std::bad_alloc& )
		{
			DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
			handleMee( "Failed to allocate memory in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		RsslDecodeIterator decIter;
		rsslClearDecodeIterator( &decIter );

		if ( (retCode = rsslSetDecodeIteratorRWFVersion( &decIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION )) != RSSL_RET_SUCCESS )
		{
			DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
			handleIue( "Internal error. Failed to set decode iterator version in OmmNiProviderImpl::loadDirectory()", retCode );
			return;
		}

		if ( (retCode = rsslSetDecodeIteratorBuffer( &decIter, &rsslMsgBuffer )) != RSSL_RET_SUCCESS )
		{
			DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
			handleIue( "Internal error. Failed to set decode iterator buffer in OmmNiProviderImpl::loadDirectory()", retCode );
			return;
		}

		RsslRefreshMsg rsslRefreshMsg;
		rsslClearRefreshMsg( &rsslRefreshMsg );
		if ( (retCode = rsslDecodeMsg( &decIter, (RsslMsg*) &rsslRefreshMsg )) != RSSL_RET_SUCCESS )
		{
			DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
			handleIue( "Internal error. Failed to decode message in OmmNiProviderImpl::loadDirectory()", retCode );
			return;
		}

		_activeConfig.pDirectoryRefreshMsg->set( &rsslRefreshMsg );

		DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
	}
	else
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Using SourceDirectory configuration from addAdminMsg( const RefreshMsg& )" );

		RsslMsg* pTempRsslMsg = (RsslMsg*) _activeConfig.pDirectoryRefreshMsg->get();

		if ( pTempRsslMsg->msgBase.containerType != RSSL_DT_MAP )
		{
			EmaString temp( "Attempt to submit RefreshMsg with SourceDirectory domain using container with wrong data type. Expected container data type is Map. Passed in is " );
			temp += DataType((DataType::DataTypeEnum)pTempRsslMsg->msgBase.containerType).toString();
			handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}
	}

	Channel* pChannel = static_cast<Channel*>(_pReactorChannel->userSpecPtr);

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*) _activeConfig.pDirectoryRefreshMsg->get();

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );

	if (_pRoutingSession == NULL)
	{
		if (rsslReactorSubmitMsg(_pRsslReactor, _pReactorChannel, &submitMsgOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::loadDirectory().");
			temp.append(CR).append(pChannel->toString()).append(CR)
				.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			handleIue(temp, rsslErrorInfo.rsslError.rsslErrorId);
			return;
		}
		else
		{
			_bIsStreamIdZeroRefreshSubmitted = true;

			if (OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
				getOmmLoggerClient().log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Configured source directory was sent out on the wire.");
		}
	}
	else
	{
		// Routing session configuration, so encode this buffer into _encodeBuffer and then memcpy it to fan out
		UInt32 msgSize = _getMsgEncodedSize(submitMsgOpts.pRsslMsg);
		NiProviderRoutingSession* pNiProvRoutingSession = static_cast<NiProviderRoutingSession*>(_pRoutingSession);

		int sentMsgCount = 0;


		for (UInt32 i = 0; i < pNiProvRoutingSession->routingChannelList.size(); i++)
		{
			NiProviderRoutingSessionChannel* pRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pNiProvRoutingSession->routingChannelList[i]);
			RsslReactorChannel* pReactorChannel = pRoutingChannel->pReactorChannel;
			
			// Do not submit the message if the channel is not active or has not gotten a login Open/OK.
			if (pReactorChannel == NULL || pReactorChannel->pRsslChannel == NULL || pRoutingChannel->channelState < OmmBaseImpl::LoginStreamOpenOkEnum)
				continue;

			if (rsslReactorSubmitMsg(_pRsslReactor, pReactorChannel, &submitMsgOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::loadDirectory().");
				temp.append(CR).append(pChannel->toString()).append(CR)
					.append("Session name ").append(pRoutingChannel->name).append(CR)
					.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
					.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo.rsslError.text);

				if (OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
					getOmmLoggerClient().log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			}
			else
			{
				++sentMsgCount;
			}

			if (sentMsgCount == 0)
			{
				EmaString temp("Internal error: rsslReactorSubmitMsg() failed to send any messages OmmNiProviderImpl::loadDirectory().");

				// This is failure because in this case, the only failure should be with a bad message.
				handleIue(temp, OmmInvalidUsageException::FailureEnum);
				return;
			}

			_bIsStreamIdZeroRefreshSubmitted = true;

			if (OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
				getOmmLoggerClient().log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Configured source directory was sent out on the wire.");
		}

	}
}

void OmmNiProviderImpl::reLoadDirectory(BaseRoutingSessionChannel* pRoutingChannel)
{
	try
	{
		_userLock.lock();

		reLoadConfigSourceDirectory(pRoutingChannel);
		reLoadUserSubmitSourceDirectory(pRoutingChannel);

		_userLock.unlock();
	}
	catch (...)
	{
		_userLock.unlock();
		throw;
	}
}

void OmmNiProviderImpl::reLoadUserSubmitSourceDirectory(BaseRoutingSessionChannel* pRoutingChannel)
{
	if ( !_activeConfig.recoverUserSubmitSourceDirectory )
		return;

	const DirectoryCache& userSubmittedDirectory = _ommNiProviderDirectoryStore.getDirectory();

	if ( userSubmittedDirectory.getServiceList().size() == 0 )
		return;

	if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Reload of user submitted source directories." );

	RsslBuffer rsslMsgBuffer;
	rsslMsgBuffer.length = 4096;
	rsslMsgBuffer.data = (char*) malloc( sizeof( char ) * rsslMsgBuffer.length );

	if ( !rsslMsgBuffer.data )
	{
		handleMee( "Failed to allocate memory in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
		return;
	}

	RsslEncIterator eIter;
	rsslClearEncodeIterator( &eIter );
	RsslRet retCode;
	if ( RSSL_RET_SUCCESS != (retCode = rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION )) )
	{
		free( rsslMsgBuffer.data );
		handleIue( "Internal error. Failed to set encode iterator version in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()", retCode );
		return;
	}

	if ( RSSL_RET_SUCCESS != (retCode = rsslSetEncodeIteratorBuffer( &eIter, &rsslMsgBuffer )) )
	{
		free( rsslMsgBuffer.data );
		handleIue( "Internal error. Failed to set encode iterator buffer in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()", retCode );
		return;
	}

	RsslRDMDirectoryRefresh directoryRefresh;
	rsslClearRDMDirectoryRefresh( &directoryRefresh );

	directoryRefresh.state.text.data = ( char* )"Refresh Complete";
	directoryRefresh.state.text.length = 16;

	if (!DirectoryServiceStore::encodeDirectoryRefreshMsg(_ommNiProviderDirectoryStore.getDirectory(), directoryRefresh))
	{
		DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
		handleMee("Failed to allocate memory in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()");
		return;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	retCode = rsslEncodeRDMDirectoryMsg( &eIter, (RsslRDMDirectoryMsg*) &directoryRefresh, &rsslMsgBuffer.length, &rsslErrorInfo );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		free( rsslMsgBuffer.data );

		rsslMsgBuffer.length += rsslMsgBuffer.length;
		rsslMsgBuffer.data = (char*) malloc( sizeof( char ) * rsslMsgBuffer.length );

		if (RSSL_RET_SUCCESS != rsslSetEncodeIteratorBuffer(&eIter, &rsslMsgBuffer))
		{
			DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
			handleMee("Internal error. Failed to set encode iterator buffer in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()");

			return;
		}

		if ( !rsslMsgBuffer.data )
		{
			DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
			handleMee( "Failed to allocate memory in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
			return;
		}

		clearRsslErrorInfo( &rsslErrorInfo );
		retCode = rsslEncodeRDMDirectoryMsg( &eIter, (RsslRDMDirectoryMsg*) &directoryRefresh, &rsslMsgBuffer.length, &rsslErrorInfo );
	}

	if ( retCode != RSSL_RET_SUCCESS )
	{
		DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);

		EmaString temp( "Internal error: failed to encode RsslRDMDirectoryMsg in OmmNiProviderImpl::loadDirectory()" );
		temp.append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp, retCode );
		return;
	}

	RsslDecodeIterator decIter;
	rsslClearDecodeIterator( &decIter );

	if ( (retCode = rsslSetDecodeIteratorRWFVersion( &decIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION )) != RSSL_RET_SUCCESS )
	{
		DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
		handleIue( "Internal error. Failed to set decode iterator version in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()", retCode);
		return;
	}

	if ( (retCode = rsslSetDecodeIteratorBuffer( &decIter, &rsslMsgBuffer )) != RSSL_RET_SUCCESS )
	{
		DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
		handleIue( "Internal error. Failed to set decode iterator buffer in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()", retCode);
		return;
	}

	RsslRefreshMsg rsslRefreshMsg;
	rsslClearRefreshMsg( &rsslRefreshMsg );
	if ( (retCode = rsslDecodeMsg( &decIter, (RsslMsg*) &rsslRefreshMsg )) != RSSL_RET_SUCCESS )
	{
		DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
		handleIue( "Internal error. Failed to decode message in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()", retCode);
		return;
	}

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*) &rsslRefreshMsg;

	EmaString temp;
	if (_activeConfig.removeItemsOnDisconnect && !_ommNiProviderDirectoryStore.decodeSourceDirectory(&submitMsgOpts.pRsslMsg->msgBase.encDataBody, temp, retCode))
	{
		DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
		handleIue( temp, retCode);
		return;
	}

	RsslReactorChannel* pReactorChannel;
	if (pRoutingChannel == NULL)
	{
		if (_activeChannel == NULL)
		{
			EmaString temp("reLoadUserSubmitSourceDirectory() error: No active channel to send message.");
			handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
			return;
		}
		pReactorChannel = _pReactorChannel;
	}
	else
	{
		if (pRoutingChannel->pReactorChannel == NULL)
		{
			EmaString temp("reLoadUserSubmitSourceDirectory() error: No active channel to send message.");
			handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
			return;
		}

		pReactorChannel = pRoutingChannel->pReactorChannel;
	}

	clearRsslErrorInfo( &rsslErrorInfo );
	if ( (retCode = rsslReactorSubmitMsg(_pRsslReactor, pReactorChannel, &submitMsgOpts, &rsslErrorInfo )) != RSSL_RET_SUCCESS )
	{
		DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);

		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::loadDirectory()." );
		temp.append( CR ).append( _activeChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp, retCode );
		return;
	}
	else
	{
		_bIsStreamIdZeroRefreshSubmitted = true;

		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "User submitted source directories were sent out on the wire after reconnect." );
	}

	DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
}

bool OmmNiProviderImpl::storeUserSubmitSourceDirectory( RsslMsg* pMsg )
{
	if (_activeConfig.recoverUserSubmitSourceDirectory)
	{

		if (OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
			getOmmLoggerClient().log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Storing user submitted source directories for connection recovery.");

		return _ommNiProviderDirectoryStore.submitSourceDirectory(0, pMsg, _rsslDirectoryMsg, _rsslDirectoryMsgBuffer, true);
	}
	else
	{
		return _ommNiProviderDirectoryStore.submitSourceDirectory(0, pMsg, _rsslDirectoryMsg, _rsslDirectoryMsgBuffer, false);
	}
}

void OmmNiProviderImpl::reLoadConfigSourceDirectory(BaseRoutingSessionChannel* pRoutingChannel)
{
	if (!_activeConfig.pDirectoryRefreshMsg)
		return;

	if (OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
		getOmmLoggerClient().log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Reload of configured source directories.");

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

	submitMsgOpts.pRsslMsg = (RsslMsg*)_activeConfig.pDirectoryRefreshMsg->get();

	EmaString temp;
	Int32 errorCode;
	if (_activeConfig.removeItemsOnDisconnect && !_ommNiProviderDirectoryStore.decodeSourceDirectory(&submitMsgOpts.pRsslMsg->msgBase.encDataBody, temp, errorCode))
	{
		handleIue(temp, errorCode);
		return;
	}

	RsslReactorChannel* pReactorChannel;
	if (pRoutingChannel == NULL)
	{
		if (_activeChannel == NULL)
		{
			EmaString temp("reLoadConfigSourceDirectory() error: No active channel to send message.");
			handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
			return;
		}
		pReactorChannel = _pReactorChannel;
	}
	else
	{
		if (pRoutingChannel->pReactorChannel == NULL)
		{
			EmaString temp("reLoadConfigSourceDirectory() error: No active channel to send message.");
			handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
			return;
		}

		pReactorChannel = pRoutingChannel->pReactorChannel;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	if ( rsslReactorSubmitMsg( _pRsslReactor, pReactorChannel, &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::reLoadConfigSourceDirectory()." );
		temp.append( CR ).append( _activeChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp, rsslErrorInfo.rsslError.rsslErrorId );
	}
	else
	{
		_bIsStreamIdZeroRefreshSubmitted = true;

		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Configured source directories were sent out on the wire after reconnect." );
	}
}

void OmmNiProviderImpl::processChannelEvent( RsslReactorChannelEvent* pEvent )
{
	bool callRemoveItems = false;
	switch ( pEvent->channelEventType )
	{
	case RSSL_RC_CET_CHANNEL_DOWN:
	case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		_userLock.lock();
		

		if (getRoutingSession() != NULL)
		{
			// Check to see if all channels are reconnecting.  If they are, call remove items.
			BaseRoutingSession* pRoutingSession = getRoutingSession();
			int reconnectingCount = pRoutingSession->getReconnectingCount();

			if (reconnectingCount == pRoutingSession->activeChannelCount)
				callRemoveItems = true;
		}
		else
		{
			callRemoveItems = true;
		}

		if (callRemoveItems)
		{
			if (_activeConfig.removeItemsOnDisconnect)
			{
				removeItems();
			}

			// Only call this if all items are fully closed
			_itemWatchList.processChannelEvent(pEvent);
		}

		_activeChannel = NULL;
		_userLock.unlock();
		break;
	default:
		break;
	}
}

void OmmNiProviderImpl::loadDictionary()
{
}

void OmmNiProviderImpl::createDictionaryCallbackClient( DictionaryCallbackClient*&, OmmBaseImpl& )
{
}

void OmmNiProviderImpl::createDirectoryCallbackClient( DirectoryCallbackClient*&, OmmBaseImpl& )
{
}

void OmmNiProviderImpl::addSocket( RsslSocket fd )
{
#ifdef USING_SELECT
	FD_SET( fd, &_readFds );
	FD_SET( fd, &_exceptFds );
#else
	addFd( fd, POLLIN | POLLERR | POLLHUP );
#endif
}

void OmmNiProviderImpl::removeSocket( RsslSocket fd )
{
#ifdef USING_SELECT
	FD_CLR( fd, &_readFds );
	FD_CLR( fd, &_exceptFds );
#else
	removeFd( fd );
#endif
}

void OmmNiProviderImpl::removeAllSocket()
{
#ifdef USING_SELECT
	FD_ZERO(&_readFds);
	FD_ZERO(&_exceptFds);
#else
	removeAllFd();
#endif
}

Int64 OmmNiProviderImpl::dispatch( Int64 timeOut )
{
	if ( _activeConfig.operationModel == OmmNiProviderConfig::UserDispatchEnum && !_atExit )
		return rsslReactorDispatchLoop( timeOut, _activeConfig.maxDispatchCountUserThread, _bMsgDispatched );

	return OmmProvider::TimeoutEnum;
}

UInt64 OmmNiProviderImpl::registerClient( const ReqMsg& reqMsg, OmmProviderClient& ommProvClient, void* closure, UInt64 parentHandle )
{
	_userLock.lock();

	const ReqMsgImpl& reqMsgEncoder = *MsgImpl::getImpl(reqMsg);

	if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType != ema::rdm::MMT_LOGIN && 
		reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType != ema::rdm::MMT_DICTIONARY )
	{
		_userLock.unlock();
		handleIue("OMM Interactive provider supports registering LOGIN and DICTIONARY domain type only.", OmmInvalidUsageException::InvalidArgumentEnum);
		return 0;
	}

	UInt64 handle = 0;

	try
	{
		handle = _pItemCallbackClient ? _pItemCallbackClient->registerClient(reqMsg, ommProvClient, closure, parentHandle) : 0;

	}
	catch (...)
	{
		_userLock.unlock();
		throw;
	}

	if ( handle )
	{
		try
		{
			Item* item = reinterpret_cast<Item*>(handle);
			StreamInfo* pStreamInfoPtr = new StreamInfo(StreamInfo::ConsumingEnum, item->getStreamId(), 0, item->getDomainType());

			if ( _handleToStreamInfo.find( handle ) != 0)
			{
				UInt64 newHandle = generateHandle(handle);
				_handleToStreamInfo.insert(newHandle, pStreamInfoPtr);
				pStreamInfoPtr->_actualHandle = handle;

				handle = newHandle;
			}
			else
			{
				_handleToStreamInfo.insert(handle, pStreamInfoPtr);
			}

			_streamInfoList.push_back(pStreamInfoPtr);
			
		}
		catch ( std::bad_alloc& )
		{
			_pItemCallbackClient->unregister( handle );
			_userLock.unlock();
			handleMee( "Failed to allocate memory in OmmNiProviderImpl::registerClient()" );
			return 0;
		}
	}

	_userLock.unlock();

	return handle;
}

void OmmNiProviderImpl::reissue(const ReqMsg& reqMsg, UInt64 handle)
{
	_userLock.lock();

	const ReqMsgImpl& reqMsgEncoder = *MsgImpl::getImpl(reqMsg);

	if ( reqMsgEncoder.isDomainTypeSet() && ( reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType != ema::rdm::MMT_LOGIN &&
		reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType != ema::rdm::MMT_DICTIONARY ) )
	{
		_userLock.unlock();
		handleIue("OMM Non-Interactive provider supports reissuing LOGIN and DICTIONARY domain type only.", OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	try
	{
		if (_pItemCallbackClient) _pItemCallbackClient->reissue(reqMsg, handle);

		_userLock.unlock();
	}
	catch (...)
	{
		_userLock.unlock();
		throw;
	}
}

void OmmNiProviderImpl::unregister( UInt64 handle )
{
	_userLock.lock();

	StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find( handle );

	if ( !pTempStreamInfoPtr )
	{
		_userLock.unlock();
		return;
	}

	if ( ( *pTempStreamInfoPtr )->_streamType != StreamInfo::ConsumingEnum )
	{
		_userLock.unlock();
		handleIhe( handle, "Attempt to unregister a handle that was not registered." );
		return;
	}

	_streamInfoList.removeValue( *pTempStreamInfoPtr );

	UInt64 unregisterHandle = handle;

	if ( (*pTempStreamInfoPtr)->_actualHandle != 0 )
	{
		unregisterHandle = (*pTempStreamInfoPtr)->_actualHandle;
	}

	delete* pTempStreamInfoPtr;

	_handleToStreamInfo.erase( handle );

	_userLock.unlock();

	OmmBaseImpl::unregister( unregisterHandle );
}

void OmmNiProviderImpl::submit( const RefreshMsg& msg, UInt64 handle )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	submitMsgOpts.pRsslMsg = MsgImpl::getImpl(msg)->getRsslMsg();

	bool bHandleAdded = false;

	_userLock.lock();

	if ( !_pChannelCallbackClient )
	{
		_userLock.unlock();
		return;
	}

	StreamInfoPtr* pStreamInfoPtr = _handleToStreamInfo.find( handle );

	if ( pStreamInfoPtr && ( *pStreamInfoPtr )->_streamType == StreamInfo::ConsumingEnum )
	{
		_userLock.unlock();
		handleIhe( handle, "Attempt to submit( const RefreshMsg& ) using a registered handle." );
		return;
	}

	if ((_pRoutingSession == NULL && _activeChannel == NULL) || (_pRoutingSession != NULL && (_pRoutingSession->activeChannelCount == 0 || _pRoutingSession->getReconnectingCount() == _pRoutingSession->activeChannelCount)))
	{
		_userLock.unlock();
		EmaString temp( "submit( const RefreshMsg& ) error: No active channel to send message." );
		handleIue( temp, OmmInvalidUsageException::NoActiveChannelEnum );
		return;
	}

	if (submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DIRECTORY)
	{
		if (OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Received RefreshMsg with SourceDirectory domain; Handle = ");
			temp.append(handle).append(", user assigned streamId = ").append(submitMsgOpts.pRsslMsg->msgBase.streamId).append(".");

			getOmmLoggerClient().log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		if (submitMsgOpts.pRsslMsg->msgBase.containerType != RSSL_DT_MAP)
		{
			_userLock.unlock();
			EmaString temp("Attempt to submit RefreshMsg with SourceDirectory domain using container with wrong data type. Expected container data type is Map. Passed in is ");
			temp += DataType((DataType::DataTypeEnum)submitMsgOpts.pRsslMsg->msgBase.containerType).toString();
			handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
			return;
		}

		EmaString temp;
		Int32 errorCode;
		if (!_ommNiProviderDirectoryStore.decodeSourceDirectory(&submitMsgOpts.pRsslMsg->msgBase.encDataBody, temp, errorCode))
		{
			_userLock.unlock();
			handleIue(temp, errorCode);
			return;
		}

		submitMsgOpts.pRsslMsg->refreshMsg.flags &= ~RSSL_RFMF_SOLICITED;

		if (_activeConfig.mergeSourceDirectoryStreams)
		{
			submitMsgOpts.pRsslMsg->msgBase.streamId = 0;
		}
		else
		{
			if (pStreamInfoPtr)
			{
				submitMsgOpts.pRsslMsg->msgBase.streamId = (*pStreamInfoPtr)->_streamId;
			}
			else
			{
				try
				{
					submitMsgOpts.pRsslMsg->msgBase.streamId = getNextProviderStreamId();
					StreamInfoPtr pTemp = new StreamInfo(StreamInfo::ProvidingEnum, submitMsgOpts.pRsslMsg->msgBase.streamId);
					_handleToStreamInfo.insert(handle, pTemp);
					_streamInfoList.push_back(pTemp);
					bHandleAdded = true;
				}
				catch (std::bad_alloc&)
				{
					returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
					_userLock.unlock();
					handleMee("Failed to allocate memory in OmmNiProviderImpl::submit( const RefreshMsg& )");
					return;
				}
			}
		}

		try
		{
			if (!storeUserSubmitSourceDirectory(submitMsgOpts.pRsslMsg))
			{
				_userLock.unlock();
				return;
			}
		}
		catch (OmmException&)
		{
			_userLock.unlock();
			throw;
		}
	}
	else
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received RefreshMsg with market domain; Handle = " );
			temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		const RefreshMsgImpl& enc = *MsgImpl::getImpl( msg );

		if ( pStreamInfoPtr )
		{
			submitMsgOpts.pRsslMsg->msgBase.streamId = ( *pStreamInfoPtr )->_streamId;
			if ( submitMsgOpts.pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY )
			{
				submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = ( *pStreamInfoPtr )->_serviceId;
				submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
			}
		}
		else if ( enc.hasServiceName() )
		{
			const EmaString& serviceName = enc.getServiceName();
			RsslUInt64* pServiceId = _ommNiProviderDirectoryStore.getServiceIdByName(&serviceName);
			if ( !pServiceId )
			{
				_userLock.unlock();

				EmaString temp( "Attempt to submit initial RefreshMsg with service name of " );
				temp.append( serviceName ).append( " that was not included in the SourceDirectory. Dropping this RefreshMsg." );
				handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum );
				return;
			}
			else if ( *pServiceId > 0xFFFF )
			{
				_userLock.unlock();

				EmaString temp( "Attempt to submit initial RefreshMsg with service name of " );
				temp.append( serviceName ).append( " whose matching service id of " ).append( *pServiceId ).append( " is out of range. Dropping this RefreshMsg." );
				handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = (RsslUInt16) *pServiceId;
			submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
			submitMsgOpts.pRsslMsg->refreshMsg.flags &= ~RSSL_RFMF_SOLICITED;

			submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo(StreamInfo::ProvidingEnum, submitMsgOpts.pRsslMsg->msgBase.streamId,
										submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
										submitMsgOpts.pRsslMsg->msgBase.domainType);
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc& )
			{
				returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
				_userLock.unlock();
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const RefreshMsg& )" );
				return;
			}
		}
		else if ( enc.hasServiceId() )
		{
			EmaStringPtr* pServiceNamePtr = _ommNiProviderDirectoryStore.getServiceNameById(submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId);

			if ( !pServiceNamePtr )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit initial RefreshMsg with service id of " );
				temp.append( submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId ).append( " that was not included in the SourceDirectory. Dropping this RefreshMsg." );
				handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum );
				return;
			}

			submitMsgOpts.pRsslMsg->refreshMsg.flags &= ~RSSL_RFMF_SOLICITED;

			submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo(StreamInfo::ProvidingEnum, submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
														submitMsgOpts.pRsslMsg->msgBase.domainType);
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc& )
			{
				returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
				_userLock.unlock();
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const RefreshMsg& )" );
				return;
			}
		}
		else
		{
			_userLock.unlock();
			handleIue( "Attempt to submit initial RefreshMsg without service name or id. Dropping this RefreshMsg.", OmmInvalidUsageException::InvalidArgumentEnum);
			return;
		}
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);

	// Submit the rsslMsg if if there is no routing session.
	if (_pRoutingSession == NULL)
	{
		if (rsslReactorSubmitMsg(_pRsslReactor, _pReactorChannel, &submitMsgOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			if (bHandleAdded)
			{
				StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
				_streamInfoList.removeValue(*pTempStreamInfoPtr);
				delete* pTempStreamInfoPtr;
				_handleToStreamInfo.erase(handle);
				returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
			}

			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit( const RefreshMsg& ).");
			temp.append(CR).append(_activeChannel->toString()).append(CR)
				.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			_userLock.unlock();
			handleIue(temp, rsslErrorInfo.rsslError.rsslErrorId);

			return;
		}
	}
	else
	{
		// Routing session configuration, so encode this buffer into _encodeBuffer and then memcpy it to fan out
		UInt32 msgSize = _getMsgEncodedSize(submitMsgOpts.pRsslMsg);
		NiProviderRoutingSession* pNiProvRoutingSession = static_cast<NiProviderRoutingSession*>(_pRoutingSession);
		RsslRet ret;

		bool msgEncoded = false;
		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo(&rsslErrorInfo);
		int sentMsgCount = 0;


		for (UInt32 i = 0; i < pNiProvRoutingSession->routingChannelList.size(); i++)
		{
			NiProviderRoutingSessionChannel* pRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pNiProvRoutingSession->routingChannelList[i]);
			RsslReactorChannel* pReactorChannel = pRoutingChannel->pReactorChannel;

			if (pReactorChannel == NULL || pReactorChannel->pRsslChannel == NULL || pRoutingChannel->channelState < OmmBaseImpl::LoginStreamOpenOkEnum)
				continue;

			// If we haven't encoded the message yet, do this here.
			if (!msgEncoded)
			{
				if (msgSize > _encodeBufferAllocatedLength)
				{
					if (_encodeBuffer.data != NULL)
					{
						free(_encodeBuffer.data);
						_encodeBuffer.data = NULL;
					}

					_encodeBuffer.data = (char*)malloc(msgSize);

					if (_encodeBuffer.data == NULL)
					{
						if (bHandleAdded)
						{
							StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
							_streamInfoList.removeValue(*pTempStreamInfoPtr);
							delete* pTempStreamInfoPtr;
							_handleToStreamInfo.erase(handle);
							returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
						}

						_userLock.unlock();
						handleMee("Failed to allocate memory in OmmNiProviderImpl::submit(RefreshMsg&)");
						return;
					}

					_encodeBufferAllocatedLength = msgSize;
				}

				_encodeBuffer.length = _encodeBufferAllocatedLength;

				while (true)
				{
					rsslClearEncodeIterator(&_encodeIterator);
					rsslSetEncodeIteratorRWFVersion(&_encodeIterator, pRoutingChannel->pReactorChannel->majorVersion, pRoutingChannel->pReactorChannel->minorVersion);
					rsslSetEncodeIteratorBuffer(&_encodeIterator, &_encodeBuffer);

					ret = rsslEncodeMsg(&_encodeIterator, submitMsgOpts.pRsslMsg);

					if (ret == RSSL_RET_SUCCESS)
					{
						_encodeBuffer.length = rsslGetEncodedBufferLength(&_encodeIterator);
						break;
					}
					else if (ret == RSSL_RET_BUFFER_TOO_SMALL)
					{
						free(_encodeBuffer.data);
						_encodeBuffer.data = NULL;

						_encodeBuffer.data = (char*)malloc(_encodeBufferAllocatedLength * 2);

						if (_encodeBuffer.data == NULL)
						{
							if (bHandleAdded)
							{
								StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
								_streamInfoList.removeValue(*pTempStreamInfoPtr);
								delete* pTempStreamInfoPtr;
								_handleToStreamInfo.erase(handle);
								returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
							}

							_userLock.unlock();
							handleMee("Failed to allocate memory in OmmNiProviderImpl::submit(RefreshMsg&)");
							return;
						}

						_encodeBufferAllocatedLength = _encodeBufferAllocatedLength * 2;
						_encodeBuffer.length = _encodeBufferAllocatedLength;
					}
					else
					{
						if (bHandleAdded)
						{
							StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
							_streamInfoList.removeValue(*pTempStreamInfoPtr);
							delete* pTempStreamInfoPtr;
							_handleToStreamInfo.erase(handle);
							returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
						}

						_userLock.unlock();
						EmaString temp("Internal error: rsslEncodeMsg() failed in OmmNiProviderImpl::submit(RefreshMsg&).");
						temp.append(CR).append(_activeChannel->toString()).append(CR)
							.append("Error Id ").append(ret);
						handleIue(temp, ret);
						return;
					}
				}

				msgEncoded = true;
			}

			// Get a buffer from the current Reactor Channel
			pRoutingChannel->_transportBuffer = rsslReactorGetBuffer(pReactorChannel, _encodeBuffer.length, RSSL_FALSE, &rsslErrorInfo);

			if (pRoutingChannel->_transportBuffer == NULL)
			{
				Int32 iueRetCode;

				if (rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
					iueRetCode = OmmInvalidUsageException::NoBuffersEnum;
				else
					iueRetCode = OmmInvalidUsageException::FailureEnum;


				// Release all previously allocated buffers from rsslReactorGetBuffer
				for (UInt32 j = 0; j < i; j++)
				{
					NiProviderRoutingSessionChannel* pPrevRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pNiProvRoutingSession->routingChannelList[j]);
					if (pPrevRoutingChannel->_transportBuffer)
					{
						rsslReactorReleaseBuffer(pPrevRoutingChannel->pReactorChannel, pPrevRoutingChannel->_transportBuffer, &rsslErrorInfo);
						pPrevRoutingChannel->_transportBuffer = NULL;
					}
				}

				if (bHandleAdded)
				{
					StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
					_streamInfoList.removeValue(*pTempStreamInfoPtr);
					delete* pTempStreamInfoPtr;
					_handleToStreamInfo.erase(handle);
					returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
				}

				_userLock.unlock();
				EmaString temp("Internal error: rsslReactorGetBuffer() failed in OmmNiProviderImpl::submit(RefreshMsg&).");
				temp.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Error Text ").append(rsslErrorInfo.rsslError.text);
				handleIue(temp, iueRetCode);
				return;
			}
		}

		// Copy the encoded message into the transport buffers for each channel and submit them to the Reactor
		for (UInt32 i = 0; i < pNiProvRoutingSession->routingChannelList.size(); i++)
		{
			NiProviderRoutingSessionChannel* pRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pNiProvRoutingSession->routingChannelList[i]);
			RsslReactorChannel* pReactorChannel = pRoutingChannel->pReactorChannel;

			// Do not attempt to submit if:
			// pReactorChannel is null(the reactor channel has been closed for this routing channel)
			// there isn't a valid RsslChannel in the Reactor Channel
			// or if the routing channel's state has not been progressed past the LoginStreamOpenOk state
			// Note: for Directory refreshes, this will allow the initial directory message to go through when user-specified directory is configured for the NiProvider. 
			//		 Any item refreshes will fail due to a directory mismatch prior to this point(as the directory cache won't have the services associated with it), and 
			//		 since recover user source directory is always turned on, EMA will recover the directory automatically after receiving a login Open/OK
			if (pReactorChannel == NULL || pReactorChannel->pRsslChannel == NULL || pRoutingChannel->channelState < OmmBaseImpl::LoginStreamOpenOkEnum)
				continue;

			memcpy(pRoutingChannel->_transportBuffer->data, _encodeBuffer.data, _encodeBuffer.length);
			pRoutingChannel->_transportBuffer->length = _encodeBuffer.length;
			RsslReactorSubmitOptions submitOpts;
			rsslClearReactorSubmitOptions(&submitOpts);

			if (rsslReactorSubmit(_pRsslReactor, pReactorChannel, pRoutingChannel->_transportBuffer, &submitOpts, &rsslErrorInfo) < RSSL_RET_SUCCESS)
			{
				// If this fails(most likely due to a channel down), log the error and release the buffer.  We will throw an exception if all fail
				if (OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit(RefreshMsg&).");
					temp.append(CR).append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
						.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
						.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
						.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
						.append("Error Text ").append(rsslErrorInfo.rsslError.text);

					getOmmLoggerClient().log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
				}

				rsslReactorReleaseBuffer(pReactorChannel, pRoutingChannel->_transportBuffer, &rsslErrorInfo);
				pRoutingChannel->_transportBuffer = NULL;

			}
			else
			{
				sentMsgCount++;
			}

			// Clear the _transportBuffer for this channel
			pRoutingChannel->_transportBuffer = NULL;
			
		}

		if (sentMsgCount == 0)
		{
			if (bHandleAdded)
			{
				StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
				_streamInfoList.removeValue(*pTempStreamInfoPtr);
				delete* pTempStreamInfoPtr;
				_handleToStreamInfo.erase(handle);
				returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
			}


			_userLock.unlock();
			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit(RefreshMsg&).");
			temp.append(CR).append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
			return;
		}


	}

	if ( submitMsgOpts.pRsslMsg->refreshMsg.state.streamState == OmmState::ClosedEnum ||
		submitMsgOpts.pRsslMsg->refreshMsg.state.streamState == OmmState::ClosedRecoverEnum ||
		submitMsgOpts.pRsslMsg->refreshMsg.state.streamState == OmmState::ClosedRedirectedEnum ||
		( submitMsgOpts.pRsslMsg->refreshMsg.state.streamState == OmmState::NonStreamingEnum && 
		( ( submitMsgOpts.pRsslMsg->refreshMsg.flags & RSSL_RFMF_REFRESH_COMPLETE ) != 0 ? true: false ) ) )
	{
		StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find( handle );
		_streamInfoList.removeValue( *pTempStreamInfoPtr );
		delete *pTempStreamInfoPtr;
		_handleToStreamInfo.erase( handle );
		returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
	}

	if ( !submitMsgOpts.pRsslMsg->msgBase.streamId )
		_bIsStreamIdZeroRefreshSubmitted = true;

	_userLock.unlock();
}

void OmmNiProviderImpl::submit( const UpdateMsg& msg, UInt64 handle )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	submitMsgOpts.pRsslMsg = MsgImpl::getImpl(msg)->getRsslMsg();

	bool bHandleAdded = false;

	_userLock.lock();

	if ( !_pChannelCallbackClient )
	{
		_userLock.unlock();
		return;
	}

	StreamInfoPtr* pStreamInfoPtr = _handleToStreamInfo.find( handle );

	if ( pStreamInfoPtr && ( *pStreamInfoPtr )->_streamType == StreamInfo::ConsumingEnum )
	{
		_userLock.unlock();
		handleIhe( handle, "Attempt to submit( const UpdateMsg& ) using a registered handle." );
		return;
	}

	if ((_pRoutingSession == NULL && _activeChannel == NULL) || (_pRoutingSession != NULL && (_pRoutingSession->activeChannelCount == 0 || _pRoutingSession->getReconnectingCount() == _pRoutingSession->activeChannelCount)))
	{
		_userLock.unlock();
		EmaString temp( "submit( const UpdateMsg& ) error: No active channel to send message." );
		handleIue( temp, OmmInvalidUsageException::NoActiveChannelEnum );
		return;
	}

	if ( submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DIRECTORY )
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received UpdateMsg with SourceDirectory domain; Handle = " );
			temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		if ( submitMsgOpts.pRsslMsg->msgBase.containerType != RSSL_DT_MAP )
		{
			_userLock.unlock();
			EmaString temp( "Attempt to submit UpdateMsg with SourceDirectory domain using container with wrong data type. Expected is Map. Passed in is " );
			temp += DataType((DataType::DataTypeEnum)submitMsgOpts.pRsslMsg->msgBase.containerType).toString();
			handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum);
			return;
		}

		EmaString temp;
		Int32 errorCode;
		if ( !_ommNiProviderDirectoryStore.decodeSourceDirectory( &submitMsgOpts.pRsslMsg->msgBase.encDataBody, temp, errorCode ) )
		{
			_userLock.unlock();
			handleIue( temp, errorCode );
			return;
		}

		if ( _activeConfig.mergeSourceDirectoryStreams )
		{
			if ( _activeConfig.refreshFirstRequired && !_bIsStreamIdZeroRefreshSubmitted )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit UpdateMsg with SourceDirectory while RefreshMsg was not submitted on this stream yet. Handle = " );
				temp.append( handle ).append( "." );
				handleIhe( handle, temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.streamId = 0;
		}
		else
		{
			if ( pStreamInfoPtr )
			{
				submitMsgOpts.pRsslMsg->msgBase.streamId = ( *pStreamInfoPtr )->_streamId;
			}
			else
			{
				if ( _activeConfig.refreshFirstRequired )
				{
					_userLock.unlock();
					EmaString temp( "Attempt to submit UpdateMsg with SourceDirectory while RefreshMsg was not submitted on this stream yet. Handle = " );
					temp.append( handle ).append( "." );
					handleIhe( handle, temp );
					return;
				}

				try
				{
					submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();
					StreamInfoPtr pTemp = new StreamInfo(StreamInfo::ProvidingEnum, submitMsgOpts.pRsslMsg->msgBase.streamId);
					_handleToStreamInfo.insert( handle, pTemp );
					_streamInfoList.push_back( pTemp );
					bHandleAdded = true;
				}
				catch ( std::bad_alloc& )
				{
					returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
					_userLock.unlock();
					handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const UpdateMsg& )" );
					return;
				}
			}
		}

		try
		{
			if (!storeUserSubmitSourceDirectory(submitMsgOpts.pRsslMsg))
			{
				_userLock.unlock();
				return;
			}
		}
		catch (OmmException&)
		{
			_userLock.unlock();
			throw;
		}
	}
	else
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received UpdateMsg with market domain; Handle = " );
			temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		const UpdateMsgImpl& enc = *MsgImpl::getImpl(msg);

		if ( pStreamInfoPtr )
		{
			submitMsgOpts.pRsslMsg->msgBase.streamId = ( *pStreamInfoPtr )->_streamId;
			if ( submitMsgOpts.pRsslMsg->updateMsg.flags & RSSL_UPMF_HAS_MSG_KEY )
			{
				submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = ( *pStreamInfoPtr )->_serviceId;
				submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
			}
		}
		else if ( _activeConfig.refreshFirstRequired )
		{
			_userLock.unlock();
			EmaString temp( "Attempt to submit UpdateMsg while RefreshMsg was not submitted on this stream yet. Handle = " );
			temp.append( handle ).append( "." );
			handleIhe( handle, temp );
			return;
		}
		else if ( enc.hasServiceName() )
		{
			const EmaString& serviceName = enc.getServiceName();
			RsslUInt64* pServiceId = _ommNiProviderDirectoryStore.getServiceIdByName(&serviceName);
			if ( !pServiceId )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit UpdateMsg with service name of " );
				temp.append( serviceName ).append( " that was not included in the SourceDirectory. Dropping this UpdateMsg." );
				handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum );
				return;
			}
			else if ( *pServiceId > 0xFFFF )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit UpdateMsg with service name of " );
				temp.append( serviceName ).append( " whose matching service id of " ).append( *pServiceId ).append( " is out of range. Dropping this UpdateMsg." );
				handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = (RsslUInt16) *pServiceId;
			submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

			submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo(StreamInfo::ProvidingEnum, submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
														submitMsgOpts.pRsslMsg->msgBase.domainType);
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc& )
			{
				returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
				_userLock.unlock();
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const UpdateMsg& )" );
				return;
			}
		}
		else if ( enc.hasServiceId() )
		{
			EmaStringPtr* pServiceNamePtr = _ommNiProviderDirectoryStore.getServiceNameById(submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId);

			if ( !pServiceNamePtr )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit UpdateMsg with service id of " );
				temp.append( submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId ).append( " that was not included in the SourceDirectory. Dropping this UpdateMsg." );
				handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo(StreamInfo::ProvidingEnum, submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
														submitMsgOpts.pRsslMsg->msgBase.domainType);
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc& )
			{
				returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
				_userLock.unlock();
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const UpdateMsg& )" );
				return;
			}
		}
		else
		{
			_userLock.unlock();
			handleIue( "Attempt to submit UpdateMsg without service name or id. Dropping this UpdateMsg.", OmmInvalidUsageException::InvalidArgumentEnum);
			return;
		}
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );

	if (_pRoutingSession == NULL)
	{
		if (rsslReactorSubmitMsg(_pRsslReactor, _pReactorChannel, &submitMsgOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			if (bHandleAdded)
			{
				StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
				_streamInfoList.removeValue(*pTempStreamInfoPtr);
				delete* pTempStreamInfoPtr;
				_handleToStreamInfo.erase(handle);
				returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
			}

			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit( const UpdateMsg& ).");
			temp.append(CR).append(_activeChannel->toString()).append(CR)
				.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			_userLock.unlock();
			handleIue(temp, rsslErrorInfo.rsslError.rsslErrorId);
			return;
		}
	}
	else
	{
		// Routing session configuration, so encode this buffer into _encodeBuffer and then memcpy it to fan out
		UInt32 msgSize = _getMsgEncodedSize(submitMsgOpts.pRsslMsg);
		NiProviderRoutingSession* pNiProvRoutingSession = static_cast<NiProviderRoutingSession*>(_pRoutingSession);
		RsslRet ret;

		bool msgEncoded = false;
		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo(&rsslErrorInfo);
		int sentMsgCount = 0;


		for (UInt32 i = 0; i < pNiProvRoutingSession->routingChannelList.size(); i++)
		{
			NiProviderRoutingSessionChannel* pRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pNiProvRoutingSession->routingChannelList[i]);
			RsslReactorChannel* pReactorChannel = pRoutingChannel->pReactorChannel;

			if (pReactorChannel == NULL || pReactorChannel->pRsslChannel == NULL || pRoutingChannel->channelState < OmmBaseImpl::LoginStreamOpenOkEnum)
				continue;

			// If we haven't encoded the message yet, do this here.
			if (!msgEncoded)
			{
				if (msgSize > _encodeBufferAllocatedLength)
				{
					if (_encodeBuffer.data != NULL)
					{
						free(_encodeBuffer.data);
						_encodeBuffer.data = NULL;
					}

					_encodeBuffer.data = (char*)malloc(msgSize);

					if (_encodeBuffer.data == NULL)
					{
						if (bHandleAdded)
						{
							StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
							_streamInfoList.removeValue(*pTempStreamInfoPtr);
							delete* pTempStreamInfoPtr;
							_handleToStreamInfo.erase(handle);
							returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
						}

						_userLock.unlock();
						handleMee("Failed to allocate memory in OmmNiProviderImpl::submit(UpdateMsg&)");
						return;
					}

					_encodeBufferAllocatedLength = msgSize;
				}

				_encodeBuffer.length = _encodeBufferAllocatedLength;

				while (true)
				{
					rsslClearEncodeIterator(&_encodeIterator);
					rsslSetEncodeIteratorRWFVersion(&_encodeIterator, pRoutingChannel->pReactorChannel->majorVersion, pRoutingChannel->pReactorChannel->minorVersion);
					rsslSetEncodeIteratorBuffer(&_encodeIterator, &_encodeBuffer);

					ret = rsslEncodeMsg(&_encodeIterator, submitMsgOpts.pRsslMsg);

					if (ret == RSSL_RET_SUCCESS)
					{
						_encodeBuffer.length = rsslGetEncodedBufferLength(&_encodeIterator);
						break;
					}
					else if (ret == RSSL_RET_BUFFER_TOO_SMALL)
					{
						free(_encodeBuffer.data);
						_encodeBuffer.data = NULL;

						_encodeBuffer.data = (char*)malloc(_encodeBufferAllocatedLength * 2);

						if (_encodeBuffer.data == NULL)
						{
							if (bHandleAdded)
							{
								StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
								_streamInfoList.removeValue(*pTempStreamInfoPtr);
								delete* pTempStreamInfoPtr;
								_handleToStreamInfo.erase(handle);
								returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
							}

							_userLock.unlock();
							handleMee("Failed to allocate memory in OmmNiProviderImpl::submit(UpdateMsg&)");
							return;
						}

						_encodeBufferAllocatedLength = _encodeBufferAllocatedLength * 2;
						_encodeBuffer.length = _encodeBufferAllocatedLength;
					}
					else
					{
						if (bHandleAdded)
						{
							StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
							_streamInfoList.removeValue(*pTempStreamInfoPtr);
							delete* pTempStreamInfoPtr;
							_handleToStreamInfo.erase(handle);
							returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
						}

						_userLock.unlock();
						EmaString temp("Internal error: rsslEncodeMsg() failed in OmmNiProviderImpl::submit(UpdateMsg&).");
						temp.append(CR).append(_activeChannel->toString()).append(CR)
							.append("Error Id ").append(ret);
						handleIue(temp, ret);
						return;
					}
				}

				msgEncoded = true;
			}

			// Get a buffer from the current Reactor Channel
			pRoutingChannel->_transportBuffer = rsslReactorGetBuffer(pReactorChannel, _encodeBuffer.length, RSSL_FALSE, &rsslErrorInfo);

			if (pRoutingChannel->_transportBuffer == NULL)
			{
				Int32 iueRetCode;

				if (rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
					iueRetCode = OmmInvalidUsageException::NoBuffersEnum;
				else
					iueRetCode = OmmInvalidUsageException::FailureEnum;


				// Release all previously allocated buffers from rsslReactorGetBuffer
				for (UInt32 j = 0; j < i; j++)
				{
					NiProviderRoutingSessionChannel* pPrevRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pNiProvRoutingSession->routingChannelList[j]);
					if (pPrevRoutingChannel->_transportBuffer)
					{
						rsslReactorReleaseBuffer(pPrevRoutingChannel->pReactorChannel, pPrevRoutingChannel->_transportBuffer, &rsslErrorInfo);
						pPrevRoutingChannel->_transportBuffer = NULL;
					}
				}

				if (bHandleAdded)
				{
					StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
					_streamInfoList.removeValue(*pTempStreamInfoPtr);
					delete* pTempStreamInfoPtr;
					_handleToStreamInfo.erase(handle);
					returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
				}

				_userLock.unlock();
				EmaString temp("Internal error: rsslReactorGetBuffer() failed in OmmNiProviderImpl::submit(RefreshMsg&).");
				temp.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Error Text ").append(rsslErrorInfo.rsslError.text);
				handleIue(temp, iueRetCode);
				return;
			}
		}

		// Copy the encoded message into the transport buffers for each channel and submit them to the Reactor
		for (UInt32 i = 0; i < pNiProvRoutingSession->routingChannelList.size(); i++)
		{
			NiProviderRoutingSessionChannel* pRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pNiProvRoutingSession->routingChannelList[i]);
			RsslReactorChannel* pReactorChannel = pRoutingChannel->pReactorChannel;

			// Do not attempt to submit if:
			// pReactorChannel is null(the reactor channel has been closed for this routing channel)
			// there isn't a valid RsslChannel in the Reactor Channel
			// or if the routing channel's state has not been progressed past the LoginStreamOpenOk state
			// Note: for Directory refreshes, this will allow the initial directory message to go through when user-specified directory is configured for the NiProvider. 
			//		 Any item refreshes will fail due to a directory mismatch prior to this point(as the directory cache won't have the services associated with it), and 
			//		 since recover user source directory is always turned on, EMA will recover the directory automatically after receiving a login Open/OK
			if (pReactorChannel == NULL || pReactorChannel->pRsslChannel == NULL || pRoutingChannel->channelState < OmmBaseImpl::LoginStreamOpenOkEnum)
				continue;

			memcpy(pRoutingChannel->_transportBuffer->data, _encodeBuffer.data, _encodeBuffer.length);
			pRoutingChannel->_transportBuffer->length = _encodeBuffer.length;
			RsslReactorSubmitOptions submitOpts;
			rsslClearReactorSubmitOptions(&submitOpts);

			if (rsslReactorSubmit(_pRsslReactor, pReactorChannel, pRoutingChannel->_transportBuffer, &submitOpts, &rsslErrorInfo) < RSSL_RET_SUCCESS)
			{
				// If this fails(most likely due to a channel down), log the error and release the buffer.  We will throw an exception if all fail
				if (OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit(UpdateMsg&).");
					temp.append(CR).append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
						.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
						.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
						.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
						.append("Error Text ").append(rsslErrorInfo.rsslError.text);

					getOmmLoggerClient().log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
				}

				rsslReactorReleaseBuffer(pReactorChannel, pRoutingChannel->_transportBuffer, &rsslErrorInfo);
				pRoutingChannel->_transportBuffer = NULL;

			}
			else
			{
				sentMsgCount++;
			}

			// Clear the _transportBuffer for this channel
			pRoutingChannel->_transportBuffer = NULL;
		}

		if (sentMsgCount == 0)
		{
			if (bHandleAdded)
			{
				StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
				_streamInfoList.removeValue(*pTempStreamInfoPtr);
				delete* pTempStreamInfoPtr;
				_handleToStreamInfo.erase(handle);
				returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
			}

			_userLock.unlock();
			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit(UpdateMsg&).");
			temp.append(CR).append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
			return;
		}
	}

	_userLock.unlock();
}

void OmmNiProviderImpl::submit( const StatusMsg& msg, UInt64 handle )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	const StatusMsgImpl* pStatusImpl = MsgImpl::getImpl(msg);
	submitMsgOpts.pRsslMsg = const_cast<RsslMsg*>(pStatusImpl->getRsslMsg());

	bool bHandleAdded = false;

	_userLock.lock();

	if ( !_pChannelCallbackClient )
	{
		_userLock.unlock();
		return;
	}

	StreamInfoPtr* pStreamInfoPtr = _handleToStreamInfo.find( handle );

	if ( pStreamInfoPtr && ( *pStreamInfoPtr )->_streamType == StreamInfo::ConsumingEnum )
	{
		_userLock.unlock();
		handleIhe( handle, "Attempt to submit( const StatusMsg& ) using a registered handle." );
		return;
	}

	if ((_pRoutingSession == NULL && _activeChannel == NULL) || (_pRoutingSession != NULL && (_pRoutingSession->activeChannelCount == 0 || _pRoutingSession->getReconnectingCount() == _pRoutingSession->activeChannelCount)))
	{
		_userLock.unlock();
		EmaString temp( "submit( const StatusMsg& ) error: No active channel to send message." );
		handleIue( temp, OmmInvalidUsageException::NoActiveChannelEnum );
		return;
	}

	if ( submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DIRECTORY )
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received StatusMsg with SourceDirectory domain; Handle = " );
			temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		if (submitMsgOpts.pRsslMsg->msgBase.containerType != RSSL_DT_MAP)
		{
			_userLock.unlock();
			EmaString temp("Attempt to submit StatusMsg with SourceDirectory domain using container with wrong data type. Expected is Map. Passed in is ");
			temp += DataType((DataType::DataTypeEnum)submitMsgOpts.pRsslMsg->msgBase.containerType).toString();
			handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		if ( _activeConfig.mergeSourceDirectoryStreams )
		{
			if ( _activeConfig.refreshFirstRequired && !_bIsStreamIdZeroRefreshSubmitted )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit StatusMsg with SourceDirectory while RefreshMsg was not submitted on this stream yet. Handle = " );
				temp.append( handle ).append( "." );
				handleIhe( handle, temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.streamId = 0;
		}
		else
		{
			if ( pStreamInfoPtr )
			{
				submitMsgOpts.pRsslMsg->msgBase.streamId = ( *pStreamInfoPtr )->_streamId;
			}
			else
			{
				if ( _activeConfig.refreshFirstRequired )
				{
					_userLock.unlock();
					EmaString temp( "Attempt to submit StatusMsg with SourceDirectory while RefreshMsg was not submitted on this stream yet. Handle = " );
					temp.append( handle ).append( "." );
					handleIhe( handle, temp );
					return;
				}

				try
				{
					submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();
					StreamInfoPtr pTemp = new StreamInfo(StreamInfo::ProvidingEnum, submitMsgOpts.pRsslMsg->msgBase.streamId);
					_handleToStreamInfo.insert( handle, pTemp );
					_streamInfoList.push_back( pTemp );
					bHandleAdded = true;
				}
				catch ( std::bad_alloc& )
				{
					returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
					_userLock.unlock();
					handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const StatusMsg& )" );
					return;
				}
			}
		}
	}
	else
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received StatusMsg with market domain; Handle = " );
			temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		if ( pStreamInfoPtr )
		{
			submitMsgOpts.pRsslMsg->msgBase.streamId = ( *pStreamInfoPtr )->_streamId;
			if ( submitMsgOpts.pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY )
			{
				submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = ( *pStreamInfoPtr )->_serviceId;
				submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
			}
		}
		else if ( _activeConfig.refreshFirstRequired )
		{
			_userLock.unlock();
			EmaString temp( "Attempt to submit StatusMsg while RefreshMsg was not submitted on this stream yet. Handle = " );
			temp.append( handle ).append( "." );
			handleIhe( handle, temp );
			return;
		}
		else if ( pStatusImpl->hasServiceName() )
		{
			const EmaString& serviceName = pStatusImpl->getServiceName();
			RsslUInt64* pServiceId = _ommNiProviderDirectoryStore.getServiceIdByName(&serviceName);
			if ( !pServiceId )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit StatusMsg with service name of " );
				temp.append( serviceName ).append( " that was not included in the SourceDirectory. Dropping this StatusMsg." );
				handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum );
				return;
			}
			else if ( *pServiceId > 0xFFFF )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit StatusMsg with service name of " );
				temp.append( serviceName ).append( " whose matching service id of " ).append( *pServiceId ).append( " is out of range. Dropping this StatusMsg." );
				handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = (RsslUInt16) *pServiceId;
			submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

			submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo(StreamInfo::ProvidingEnum, submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
														submitMsgOpts.pRsslMsg->msgBase.domainType);
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc& )
			{
				returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
				_userLock.unlock();
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const StatusMsg& )" );
				return;
			}
		}
		else if ( pStatusImpl->hasServiceId() )
		{
			EmaStringPtr* pServiceNamePtr = _ommNiProviderDirectoryStore.getServiceNameById(submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId);

			if ( !pServiceNamePtr )
			{
				_userLock.unlock();

				EmaString temp( "Attempt to submit StatusMsg with service id of " );
				temp.append( submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId ).append( " that was not included in the SourceDirectory. Dropping this StatusMsg." );
				handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo(StreamInfo::ProvidingEnum, submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
														submitMsgOpts.pRsslMsg->msgBase.domainType);
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc& )
			{
				returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
				_userLock.unlock();
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const StatusMsg& )" );
				return;
			}
		}
		else
		{
			_userLock.unlock();
			handleIue( "Attempt to submit StatusMsg without service name or id. Dropping this StatusMsg.", OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );

	if (_pRoutingSession == NULL)
	{
		if (rsslReactorSubmitMsg(_pRsslReactor, _pReactorChannel, &submitMsgOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			if (bHandleAdded)
			{
				StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
				_streamInfoList.removeValue(*pTempStreamInfoPtr);
				delete* pTempStreamInfoPtr;
				_handleToStreamInfo.erase(handle);
				returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
			}

			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit( const StatusMsg& ).");
			temp.append(CR).append(_activeChannel->toString()).append(CR)
				.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			_userLock.unlock();
			handleIue(temp, rsslErrorInfo.rsslError.rsslErrorId);

			return;
		}
	}
	else
	{
		// Routing session configuration, so encode this buffer into _encodeBuffer and then memcpy it to fan out
		UInt32 msgSize = _getMsgEncodedSize(submitMsgOpts.pRsslMsg);
		NiProviderRoutingSession* pNiProvRoutingSession = static_cast<NiProviderRoutingSession*>(_pRoutingSession);
		RsslRet ret;

		bool msgEncoded = false;
		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo(&rsslErrorInfo);
		int sentMsgCount = 0;


		for (UInt32 i = 0; i < pNiProvRoutingSession->routingChannelList.size(); i++)
		{
			NiProviderRoutingSessionChannel* pRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pNiProvRoutingSession->routingChannelList[i]);
			RsslReactorChannel* pReactorChannel = pRoutingChannel->pReactorChannel;

			if (pReactorChannel == NULL || pReactorChannel->pRsslChannel == NULL || pRoutingChannel->channelState < OmmBaseImpl::LoginStreamOpenOkEnum)
				continue;

			// If we haven't encoded the message yet, do this here.
			if (!msgEncoded)
			{
				if (msgSize > _encodeBufferAllocatedLength)
				{
					if (_encodeBuffer.data != NULL)
					{
						free(_encodeBuffer.data);
						_encodeBuffer.data = NULL;
					}

					_encodeBuffer.data = (char*)malloc(msgSize);

					if (_encodeBuffer.data == NULL)
					{
						if (bHandleAdded)
						{
							StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
							_streamInfoList.removeValue(*pTempStreamInfoPtr);
							delete* pTempStreamInfoPtr;
							_handleToStreamInfo.erase(handle);
							returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
						}

						_userLock.unlock();
						handleMee("Failed to allocate memory in OmmNiProviderImpl::submit(StatusMsg&)");
						return;
					}

					_encodeBufferAllocatedLength = msgSize;
				}

				_encodeBuffer.length = _encodeBufferAllocatedLength;

				while (true)
				{
					rsslClearEncodeIterator(&_encodeIterator);
					rsslSetEncodeIteratorRWFVersion(&_encodeIterator, pRoutingChannel->pReactorChannel->majorVersion, pRoutingChannel->pReactorChannel->minorVersion);
					rsslSetEncodeIteratorBuffer(&_encodeIterator, &_encodeBuffer);

					ret = rsslEncodeMsg(&_encodeIterator, submitMsgOpts.pRsslMsg);

					if (ret == RSSL_RET_SUCCESS)
					{
						_encodeBuffer.length = rsslGetEncodedBufferLength(&_encodeIterator);
						break;
					}
					else if (ret == RSSL_RET_BUFFER_TOO_SMALL)
					{
						free(_encodeBuffer.data);
						_encodeBuffer.data = NULL;

						_encodeBuffer.data = (char*)malloc(_encodeBufferAllocatedLength * 2);

						if (_encodeBuffer.data == NULL)
						{
							if (bHandleAdded)
							{
								StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
								_streamInfoList.removeValue(*pTempStreamInfoPtr);
								delete* pTempStreamInfoPtr;
								_handleToStreamInfo.erase(handle);
								returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
							}

							_userLock.unlock();
							handleMee("Failed to allocate memory in OmmNiProviderImpl::submit(StatusMsg&)");
							return;
						}

						_encodeBufferAllocatedLength = _encodeBufferAllocatedLength * 2;
						_encodeBuffer.length = _encodeBufferAllocatedLength;
					}
					else
					{
						if (bHandleAdded)
						{
							StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
							_streamInfoList.removeValue(*pTempStreamInfoPtr);
							delete* pTempStreamInfoPtr;
							_handleToStreamInfo.erase(handle);
							returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
						}

						_userLock.unlock();
						EmaString temp("Internal error: rsslEncodeMsg() failed in OmmNiProviderImpl::submit(StatusMsg&).");
						temp.append(CR).append(_activeChannel->toString()).append(CR)
							.append("Error Id ").append(ret);
						handleIue(temp, ret);
						return;
					}
				}

				msgEncoded = true;
			}

			// Get a buffer from the current Reactor Channel
			pRoutingChannel->_transportBuffer = rsslReactorGetBuffer(pReactorChannel, _encodeBuffer.length, RSSL_FALSE, &rsslErrorInfo);

			if (pRoutingChannel->_transportBuffer == NULL)
			{
				Int32 iueRetCode;

				if (rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
					iueRetCode = OmmInvalidUsageException::NoBuffersEnum;
				else
					iueRetCode = OmmInvalidUsageException::FailureEnum;


				// Release all previously allocated buffers from rsslReactorGetBuffer
				for (UInt32 j = 0; j < i; j++)
				{
					NiProviderRoutingSessionChannel* pPrevRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pNiProvRoutingSession->routingChannelList[j]);
					if (pPrevRoutingChannel->_transportBuffer)
					{
						rsslReactorReleaseBuffer(pPrevRoutingChannel->pReactorChannel, pPrevRoutingChannel->_transportBuffer, &rsslErrorInfo);
						pPrevRoutingChannel->_transportBuffer = NULL;
					}
				}

				if (bHandleAdded)
				{
					StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
					_streamInfoList.removeValue(*pTempStreamInfoPtr);
					delete* pTempStreamInfoPtr;
					_handleToStreamInfo.erase(handle);
					returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
				}

				_userLock.unlock();
				EmaString temp("Internal error: rsslReactorGetBuffer() failed in OmmNiProviderImpl::submit(StatusMsg&).");
				temp.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Error Text ").append(rsslErrorInfo.rsslError.text);
				handleIue(temp, iueRetCode);
				return;
			}
		}

		// Copy the encoded message into the transport buffers for each channel and submit them to the Reactor
		for (UInt32 i = 0; i < pNiProvRoutingSession->routingChannelList.size(); i++)
		{
			NiProviderRoutingSessionChannel* pRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pNiProvRoutingSession->routingChannelList[i]);
			RsslReactorChannel* pReactorChannel = pRoutingChannel->pReactorChannel;

			// Do not attempt to submit if:
			// pReactorChannel is null(the reactor channel has been closed for this routing channel)
			// there isn't a valid RsslChannel in the Reactor Channel
			// or if the routing channel's state has not been progressed past the LoginStreamOpenOk state
			// Note: for Directory refreshes, this will allow the initial directory message to go through when user-specified directory is configured for the NiProvider. 
			//		 Any item refreshes will fail due to a directory mismatch prior to this point(as the directory cache won't have the services associated with it), and 
			//		 since recover user source directory is always turned on, EMA will recover the directory automatically after receiving a login Open/OK
			if (pReactorChannel == NULL || pReactorChannel->pRsslChannel == NULL || pRoutingChannel->channelState < OmmBaseImpl::LoginStreamOpenOkEnum)
				continue;

			memcpy(pRoutingChannel->_transportBuffer->data, _encodeBuffer.data, _encodeBuffer.length);
			pRoutingChannel->_transportBuffer->length = _encodeBuffer.length;
			RsslReactorSubmitOptions submitOpts;
			rsslClearReactorSubmitOptions(&submitOpts);

			if (rsslReactorSubmit(_pRsslReactor, pReactorChannel, pRoutingChannel->_transportBuffer, &submitOpts, &rsslErrorInfo) < RSSL_RET_SUCCESS)
			{
				// If this fails(most likely due to a channel down), log the error and release the buffer.  We will throw an exception if all fail
				if (OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit(StatusMsg&).");
					temp.append(CR).append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
						.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
						.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
						.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
						.append("Error Text ").append(rsslErrorInfo.rsslError.text);

					getOmmLoggerClient().log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
				}

				rsslReactorReleaseBuffer(pReactorChannel, pRoutingChannel->_transportBuffer, &rsslErrorInfo);
				pRoutingChannel->_transportBuffer = NULL;

			}
			else
			{
				sentMsgCount++;
			}

			// Clear the _transportBuffer for this channel
			pRoutingChannel->_transportBuffer = NULL;
		}

		if (sentMsgCount == 0)
		{
			if (bHandleAdded)
			{
				StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find(handle);
				_streamInfoList.removeValue(*pTempStreamInfoPtr);
				delete* pTempStreamInfoPtr;
				_handleToStreamInfo.erase(handle);
				returnProviderStreamId(submitMsgOpts.pRsslMsg->msgBase.streamId);
			}

			_userLock.unlock();
			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit(StatusMsg&).");
			temp.append(CR).append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
			return;
		}
	}

	if ( submitMsgOpts.pRsslMsg->statusMsg.state.streamState == OmmState::ClosedEnum ||
		submitMsgOpts.pRsslMsg->statusMsg.state.streamState == OmmState::ClosedRecoverEnum ||
		submitMsgOpts.pRsslMsg->statusMsg.state.streamState == OmmState::ClosedRedirectedEnum )
	{
		StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find( handle );
		_streamInfoList.removeValue( *pTempStreamInfoPtr );
		delete *pTempStreamInfoPtr;
		_handleToStreamInfo.erase( handle );
		returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
	}

	_userLock.unlock();
}

void OmmNiProviderImpl::submit(const GenericMsg& msg, UInt64 handle)
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
	submitMsgOpts.pRsslMsg = MsgImpl::getImpl(msg)->getRsslMsg();

	_userLock.lock();

	if (!_pChannelCallbackClient)
	{
		_userLock.unlock();
		return;
	}

	if ((_pRoutingSession == NULL && _activeChannel == NULL) || (_pRoutingSession != NULL && (_pRoutingSession->activeChannelCount == 0 || _pRoutingSession->getReconnectingCount() == _pRoutingSession->activeChannelCount)))
	{
		_userLock.unlock();
		EmaString temp("submit( const GenericMsg& ) error: No active channel to send message.");
		handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
		return;
	}

	if (OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
	{
		EmaString temp("Received GenericMsg; Handle = ");
		temp.append(handle).append(", user assigned streamId = ").append(submitMsgOpts.pRsslMsg->msgBase.streamId).append(".");

		getOmmLoggerClient().log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
	}

	StreamInfoPtr* pStreamInfoPtr = _handleToStreamInfo.find(handle);

	if (pStreamInfoPtr)
	{
		if ((*pStreamInfoPtr)->_streamType == StreamInfo::ConsumingEnum)
		{
			_userLock.unlock();
			OmmBaseImpl::submit(msg, handle);
			return;
		}

		submitMsgOpts.pRsslMsg->msgBase.streamId = (*pStreamInfoPtr)->_streamId;
		if (submitMsgOpts.pRsslMsg->msgBase.domainType == 0)
			submitMsgOpts.pRsslMsg->msgBase.domainType = (*pStreamInfoPtr)->_domainType;
	}
	else
	{
		_userLock.unlock();
		EmaString temp("Attempt to submit GenericMsg on stream that is not open yet. Handle = ");
		temp.append(handle).append(".");
		handleIhe(handle, temp);
		return;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);

	if (_pRoutingSession == NULL)
	{
		if (rsslReactorSubmitMsg(_pRsslReactor, _pReactorChannel, &submitMsgOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
		{
			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit( const GenericMsg& ).");
			temp.append(CR).append(_activeChannel->toString()).append(CR)
				.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			_userLock.unlock();
			handleIue(temp, rsslErrorInfo.rsslError.rsslErrorId);
			return;
		}
	}
	else
	{
		// Routing session configuration, so encode this buffer into _encodeBuffer and then memcpy it to fan out
		UInt32 msgSize = _getMsgEncodedSize(submitMsgOpts.pRsslMsg);
		NiProviderRoutingSession* pNiProvRoutingSession = static_cast<NiProviderRoutingSession*>(_pRoutingSession);
		RsslRet ret;

		bool msgEncoded = false;
		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo(&rsslErrorInfo);
		int sentMsgCount = 0;


		for (UInt32 i = 0; i < pNiProvRoutingSession->routingChannelList.size(); i++)
		{
			NiProviderRoutingSessionChannel* pRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pNiProvRoutingSession->routingChannelList[i]);
			RsslReactorChannel* pReactorChannel = pRoutingChannel->pReactorChannel;

			if (pReactorChannel == NULL || pReactorChannel->pRsslChannel == NULL || pRoutingChannel->channelState < OmmBaseImpl::LoginStreamOpenOkEnum)
				continue;

			// If we haven't encoded the message yet, do this here.
			if (!msgEncoded)
			{
				if (msgSize > _encodeBufferAllocatedLength)
				{
					if (_encodeBuffer.data != NULL)
					{
						free(_encodeBuffer.data);
						_encodeBuffer.data = NULL;
					}

					_encodeBuffer.data = (char*)malloc(msgSize);

					if (_encodeBuffer.data == NULL)
					{
						_userLock.unlock();
						handleMee("Failed to allocate memory in OmmNiProviderImpl::submit(GenericMsg&)");
						return;
					}

					_encodeBufferAllocatedLength = msgSize;
				}

				_encodeBuffer.length = _encodeBufferAllocatedLength;

				while (true)
				{
					rsslClearEncodeIterator(&_encodeIterator);
					rsslSetEncodeIteratorRWFVersion(&_encodeIterator, pRoutingChannel->pReactorChannel->majorVersion, pRoutingChannel->pReactorChannel->minorVersion);
					rsslSetEncodeIteratorBuffer(&_encodeIterator, &_encodeBuffer);

					ret = rsslEncodeMsg(&_encodeIterator, submitMsgOpts.pRsslMsg);

					if (ret == RSSL_RET_SUCCESS)
					{
						_encodeBuffer.length = rsslGetEncodedBufferLength(&_encodeIterator);
						break;
					}
					else if (ret == RSSL_RET_BUFFER_TOO_SMALL)
					{
						free(_encodeBuffer.data);
						_encodeBuffer.data = NULL;

						_encodeBuffer.data = (char*)malloc(_encodeBufferAllocatedLength * 2);

						if (_encodeBuffer.data == NULL)
						{
							_userLock.unlock();
							handleMee("Failed to allocate memory in OmmNiProviderImpl::submit(RefreshMsg&)");
							return;
						}

						_encodeBufferAllocatedLength = _encodeBufferAllocatedLength * 2;
						_encodeBuffer.length = _encodeBufferAllocatedLength;
					}
					else
					{
						_userLock.unlock();
						EmaString temp("Internal error: rsslEncodeMsg() failed in OmmNiProviderImpl::submit(GenericMsg&).");
						temp.append(CR).append(_activeChannel->toString()).append(CR)
							.append("Error Id ").append(ret);
						handleIue(temp, ret);
						return;
					}
				}

				msgEncoded = true;
			}

			// Get a buffer from the current Reactor Channel
			pRoutingChannel->_transportBuffer = rsslReactorGetBuffer(pReactorChannel, _encodeBuffer.length, RSSL_FALSE, &rsslErrorInfo);

			if (pRoutingChannel->_transportBuffer == NULL)
			{
				Int32 iueRetCode;

				if (rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_BUFFER_NO_BUFFERS)
					iueRetCode = OmmInvalidUsageException::NoBuffersEnum;
				else
					iueRetCode = OmmInvalidUsageException::FailureEnum;


				// Release all previously allocated buffers from rsslReactorGetBuffer
				for (UInt32 j = 0; j < i; j++)
				{
					NiProviderRoutingSessionChannel* pPrevRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pNiProvRoutingSession->routingChannelList[j]);
					if (pPrevRoutingChannel->_transportBuffer)
					{
						rsslReactorReleaseBuffer(pPrevRoutingChannel->pReactorChannel, pPrevRoutingChannel->_transportBuffer, &rsslErrorInfo);
						pPrevRoutingChannel->_transportBuffer = NULL;
					}
				}

				_userLock.unlock();
				EmaString temp("Internal error: rsslReactorGetBuffer() failed in OmmNiProviderImpl::submit(GenericMsg&).");
				temp.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Error Text ").append(rsslErrorInfo.rsslError.text);
				handleIue(temp, iueRetCode);
				return;
			}
		}

		// Copy the encoded message into the transport buffers for each channel and submit them to the Reactor
		for (UInt32 i = 0; i < pNiProvRoutingSession->routingChannelList.size(); i++)
		{
			NiProviderRoutingSessionChannel* pRoutingChannel = static_cast<NiProviderRoutingSessionChannel*>(pNiProvRoutingSession->routingChannelList[i]);
			RsslReactorChannel* pReactorChannel = pRoutingChannel->pReactorChannel;

			// Do not attempt to submit if:
			// pReactorChannel is null(the reactor channel has been closed for this routing channel)
			// there isn't a valid RsslChannel in the Reactor Channel
			// or if the routing channel's state has not been progressed past the LoginStreamOpenOk state
			// Note: for Directory refreshes, this will allow the initial directory message to go through when user-specified directory is configured for the NiProvider. 
			//		 Any item refreshes will fail due to a directory mismatch prior to this point(as the directory cache won't have the services associated with it), and 
			//		 since recover user source directory is always turned on, EMA will recover the directory automatically after receiving a login Open/OK
			if (pReactorChannel == NULL || pReactorChannel->pRsslChannel == NULL || pRoutingChannel->channelState < OmmBaseImpl::LoginStreamOpenOkEnum)
				continue;

			memcpy(pRoutingChannel->_transportBuffer->data, _encodeBuffer.data, _encodeBuffer.length);
			pRoutingChannel->_transportBuffer->length = _encodeBuffer.length;
			RsslReactorSubmitOptions submitOpts;
			rsslClearReactorSubmitOptions(&submitOpts);

			if (rsslReactorSubmit(_pRsslReactor, pReactorChannel, pRoutingChannel->_transportBuffer, &submitOpts, &rsslErrorInfo) < RSSL_RET_SUCCESS)
			{
				// If this fails(most likely due to a channel down), log the error and release the buffer.  We will throw an exception if all fail
				if (OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit(GenericMsg&).");
					temp.append(CR).append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
						.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
						.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
						.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
						.append("Error Text ").append(rsslErrorInfo.rsslError.text);

					getOmmLoggerClient().log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
				}

				rsslReactorReleaseBuffer(pReactorChannel, pRoutingChannel->_transportBuffer, &rsslErrorInfo);
				pRoutingChannel->_transportBuffer = NULL;

			}
			else
			{
				sentMsgCount++;
			}

			// Clear the _transportBuffer for this channel
			pRoutingChannel->_transportBuffer = NULL;

		}

		if (sentMsgCount == 0)
		{
			_userLock.unlock();
			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit(GenericMsg&).");
			temp.append(CR).append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);

			handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
			return;
		}


	}

	_userLock.unlock();
}

void OmmNiProviderImpl::submit(const AckMsg&, UInt64)
{
	handleIue("Non-interactive provider does not support submitting AckMsg.", OmmInvalidUsageException::InvalidOperationEnum);
}

void OmmNiProviderImpl::submit(const PackedMsg& packedMsg)
{
	_userLock.lock();

	PackedMsgImpl* packedMsgImpl = packedMsg._pImpl;

	if ((_pRoutingSession == NULL && _activeChannel == NULL) || (_pRoutingSession != NULL && (_pRoutingSession->activeChannelCount == 0 || _pRoutingSession->getReconnectingCount() == _pRoutingSession->activeChannelCount)))
	{
		_userLock.unlock();
		packedMsgImpl->clear();
		EmaString temp("submit( const packedMsg& ) error: No active channel to send message.");
		temp.append(CR);
		handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
		return;
	}

	RsslReactorSubmitOptions submitOpts;
	RsslErrorInfo rsslErrorInfo;
	RsslRet ret = RSSL_RET_FAILURE;

	rsslClearReactorSubmitOptions(&submitOpts);


	if (_pRoutingSession == NULL)
	{
		RsslBuffer* transportBuffer = packedMsgImpl->getTransportBuffer();

		if (transportBuffer == NULL)
		{
			_userLock.unlock();
			EmaString temp("Attempt to submit PackedMsg with non init transport buffer");
			handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
			return;
		}

		transportBuffer->length = 0;

		if ((ret = rsslReactorSubmit(_pRsslReactor, _pReactorChannel, transportBuffer, &submitOpts, &rsslErrorInfo)) < RSSL_RET_SUCCESS)
		{
			packedMsgImpl->clear();

			EmaString temp("Internal error: rsslReactorSubmit() failed in OmmNiProviderImpl::submit( const PackedMsg& ).");
			temp.append(CR).append(_activeChannel->toString()).append(CR)
				.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);
			_userLock.unlock();
			handleIue(temp, rsslErrorInfo.rsslError.rsslErrorId);
			return;
		}

		packedMsgImpl->setTransportBuffer(NULL);
		packedMsgImpl->clear();
	}
	else
	{
		int sentBufferCount = 0;
		EmaVector<NiProvSessionTransportBuffer*>& transportBufferList = packedMsgImpl->getRwfSessionBufferList();

		// Iterate throught the packed buffer list
		for (UInt32 i = 0; i < transportBufferList.size(); ++i)
		{
			// If there isn't a buffer in this index, or the current reactor channel's RSSL channel does not match the index's RSSL channel, continue.
			if (transportBufferList[i]->pBuffer == NULL || transportBufferList[i]->pSessionChannel == NULL || transportBufferList[i]->pSessionChannel->pReactorChannel == NULL || transportBufferList[i]->pSessionChannel->pReactorChannel->pRsslChannel == NULL || transportBufferList[i]->pSessionChannel->pReactorChannel->pRsslChannel->state != RSSL_CH_STATE_ACTIVE || transportBufferList[i]->pSessionChannel->pReactorChannel->pRsslChannel != transportBufferList[i]->pRsslChannel)
			{
				transportBufferList[i]->clear();
				continue;
			}

			transportBufferList[i]->pBuffer->length = 0;
			
			if ((ret = rsslReactorSubmit(_pRsslReactor, transportBufferList[i]->pSessionChannel->pReactorChannel, transportBufferList[i]->pBuffer, &submitOpts, &rsslErrorInfo)) < RSSL_RET_SUCCESS)
			{
				// Just log the issue here, do not 
				if (OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Internal error: rsslReactorSubmit() failed in OmmNiProviderImpl::submit( const PackedMsg& ).");
					temp.append(CR).append(_activeChannel->toString()).append(CR)
						.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
						.append("Session Channel Name ").append(transportBufferList[i]->pSessionChannel->name).append(CR)
						.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
						.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
						.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
						.append("Error Text ").append(rsslErrorInfo.rsslError.text);

					getOmmLoggerClient().log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
				}	
				transportBufferList[i]->clear();
			}
			else
			{
				transportBufferList[i]->clear();
				sentBufferCount++;
			}
		}

		if (sentBufferCount == 0)
		{
			EmaString temp("Failed to submit packed buffer on all channels.  See log for more details.");
			_userLock.unlock();
			handleIue(temp, OmmInvalidUsageException::FailureEnum);
			return;
		}
		
		packedMsgImpl->reset();
	}

	_userLock.unlock();
}

void OmmNiProviderImpl::setRsslReactorChannelRole( RsslReactorChannelRole& role)
{
	RsslReactorOMMNIProviderRole& niProviderRole = role.ommNIProviderRole;
	rsslClearOMMNIProviderRole( &niProviderRole );
	niProviderRole.pLoginRequest = getLoginCallbackClient().getLoginRequest();
	niProviderRole.pDirectoryRefresh = 0;
	niProviderRole.loginMsgCallback = OmmBaseImpl::loginCallback;
	niProviderRole.base.defaultMsgCallback = OmmBaseImpl::itemCallback;
	niProviderRole.base.channelEventCallback = OmmBaseImpl::channelCallback;
}

bool OmmNiProviderImpl::realocateBuffer( RsslBuffer* pBuffer1, RsslBuffer* pBuffer2, RsslEncodeIterator* pEncIter, EmaString& errorText )
{
	RsslBuffer* pTemp1 = pBuffer1->data ? pBuffer1 : pBuffer2;
	RsslBuffer* pTemp2 = pBuffer1->data ? pBuffer2 : pBuffer1;

	pTemp2->length = pTemp1->length << 1;
	pTemp2->data = (char*) malloc( pTemp2->length * sizeof( char ) );

	if ( !pTemp2->data )
	{
		free( pTemp1->data );
		rsslClearBuffer( pTemp1 );

		errorText.set( "Failed to allocate memory in OmmNiProviderImpl::swapServiceNameAndId()" );
		return false;
	}

	rsslRealignEncodeIteratorBuffer( pEncIter, pTemp2 );

	free( pTemp1->data );
	rsslClearBuffer( pTemp1 );

	return true;
}

size_t OmmNiProviderImpl::UInt64rHasher::operator()( const UInt64& value ) const
{
	return value;
}

bool OmmNiProviderImpl::UInt64Equal_To::operator()( const UInt64& x, const UInt64& y ) const
{
	return x == y;
}

bool OmmNiProviderImpl::isApiDispatching() const
{
	return _activeConfig.operationModel == OmmNiProviderConfig::ApiDispatchEnum ? true : false;
}

bool OmmNiProviderImpl::getServiceId( const EmaString& serviceName, UInt64& serviceId )
{
	bool retCode = false;

	UInt64* pServiceId = _ommNiProviderDirectoryStore.getServiceIdByName(&serviceName);

	if ( pServiceId )
	{
		serviceId = *pServiceId;
		retCode = true;
	}

	return retCode;
}

bool OmmNiProviderImpl::getServiceName( UInt64 serviceId, EmaString& serviceName )
{
	bool retCode = false;

	EmaStringPtr* pServiceName = _ommNiProviderDirectoryStore.getServiceNameById(serviceId);

	if ( pServiceName )
	{
		serviceName = **pServiceName;
		retCode = true;
	}

	return retCode;
}

Int32 OmmNiProviderImpl::getNextProviderStreamId()
{
	if (_reusedProviderStreamIds.size() == 0)
	{
		if ( _nextProviderStreamId == INT_MIN )
		{
			handleIue("Unable to obtain next available stream id for submitting item.", OmmInvalidUsageException::InternalErrorEnum);
		}

		return --_nextProviderStreamId;
	}
	else
	{
		StreamId* tmp(_reusedProviderStreamIds.pop_back());
		Int32 retVal = (*tmp)();
		delete tmp;
		return retVal;
	}
}

void OmmNiProviderImpl::returnProviderStreamId(Int32 streamId)
{
	try
	{
		StreamId* sId = new StreamId(streamId);
		_reusedProviderStreamIds.push_back(sId);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory in OmmNiProviderImpl::returnProviderStreamId()");
	}
}

void OmmNiProviderImpl::setActiveRsslReactorChannel( Channel* activeChannel )
{
	_activeChannel = activeChannel;
}

void OmmNiProviderImpl::unsetActiveRsslReactorChannel( Channel* cancelChannel )
{
	if (cancelChannel == _activeChannel)
		_activeChannel = NULL;
}
 
DirectoryServiceStore& OmmNiProviderImpl::getDirectoryServiceStore() const
{
	return (DirectoryServiceStore&)_ommNiProviderDirectoryStore;
}

OmmCommonImpl::ImplementationType OmmNiProviderImpl::getImplType()
{
	return OmmCommonImpl::NiProviderEnum;
}

ItemWatchList& OmmNiProviderImpl::getItemWatchList()
{
	return _itemWatchList;
}

UInt64 OmmNiProviderImpl::generateHandle(UInt64 duplicateHandle)
{
	UInt64 newHandle = (duplicateHandle + 1);

	while ( _handleToStreamInfo.find(newHandle) )
	{
		++newHandle;
	}

	return newHandle;
}

void OmmNiProviderImpl::onServiceDelete(ClientSession* clientSession, RsslUInt serviceId)
{
	_itemWatchList.processServiceDelete(clientSession, serviceId);
}

UInt32 OmmNiProviderImpl::getRequestTimeout()
{
	return _activeConfig.requestTimeout;
}

/* method getConnectedClientChannelInfo not supported for NIProvider applications. Function is
 * defined here because the function is defined in a common base class
 */
void OmmNiProviderImpl::getConnectedClientChannelInfo(EmaVector<ChannelInformation>&) {
  throwIueException( "NIProvider applications do not support the getConnectedClientChannelInfo method", OmmInvalidUsageException::InvalidOperationEnum );
}

/* method getConnectedClientChannelStats not supported for NIProvider applications. Function is
 * defined here because the function is defined in a common base class
 */
void OmmNiProviderImpl::getConnectedClientChannelStats(UInt64, ChannelStatistics&) {
  throwIueException("NIProvider applications do not support the getConnectedClientChannelStats method", OmmInvalidUsageException::InvalidOperationEnum);
}

void OmmNiProviderImpl::getChannelInformation(ChannelInformation& ci) {
	RsslReactorChannel* rsslReactorChannel;
	if ((rsslReactorChannel = _pReactorChannel) == 0) {
		ci.clear();
		return;
	}

	return ChannelInfoImpl::getChannelInformationImpl(rsslReactorChannel, OmmCommonImpl::NiProviderEnum, ci);
}

void OmmNiProviderImpl::getSessionInformation(EmaVector<ChannelInformation>& infoVector)
{
	infoVector.clear();
	if (_state == NotInitializedEnum || _pRoutingSession == NULL) {

		return;
	}

	ChannelInformation channelInfo;
	for (UInt32 i = 0; i < _pRoutingSession->routingChannelList.size(); ++i)
	{
		if (_pRoutingSession->routingChannelList[i] == NULL || _pRoutingSession->routingChannelList[i]->pReactorChannel == NULL)
			continue;

		// This will clear channelInfo, so we do not need to clear it prior to now.
		ChannelInfoImpl::getChannelInformationImpl(_pRoutingSession->routingChannelList[i]->pReactorChannel, OmmCommonImpl::NiProviderEnum, channelInfo);

		infoVector.push_back(channelInfo);
	}
	return;
}

void OmmNiProviderImpl::modifyIOCtl(Int32 code, Int32 value, UInt64 handle)
{
	_userLock.lock();

	if (_activeChannel == NULL || _pReactorChannel == NULL)
	{
		_userLock.unlock();
		EmaString temp("No active channel to modify I/O option.");
		handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
		return;
	}

	RsslError rsslError;
	RsslRet ret = rsslIoctl(_pReactorChannel->pRsslChannel, (RsslIoctlCodes)code, &value, &rsslError);

	if (ret != RSSL_RET_SUCCESS)
	{
		_userLock.unlock();
		EmaString temp("Failed to modify I/O option for code = ");
			temp.append(code).append(".").append(CR)
			.append("RsslChannel ").append(ptrToStringAsHex(rsslError.channel)).append(CR)
			.append("Error Id ").append(rsslError.rsslErrorId).append(CR)
			.append("Internal sysError ").append(rsslError.sysError).append(CR)
			.append("Error Text ").append(rsslError.text);
		handleIue(temp, ret);
		return;
	}

	_userLock.unlock();
}

void OmmNiProviderImpl::closeChannel(UInt64 clientHandle)
{
	throwIueException("NIProvider applications do not support the closeChannel() method", OmmInvalidUsageException::InvalidOperationEnum);
}

OmmNiProviderImpl::StreamInfoPtr* OmmNiProviderImpl::getStreamInfo(UInt64 handle)
{
	_userLock.lock();

	StreamInfoPtr* streamPtr =  const_cast<OmmNiProviderImpl::StreamInfoPtr*>(_handleToStreamInfo.find(handle));

	_userLock.unlock();

	return streamPtr;
}

