/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "TestReactorComponent.h"
#include "Consumer.h"
#include "Provider.h"
#include "TestUtil.h"
#include "gtest/gtest.h"

using namespace testing;
using namespace std;

RsslInt TestReactorComponent::nextServerPort()
{
	return _portToBind;
}

RsslInt TestReactorComponent::serverPort()
{
	EXPECT_TRUE(_pServer != NULL);

	return _pServer->portNumber;
}
	
TestReactor* TestReactorComponent::testReactor()
{
	return _pTestReactor;
}
	
void TestReactorComponent::testReactor(TestReactor* pTestReactor)
{
	_pTestReactor = pTestReactor;
}
	
RsslReactorChannel* TestReactorComponent::channel()
{
	return _pReactorChannel;
}
		
TestReactorComponent::TestReactorComponent(TestReactor* pTestReactor)
{
	_pReactorChannel = NULL;
	_reactorChannelIsUp = false;
	_pServer = NULL;
	_portToBind = 16123;
	_pTestReactor = pTestReactor;
	_pTestReactor->addComponent(this);
}

RsslReactorChannelRole* TestReactorComponent::reactorRole()
{
	return _pReactorRole;
}
	
RsslReactorChannel* TestReactorComponent::reactorChannel()
{
	return _pReactorChannel;
}
	
void TestReactorComponent::reactorChannel(RsslReactorChannel* pReactorChannel)
{
	_pReactorChannel = pReactorChannel;
}
	
bool TestReactorComponent::reactorChannelIsUp()
{
	return _reactorChannelIsUp;
}
	
void TestReactorComponent::reactorChannelIsUp(bool reactorChannelIsUp)
{
	_reactorChannelIsUp = reactorChannelIsUp;
}
	
void TestReactorComponent::defaultSessionLoginStreamId(RsslInt defaultSessionLoginStreamId)
{
	_defaultSessionLoginStreamIdIsSet = true;
	_defaultSessionLoginStreamId = defaultSessionLoginStreamId;
	_defaultSessionLoginStreamIdIsSet = true;
}
	
RsslInt TestReactorComponent::defaultSessionLoginStreamId()
{
	EXPECT_TRUE(_defaultSessionLoginStreamIdIsSet);
	return _defaultSessionLoginStreamId;
}
	
void TestReactorComponent::defaultSessionDirectoryStreamId(RsslInt defaultSessionDirectoryStreamId)
{
	_defaultSessionDirectoryStreamIdIsSet = true;
	_defaultSessionDirectoryStreamId = defaultSessionDirectoryStreamId;
	_defaultSessionDirectoryStreamIdIsSet = true;
}

RsslInt TestReactorComponent::defaultSessionDirectoryStreamId()
{
	EXPECT_TRUE(_defaultSessionDirectoryStreamIdIsSet);
	return _defaultSessionDirectoryStreamId;
}
	
RsslServer* TestReactorComponent::server()
{
	return _pServer;
}
	
void TestReactorComponent::bind(ConsumerProviderSessionOptions* pOpts)
{
	if (pOpts->connectionType() != RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		RsslBindOptions bindOpts;
		rsslClearBindOpts(&bindOpts);
		bindOpts.majorVersion = RSSL_RWF_MAJOR_VERSION;
		bindOpts.minorVersion = RSSL_RWF_MINOR_VERSION;
		snprintf(_portToBindString, sizeof (_portToBindString), "%d", (int)_portToBind++);
		bindOpts.serviceName = _portToBindString;
		bindOpts.pingTimeout = pOpts->pingTimeout();
		bindOpts.minPingTimeout = pOpts->pingTimeout();
		_pServer = rsslBind(&bindOpts, &_errorInfo.rsslError);
		ASSERT_TRUE(_pServer != NULL) << "bind failed: " << _errorInfo.rsslError.rsslErrorId << " (" << _errorInfo.rsslError.text << ')' << endl;
	}
		
	_pTestReactor->registerComponentServer(this);
}
	
	
RsslInt TestReactorComponent::submit(RsslReactorSubmitMsgOptions* pSubmitOptions)
{
	RsslInt ret;

	ret = rsslReactorSubmitMsg(_pTestReactor->reactor(), _pReactorChannel, pSubmitOptions, &_errorInfo);
		
	EXPECT_TRUE(ret >= RSSL_RET_SUCCESS) << "submit failed: " << ret << " (" << _errorInfo.errorLocation << " -- " << _errorInfo.rsslError.text << ")";

	return ret;
}
	
RsslInt TestReactorComponent::submitAndDispatch(RsslReactorSubmitMsgOptions* pSubmitOptions)
{
	RsslInt ret = submit(pSubmitOptions);
	_pTestReactor->dispatch(0);
	return ret;
}
	
void TestReactorComponent::closeSession(Consumer* pConsumer, Provider* pProvider)
{
	/* Make sure there's nothing left in the dispatch queue. */
	pConsumer->testReactor()->dispatch(0);
	pProvider->testReactor()->dispatch(0);
		
	pConsumer->close();
	pProvider->close();
}

void TestReactorComponent::closeChannel()
{
	ASSERT_EQ(RSSL_RET_SUCCESS, rsslReactorCloseChannel(_pTestReactor->reactor(), _pReactorChannel, &_errorInfo));
	_reactorChannelIsUp = false;
	_pReactorChannel = NULL;
}
	
void TestReactorComponent::close()
{
	ASSERT_TRUE(_pTestReactor != NULL);
	if (_pServer != NULL)
	{
		ASSERT_EQ(RSSL_RET_SUCCESS, rsslCloseServer(_pServer, &_errorInfo.rsslError));
		_pServer = NULL;
	}

	if (_pReactorChannel != NULL)
		closeChannel();

	_pTestReactor -> removeComponent(this);
	_pTestReactor = NULL;
}
