/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
*/

#include "OmmIProviderImpl.h"
#include "OmmIProviderConfigImpl.h"
#include "ItemInfo.h"
#include "ClientSession.h"
#include "OmmProvider.h"
#include "RefreshMsgEncoder.h"
#include "UpdateMsgEncoder.h"
#include "StatusMsgEncoder.h"
#include "GenericMsgEncoder.h"
#include "Utilities.h"
#include "EmaRdm.h"
#include "OmmQosDecoder.h"
#include "DictionaryHandler.h"
#include "DirectoryHandler.h"
#include "LoginHandler.h"
#include "ServerChannelHandler.h"
#include "RdmUtilities.h"

using namespace thomsonreuters::ema::access;

OmmIProviderImpl::OmmIProviderImpl(OmmProvider* ommProvider, const OmmIProviderConfig& ommIProviderConfig, OmmProviderClient& ommProviderClient, void* closure) :
	OmmProviderImpl(ommProvider),
	OmmServerBaseImpl(_ommIProviderActiveConfig, ommProviderClient, closure),
	_ommIProviderActiveConfig(),
	_ommIProviderDirectoryStore(*this, _ommIProviderActiveConfig)
{
	_ommIProviderActiveConfig.operationModel = ommIProviderConfig._pImpl->getOperationModel();
	_ommIProviderActiveConfig.dictionaryAdminControl = ommIProviderConfig._pImpl->getAdminControlDictionary();
	_ommIProviderActiveConfig.directoryAdminControl = ommIProviderConfig._pImpl->getAdminControlDirectory();

	_storeUserSubmitted = _ommIProviderActiveConfig.directoryAdminControl == OmmIProviderConfig::ApiControlEnum ? true : false;

	_ommIProviderDirectoryStore.setClient(this);

	_rsslDirectoryMsgBuffer.length = 2048;
	_rsslDirectoryMsgBuffer.data = (char*)malloc(_rsslDirectoryMsgBuffer.length * sizeof(char));
	if (!_rsslDirectoryMsgBuffer.data)
	{
		handleMee("Failed to allocate memory in OmmIProviderImpl::OmmIProviderImpl()");
		return;
	}

	initialize(ommIProviderConfig._pImpl);
}

OmmIProviderImpl::OmmIProviderImpl(OmmProvider* ommProvider, const OmmIProviderConfig& ommIProviderConfig, OmmProviderClient& ommProviderClient, OmmProviderErrorClient& ommProviderErrorClient, void* closure) :
	OmmProviderImpl(ommProvider),
	OmmServerBaseImpl(_ommIProviderActiveConfig, ommProviderClient, ommProviderErrorClient, closure),
	_ommIProviderActiveConfig(),
	_ommIProviderDirectoryStore(*this, _ommIProviderActiveConfig),
	_storeUserSubmitted(false)
{
	_ommIProviderActiveConfig.operationModel = ommIProviderConfig._pImpl->getOperationModel();
	_ommIProviderActiveConfig.dictionaryAdminControl = ommIProviderConfig._pImpl->getAdminControlDictionary();
	_ommIProviderActiveConfig.directoryAdminControl = ommIProviderConfig._pImpl->getAdminControlDirectory();

	_storeUserSubmitted = _ommIProviderActiveConfig.directoryAdminControl == OmmIProviderConfig::ApiControlEnum ? true : false;

	_ommIProviderDirectoryStore.setClient(this);

	_rsslDirectoryMsgBuffer.length = 2048;
	_rsslDirectoryMsgBuffer.data = (char*)malloc(_rsslDirectoryMsgBuffer.length * sizeof(char));
	if (!_rsslDirectoryMsgBuffer.data)
	{
		handleMee("Failed to allocate memory in OmmIProviderImpl::OmmIProviderImpl()");
		return;
	}

	initialize(ommIProviderConfig._pImpl);
}

OmmIProviderImpl::~OmmIProviderImpl()
{
	free(_rsslDirectoryMsgBuffer.data);

	OmmServerBaseImpl::uninitialize(false, false);
}

bool OmmIProviderImpl::isApiDispatching() const
{
	return _ommIProviderActiveConfig.operationModel == OmmIProviderConfig::ApiDispatchEnum ? true : false;
}

const EmaString& OmmIProviderImpl::getInstanceName() const
{
	return _ommIProviderActiveConfig.instanceName;
}

OmmProvider* OmmIProviderImpl::getProvider() const
{
	return _pOmmProvider;
}

OmmProviderConfig::ProviderRole OmmIProviderImpl::getProviderRole() const
{
	return OmmProviderConfig::InteractiveEnum;
}

void OmmIProviderImpl::readCustomConfig(EmaConfigServerImpl* pConfigImpl)
{
	EmaString instanceNodeName(pConfigImpl->getInstanceNodeName());
	instanceNodeName.append(_activeServerConfig.configuredName).append("|");

	_ommIProviderActiveConfig.pDirectoryRefreshMsg = pConfigImpl->getDirectoryRefreshMsg();

	try
	{
		if (_storeUserSubmitted && (_ommIProviderActiveConfig.pDirectoryRefreshMsg))
		{
			RsslMsg rsslMsg;
			rsslMsg.refreshMsg = *_ommIProviderActiveConfig.pDirectoryRefreshMsg->get();

			EmaString text;
			if (_ommIProviderDirectoryStore.decodeSourceDirectory(&rsslMsg.refreshMsg.msgBase.encDataBody, text) == false)
			{
				handleIue(text);
				return;
			}

			if (_ommIProviderDirectoryStore.submitSourceDirectory(0, &rsslMsg, _rsslDirectoryMsg, _rsslDirectoryMsgBuffer, _storeUserSubmitted) == false)
			{
				return;
			}

			_ommIProviderDirectoryStore.loadConfigDirectory(pConfigImpl, false);
		}
		else
		{
			_ommIProviderDirectoryStore.loadConfigDirectory(pConfigImpl, _storeUserSubmitted);
		}
	}
	catch (std::bad_alloc)
	{
		throwMeeException("Failed to allocate memory in OmmIProviderImpl::readCustomConfig()");
	}

	UInt64 tmp = 0;
	if (pConfigImpl->get<UInt64>(instanceNodeName + "RefreshFirstRequired", tmp))
		_ommIProviderActiveConfig.refreshFirstRequired = (tmp > 0 ? true : false);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "AcceptMessageWithoutAcceptingRequests", tmp))
		_ommIProviderActiveConfig.acceptMessageWithoutAcceptingRequests = (tmp > 0 ? true : false);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "AcceptDirMessageWithoutMinFilters", tmp))
		_ommIProviderActiveConfig.acceptDirMessageWithoutMinFilters = (tmp > 0 ? true : false);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "AcceptMessageWithoutBeingLogin", tmp))
		_ommIProviderActiveConfig.acceptMessageWithoutBeingLogin = (tmp > 0 ? true : false);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "AcceptMessageSameKeyButDiffStream", tmp))
		_ommIProviderActiveConfig.acceptMessageSameKeyButDiffStream = (tmp > 0 ? true : false);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "AcceptMessageThatChangesService", tmp))
		_ommIProviderActiveConfig.acceptMessageThatChangesService = (tmp > 0 ? true : false);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "AcceptMessageWithoutQosInRange", tmp))
		_ommIProviderActiveConfig.acceptMessageWithoutQosInRange = (tmp > 0 ? true : false);

	_ommIProviderActiveConfig.directoryAdminControl = static_cast<OmmIProviderConfigImpl*>(pConfigImpl)->getAdminControlDirectory();

	_ommIProviderActiveConfig.dictionaryAdminControl = static_cast<OmmIProviderConfigImpl*>(pConfigImpl)->getAdminControlDictionary();

	if (pConfigImpl->get<UInt64>(instanceNodeName + "FieldDictionaryFragmentSize", tmp))
		_ommIProviderActiveConfig.maxFieldDictFragmentSize = tmp > 0xFFFFFFFF ? 0xFFFFFFFF : (UInt32)tmp;

	if (pConfigImpl->get<UInt64>(instanceNodeName + "EnumTypeFragmentSize", tmp))
		_ommIProviderActiveConfig.maxEnumTypeFragmentSize = tmp > 0xFFFFFFFF ? 0xFFFFFFFF : (UInt32)tmp;

	if (ProgrammaticConfigure* ppc = pConfigImpl->getProgrammaticConfigure())
	{
		// Todo: Retrieve from programmatic configure
	}
}

UInt64 OmmIProviderImpl::registerClient(const ReqMsg&, OmmProviderClient&, void* closure, UInt64 parentHandle)
{
	handleIue("Calling the OmmIProviderImpl::registerClient() method is not support in this release.");
	return 0;
}

void OmmIProviderImpl::reissue(const ReqMsg&, UInt64 handle)
{
	handleIue("Calling the OmmIProviderImpl::reissue() method is not support in this release.");
	return;
}

void OmmIProviderImpl::submit(const GenericMsg& genericMsg, UInt64 handle)
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
	const GenericMsgEncoder& genericMsgEncoder = static_cast<const GenericMsgEncoder&>(genericMsg.getEncoder());
	submitMsgOpts.pRsslMsg = (RsslMsg*)genericMsgEncoder.getRsslGenericMsg();

	_userLock.lock();

	ItemInfoPtr itemInfo = getItemInfo(handle);

	if ( (itemInfo == 0) )
	{
		_userLock.unlock();
		EmaString temp("Attempt to submit GenericMsg with non existent Handle = ");
		temp.append(handle).append(".");
		handleIue(temp);
		return;
	}

	if (itemInfo->getDomainType() == ema::rdm::MMT_DICTIONARY)
	{
		_userLock.unlock();
		EmaString temp("Attempt to submit GenericMsg with Dictionary domain while this is not supported.");
		temp.append(handle).append(".");
		handleIue(temp);
		return;
	}

	submitMsgOpts.pRsslMsg->msgBase.streamId = itemInfo->getStreamId();
	if (submitMsgOpts.pRsslMsg->msgBase.domainType == 0 )
		submitMsgOpts.pRsslMsg->msgBase.domainType = itemInfo->getDomainType();

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);
	if (rsslReactorSubmitMsg(_pRsslReactor, itemInfo->getClientSession()->getChannel(), &submitMsgOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		_userLock.unlock();
		EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmIProviderImpl::submit( const GenericMsg& ).");
		temp.append(CR).append(itemInfo->getClientSession()->toString()).append(CR)
			.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
			.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
			.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
			.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
			.append("Error Text ").append(rsslErrorInfo.rsslError.text);

		handleIue(temp);

		return;
	}

	_userLock.unlock();
}

void OmmIProviderImpl::submit(const RefreshMsg& refreshMsg, UInt64 handle)
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
	const RefreshMsgEncoder& refreshMsgEncoder = static_cast<const RefreshMsgEncoder&>(refreshMsg.getEncoder());
	submitMsgOpts.pRsslMsg = (RsslMsg*)refreshMsgEncoder.getRsslRefreshMsg();

	_userLock.lock();

	ItemInfoPtr itemInfo = getItemInfo(handle);

	if ( ( itemInfo == 0 ) && handle != 0 ) 
	{
		_userLock.unlock();
		EmaString temp("Attempt to submit RefreshMsg with non existent Handle = ");
		temp.append(handle).append(".");
		handleIue(temp);
		return;
	}

	RsslReactorChannel* pReactorChannel;

	if (submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_LOGIN)
	{
		if (handle == 0)
		{
			EmaString text("Fanout login message for item handle = ");

			RsslErrorInfo rsslErrorInfo;
			if (submit(submitMsgOpts, _pLoginHandler->getLoginItemList(), text, false, rsslErrorInfo) == false)
			{
				_userLock.unlock();
				EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmIProviderImpl::submit( const RefreshMsg& ).");
				temp.append(CR).append(itemInfo->getClientSession()->toString()).append(CR)
					.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
					.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo.rsslError.text);

				handleIue(temp);
				return;
			}

			_userLock.unlock();
			return;
		}
		else
		{
			pReactorChannel = itemInfo->getClientSession()->getChannel();
			submitMsgOpts.pRsslMsg->msgBase.streamId = itemInfo->getStreamId();

			if ((submitMsgOpts.pRsslMsg->refreshMsg.state.streamState == RsslStreamStates::RSSL_STREAM_OPEN) &&
				(submitMsgOpts.pRsslMsg->refreshMsg.state.dataState == RsslDataStates::RSSL_DATA_OK))
			{
				itemInfo->getClientSession()->setLogin(true);
			}
		}
	}
	else if (submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DIRECTORY)
	{
		if (submitMsgOpts.pRsslMsg->msgBase.containerType != RSSL_DT_MAP)
		{
			_userLock.unlock();
			EmaString temp("Attempt to submit RefreshMsg with Directory domain using container with wrong data type. Expected container data type is Map. Passed in is ");
			temp += DataType((DataType::DataTypeEnum)submitMsgOpts.pRsslMsg->msgBase.containerType).toString();
			handleIue(temp);
			return;
		}

		EmaString temp;

		if (_ommIProviderDirectoryStore.decodeSourceDirectory(&submitMsgOpts.pRsslMsg->msgBase.encDataBody, temp) == false)
		{
			_userLock.unlock();
			handleIue(temp);
			return;
		}

		ClientSession* clientSession = handle != 0 ? itemInfo->getClientSession() : 0;
		  
		if (_ommIProviderDirectoryStore.submitSourceDirectory(clientSession, submitMsgOpts.pRsslMsg, _rsslDirectoryMsg, _rsslDirectoryMsgBuffer, _storeUserSubmitted) == false)
		{
			_userLock.unlock();
			return;
		}

		if (handle == 0)
		{
			EmaString text("Fanout source directory message for item handle = ");

			RsslErrorInfo rsslErrorInfo;
			if (submit(submitMsgOpts, _pDirectoryHandler->getDirectoryItemList(), text, true, rsslErrorInfo) == false)
			{
				_userLock.unlock();
				EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmIProviderImpl::submit( const RefreshMsg& ).");
				temp.append(CR).append(itemInfo->getClientSession()->toString()).append(CR)
					.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
					.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo.rsslError.text);

				handleIue(temp);

				return;
			}

			_userLock.unlock();
			return;
		}
		else
		{
			pReactorChannel = itemInfo->getClientSession()->getChannel();
			submitMsgOpts.pRsslMsg->msgBase.streamId = itemInfo->getStreamId();
		}
	}
	else if (submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DICTIONARY)
	{
		if (submitMsgOpts.pRsslMsg->msgBase.containerType != RSSL_DT_SERIES)
		{
			_userLock.unlock();
			EmaString temp("Attempt to submit RefreshMsg with Dictionary domain using container with wrong data type. Expected container data type is Series. Passed in is ");
			temp += DataType((DataType::DataTypeEnum)submitMsgOpts.pRsslMsg->msgBase.containerType).toString();
			handleIue(temp);

			return;
		}

		if (refreshMsgEncoder.hasServiceName())
		{
			if ( encodeServiceIdFromName(refreshMsgEncoder.getServiceName(), submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
				submitMsgOpts.pRsslMsg->msgBase) )
			{
				submitMsgOpts.pRsslMsg->refreshMsg.flags |= RSSL_RFMF_HAS_MSG_KEY;
			}
			else
			{
				return;
			}
		}
		else if (refreshMsgEncoder.hasServiceId())
		{
			if (validateServiceId(submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId, submitMsgOpts.pRsslMsg->msgBase) == false)
			{
				return;
			}
		}

		if (handle == 0)
		{
			EmaString text("Fanout dictionary message for item handle = ");

			RsslErrorInfo rsslErrorInfo;
			if (submit(submitMsgOpts, _pDictionaryHandler->getDicitonaryItemList(), text, false, rsslErrorInfo) == false)
			{
				_userLock.unlock();
				EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmIProviderImpl::submit( const RefreshMsg& ).");
				temp.append(CR).append(itemInfo->getClientSession()->toString()).append(CR)
					.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
					.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo.rsslError.text);

				handleIue(temp);

				return;
			}

			_userLock.unlock();
			return;
		}
		else
		{
			pReactorChannel = itemInfo->getClientSession()->getChannel();
			submitMsgOpts.pRsslMsg->msgBase.streamId = itemInfo->getStreamId();
		}
	}
	else
	{
		if (handle == 0)
		{
			_userLock.unlock();
			EmaString temp("Attempt to fanout RefreshMsg with domain type ");
			temp.append(rdmDomainToString(submitMsgOpts.pRsslMsg->msgBase.domainType))
			.append(" while this is not supported.");
			handleIue(temp);
			return;
		}

		if (OmmLoggerClient::VerboseEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Received RefreshMsg with domain type ");
			temp.append(rdmDomainToString(submitMsgOpts.pRsslMsg->msgBase.domainType))
				.append("; handle = ").append(handle).append(", user assigned streamId = ")
				.append(submitMsgOpts.pRsslMsg->msgBase.streamId).append(".");

			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		pReactorChannel = itemInfo->getClientSession()->getChannel();
		submitMsgOpts.pRsslMsg->msgBase.streamId = itemInfo->getStreamId();

		if (refreshMsgEncoder.hasServiceName())
		{
			if ( encodeServiceIdFromName(refreshMsgEncoder.getServiceName(), submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
				submitMsgOpts.pRsslMsg->msgBase) )
			{
				submitMsgOpts.pRsslMsg->refreshMsg.flags |= RSSL_RFMF_HAS_MSG_KEY;
			}
			else
			{
				return;
			}
		}
		else if (refreshMsgEncoder.hasServiceId())
		{
			if (validateServiceId(submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId, submitMsgOpts.pRsslMsg->msgBase) == false)
			{
				return;
			}
		}

		if ( itemInfo->isPrivateStream() )
			submitMsgOpts.pRsslMsg->refreshMsg.flags |= RSSL_RFMF_PRIVATE_STREAM;

		handleItemGroup(itemInfo, submitMsgOpts.pRsslMsg->refreshMsg.groupId, submitMsgOpts.pRsslMsg->refreshMsg.state);
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);
	if (rsslReactorSubmitMsg(_pRsslReactor, pReactorChannel, &submitMsgOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		_userLock.unlock();
		EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmIProviderImpl::submit( const RefreshMsg& ).");
		temp.append(CR).append(itemInfo->getClientSession()->toString()).append(CR)
			.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
			.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
			.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
			.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
			.append("Error Text ").append(rsslErrorInfo.rsslError.text);

		handleIue(temp);

		return;
	}

	itemInfo->setSentRefresh();

	handleItemInfo(submitMsgOpts.pRsslMsg->msgBase.domainType, handle, submitMsgOpts.pRsslMsg->refreshMsg.state);

	_userLock.unlock();
}

void OmmIProviderImpl::submit(const UpdateMsg& updateMsg, UInt64 handle)
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
	const UpdateMsgEncoder& updateMsgEncoder = static_cast<const UpdateMsgEncoder&>(updateMsg.getEncoder());
	submitMsgOpts.pRsslMsg = (RsslMsg*)updateMsgEncoder.getRsslUpdateMsg();

	_userLock.lock();

	ItemInfoPtr itemInfo = getItemInfo(handle);

	if ( ( itemInfo == 0 ) && handle != 0 )
	{
		_userLock.unlock();
		EmaString temp("Attempt to submit UpdateMsg with non existent Handle = ");
		temp.append(handle).append(".");
		handleIue(temp);
		return;
	}

	if (submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_LOGIN)
	{
		_userLock.unlock();
		EmaString temp("Attempt to submit UpdateMsg with login domain while this is not supported.");
		handleIue(temp);
		return;
	}
	else if (submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DIRECTORY)
	{
		if ( _ommIProviderActiveConfig.refreshFirstRequired )
		{
			if (itemInfo && !itemInfo->isSentRefresh())
			{
				_userLock.unlock();
				EmaString temp("Attempt to submit UpdateMsg with SourceDirectory while RefreshMsg was not submitted on this stream yet. Handle = ");
				temp.append(handle).append(".");
				handleIhe(handle, temp);
				return;
			}
		}

		if (submitMsgOpts.pRsslMsg->msgBase.containerType != RSSL_DT_MAP)
		{
			_userLock.unlock();
			EmaString temp("Attempt to submit UpdateMsg with Directory domain using container with wrong data type. Expected container data type is Map. Passed in is ");
			temp += DataType((DataType::DataTypeEnum)submitMsgOpts.pRsslMsg->msgBase.containerType).toString();
			handleIue(temp);
			return;
		}

		EmaString temp;

		if (_ommIProviderDirectoryStore.decodeSourceDirectory(&submitMsgOpts.pRsslMsg->msgBase.encDataBody, temp) == false)
		{
			_userLock.unlock();
			handleIue(temp);
			return;
		}

		ClientSession* clientSession = handle != 0 ? itemInfo->getClientSession() : 0;

		if (_ommIProviderDirectoryStore.submitSourceDirectory(clientSession, submitMsgOpts.pRsslMsg, _rsslDirectoryMsg, _rsslDirectoryMsgBuffer, _storeUserSubmitted) == false)
		{
			_userLock.unlock();
			return;
		}

		if (handle == 0)
		{
			EmaString text("Fanout source directory message for item handle = ");

			RsslErrorInfo rsslErrorInfo;
			if (submit(submitMsgOpts, _pDirectoryHandler->getDirectoryItemList(), text, true, rsslErrorInfo) == false)
			{
				_userLock.unlock();
				EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmIProviderImpl::submit( const UpdateMsg& ).");
				temp.append(CR).append(itemInfo->getClientSession()->toString()).append(CR)
					.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
					.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo.rsslError.text);

				handleIue(temp);

				return;
			}

			_userLock.unlock();
			return;
		}
		else
		{
			submitMsgOpts.pRsslMsg->msgBase.streamId = itemInfo->getStreamId();
		}
	}
	else if (submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DICTIONARY)
	{
		_userLock.unlock();
		EmaString temp("Attempt to submit UpdateMsg with dictionary domain while this is not supported.");
		handleIue(temp);
		return;
	}
	else
	{
		if (handle == 0)
		{
			_userLock.unlock();
			EmaString temp("Attempt to fanout UpdateMsg with domain type ");
			temp.append(rdmDomainToString(submitMsgOpts.pRsslMsg->msgBase.domainType))
				.append(" while this is not supported.");
			handleIue(temp);
			return;
		}

		if (OmmLoggerClient::VerboseEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Received UpdateMsg with domain type ");
			temp.append(rdmDomainToString(submitMsgOpts.pRsslMsg->msgBase.domainType))
				.append("; handle = ").append(handle).append(", user assigned streamId = ")
				.append(submitMsgOpts.pRsslMsg->msgBase.streamId).append(".");

			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		if (_ommIProviderActiveConfig.refreshFirstRequired && !itemInfo->isSentRefresh() )
		{
			_userLock.unlock();
			EmaString temp("Attempt to submit UpdateMsg while RefreshMsg was not submitted on this stream yet. Handle = ");
			temp.append(handle).append(".");
			handleIhe(handle, temp);
			return;
		}

		submitMsgOpts.pRsslMsg->msgBase.streamId = itemInfo->getStreamId();

		if (updateMsgEncoder.hasServiceName())
		{
			if ( encodeServiceIdFromName(updateMsgEncoder.getServiceName(), submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
				submitMsgOpts.pRsslMsg->msgBase) )
			{
				submitMsgOpts.pRsslMsg->updateMsg.flags |= RSSL_UPMF_HAS_MSG_KEY;
			}
			else
			{
				return;
			}
		}
		else if (updateMsgEncoder.hasServiceId())
		{
			if (validateServiceId(submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId, submitMsgOpts.pRsslMsg->msgBase) == false)
			{
				return;
			}
		}
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);
	if (rsslReactorSubmitMsg(_pRsslReactor, itemInfo->getClientSession()->getChannel(), &submitMsgOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		_userLock.unlock();
		EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmIProviderImpl::submit( const UpdateMsg& ).");
		temp.append(CR).append(itemInfo->getClientSession()->toString()).append(CR)
			.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
			.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
			.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
			.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
			.append("Error Text ").append(rsslErrorInfo.rsslError.text);

		handleIue(temp);

		return;
	}

	_userLock.unlock();
}

void OmmIProviderImpl::submit(const StatusMsg& stausMsg, UInt64 handle)
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);
	const StatusMsgEncoder& statusMsgEncoder = static_cast<const StatusMsgEncoder&>(stausMsg.getEncoder());
	submitMsgOpts.pRsslMsg = (RsslMsg*)statusMsgEncoder.getRsslStatusMsg();

	_userLock.lock();

	ItemInfoPtr itemInfo = getItemInfo(handle);

	if ( (itemInfo == 0 ) && handle != 0 )
	{
		_userLock.unlock();
		EmaString temp("Attempt to submit StatusMsg with non existent Handle = ");
		temp.append(handle).append(".");
		handleIue(temp);
		return;
	}

	RsslReactorChannel* pReactorChannel;

	if (submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_LOGIN)
	{
		if (handle == 0)
		{
			EmaString text("Fanout login message for item handle = ");

			RsslErrorInfo rsslErrorInfo;
			if (submit(submitMsgOpts, _pLoginHandler->getLoginItemList(), text, false, rsslErrorInfo) == false)
			{
				_userLock.unlock();
				EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmIProviderImpl::submit( const StatusMsg& ).");
				temp.append(CR).append(itemInfo->getClientSession()->toString()).append(CR)
					.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
					.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo.rsslError.text);

				handleIue(temp);

				return;
			}

			_userLock.unlock();
			return;
		}
		else
		{
			pReactorChannel = itemInfo->getClientSession()->getChannel();
			submitMsgOpts.pRsslMsg->msgBase.streamId = itemInfo->getStreamId();
		}
	}
	else if (submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DIRECTORY)
	{
		if (handle == 0)
		{
			EmaString text("Fanout source directory message for item handle = ");

			RsslErrorInfo rsslErrorInfo;
			if (submit(submitMsgOpts, _pDirectoryHandler->getDirectoryItemList(), text, false, rsslErrorInfo) == false)
			{
				_userLock.unlock();
				EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmIProviderImpl::submit( const StatusMsg& ).");
				temp.append(CR).append(itemInfo->getClientSession()->toString()).append(CR)
					.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
					.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo.rsslError.text);

				handleIue(temp);

				return;
			}

			_userLock.unlock();
			return;
		}
		else
		{
			pReactorChannel = itemInfo->getClientSession()->getChannel();
			submitMsgOpts.pRsslMsg->msgBase.streamId = itemInfo->getStreamId();
		}
	}
	else if (submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DICTIONARY)
	{
		if (statusMsgEncoder.hasServiceName())
		{
			if ( encodeServiceIdFromName(statusMsgEncoder.getServiceName(), submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
				submitMsgOpts.pRsslMsg->msgBase) )
			{
				submitMsgOpts.pRsslMsg->statusMsg.flags |= RSSL_STMF_HAS_MSG_KEY;
			}
			else
			{
				return;
			}
		}
		else if (statusMsgEncoder.hasServiceId())
		{
			if (validateServiceId(submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId, submitMsgOpts.pRsslMsg->msgBase) == false)
			{
				return;
			}
		}

		if (handle == 0)
		{
			EmaString text("Fanout dictionary message for item handle = ");

			RsslErrorInfo rsslErrorInfo;
			if (submit(submitMsgOpts, _pDictionaryHandler->getDicitonaryItemList(), text, false, rsslErrorInfo) == false)
			{
				_userLock.unlock();
				EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmIProviderImpl::submit( const RefreshMsg& ).");
				temp.append(CR).append(itemInfo->getClientSession()->toString()).append(CR)
					.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
					.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo.rsslError.text);

				handleIue(temp);

				return;
			}

			_userLock.unlock();
			return;
		}
		else
		{
			pReactorChannel = itemInfo->getClientSession()->getChannel();
			submitMsgOpts.pRsslMsg->msgBase.streamId = itemInfo->getStreamId();
		}
	}
	else
	{
		if (handle == 0)
		{
			_userLock.unlock();
			EmaString temp("Attempt to fanout StatusMsg with domain type ");
			temp.append(rdmDomainToString(submitMsgOpts.pRsslMsg->msgBase.domainType))
				.append(" while this is not supported.");
			handleIue(temp);
			return;
		}

		pReactorChannel = itemInfo->getClientSession()->getChannel();

		if (OmmLoggerClient::VerboseEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Received StatusMsg with domain type ");
			temp.append(rdmDomainToString(submitMsgOpts.pRsslMsg->msgBase.domainType))
				.append("; handle = ").append(handle).append(", user assigned streamId = ")
				.append(submitMsgOpts.pRsslMsg->msgBase.streamId).append(".");

			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		pReactorChannel = itemInfo->getClientSession()->getChannel();
		submitMsgOpts.pRsslMsg->msgBase.streamId = itemInfo->getStreamId();

		if (statusMsgEncoder.hasServiceName())
		{
			if ( encodeServiceIdFromName(statusMsgEncoder.getServiceName(), submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId,
				submitMsgOpts.pRsslMsg->msgBase) )
			{
				submitMsgOpts.pRsslMsg->statusMsg.flags |= RSSL_STMF_HAS_MSG_KEY;
			}
			else
			{
				return;
			}
		}
		else if (statusMsgEncoder.hasServiceId())
		{
			if (validateServiceId(submitMsgOpts.pRsslMsg->msgBase.msgKey.serviceId, submitMsgOpts.pRsslMsg->msgBase) == false)
			{
				return;
			}
		}

		if ((submitMsgOpts.pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_GROUP_ID) == RSSL_STMF_HAS_GROUP_ID)
		{
			handleItemGroup(itemInfo, submitMsgOpts.pRsslMsg->statusMsg.groupId, submitMsgOpts.pRsslMsg->statusMsg.state);
		}
	}

	if (itemInfo->isPrivateStream())
		submitMsgOpts.pRsslMsg->statusMsg.flags |= RSSL_STMF_PRIVATE_STREAM;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);
	if (rsslReactorSubmitMsg(_pRsslReactor, pReactorChannel, &submitMsgOpts, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		_userLock.unlock();
		EmaString temp("Internal error: rsslReactorSubmitMsg() failed in OmmIProviderImpl::submit( const StatusMsg& ).");
		temp.append(CR).append(itemInfo->getClientSession()->toString()).append(CR)
			.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
			.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
			.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
			.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
			.append("Error Text ").append(rsslErrorInfo.rsslError.text);

		handleIue(temp);

		return;
	}

	handleItemInfo(submitMsgOpts.pRsslMsg->msgBase.domainType, handle, submitMsgOpts.pRsslMsg->statusMsg.state);

	_userLock.unlock();
}

bool OmmIProviderImpl::submit(RsslReactorSubmitMsgOptions submitMsgOptions, const EmaVector< ItemInfo* >& itemList, EmaString& text, bool applyDirectoryFilter, RsslErrorInfo& rsslErrorInfo)
{
	RsslMsg* pRsslMsg = submitMsgOptions.pRsslMsg;

	for (UInt32 idx = 0; idx < itemList.size(); idx++)
	{
		ItemInfo* itemInfo = itemList[idx];

		if (OmmLoggerClient::VerboseEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp(text);
			temp.append(itemInfo->getHandle()).append(", client handle = ")
				.append(itemInfo->getClientSession()->getClientHandle()).append(".");
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		clearRsslErrorInfo(&rsslErrorInfo);

		if (applyDirectoryFilter)
		{
			RsslRDMMsg rsslRDMMsg;
			rsslClearRDMMsg(&rsslRDMMsg);
			submitMsgOptions.pRsslMsg = 0;

			switch (_rsslDirectoryMsg.rdmMsgBase.rdmMsgType)
			{
				case RDM_DR_MT_REFRESH:
				{
					RsslRDMDirectoryMsg rsslRDMDirectoryMsg;
					rsslClearRDMDirectoryMsg(&rsslRDMDirectoryMsg);

					rsslRDMDirectoryMsg.refresh = _rsslDirectoryMsg.refresh;
					rsslRDMDirectoryMsg.refresh.filter = 0;

					if (DirectoryServiceStore::encodeDirectoryMsg(_rsslDirectoryMsg, rsslRDMDirectoryMsg, itemInfo->getFilter(), itemInfo->hasServiceId(), itemInfo->getServiceId()) == false)
					{
						return true;
					}

					rsslRDMMsg.directoryMsg.refresh = rsslRDMDirectoryMsg.refresh;
					rsslRDMMsg.rdmMsgBase.streamId = itemInfo->getStreamId();
					submitMsgOptions.pRDMMsg = &rsslRDMMsg;

					if (rsslReactorSubmitMsg(_pRsslReactor, itemInfo->getClientSession()->getChannel(), &submitMsgOptions, &rsslErrorInfo) != RSSL_RET_SUCCESS)
					{
						return false;
					}

					if (rsslRDMDirectoryMsg.refresh.serviceList)
					{
						delete[] rsslRDMDirectoryMsg.refresh.serviceList;
					}

				}
					break;
				case RDM_DR_MT_UPDATE:
				{
					if (!_storeUserSubmitted &&_ommIProviderActiveConfig.refreshFirstRequired)
					{
						if (!itemInfo->isSentRefresh())
						{
							if (OmmLoggerClient::WarningEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
							{
								EmaString temp("Skip sending source directory update message for handle ");
								temp.append(itemInfo->getHandle()).append(", client handle ")
									.append(itemInfo->getClientSession()->getClientHandle()).append(" as refresh message is required first.");
								_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::WarningEnum, temp);
							}

							break;
						}
					}

					RsslRDMDirectoryMsg rsslRDMDirectoryMsg;
					rsslClearRDMDirectoryMsg(&rsslRDMDirectoryMsg);

					rsslRDMDirectoryMsg.update = _rsslDirectoryMsg.update;
					rsslRDMDirectoryMsg.update.flags &= ~RDM_DR_UPF_HAS_FILTER;
					rsslRDMDirectoryMsg.update.filter = 0;

					if (DirectoryServiceStore::encodeDirectoryMsg(_rsslDirectoryMsg, rsslRDMDirectoryMsg, itemInfo->getFilter(), itemInfo->hasServiceId(), itemInfo->getServiceId()) == false)
					{
						return true;
					}

					rsslRDMMsg.directoryMsg.update = rsslRDMDirectoryMsg.update;
					rsslRDMMsg.rdmMsgBase.streamId = itemInfo->getStreamId();
					submitMsgOptions.pRDMMsg = &rsslRDMMsg;

					if (rsslReactorSubmitMsg(_pRsslReactor, itemInfo->getClientSession()->getChannel(), &submitMsgOptions, &rsslErrorInfo) != RSSL_RET_SUCCESS)
					{
						return false;
					}

					if (rsslRDMDirectoryMsg.update.serviceList)
					{
						delete[] rsslRDMDirectoryMsg.update.serviceList;
					}
				}
					break;
			}
		}
		else
		{
			if (!_storeUserSubmitted && _ommIProviderActiveConfig.refreshFirstRequired && submitMsgOptions.pRsslMsg->msgBase.msgClass == RSSL_MC_UPDATE)
			{
				if (!itemInfo->isSentRefresh())
				{
					if (OmmLoggerClient::WarningEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
					{
						EmaString temp("Skip sending update message for handle ");
						temp.append(itemInfo->getHandle()).append(", client handle ")
							.append(itemInfo->getClientSession()->getClientHandle()).append(" as refresh message is required first.");
						_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::WarningEnum, temp);
					}

					continue;
				}
			}

			submitMsgOptions.pRsslMsg->msgBase.streamId = itemInfo->getStreamId();

			if (rsslReactorSubmitMsg(_pRsslReactor, itemInfo->getClientSession()->getChannel(), &submitMsgOptions, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				return false;
			}
		}

		switch (pRsslMsg->msgBase.msgClass)
		{
		case RSSL_MC_REFRESH:
			itemInfo->isSentRefresh();
			handleItemInfo(pRsslMsg->msgBase.domainType, itemInfo->getHandle(), pRsslMsg->refreshMsg.state);
			break;
		case RSSL_MC_STATUS:
			handleItemInfo(pRsslMsg->msgBase.domainType, itemInfo->getHandle(), pRsslMsg->statusMsg.state);
			break;
		}
	}

	return true;
}

void OmmIProviderImpl::handleItemInfo(int domainType, UInt64 handle, RsslState& state)
{
	if ( state.streamState != RsslStreamStates::RSSL_STREAM_OPEN )
	{
		ItemInfoPtr itemInfo = getItemInfo(handle);

		if (itemInfo != 0)
		{
			switch (domainType)
			{
			case ema::rdm::MMT_LOGIN:
			{
				_pServerChannelHandler->closeChannel(itemInfo->getClientSession()->getChannel());
			}
			break;
			case ema::rdm::MMT_DIRECTORY:
			{
				_pDirectoryHandler->removeItemInfo(itemInfo);
				removeItemInfo(itemInfo, false);
			}
			break;
			case ema::rdm::MMT_DICTIONARY:
			{
				_pDictionaryHandler->removeItemInfo(itemInfo);
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

void OmmIProviderImpl::handleItemGroup(ItemInfo* itemInfo, RsslBuffer& groupId, RsslState& state)
{
	if ( ( groupId.length < 2 ) || (groupId.data[0] == '\0' && groupId.data[1] == '\0') || !itemInfo->hasServiceId() )
	{
		return;
	}

	if ( itemInfo->hasItemGroup() )
	{
		EmaBufferInt buffer;
		buffer.setFromInt(groupId.data, groupId.length);

		if ( ( buffer.toBuffer() == itemInfo->getItemGroup() ) == false)
		{
			updateItemGroup(itemInfo, buffer.toBuffer());
			itemInfo->setItemGroup(groupId);
		}
	}
	else
	{
		itemInfo->setItemGroup(groupId);
		addItemGroup(itemInfo, itemInfo->getItemGroup());
	}
}

Int64 OmmIProviderImpl::dispatch(Int64 timeOut)
{
	if (_ommIProviderActiveConfig.operationModel == OmmIProviderConfig::UserDispatchEnum && !_atExit)
		return rsslReactorDispatchLoop(timeOut, _ommIProviderActiveConfig.maxDispatchCountUserThread, _bMsgDispatched);

	return OmmProvider::TimeoutEnum;
}

void OmmIProviderImpl::unregister(UInt64)
{
	handleIue("Calling the OmmIProviderImpl::unregister(UInt64) method is not support in this release.");
}

void OmmIProviderImpl::submit(const AckMsg& ackMsg, UInt64 handle)
{
	handleIue("Calling the OmmIProviderImpl::submit(const AckMsg& ackMsg, UInt64 handle) method is not support in this release.");
}

DirectoryServiceStore& OmmIProviderImpl::getDirectoryServiceStore()
{
	return _ommIProviderDirectoryStore;
}

bool OmmIProviderImpl::encodeServiceIdFromName(const EmaString& serviceName, RsslUInt16& serviceId, RsslMsgBase& rsslMsgBase)
{
	UInt64* pServiceId = _ommIProviderDirectoryStore.getServiceIdByName(&serviceName);

	if (!pServiceId)
	{
		_userLock.unlock();
		EmaString temp("Attempt to submit ");
		temp.append(DataType(msgDataType[rsslMsgBase.msgClass]).toString()).
			append(" with service name of ").append(serviceName).
			append(" that was not included in the SourceDirectory. Dropping this ").
			append(DataType(msgDataType[rsslMsgBase.msgClass]).toString()).append(".");
		handleIue(temp);
		return false;
	}
	else if (*pServiceId > 0xFFFF)
	{
		_userLock.unlock();
		EmaString temp("Attempt to submit ");
		temp.append(DataType(msgDataType[rsslMsgBase.msgClass]).toString()).
			append(" with service name of ").append(serviceName).
			append(" whose matching service id of ").append(*pServiceId).append(" is out of range. Dropping this ").
			append(DataType(msgDataType[rsslMsgBase.msgClass]).toString()).append(".");
		handleIue(temp);
		return false;
	}

	serviceId = (RsslUInt16)*pServiceId;
	rsslMsgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

	return true;
}

bool OmmIProviderImpl::validateServiceId(RsslUInt16 serviceId, RsslMsgBase& rsslMsgBase)
{
	EmaStringPtr* pServiceName = _ommIProviderDirectoryStore.getServiceNameById(serviceId);

	if (!pServiceName)
	{
		_userLock.unlock();
		EmaString temp("Attempt to submit ");
		temp.append(DataType(msgDataType[rsslMsgBase.msgClass]).toString()).
			append(" with service Id of ").append(serviceId).
			append(" that was not included in the SourceDirectory. Dropping this ").
			append(DataType(msgDataType[rsslMsgBase.msgClass]).toString()).append(".");
		handleIue(temp);
		return false;
	}
	else if (serviceId > 0xFFFF)
	{
		_userLock.unlock();
		EmaString temp("Attempt to submit ");
		temp.append(DataType(msgDataType[rsslMsgBase.msgClass]).toString()).
			append(" with service Id of ").append(serviceId).append(" is out of range. Dropping this ").
			append(DataType(msgDataType[rsslMsgBase.msgClass]).toString()).append(".");
		handleIue(temp);
		return false;
	}

	return true;
}

void OmmIProviderImpl::onServiceDelete(ClientSession* clientSession, RsslUInt serviceId)
{
	if (clientSession)
	{
		removeServiceId(clientSession, serviceId);
	}
	else
	{
		if (_pDirectoryHandler)
		{
			const EmaVector<ItemInfo*>& itemInfoList = _pDirectoryHandler->getDirectoryItemList();

			for (UInt32 itemInfoIndex = 0; itemInfoIndex < itemInfoList.size(); itemInfoIndex++)
			{
				removeServiceId(itemInfoList[itemInfoIndex]->getClientSession(), serviceId);
			}
		}
	}
}

void OmmIProviderImpl::onServiceStateChange(ClientSession* clientSession, RsslUInt serviceId, const RsslRDMServiceState& serviceState)
{
	if (serviceState.flags & RDM_SVC_STF_HAS_STATUS)
	{
		if (serviceState.status.streamState == RsslStreamStates::RSSL_STREAM_CLOSED_RECOVER)
		{
			if (clientSession)
			{
				removeServiceId(clientSession, serviceId);
			}
			else
			{
				if (_pDirectoryHandler)
				{
					const EmaVector<ItemInfo*>& itemInfoList = _pDirectoryHandler->getDirectoryItemList();

					for (UInt32 itemInfoIndex = 0; itemInfoIndex < itemInfoList.size(); itemInfoIndex++)
					{
						removeServiceId(itemInfoList[itemInfoIndex]->getClientSession(), serviceId);
					}
				}
			}
		}
	}
}

void OmmIProviderImpl::onServiceGroupChange(ClientSession* clientSession, RsslUInt serviceId, RsslRDMServiceGroupState*& pServiceGroupState, RsslUInt32 groupStateCount)
{
	for (UInt32 idx = 0; idx < groupStateCount; ++idx)
	{
		RsslRDMServiceGroupState* pGroupState = pServiceGroupState;

		if (pGroupState->flags & RDM_SVC_GRF_HAS_MERGED_TO_GROUP)
		{
			if (clientSession)
			{
				mergeToGroupId(clientSession, serviceId, pGroupState->group, pGroupState->mergedToGroup);
			}
			else
			{
				if (_pDirectoryHandler)
				{
					const EmaVector<ItemInfo*>& itemInfoList = _pDirectoryHandler->getDirectoryItemList();

					for (UInt32 itemInfoIndex = 0; itemInfoIndex < itemInfoList.size(); itemInfoIndex++)
					{
						mergeToGroupId(itemInfoList[itemInfoIndex]->getClientSession(), serviceId, pGroupState->group, pGroupState->mergedToGroup);
					}
				}
			}
		}
		
		if (pGroupState->flags & RDM_SVC_GRF_HAS_STATUS)
		{
			if (pGroupState->status.streamState == RsslStreamStates::RSSL_STREAM_CLOSED_RECOVER)
			{
				if (clientSession)
				{
					removeGroupId(clientSession, serviceId, pGroupState->group);
				}
				else
				{
					if (_pDirectoryHandler)
					{
						const EmaVector<ItemInfo*>& itemInfoList = _pDirectoryHandler->getDirectoryItemList();

						for (UInt32 itemInfoIndex = 0; itemInfoIndex < itemInfoList.size(); itemInfoIndex++)
						{
							removeGroupId(itemInfoList[itemInfoIndex]->getClientSession(), serviceId, pGroupState->group);
						}
					}
				}
			}
		}

		++pGroupState;
	}
}

