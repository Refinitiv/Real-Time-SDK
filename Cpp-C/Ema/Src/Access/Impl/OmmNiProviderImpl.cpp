/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the projects LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmProvider.h"
#include "OmmNiProviderImpl.h"
#include "OmmNiProviderConfigImpl.h"
#include "ExceptionTranslator.h"
#include "LoginCallbackClient.h"
#include "RefreshMsgEncoder.h"
#include "UpdateMsgEncoder.h"
#include "StatusMsgEncoder.h"
#include "GenericMsgEncoder.h"
#include "Utilities.h"
#include "EmaRdm.h"
#include "OmmQosDecoder.h"
#include "StreamId.h"
#include "DirectoryServiceStore.h"

#include <new>

#ifdef WIN32
#pragma warning( disable : 4355)
#endif

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

OmmNiProviderImpl::OmmNiProviderImpl( OmmProvider* ommProvider, const OmmNiProviderConfig& config ) :
	_activeConfig(),
	OmmProviderImpl( ommProvider ),
	OmmBaseImpl( _activeConfig ),
	_handleToStreamInfo(),
	_streamInfoList(),
	_bIsStreamIdZeroRefreshSubmitted( false ),
	_ommNiProviderDirectoryStore(*this, _activeConfig),
	_nextProviderStreamId(0),
	_reusedProviderStreamIds()
{
	_activeConfig.operationModel = config._pImpl->getOperationModel();

	_activeConfig.directoryAdminControl = config.getConfigImpl()->getAdminControlDirectory();

	_rsslDirectoryMsgBuffer.length = 2048;
	_rsslDirectoryMsgBuffer.data = (char*)malloc(_rsslDirectoryMsgBuffer.length * sizeof(char));
	if (!_rsslDirectoryMsgBuffer.data)
	{
		handleMee("Failed to allocate memory in OmmNiProviderImpl::OmmNiProviderImpl()");
		return;
	}

	initialize( config._pImpl );

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
	_reusedProviderStreamIds()
{
	_activeConfig.operationModel = config._pImpl->getOperationModel();

	_activeConfig.directoryAdminControl = config.getConfigImpl()->getAdminControlDirectory();

	_rsslDirectoryMsgBuffer.length = 2048;
	_rsslDirectoryMsgBuffer.data = (char*)malloc(_rsslDirectoryMsgBuffer.length * sizeof(char));
	if (!_rsslDirectoryMsgBuffer.data)
	{
		handleMee("Failed to allocate memory in OmmNiProviderImpl::OmmNiProviderImpl()");
		return;
	}

	initialize(config._pImpl);

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
	_reusedProviderStreamIds()
{
	_activeConfig.operationModel = config._pImpl->getOperationModel();

	_activeConfig.directoryAdminControl = config.getConfigImpl()->getAdminControlDirectory();

	_rsslDirectoryMsgBuffer.length = 2048;
	_rsslDirectoryMsgBuffer.data = (char*)malloc(_rsslDirectoryMsgBuffer.length * sizeof(char));
	if (!_rsslDirectoryMsgBuffer.data)
	{
		handleMee("Failed to allocate memory in OmmNiProviderImpl::OmmNiProviderImpl()");
		return;
	}

	initialize(config._pImpl);

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
	_reusedProviderStreamIds()
{
	_activeConfig.operationModel = config._pImpl->getOperationModel();

	_activeConfig.directoryAdminControl = config.getConfigImpl()->getAdminControlDirectory();

	_rsslDirectoryMsgBuffer.length = 2048;
	_rsslDirectoryMsgBuffer.data = (char*)malloc(_rsslDirectoryMsgBuffer.length * sizeof(char));
	if (!_rsslDirectoryMsgBuffer.data)
	{
		handleMee("Failed to allocate memory in OmmNiProviderImpl::OmmNiProviderImpl()");
		return;
	}

	initialize(config._pImpl);

	_handleToStreamInfo.rehash(_activeConfig.itemCountHint);
}

OmmNiProviderImpl::~OmmNiProviderImpl()
{
	free(_rsslDirectoryMsgBuffer.data);

	OmmBaseImpl::uninitialize( false, false );
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
			if (_ommNiProviderDirectoryStore.decodeSourceDirectory(&rsslMsg.refreshMsg.msgBase.encDataBody, text) == false)
			{
				handleIue(text);
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
	catch (std::bad_alloc)
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

		if ( RSSL_RET_SUCCESS != rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) )
		{
			free( rsslMsgBuffer.data );
			handleIue( "Internal error. Failed to set encode iterator version in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		if ( RSSL_RET_SUCCESS != rsslSetEncodeIteratorBuffer( &eIter, &rsslMsgBuffer ) )
		{
			free( rsslMsgBuffer.data );
			handleIue( "Internal error. Failed to set encode iterator buffer in OmmNiProviderImpl::loadDirectory()" );
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
		RsslRet retCode = rsslEncodeRDMDirectoryMsg( &eIter, (RsslRDMDirectoryMsg*) &directoryRefresh, &rsslMsgBuffer.length, &rsslErrorInfo );

		while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
		{
			free( rsslMsgBuffer.data );

			rsslMsgBuffer.length += rsslMsgBuffer.length;
			rsslMsgBuffer.data = (char*) malloc( sizeof( char ) * rsslMsgBuffer.length );

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

			handleIue( temp );
			return;
		}

		try
		{
			_activeConfig.pDirectoryRefreshMsg = new AdminRefreshMsg( 0 );
		}
		catch ( std::bad_alloc )
		{
			DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
			handleMee( "Failed to allocate memory in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		RsslDecodeIterator decIter;
		rsslClearDecodeIterator( &decIter );

		if ( rsslSetDecodeIteratorRWFVersion( &decIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) != RSSL_RET_SUCCESS )
		{
			DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
			handleIue( "Internal error. Failed to set decode iterator version in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		if ( rsslSetDecodeIteratorBuffer( &decIter, &rsslMsgBuffer ) != RSSL_RET_SUCCESS )
		{
			DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
			handleIue( "Internal error. Failed to set decode iterator buffer in OmmNiProviderImpl::loadDirectory()" );
			return;
		}

		RsslRefreshMsg rsslRefreshMsg;
		rsslClearRefreshMsg( &rsslRefreshMsg );
		if ( rsslDecodeMsg( &decIter, (RsslMsg*) &rsslRefreshMsg ) != RSSL_RET_SUCCESS )
		{
			DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
			handleIue( "Internal error. Failed to decode message in OmmNiProviderImpl::loadDirectory()" );
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
			handleIue( temp );
			return;
		}
	}

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*) _activeConfig.pDirectoryRefreshMsg->get();

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	Channel* pChannel = getChannelCallbackClient().getChannelList().front();
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::loadDirectory()." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );
		return;
	}
	else
	{
		_bIsStreamIdZeroRefreshSubmitted = true;

		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Configured source directory was sent out on the wire." );
	}
}

void OmmNiProviderImpl::reLoadDirectory()
{
	reLoadConfigSourceDirectory();
	reLoadUserSubmitSourceDirectory();
}

void OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()
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

	if ( RSSL_RET_SUCCESS != rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) )
	{
		free( rsslMsgBuffer.data );
		handleIue( "Internal error. Failed to set encode iterator version in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
		return;
	}

	if ( RSSL_RET_SUCCESS != rsslSetEncodeIteratorBuffer( &eIter, &rsslMsgBuffer ) )
	{
		free( rsslMsgBuffer.data );
		handleIue( "Internal error. Failed to set encode iterator buffer in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
		return;
	}

	RsslRDMDirectoryRefresh directoryRefresh;
	rsslClearRDMDirectoryRefresh( &directoryRefresh );

	directoryRefresh.filter = RDM_DIRECTORY_SERVICE_INFO_FILTER | RDM_DIRECTORY_SERVICE_STATE_FILTER;

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
	RsslRet retCode = rsslEncodeRDMDirectoryMsg( &eIter, (RsslRDMDirectoryMsg*) &directoryRefresh, &rsslMsgBuffer.length, &rsslErrorInfo );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		free( rsslMsgBuffer.data );

		rsslMsgBuffer.length += rsslMsgBuffer.length;
		rsslMsgBuffer.data = (char*) malloc( sizeof( char ) * rsslMsgBuffer.length );

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

		handleIue( temp );
		return;
	}

	RsslDecodeIterator decIter;
	rsslClearDecodeIterator( &decIter );

	if ( rsslSetDecodeIteratorRWFVersion( &decIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) != RSSL_RET_SUCCESS )
	{
		DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
		handleIue( "Internal error. Failed to set decode iterator version in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
		return;
	}

	if ( rsslSetDecodeIteratorBuffer( &decIter, &rsslMsgBuffer ) != RSSL_RET_SUCCESS )
	{
		DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
		handleIue( "Internal error. Failed to set decode iterator buffer in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
		return;
	}

	RsslRefreshMsg rsslRefreshMsg;
	rsslClearRefreshMsg( &rsslRefreshMsg );
	if ( rsslDecodeMsg( &decIter, (RsslMsg*) &rsslRefreshMsg ) != RSSL_RET_SUCCESS )
	{
		DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
		handleIue( "Internal error. Failed to decode message in OmmNiProviderImpl::reLoadUserSubmitSourceDirectory()" );
		return;
	}

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*) &rsslRefreshMsg;

	EmaString temp;

	if (_activeConfig.removeItemsOnDisconnect && !_ommNiProviderDirectoryStore.decodeSourceDirectory(&submitMsgOpts.pRsslMsg->msgBase.encDataBody, temp))
	{
		DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);
		handleIue( temp );
		return;
	}

	clearRsslErrorInfo( &rsslErrorInfo );
	Channel* pChannel = getChannelCallbackClient().getChannelList().front();
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		DirectoryServiceStore::freeMemory(directoryRefresh, &rsslMsgBuffer);

		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::loadDirectory()." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );
		return;
	}
	else
	{
		_bIsStreamIdZeroRefreshSubmitted = true;

		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "User submitted source directoies were sent out on the wire after reconnect." );
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

void OmmNiProviderImpl::reLoadConfigSourceDirectory()
{
	if ( !_activeConfig.pDirectoryRefreshMsg )
		return;

	if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Reload of configured source directories." );

	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*) _activeConfig.pDirectoryRefreshMsg->get();

	EmaString temp;

	if (_activeConfig.removeItemsOnDisconnect && !_ommNiProviderDirectoryStore.decodeSourceDirectory(&submitMsgOpts.pRsslMsg->msgBase.encDataBody, temp))
	{
		handleIue( temp );
		return;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	Channel* pChannel = getChannelCallbackClient().getChannelList().front();
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::reLoadConfigSourceDirectory()." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );
	}
	else
	{
		_bIsStreamIdZeroRefreshSubmitted = true;

		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, "Configured source directoies were sent out on the wire after reconnect." );
	}
}

void OmmNiProviderImpl::processChannelEvent( RsslReactorChannelEvent* pEvent )
{
	switch ( pEvent->channelEventType )
	{
	case RSSL_RC_CET_CHANNEL_DOWN:
	case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		if ( _activeConfig.removeItemsOnDisconnect )
			removeItems();
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

Int64 OmmNiProviderImpl::dispatch( Int64 timeOut )
{
	if ( _activeConfig.operationModel == OmmNiProviderConfig::UserDispatchEnum && !_atExit )
		return rsslReactorDispatchLoop( timeOut, _activeConfig.maxDispatchCountUserThread, _bMsgDispatched );

	return OmmProvider::TimeoutEnum;
}

UInt64 OmmNiProviderImpl::registerClient( const ReqMsg& reqMsg, OmmProviderClient& ommProvClient, void* closure, UInt64 parentHandle )
{
	_userLock.lock();

	UInt64 handle = _pItemCallbackClient ? _pItemCallbackClient->registerClient( reqMsg, ommProvClient, closure, parentHandle ) : 0;

	if ( handle )
	{
		try
		{
			// todo ... check for possible conflict with a user assigned handle

			StreamInfoPtr pTemp = new StreamInfo( reinterpret_cast<Item*>( handle )->getStreamId() );
			_handleToStreamInfo.insert( handle, pTemp );
			_streamInfoList.push_back( pTemp );
		}
		catch ( std::bad_alloc )
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

	if (_pItemCallbackClient) _pItemCallbackClient->reissue(reqMsg, handle);

	_userLock.unlock();
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

	if ( ( *pTempStreamInfoPtr )->_streamId < 0 )
	{
		_userLock.unlock();
		handleIhe( handle, "Attempt to unregister a handle that was not registered." );
		return;
	}

	_streamInfoList.removeValue( *pTempStreamInfoPtr );
	delete *pTempStreamInfoPtr;
	_handleToStreamInfo.erase( handle );

	_userLock.unlock();

	OmmBaseImpl::unregister( handle );
}

void OmmNiProviderImpl::submit( const RefreshMsg& msg, UInt64 handle )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	submitMsgOpts.pRsslMsg = ( RsslMsg* )static_cast<const RefreshMsgEncoder&>( msg.getEncoder() ).getRsslRefreshMsg();

	bool bHandleAdded = false;

	_userLock.lock();

	if ( !_pChannelCallbackClient )
	{
		_userLock.unlock();
		return;
	}

	StreamInfoPtr* pStreamInfoPtr = _handleToStreamInfo.find( handle );

	if ( pStreamInfoPtr && ( *pStreamInfoPtr )->_streamId > 0 )
	{
		_userLock.unlock();
		handleIhe( handle, "Attempt to submit( const RefreshMsg& ) using a registered handle." );
		return;
	}

	Channel* pChannel = getChannelCallbackClient().getChannelList().front();

	if (pChannel == NULL)
	{
		_userLock.unlock();
		EmaString temp("No active channel to send message.");
		handleIue(temp);
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
			handleIue(temp);
			return;
		}

		EmaString temp;

		if (!_ommNiProviderDirectoryStore.decodeSourceDirectory(&submitMsgOpts.pRsslMsg->msgBase.encDataBody, temp))
		{
			_userLock.unlock();
			handleIue(temp);
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
					submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();
					StreamInfoPtr pTemp = new StreamInfo(submitMsgOpts.pRsslMsg->msgBase.streamId);
					_handleToStreamInfo.insert(handle, pTemp);
					_streamInfoList.push_back(pTemp);
					bHandleAdded = true;
				}
				catch (std::bad_alloc)
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

		const RefreshMsgEncoder& enc = static_cast<const RefreshMsgEncoder&>( msg.getEncoder() );

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
				handleIue( temp );
				return;
			}
			else if ( *pServiceId > 0xFFFF )
			{
				_userLock.unlock();

				EmaString temp( "Attempt to submit initial RefreshMsg with service name of " );
				temp.append( serviceName ).append( " whose matching service id of " ).append( *pServiceId ).append( " is out of range. Dropping this RefreshMsg." );
				handleIue( temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = (RsslUInt16) *pServiceId;
			submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
			submitMsgOpts.pRsslMsg->refreshMsg.flags &= ~RSSL_RFMF_SOLICITED;

			submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId,
										submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
										submitMsgOpts.pRsslMsg->msgBase.domainType);
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc )
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
				handleIue( temp );
				return;
			}

			submitMsgOpts.pRsslMsg->refreshMsg.flags &= ~RSSL_RFMF_SOLICITED;

			submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
														submitMsgOpts.pRsslMsg->msgBase.domainType);
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc )
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
			handleIue( "Attempt to submit initial RefreshMsg without service name or id. Dropping this RefreshMsg." );
			return;
		}
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		if ( bHandleAdded )
		{
			StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find( handle );
			_streamInfoList.removeValue( *pTempStreamInfoPtr );
			delete *pTempStreamInfoPtr;
			_handleToStreamInfo.erase( handle );
			returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
		}

		_userLock.unlock();

		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit( const RefreshMsg& )." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );

		return;
	}

	if ( submitMsgOpts.pRsslMsg->refreshMsg.state.streamState == OmmState::ClosedEnum ||
		submitMsgOpts.pRsslMsg->refreshMsg.state.streamState == OmmState::ClosedRecoverEnum ||
		submitMsgOpts.pRsslMsg->refreshMsg.state.streamState == OmmState::ClosedRedirectedEnum )
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
	submitMsgOpts.pRsslMsg = ( RsslMsg* )static_cast<const UpdateMsgEncoder&>( msg.getEncoder() ).getRsslUpdateMsg();

	bool bHandleAdded = false;

	_userLock.lock();

	if ( !_pChannelCallbackClient )
	{
		_userLock.unlock();
		return;
	}

	StreamInfoPtr* pStreamInfoPtr = _handleToStreamInfo.find( handle );

	if ( pStreamInfoPtr && ( *pStreamInfoPtr )->_streamId > 0 )
	{
		_userLock.unlock();
		handleIhe( handle, "Attempt to submit( const UpdateMsg& ) using a registered handle." );
		return;
	}

	Channel* pChannel = getChannelCallbackClient().getChannelList().front();

	if (pChannel == NULL)
	{
		_userLock.unlock();
		EmaString temp("Internal error: No active channel exists to send data.");

		handleIue(temp);
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
			handleIue( temp );
			return;
		}

		EmaString temp;

		if ( !_ommNiProviderDirectoryStore.decodeSourceDirectory( &submitMsgOpts.pRsslMsg->msgBase.encDataBody, temp ) )
		{
			_userLock.unlock();
			handleIue( temp );
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
					StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId );
					_handleToStreamInfo.insert( handle, pTemp );
					_streamInfoList.push_back( pTemp );
					bHandleAdded = true;
				}
				catch ( std::bad_alloc )
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

		const UpdateMsgEncoder& enc = static_cast<const UpdateMsgEncoder&>( msg.getEncoder() );

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
				handleIue( temp );
				return;
			}
			else if ( *pServiceId > 0xFFFF )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit UpdateMsg with service name of " );
				temp.append( serviceName ).append( " whose matching service id of " ).append( *pServiceId ).append( " is out of range. Dropping this UpdateMsg." );
				handleIue( temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = (RsslUInt16) *pServiceId;
			submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

			submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
														submitMsgOpts.pRsslMsg->msgBase.domainType);
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc )
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
				handleIue( temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
														submitMsgOpts.pRsslMsg->msgBase.domainType);
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc )
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
			handleIue( "Attempt to submit UpdateMsg without service name or id. Dropping this UpdateMsg." );
			return;
		}
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		if ( bHandleAdded )
		{
			StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find( handle );
			_streamInfoList.removeValue( *pTempStreamInfoPtr );
			delete *pTempStreamInfoPtr;
			_handleToStreamInfo.erase( handle );
			returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
		}

		_userLock.unlock();

		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit( const UpdateMsg& )." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );
		return;
	}

	_userLock.unlock();
}

void OmmNiProviderImpl::submit( const StatusMsg& msg, UInt64 handle )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	submitMsgOpts.pRsslMsg = ( RsslMsg* )static_cast<const StatusMsgEncoder&>( msg.getEncoder() ).getRsslStatusMsg();

	bool bHandleAdded = false;

	_userLock.lock();

	if ( !_pChannelCallbackClient )
	{
		_userLock.unlock();
		return;
	}

	StreamInfoPtr* pStreamInfoPtr = _handleToStreamInfo.find( handle );

	if ( pStreamInfoPtr && ( *pStreamInfoPtr )->_streamId > 0 )
	{
		_userLock.unlock();
		handleIhe( handle, "Attempt to submit( const StatusMsg& ) using a registered handle." );
		return;
	}

	Channel* pChannel = getChannelCallbackClient().getChannelList().front();

	if (pChannel == NULL)
	{
		_userLock.unlock();
		EmaString temp("No active channel to send message.");

		handleIue(temp);
	}

	if ( submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DIRECTORY )
	{
		if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received StatusMsg with SourceDirectory domain; Handle = " );
			temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

			getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
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
					StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId );
					_handleToStreamInfo.insert( handle, pTemp );
					_streamInfoList.push_back( pTemp );
					bHandleAdded = true;
				}
				catch ( std::bad_alloc )
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

		const StatusMsgEncoder& enc = static_cast<const StatusMsgEncoder&>( msg.getEncoder() );

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
		else if ( enc.hasServiceName() )
		{
			const EmaString& serviceName = enc.getServiceName();
			RsslUInt64* pServiceId = _ommNiProviderDirectoryStore.getServiceIdByName(&serviceName);
			if ( !pServiceId )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit StatusMsg with service name of " );
				temp.append( serviceName ).append( " that was not included in the SourceDirectory. Dropping this StatusMsg." );
				handleIue( temp );
				return;
			}
			else if ( *pServiceId > 0xFFFF )
			{
				_userLock.unlock();
				EmaString temp( "Attempt to submit StatusMsg with service name of " );
				temp.append( serviceName ).append( " whose matching service id of " ).append( *pServiceId ).append( " is out of range. Dropping this StatusMsg." );
				handleIue( temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId = (RsslUInt16) *pServiceId;
			submitMsgOpts.pRsslMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

			submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
														submitMsgOpts.pRsslMsg->msgBase.domainType);
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc )
			{
				returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
				_userLock.unlock();
				handleMee( "Failed to allocate memory in OmmNiProviderImpl::submit( const StatusMsg& )" );
				return;
			}
		}
		else if ( enc.hasServiceId() )
		{
			EmaStringPtr* pServiceNamePtr = _ommNiProviderDirectoryStore.getServiceNameById(submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId);

			if ( !pServiceNamePtr )
			{
				_userLock.unlock();

				EmaString temp( "Attempt to submit StatusMsg with service id of " );
				temp.append( submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId ).append( " that was not included in the SourceDirectory. Dropping this StatusMsg." );
				handleIue( temp );
				return;
			}

			submitMsgOpts.pRsslMsg->msgBase.streamId =getNextProviderStreamId();

			try
			{
				StreamInfoPtr pTemp = new StreamInfo( submitMsgOpts.pRsslMsg->msgBase.streamId, submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
														submitMsgOpts.pRsslMsg->msgBase.domainType);
				_handleToStreamInfo.insert( handle, pTemp );
				_streamInfoList.push_back( pTemp );
				bHandleAdded = true;
			}
			catch ( std::bad_alloc )
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
			handleIue( "Attempt to submit StatusMsg without service name or id. Dropping this StatusMsg." );
			return;
		}
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		if ( bHandleAdded )
		{
			StreamInfoPtr* pTempStreamInfoPtr = _handleToStreamInfo.find( handle );
			_streamInfoList.removeValue( *pTempStreamInfoPtr );
			delete *pTempStreamInfoPtr;
			_handleToStreamInfo.erase( handle );
			returnProviderStreamId( submitMsgOpts.pRsslMsg->msgBase.streamId );
		}

		_userLock.unlock();

		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit( const StatusMsg& )." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );

		return;
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

void OmmNiProviderImpl::submit( const GenericMsg& msg, UInt64 handle )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	submitMsgOpts.pRsslMsg = ( RsslMsg* )static_cast<const GenericMsgEncoder&>( msg.getEncoder() ).getRsslGenericMsg();

	_userLock.lock();

	if ( !_pChannelCallbackClient )
	{
		_userLock.unlock();
		return;
	}

	Channel* pChannel = getChannelCallbackClient().getChannelList().front();

	if (pChannel == NULL)
	{
		_userLock.unlock();
		EmaString temp("No active channel to send message.");

		handleIue(temp);
	}

	if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "Received GenericMsg; Handle = " );
		temp.append( handle ).append( ", user assigned streamId = " ).append( submitMsgOpts.pRsslMsg->msgBase.streamId ).append( "." );

		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
	}

	StreamInfoPtr* pStreamInfoPtr = _handleToStreamInfo.find( handle );

	if ( pStreamInfoPtr )
	{
		if ( ( *pStreamInfoPtr )->_streamId > 0 )
		{
			_userLock.unlock();
			OmmBaseImpl::submit( msg, handle );
			return;
		}

		submitMsgOpts.pRsslMsg->msgBase.streamId = ( *pStreamInfoPtr )->_streamId;
		if (submitMsgOpts.pRsslMsg->msgBase.domainType == 0)
			submitMsgOpts.pRsslMsg->msgBase.domainType = (*pStreamInfoPtr)->_domainType;
	}
	else
	{
		_userLock.unlock();
		EmaString temp( "Attempt to submit GenericMsg on stream that is not open yet. Handle = " );
		temp.append( handle ).append( "." );
		handleIhe( handle, temp );
		return;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	if ( rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		_userLock.unlock();
		EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in OmmNiProviderImpl::submit( const GenericMsg& )." );
		temp.append( CR ).append( pChannel->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

		handleIue( temp );
		return;
	}

	_userLock.unlock();
}

void OmmNiProviderImpl::submit(const AckMsg&, UInt64)
{
	handleIue("Non-interactive provider does not support submmiting AckMsg.");
}

void OmmNiProviderImpl::setRsslReactorChannelRole( RsslReactorChannelRole& role )
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
		return --_nextProviderStreamId;
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
	catch (std::bad_alloc)
	{
		throwMeeException("Failed to allocate memory in OmmNiProviderImpl::returnProviderStreamId()");
	}
}
