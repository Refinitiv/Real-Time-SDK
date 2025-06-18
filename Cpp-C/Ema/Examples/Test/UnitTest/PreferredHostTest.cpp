/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2024, 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "rtr/rsslReactor.h"
#include "rtr/rsslTransport.h"
#include <thread>
#include <atomic>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

static RsslUInt64 itemHandle = 0;
static EmaString consChannelInfo;
static EmaString iprovChannelInfo;
static EmaString niprovChannelInfo;

static void sleep(int millisecs)
{
#if defined WIN32
	::Sleep((DWORD)(millisecs));
#else
	struct timespec sleeptime;
	sleeptime.tv_sec = millisecs / 1000;
	sleeptime.tv_nsec = (millisecs % 1000) * 1000000;
	nanosleep(&sleeptime, 0);
#endif
}

class PreferredHostTest : public ::testing::Test {
public:
	static DataDictionary dataDictionary;
	static const char* emaConfigTest;

	void SetUp()
	{
		try
		{
			dataDictionary.loadFieldDictionary("RDMFieldDictionaryTest");
			dataDictionary.loadEnumTypeDictionary("enumtypeTest.def");
		}
		catch (const OmmException& excp)
		{
			std::cout << "Caught unexpected exception!!!" << std::endl << excp << std::endl;
			EXPECT_TRUE(false) << "Unexpected exception in EmaMsgPackingTest load dictionary";
		}
	}

	void TearDown()
	{
		dataDictionary.clear();
	}
};

DataDictionary PreferredHostTest::dataDictionary;
const char* PreferredHostTest::emaConfigTest = "./EmaConfigTest.xml";

class OmmProviderTestClientPH : public refinitiv::ema::access::OmmProviderClient
{
public:

	void processLoginRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
	{
		event.getProvider().submit(RefreshMsg().domainType(MMT_LOGIN).name(reqMsg.getName()).nameType(USER_NAME).complete().
			attrib(ElementList().complete()).solicited(true).state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Login accepted"),
			event.getHandle());
	}

	void processMarketPriceRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
	{
		if (itemHandle != 0)
		{
			processInvalidItemRequest(reqMsg, event);
			return;
		}

		event.getProvider().submit(RefreshMsg().name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).solicited(true).
			state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").
			payload(FieldList().
				addReal(22, 3990, OmmReal::ExponentNeg2Enum).
				addReal(25, 3994, OmmReal::ExponentNeg2Enum).
				addReal(30, 9, OmmReal::Exponent0Enum).
				addReal(31, 19, OmmReal::Exponent0Enum).
				complete()).
			complete(), event.getHandle());

		itemHandle = event.getHandle();
	}

	void processInvalidItemRequest(const ReqMsg& reqMsg, const OmmProviderEvent& event)
	{
		event.getProvider().submit(StatusMsg().name(reqMsg.getName()).serviceName(reqMsg.getServiceName()).
			domainType(reqMsg.getDomainType()).
			state(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotFoundEnum, "Item not found"),
			event.getHandle());
	}

	void onReqMsg(const ReqMsg& reqMsg, const OmmProviderEvent& event)
	{
		switch (reqMsg.getDomainType())
		{
		case MMT_LOGIN:
			processLoginRequest(reqMsg, event);
			break;
		case MMT_MARKET_PRICE:
			processMarketPriceRequest(reqMsg, event);
			break;
		default:
			processInvalidItemRequest(reqMsg, event);
			break;
		}

		iprovChannelInfo = event.getChannelInformation();
	}
};

class OmmConsumerTestClientPH : public refinitiv::ema::access::OmmConsumerClient
{
public:
	OmmConsumerTestClientPH() :updateCalled(false) {};

	void onRefreshMsg(const RefreshMsg& refreshMsg, const OmmConsumerEvent& event)
	{
		consChannelInfo = event.getChannelInformation();
	}

	void onUpdateMsg(const UpdateMsg& updateMsg, const OmmConsumerEvent& event)
	{
		if (!updateCalled)
		{
			updateCalled = true;
			consChannelInfo = event.getChannelInformation();
		}
	}

	void onStatusMsg(const StatusMsg& statusMsg, const OmmConsumerEvent& event)
	{
		consChannelInfo = event.getChannelInformation();
	}

	bool updateCalled;
};


TEST_F(PreferredHostTest, PreferredHostTest_SwitchToPreferred_ChannelInfo)
{
	try
	{
		OmmProviderTestClientPH providerCallback;
		OmmProvider provider(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14003"), providerCallback);

		OmmConsumerTestClientPH consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig(emaConfigTest).dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).consumerName("Consumer_10").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;

		while (itemHandle == 0)
		{
			if (count == 30)
			{
				FAIL() << "UNABLE TO CONNECT";
			}
			consumer.dispatch(1000);
			sleep(1000);
			count++;
		}

		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("14003"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph is channel preferred: non-preferred"));

		OmmProviderTestClientPH providerCallback1;
		OmmProvider provider1(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14004"), providerCallback1);

		/*Wait consumer fall back to the preferred host*/
		count = 0;
		while (count != 15)
		{
			consumer.dispatch(1000);
			sleep(1000);
			count++;
		}

		//Check that consumer switched on the preferred host
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("14004"));

		//check consumer PH info
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph detection time schedule: */10 * * * * *"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph detection time interval: 10"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph channel name: Channel_13"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph wsb channel name:"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph fall back with in WSB group: disabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph is channel preferred: preferred"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph remaining detection time:"));

		//check provider PH info. Values would be default cause PH is not applicable to provider.
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph preferred host option: disabled"));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph detection time schedule: "));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph detection time interval: 0"));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph channel name: "));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph wsb channel name: "));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph fall back with in WSB group: disabled"));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph is channel preferred: non-preferred"));
		EXPECT_TRUE(EmaString::npos != iprovChannelInfo.find("ph remaining detection time: 0"));


	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "PreferredHostTest_FileConfig -- exception NOT expected : " << excp;
	}
}

TEST_F(PreferredHostTest, PreferredHostTest_ModifiedPHWithIOCTL)
{
	try
	{
		OmmProviderTestClientPH providerCallback;
		OmmProvider provider(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14004"), providerCallback);

		OmmProviderTestClientPH providerCallback1;
		OmmProvider provider1(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14003"), providerCallback1);

		OmmConsumerTestClientPH consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig(emaConfigTest).dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).consumerName("Consumer_10").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;
		itemHandle = 0;

		while (itemHandle == 0)
		{
			if (count == 30)
			{
				FAIL() << "UNABLE TO CONNECT";
			}
			consumer.dispatch(1000);
			sleep(1000);
			count++;
		}

		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("14004"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph is channel preferred: preferred"));

		PreferredHostOptions phOptions;

		phOptions.enablePreferredHostOptions(true);
		phOptions.preferredChannelName("Channel_10");
		phOptions.phDetectionTimeInterval(5);
		phOptions.phDetectionTimeSchedule("*/5 * * * * *");
		consumer.modifyReactorChannelIOCtl(IOCtlReactorChannelCode::ReactorChannelPreferredHost, (void*)&phOptions);

		/*Wait 10 sec while consumer fall back to new preferred host*/
		count = 0;
		while (count != 10)
		{
			consumer.dispatch(1000);
			sleep(1000);
			count++;
		}

		//Check that consumer switched on the preferred host
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("14003"));

		//check PH info
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph detection time schedule: */5 * * * * *"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph detection time interval: 5"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph channel name: Channel_10"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph fall back with in WSB group: disabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph is channel preferred: preferred"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph remaining detection time:"));

	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "PreferredHostTest_FileConfig -- exception NOT expected : " << excp;
	}
}

TEST_F(PreferredHostTest, PreferredHostTest_SetPHWithIOCtlAndPerformFallback)
{
	try
	{
		OmmProviderTestClientPH providerCallback;
		OmmProvider provider(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14004"), providerCallback);

		OmmProviderTestClientPH providerCallback1;
		OmmProvider provider1(OmmIProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").port("14003"), providerCallback1);

		OmmConsumerTestClientPH consumerCallback;
		OmmConsumer consumer(OmmConsumerConfig(emaConfigTest).dataDictionary(dataDictionary).operationModel(OmmConsumerConfig::UserDispatchEnum).consumerName("Consumer_10").username("user"));
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), consumerCallback);

		Int32 count = 0;
		itemHandle = 0;

		while (itemHandle == 0)
		{
			if (count == 30)
			{
				FAIL() << "UNABLE TO CONNECT";
			}
			consumer.dispatch(1000);
			sleep(1000);
			count++;
		}

		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("14004"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph is channel preferred: preferred"));

		PreferredHostOptions phOptions;

		phOptions.enablePreferredHostOptions(true);
		phOptions.preferredChannelName("Channel_10");
		phOptions.phDetectionTimeInterval(50);
		phOptions.phDetectionTimeSchedule("*/50 * * * * *");
		consumer.modifyReactorChannelIOCtl(IOCtlReactorChannelCode::ReactorChannelPreferredHost, (void*)&phOptions);

		consumer.dispatch(1000);
		sleep(1000);

		consumer.fallbackPreferredHost();

		/*Wait while consumer fall back to new preferred host.
		Fall back would happen before the time set in schedule because fallbackPreferredHost called  */

		count = 0;
		while (count != 20)
		{
			consumer.dispatch(1000);
			sleep(1000);
			count++;
		}

		//Check that consumer switched on the preferred host
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("14003"));

		//check PH info
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph preferred host option: enabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph detection time schedule: */50 * * * * *"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph detection time interval: 50"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph channel name: Channel_10"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph fall back with in WSB group: disabled"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph is channel preferred: preferred"));
		EXPECT_TRUE(EmaString::npos != consChannelInfo.find("ph remaining detection time:"));

	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "PreferredHostTest_FileConfig -- exception NOT expected : " << excp;
	}
}

class ADHSimulator
{
public:

	fd_set	readFds;
	fd_set	writeFds;
	fd_set	exceptFds;
	ADHSimulator(RsslCreateReactorOptions& reactorOpts, char* portNo)
	{
		runFlag = false;

		if (!(pReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo)))
		{
			EXPECT_TRUE(false) << "Reactor creation failed: %s\n", rsslErrorInfo.rsslError.text;
		}

		rsslClearBindOpts(&sopts);

		sopts = RSSL_INIT_BIND_OPTS;
		sopts.guaranteedOutputBuffers = 2000;
		sopts.serviceName = portNo;
		sopts.majorVersion = RSSL_RWF_MAJOR_VERSION;
		sopts.minorVersion = RSSL_RWF_MINOR_VERSION;
		sopts.protocolType = RSSL_RWF_PROTOCOL_TYPE;
		sopts.connectionType = RSSL_CONN_TYPE_SOCKET;

		if (!(rsslSrvr = rsslBind(&sopts, &rsslErrorInfo.rsslError)))
		{
			EXPECT_TRUE(false) << "rsslBind failed: %s\n", rsslErrorInfo.rsslError.text;
		}

		FD_ZERO(&readFds);
		FD_ZERO(&writeFds);
		FD_ZERO(&exceptFds);
		FD_SET(rsslSrvr->socketId, &readFds);
		FD_SET(rsslSrvr->socketId, &writeFds);
		FD_SET(rsslSrvr->socketId, &exceptFds);
		FD_SET(pReactor->eventFd, &readFds);
		FD_SET(pReactor->eventFd, &writeFds);
	}

	static RsslRet sendMessage(RsslReactor* pReactor, RsslReactorChannel* chnl, RsslBuffer* msgBuf)
	{
		RsslErrorInfo rsslErrorInfo;
		RsslRet	retval = 0;
		RsslUInt32 outBytes = 0;
		RsslUInt32 uncompOutBytes = 0;
		RsslUInt8 writeFlags = RSSL_WRITE_NO_FLAGS;
		RsslReactorSubmitOptions submitOpts;

		rsslClearReactorSubmitOptions(&submitOpts);

		/* send the request */
		if ((retval = rsslReactorSubmit(pReactor, chnl, msgBuf, &submitOpts, &rsslErrorInfo)) < RSSL_RET_SUCCESS)
		{
			while (retval == RSSL_RET_WRITE_CALL_AGAIN)
				retval = rsslReactorSubmit(pReactor, chnl, msgBuf, &submitOpts, &rsslErrorInfo);

			if (retval < RSSL_RET_SUCCESS)	/* Connection should be closed, return failure */
			{
				/* rsslWrite failed, release buffer */
				EXPECT_TRUE(false) << "rsslReactorSubmit() failed with return code: " << retval << " " << rsslErrorInfo.rsslError.text << "\n";
				if ((retval = rsslReactorReleaseBuffer(chnl, msgBuf, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
					EXPECT_TRUE(false) << "rsslReactorReleaseBuffer() failed with return code: " << retval << " " << rsslErrorInfo.rsslError.text << "\n";

				return RSSL_RET_FAILURE;
			}
		}

		return RSSL_RET_SUCCESS;
	}

	/* Callbacks*/
	static RsslReactorCallbackRet loginMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pChannel, RsslRDMLoginMsgEvent* pLoginMsgEvent)
	{
		/* Accept login */
		RsslRDMLoginMsg* pLoginMsg = pLoginMsgEvent->pRDMLoginMsg;
		RsslRDMLoginRequest* pLoginRequest = &pLoginMsg->request;

		if (pLoginMsg->rdmMsgBase.rdmMsgType == RDM_LG_MT_REQUEST)
		{
			RsslErrorInfo rsslErrorInfo;
			RsslBuffer* msgBuf = 0;
			RsslUInt32 ipAddress = 0;
			RsslRet ret;

			/* get a buffer for the login response */
			msgBuf = rsslReactorGetBuffer(pChannel, 4096, RSSL_FALSE, &rsslErrorInfo);

			if (msgBuf != NULL)
			{
				RsslRDMLoginRefresh loginRefresh;
				RsslEncodeIterator eIter;

				rsslClearRDMLoginRefresh(&loginRefresh);

				/* Set state information */
				loginRefresh.state.streamState = RSSL_STREAM_OPEN;
				loginRefresh.state.dataState = RSSL_DATA_OK;
				loginRefresh.state.code = RSSL_SC_NONE;

				/* Set stream ID */
				loginRefresh.rdmMsgBase.streamId = pLoginRequest->rdmMsgBase.streamId;

				/* Mark refresh as solicited since it is a response to a request. */
				loginRefresh.flags = RDM_LG_RFF_SOLICITED;

				/* Echo the userName, applicationId, applicationName, and position */
				loginRefresh.flags |= RDM_LG_RFF_HAS_USERNAME;
				loginRefresh.userName = pLoginRequest->userName;
				if (pLoginRequest->flags & RDM_LG_RQF_HAS_USERNAME_TYPE)
				{
					loginRefresh.flags |= RDM_LG_RFF_HAS_USERNAME_TYPE;
					loginRefresh.userNameType = pLoginRequest->userNameType;
				}

				loginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_ID;
				loginRefresh.applicationId = pLoginRequest->applicationId;

				loginRefresh.flags |= RDM_LG_RFF_HAS_APPLICATION_NAME;
				loginRefresh.applicationName = pLoginRequest->applicationName;

				loginRefresh.flags |= RDM_LG_RFF_HAS_POSITION;
				loginRefresh.position = pLoginRequest->position;

				/* Leave all other parameters as default values. */

				/* Encode the refresh. */
				rsslClearEncodeIterator(&eIter);
				rsslSetEncodeIteratorRWFVersion(&eIter, pChannel->majorVersion, pChannel->minorVersion);
				if ((ret = rsslSetEncodeIteratorBuffer(&eIter, msgBuf)) < RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(pChannel, msgBuf, &rsslErrorInfo);
					EXPECT_TRUE(false) << "rsslSetEncodeIteratorBuffer() failed with return code: " << ret << "\n";
					return RSSL_RC_CRET_FAILURE;
				}
				if (rsslEncodeRDMLoginMsg(&eIter, (RsslRDMLoginMsg*)&loginRefresh, &msgBuf->length, &rsslErrorInfo) != RSSL_RET_SUCCESS)
				{
					rsslReactorReleaseBuffer(pChannel, msgBuf, &rsslErrorInfo);
					EXPECT_TRUE(false) << "rsslEncodeRDMLoginRefresh() failed: " << rsslErrorInfo.rsslError.text << "(" << rsslErrorInfo.errorLocation << ")" << "\n";
					return RSSL_RC_CRET_FAILURE;
				}

				/* Send the refresh. */
				if (sendMessage(pReactor, pChannel, msgBuf) != RSSL_RET_SUCCESS)
					return RSSL_RC_CRET_FAILURE;
			}
			else
			{
				EXPECT_TRUE(false) << "rsslReactorGetBuffer(): Failed: " << rsslErrorInfo.rsslError.text << "\n";
				return RSSL_RC_CRET_FAILURE;
			}
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	static RsslReactorCallbackRet defaultMsgCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslMsgEvent* pRsslMsgEvent)
	{
		return RSSL_RC_CRET_SUCCESS;
	}
	static RsslReactorCallbackRet channelEventCallback(RsslReactor* pReactor, RsslReactorChannel* pReactorChannel, RsslReactorChannelEvent* pChannelEvent)
	{
		ADHSimulator* pAdhSim = static_cast<ADHSimulator*>(pReactorChannel->userSpecPtr);

		switch (pChannelEvent->channelEventType)
		{
		case RSSL_RC_CET_CHANNEL_UP:
			FD_SET(pReactorChannel->socketId, &pAdhSim->readFds);
			FD_SET(pReactorChannel->socketId, &pAdhSim->exceptFds);
			break;
		case RSSL_RC_CET_CHANNEL_DOWN:
			FD_CLR(pReactorChannel->socketId, &pAdhSim->readFds);
			FD_CLR(pReactorChannel->socketId, &pAdhSim->exceptFds);
			break;
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
			FD_CLR(pReactorChannel->socketId, &pAdhSim->readFds);
			FD_CLR(pReactorChannel->socketId, &pAdhSim->exceptFds);
			break;
		case RSSL_RC_CET_FD_CHANGE:
			FD_CLR(pReactorChannel->oldSocketId, &pAdhSim->readFds);
			FD_CLR(pReactorChannel->oldSocketId, &pAdhSim->exceptFds);
			FD_SET(pReactorChannel->socketId, &pAdhSim->readFds);
			FD_SET(pReactorChannel->socketId, &pAdhSim->exceptFds);
			break;
		}
		return RSSL_RC_CRET_SUCCESS;
	}

	void start()
	{
		runFlag = true;
		tr = std::thread(&ADHSimulator::run, this);
	}

	void run()
	{
		struct timeval time_interval;
		fd_set	useRead;
		fd_set	useWrite;
		fd_set	useExcept;

		int selRet;
		RsslReactorDispatchOptions dispatchOpts;
		RsslReactorOMMProviderRole providerRole;

		rsslClearOMMProviderRole(&providerRole);
		providerRole.base.channelEventCallback = channelEventCallback;
		providerRole.base.defaultMsgCallback = defaultMsgCallback;
		providerRole.loginMsgCallback = loginMsgCallback;

		rsslClearReactorDispatchOptions(&dispatchOpts);
		dispatchOpts.maxMessages = 100;

		FD_ZERO(&useRead);
		FD_ZERO(&useWrite);
		FD_ZERO(&useExcept);

		while (runFlag)
		{
			useRead = readFds;
			useWrite = writeFds;
			useExcept = exceptFds;
			time_interval.tv_sec = 1;
			time_interval.tv_usec = 0;

			selRet = select(FD_SETSIZE, &useRead,
				&useWrite, &useExcept, &time_interval);

			if (selRet > 0)
			{
				RsslRet ret;

				//Accept connection here
				if (rsslSrvr && pReactor && FD_ISSET(rsslSrvr->socketId, &useRead))
				{
					RsslReactorAcceptOptions aopts;
					rsslClearReactorAcceptOptions(&aopts);
					aopts.rsslAcceptOptions.userSpecPtr = this;

					if ((ret = rsslReactorAccept(pReactor, rsslSrvr, &aopts, (RsslReactorChannelRole*)&providerRole, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
					{
						EXPECT_TRUE(false) << "rsslReactorAccept failed: %s\n", rsslErrorInfo.rsslError.text;
					}
				}

				while ((ret = rsslReactorDispatch(pReactor, &dispatchOpts, &rsslErrorInfo)) > RSSL_RET_SUCCESS);

				if (ret < RSSL_RET_SUCCESS)
				{
					EXPECT_TRUE(false) << ("rsslReactorDispatch() failed: %s\n", rsslErrorInfo.rsslError.text);
				}
			}
		}

		FD_CLR(rsslSrvr->socketId, &readFds);
		FD_CLR(rsslSrvr->socketId, &exceptFds);
	}

	~ADHSimulator()
	{
		runFlag = false;
		tr.join();

		if (pReactor)
			rsslDestroyReactor(pReactor, &rsslErrorInfo);

		if (rsslSrvr)
			rsslCloseServer(rsslSrvr, &rsslErrorInfo.rsslError);
	}

private:
	ADHSimulator() {};
	RsslReactor* pReactor;
	RsslBindOptions sopts;
	RsslServer* rsslSrvr;
	std::thread tr;
	RsslErrorInfo rsslErrorInfo;
	std::atomic <bool> runFlag;
};

class OmmNIProviderTestClientPH : public refinitiv::ema::access::OmmProviderClient
{
	void onRefreshMsg(const RefreshMsg& refreshMsg, const OmmProviderEvent& event)
	{
		niprovChannelInfo = event.getChannelInformation();
	}

	void onStatusMsg(const StatusMsg& statusMsg, const OmmProviderEvent& event)
	{
		niprovChannelInfo = event.getChannelInformation();
	}

	void onClose(const ReqMsg& reqMsg, const OmmProviderEvent& event)
	{
		niprovChannelInfo = event.getChannelInformation();
	}

	void onReqMsg(const ReqMsg& reqMsg, const OmmProviderEvent& event)
	{
		niprovChannelInfo = event.getChannelInformation();
	}
};


TEST_F(PreferredHostTest, PreferredHostTest_ChannelInfo_NIProv)
{
	try
	{
		RsslCreateReactorOptions reactorOpts;
		rsslClearCreateReactorOptions(&reactorOpts);
		ADHSimulator adh(reactorOpts, (char*)"14003");
		adh.start();

		niprovChannelInfo.clear();
		OmmNIProviderTestClientPH niProviderCallback;
		OmmProvider niProvider(OmmNiProviderConfig(PreferredHostTest::emaConfigTest).providerName("Provider_1").host("localhost:14003").username("user"), niProviderCallback);

		//check niprovider PH info. Values would be default cause PH is not applicable to niprovider.
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph preferred host option: disabled"));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph detection time schedule: "));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph detection time interval: 0"));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph channel name: "));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph wsb channel name: "));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph fall back with in WSB group: disabled"));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph is channel preferred: non-preferred"));
		EXPECT_TRUE(EmaString::npos != niprovChannelInfo.find("ph remaining detection time: 0"));
	}
	catch (const OmmException& excp)
	{
		EXPECT_TRUE(false) << "PreferredHostTest_FileConfig -- exception NOT expected : " << excp;
	}
}