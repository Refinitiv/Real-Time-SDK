/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2020-2025 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#include "TestUtilities.h"
#include "ActiveConfig.h"
#include "OmmIProviderActiveConfig.h"

using namespace refinitiv::ema::access;
using namespace std;

class EmaActiveConfigTest : public ::testing::Test {
public:
	void ChannelConfigTestDefaultValues(SocketChannelConfig& channelConfig);
	void SocketChannelConfigTestDefaultValues(SocketChannelConfig& socketChannelConfig);
	void ServerConfigTestDefaultValues(SocketServerConfig& serverConfig);
	void SocketServerConfigTestDefaultValues(SocketServerConfig& socketServerConfig);
	void ReliableMcastChannelConfigTestDefaultValues(ReliableMcastChannelConfig& reliableMcastChannelConfig);
	void BaseConfigTestDefaultValues(BaseConfig& baseConfig);
	void ActiveConfigTestDefaultValues(ActiveConfig& activeConfig);
	void ActiveServerConfigTestDefaultValues(OmmIProviderActiveConfig& activeServerConfig);
	void OmmIProviderActiveConfigTestDefaultValues(OmmIProviderActiveConfig& ommIProvideractiveServerConfig);
};


void EmaActiveConfigTest::ChannelConfigTestDefaultValues(SocketChannelConfig & channelConfig)
{
	// Tests default values
	EXPECT_TRUE(channelConfig.name.empty());
	EXPECT_TRUE(channelConfig.interfaceName.empty());
	EXPECT_EQ(channelConfig.compressionType, DEFAULT_COMPRESSION_TYPE);
	EXPECT_EQ(channelConfig.compressionThreshold, DEFAULT_COMPRESSION_THRESHOLD);
	EXPECT_EQ(channelConfig.connectionType, RSSL_CONN_TYPE_SOCKET);
	EXPECT_EQ(channelConfig.getType(), RSSL_CONN_TYPE_SOCKET);

	EXPECT_EQ(channelConfig.connectionPingTimeout, DEFAULT_CONNECTION_PINGTIMEOUT);
	EXPECT_EQ(channelConfig.directWrite, DEFAULT_DIRECT_WRITE);
	EXPECT_EQ(channelConfig.initializationTimeout, DEFAULT_INITIALIZATION_TIMEOUT);
	EXPECT_EQ(channelConfig.guaranteedOutputBuffers, DEFAULT_GUARANTEED_OUTPUT_BUFFERS);
	EXPECT_EQ(channelConfig.numInputBuffers, DEFAULT_NUM_INPUT_BUFFERS);
	EXPECT_EQ(channelConfig.sysRecvBufSize, DEFAULT_SYS_RECEIVE_BUFFER_SIZE);
	EXPECT_EQ(channelConfig.sysSendBufSize, DEFAULT_SYS_SEND_BUFFER_SIZE);

	EXPECT_EQ(channelConfig.highWaterMark, DEFAULT_HIGH_WATER_MARK);
	EXPECT_TRUE(channelConfig.pChannel == NULL);
}

TEST_F(EmaActiveConfigTest, ChannelConfigTest)
{
	SocketChannelConfig channelConfig("defaultHostName", "defaultServiceName", RSSL_CONN_TYPE_SOCKET);

	// Tests default values
	ChannelConfigTestDefaultValues(channelConfig);

	const int midValue = 120;
	// setGuaranteedOutputBuffers
	channelConfig.setGuaranteedOutputBuffers( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), channelConfig.guaranteedOutputBuffers) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	channelConfig.setGuaranteedOutputBuffers( midValue );
	EXPECT_EQ(midValue, channelConfig.guaranteedOutputBuffers) << "Should be equal to " << midValue;
	channelConfig.setGuaranteedOutputBuffers( 0 );
	EXPECT_EQ(midValue, channelConfig.guaranteedOutputBuffers) << "Should be not 0, previous value: " << midValue;

	// setNumInputBuffers
	channelConfig.setNumInputBuffers( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), channelConfig.numInputBuffers) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	channelConfig.setNumInputBuffers( midValue );
	EXPECT_EQ(midValue, channelConfig.numInputBuffers) << "Should be equal to " << midValue;
	channelConfig.setNumInputBuffers( 0 );
	EXPECT_EQ(midValue, channelConfig.numInputBuffers) << "Should be not 0, previous value: " << midValue;

	// Make some changes...
	channelConfig.name = "nnn";
	channelConfig.interfaceName = "nnn interface";
	channelConfig.compressionType = RSSL_COMP_ZLIB;
	channelConfig.compressionThreshold = 2020;
	//channelConfig.connectionType = RSSL_CONN_TYPE_UNIDIR_SHMEM;

	channelConfig.connectionPingTimeout = 3002;
	channelConfig.directWrite = 1;
	channelConfig.initializationTimeout = 55;
	channelConfig.guaranteedOutputBuffers = 45;
	channelConfig.numInputBuffers = 33;
	channelConfig.sysRecvBufSize = 12;
	channelConfig.sysSendBufSize = 20;
	channelConfig.highWaterMark = 2;

	// Tests clear method
	channelConfig.clear();
	ChannelConfigTestDefaultValues(channelConfig);
}


void EmaActiveConfigTest::SocketChannelConfigTestDefaultValues(SocketChannelConfig& socketChannelConfig)
{
	// Tests default values
	EXPECT_STREQ(socketChannelConfig.hostName.c_str(), "defaultHostName");
	EXPECT_STREQ(socketChannelConfig.serviceName.c_str(), "defaultServiceName");
	EXPECT_EQ(socketChannelConfig.tcpNodelay, RSSL_TRUE);
	EXPECT_TRUE(socketChannelConfig.objectName.empty());
	EXPECT_TRUE(socketChannelConfig.sslCAStore.empty());

	EXPECT_TRUE(socketChannelConfig.proxyHostName.empty());
	EXPECT_TRUE(socketChannelConfig.proxyPort.empty());
	EXPECT_TRUE(socketChannelConfig.proxyUserName.empty());
	EXPECT_TRUE(socketChannelConfig.proxyPasswd.empty());
	EXPECT_TRUE(socketChannelConfig.proxyDomain.empty());
	EXPECT_EQ(socketChannelConfig.proxyConnectionTimeout, DEFAULT_PROXY_CONNECTION_TIMEOUT);

	EXPECT_EQ(socketChannelConfig.encryptedConnectionType, RSSL_CONN_TYPE_INIT);
	EXPECT_EQ(socketChannelConfig.securityProtocol, (RSSL_ENC_TLSV1_2 | RSSL_ENC_TLSV1_3));

	EXPECT_EQ(socketChannelConfig.enableSessionMgnt, RSSL_FALSE);
	EXPECT_TRUE(socketChannelConfig.location == DEFAULT_RDP_RT_LOCATION);
	EXPECT_EQ(socketChannelConfig.serviceDiscoveryRetryCount, DEFAULT_SERVICE_DISCOVERY_RETRY_COUNT);
	EXPECT_EQ(socketChannelConfig.wsMaxMsgSize, DEFAULT_WS_MAXMSGSIZE);
	EXPECT_TRUE(socketChannelConfig.wsProtocols == DEFAULT_WS_PROTOCLOS);

	EXPECT_EQ(socketChannelConfig.connectionType, RSSL_CONN_TYPE_ENCRYPTED);
	EXPECT_EQ(socketChannelConfig.getType(), RSSL_CONN_TYPE_SOCKET);
}

TEST_F(EmaActiveConfigTest, SocketChannelConfigTest)
{
	SocketChannelConfig socketChannelConfig("defaultHostName", "defaultServiceName", RSSL_CONN_TYPE_ENCRYPTED);

	// Tests default values
	SocketChannelConfigTestDefaultValues(socketChannelConfig);

	const int midValue = 120;
	// proxyConnectionTimeout
	socketChannelConfig.setProxyConnectionTimeout((RWF_MAX_32)+1ULL);
	EXPECT_EQ((RWF_MAX_32), socketChannelConfig.proxyConnectionTimeout) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	socketChannelConfig.setProxyConnectionTimeout(midValue);
	EXPECT_EQ(midValue, socketChannelConfig.proxyConnectionTimeout) << "Should be equal to " << midValue;
	socketChannelConfig.setProxyConnectionTimeout(0);
	EXPECT_EQ(0, socketChannelConfig.proxyConnectionTimeout) << "Should be equal to 0";

	// serviceDiscoveryRetryCount
	socketChannelConfig.setServiceDiscoveryRetryCount((RWF_MAX_32)+1ULL);
	EXPECT_EQ((RWF_MAX_32), socketChannelConfig.serviceDiscoveryRetryCount) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	socketChannelConfig.setServiceDiscoveryRetryCount(midValue);
	EXPECT_EQ(midValue, socketChannelConfig.serviceDiscoveryRetryCount) << "Should be equal to " << midValue;
	socketChannelConfig.setServiceDiscoveryRetryCount(0);
	EXPECT_EQ(0, socketChannelConfig.serviceDiscoveryRetryCount) << "Should be equal to 0";

	// wsMaxMsgSize
	socketChannelConfig.setWsMaxMsgSize((RWF_MAX_32)+1ULL);
	EXPECT_EQ((RWF_MAX_32), socketChannelConfig.wsMaxMsgSize) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	socketChannelConfig.setWsMaxMsgSize(midValue);
	EXPECT_EQ(midValue, socketChannelConfig.wsMaxMsgSize) << "Should be equal to " << midValue;
	socketChannelConfig.setWsMaxMsgSize(0);
	EXPECT_EQ(0, socketChannelConfig.wsMaxMsgSize) << "Should be equal 0";


	// Make some changes...
	socketChannelConfig.hostName = "non defaultHostName";
	socketChannelConfig.serviceName = "non defaultServiceName";
	socketChannelConfig.tcpNodelay = RSSL_FALSE;
	socketChannelConfig.objectName = "ooo";
	socketChannelConfig.sslCAStore = "ccc";

	socketChannelConfig.proxyHostName = "non def proxyHostName";
	socketChannelConfig.proxyPort = "non def proxyHostPort";
	socketChannelConfig.proxyUserName = "non def proxyUserName";
	socketChannelConfig.proxyPasswd = "non def proxyPasswd";
	socketChannelConfig.proxyDomain = "non def proxyDomain";
	socketChannelConfig.proxyConnectionTimeout = 420;

	//socketChannelConfig.encryptedConnectionType = RSSL_CONN_TYPE_HTTP;
	socketChannelConfig.securityProtocol = RSSL_ENC_NONE;

	socketChannelConfig.enableSessionMgnt = RSSL_TRUE;
	socketChannelConfig.location = "Oak Brooke Terrace";
	socketChannelConfig.serviceDiscoveryRetryCount = 5671;
	socketChannelConfig.wsMaxMsgSize = 42;
	socketChannelConfig.wsProtocols = "Protos forever";
	//socketChannelConfig.connectionType = RSSL_CONN_TYPE_UNIDIR_SHMEM;

	// Tests clear method
	socketChannelConfig.clear();
	SocketChannelConfigTestDefaultValues(socketChannelConfig);
}


void EmaActiveConfigTest::ServerConfigTestDefaultValues(SocketServerConfig& serverConfig)
{
	// Tests default values
	EXPECT_TRUE(serverConfig.name.empty());
	EXPECT_TRUE(serverConfig.interfaceName.empty());
	EXPECT_EQ(serverConfig.compressionType, DEFAULT_COMPRESSION_TYPE);
	EXPECT_EQ(serverConfig.compressionThreshold, DEFAULT_COMPRESSION_THRESHOLD);
	EXPECT_EQ(serverConfig.connectionType, RSSL_CONN_TYPE_SOCKET);
	EXPECT_EQ(serverConfig.getType(), RSSL_CONN_TYPE_SOCKET);

	EXPECT_EQ(serverConfig.connectionPingTimeout, DEFAULT_CONNECTION_PINGTIMEOUT);
	EXPECT_EQ(serverConfig.connectionMinPingTimeout, DEFAULT_CONNECTION_MINPINGTIMEOUT);
	EXPECT_EQ(serverConfig.directWrite, DEFAULT_DIRECT_WRITE);
	EXPECT_EQ(serverConfig.initializationTimeout, DEFAULT_INITIALIZATION_ACCEPT_TIMEOUT);
	EXPECT_EQ(serverConfig.guaranteedOutputBuffers, DEFAULT_PROVIDER_GUARANTEED_OUTPUT_BUFFERS);
	EXPECT_EQ(serverConfig.numInputBuffers, DEFAULT_NUM_INPUT_BUFFERS);
	EXPECT_EQ(serverConfig.sysRecvBufSize, DEFAULT_PROVIDER_SYS_RECEIVE_BUFFER_SIZE);
	EXPECT_EQ(serverConfig.sysSendBufSize, DEFAULT_PROVIDER_SYS_SEND_BUFFER_SIZE);

	EXPECT_EQ(serverConfig.highWaterMark, DEFAULT_HIGH_WATER_MARK);
}

TEST_F(EmaActiveConfigTest, ServerConfigTest)
{
	SocketServerConfig serverConfig("defaultServiceName");

	// Tests default values
	ServerConfigTestDefaultValues(serverConfig);

	const int midValue = 120;
	// setGuaranteedOutputBuffers
	serverConfig.setGuaranteedOutputBuffers( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), serverConfig.guaranteedOutputBuffers) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	serverConfig.setGuaranteedOutputBuffers( midValue );
	EXPECT_EQ(midValue, serverConfig.guaranteedOutputBuffers) << "Should be equal to " << midValue;
	serverConfig.setGuaranteedOutputBuffers( 0 );
	EXPECT_EQ(midValue, serverConfig.guaranteedOutputBuffers) << "Should be not 0, previous value: " << midValue;

	// setNumInputBuffers
	serverConfig.setNumInputBuffers( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), serverConfig.numInputBuffers) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	serverConfig.setNumInputBuffers( midValue );
	EXPECT_EQ(midValue, serverConfig.numInputBuffers) << "Should be equal to " << midValue;
	serverConfig.setNumInputBuffers( 0 );
	EXPECT_EQ(midValue, serverConfig.numInputBuffers) << "Should be not 0, previous value: " << midValue;

	// Make some changes...
	serverConfig.name = "sdfs";
	serverConfig.interfaceName = "dsfsa";
	serverConfig.compressionType = RSSL_COMP_LZ4;
	serverConfig.compressionThreshold = 342;
	serverConfig.connectionType = RSSL_CONN_TYPE_HTTP;

	serverConfig.connectionPingTimeout = 7897;
	serverConfig.connectionMinPingTimeout = 12;
	serverConfig.directWrite = 1;
	serverConfig.initializationTimeout = 7;
	serverConfig.guaranteedOutputBuffers = 8;
	serverConfig.numInputBuffers = 33;
	serverConfig.sysRecvBufSize = 777;
	serverConfig.sysSendBufSize = 23;
	serverConfig.highWaterMark = 12;

	// Tests clear method
	serverConfig.clear();
	ServerConfigTestDefaultValues(serverConfig);
}


void EmaActiveConfigTest::SocketServerConfigTestDefaultValues(SocketServerConfig & socketServerConfig)
{
	// Tests default values
	EXPECT_STREQ(socketServerConfig.serviceName.c_str(), "defaultHostName");
	EXPECT_EQ(socketServerConfig.tcpNodelay, DEFAULT_TCP_NODELAY);
	EXPECT_EQ(socketServerConfig.serverSharedSocket, DEFAULT_SERVER_SHAREDSOCKET);
	EXPECT_EQ(socketServerConfig.maxFragmentSize, DEFAULT_MAX_FRAGMENT_SIZE);
	EXPECT_EQ(socketServerConfig.wsProtocols, DEFAULT_WS_PROTOCLOS);

	EXPECT_EQ(socketServerConfig.securityProtocol, (RSSL_ENC_TLSV1_2 | RSSL_ENC_TLSV1_3));

	EXPECT_TRUE(socketServerConfig.libSslName.empty());
	EXPECT_TRUE(socketServerConfig.libCryptoName.empty());
	EXPECT_TRUE(socketServerConfig.libCurlName.empty());
	EXPECT_TRUE(socketServerConfig.serverCert.empty());
	EXPECT_TRUE(socketServerConfig.serverPrivateKey.empty());
	EXPECT_TRUE(socketServerConfig.cipherSuite.empty());
	EXPECT_TRUE(socketServerConfig.dhParams.empty());
}

TEST_F(EmaActiveConfigTest, SocketServerConfigTest)
{
	SocketServerConfig socketServerConfig("defaultHostName");

	// Tests default values
	SocketServerConfigTestDefaultValues(socketServerConfig);

	// Make some changes...
	socketServerConfig.serviceName = "non-defaultHostName";
	socketServerConfig.tcpNodelay = RSSL_FALSE;
	socketServerConfig.serverSharedSocket = RSSL_TRUE;
	socketServerConfig.maxFragmentSize = 33;
	socketServerConfig.wsProtocols = "list of protocols";

	socketServerConfig.libSslName = "libSslName";
	socketServerConfig.libCryptoName = "libCryptoName";
	socketServerConfig.libCurlName = "libCurlName";
	socketServerConfig.serverCert = "serverCert";
	socketServerConfig.serverPrivateKey = "liserverPrivateKeybSslName";
	socketServerConfig.cipherSuite = "cipherSuite";
	socketServerConfig.dhParams = "dhParams";

	socketServerConfig.securityProtocol = RSSL_ENC_NONE;

	// Tests clear method
	socketServerConfig.clear();
	SocketServerConfigTestDefaultValues(socketServerConfig);
}


void EmaActiveConfigTest::ReliableMcastChannelConfigTestDefaultValues(ReliableMcastChannelConfig & reliableMcastChannelConfig)
{
	// Tests default values
	EXPECT_TRUE(reliableMcastChannelConfig.recvAddress.empty());
	EXPECT_TRUE(reliableMcastChannelConfig.recvServiceName.empty());
	EXPECT_TRUE(reliableMcastChannelConfig.unicastServiceName.empty());
	EXPECT_TRUE(reliableMcastChannelConfig.sendAddress.empty());
	EXPECT_TRUE(reliableMcastChannelConfig.sendServiceName.empty());
	EXPECT_TRUE(reliableMcastChannelConfig.hsmInterface.empty());
	EXPECT_TRUE(reliableMcastChannelConfig.tcpControlPort.empty());
	EXPECT_TRUE(reliableMcastChannelConfig.hsmMultAddress.empty());
	EXPECT_TRUE(reliableMcastChannelConfig.hsmPort.empty());

	EXPECT_EQ(reliableMcastChannelConfig.hsmInterval, 0);
	EXPECT_EQ(reliableMcastChannelConfig.packetTTL, DEFAULT_PACKET_TTL);
	EXPECT_EQ(reliableMcastChannelConfig.ndata, DEFAULT_NDATA);
	EXPECT_EQ(reliableMcastChannelConfig.nmissing, DEFAULT_NMISSING);
	EXPECT_EQ(reliableMcastChannelConfig.nrreq, DEFAULT_NREQ);
	EXPECT_EQ(reliableMcastChannelConfig.pktPoolLimitHigh, DEFAULT_PKT_POOLLIMIT_HIGH);
	EXPECT_EQ(reliableMcastChannelConfig.pktPoolLimitLow, DEFAULT_PKT_POOLLIMIT_LOW);
	EXPECT_EQ(reliableMcastChannelConfig.tdata, DEFAULT_TDATA);
	EXPECT_EQ(reliableMcastChannelConfig.trreq, DEFAULT_TRREQ);
	EXPECT_EQ(reliableMcastChannelConfig.twait, DEFAULT_TWAIT);
	EXPECT_EQ(reliableMcastChannelConfig.tbchold, DEFAULT_TBCHOLD);
	EXPECT_EQ(reliableMcastChannelConfig.tpphold, DEFAULT_TPPHOLD);
	EXPECT_EQ(reliableMcastChannelConfig.userQLimit, DEFAULT_USER_QLIMIT);
	EXPECT_EQ(reliableMcastChannelConfig.disconnectOnGap, RSSL_FALSE);

	EXPECT_EQ(reliableMcastChannelConfig.getType(), RSSL_CONN_TYPE_RELIABLE_MCAST);
}

TEST_F(EmaActiveConfigTest, ReliableMcastChannelConfigTest)
{
	ReliableMcastChannelConfig reliableMcastChannelConfig;

	// Tests default values
	ReliableMcastChannelConfigTestDefaultValues(reliableMcastChannelConfig);

	const int midValue = 150;
	// setPacketTTL
	reliableMcastChannelConfig.setPacketTTL( (RWF_MAX_8) + 1ULL );
	EXPECT_EQ((RWF_MAX_8), reliableMcastChannelConfig.packetTTL) << "Should be equal to RWF_MAX_8 - maximum allowed value";
	reliableMcastChannelConfig.setPacketTTL( midValue );
	EXPECT_EQ(midValue, reliableMcastChannelConfig.packetTTL) << "Should be equal to " << midValue;
	reliableMcastChannelConfig.setPacketTTL( 0 );
	EXPECT_EQ(DEFAULT_PACKET_TTL, reliableMcastChannelConfig.packetTTL) << "Should be not 0, default value: " << DEFAULT_PACKET_TTL;

	// setHsmInterval
	reliableMcastChannelConfig.setHsmInterval( (RWF_MAX_16) + 1ULL );
	EXPECT_EQ((RWF_MAX_16), reliableMcastChannelConfig.hsmInterval) << "Should be equal to RWF_MAX_16 - maximum allowed value";
	reliableMcastChannelConfig.setHsmInterval( midValue );
	EXPECT_EQ(midValue, reliableMcastChannelConfig.hsmInterval) << "Should be equal to " << midValue;
	reliableMcastChannelConfig.setHsmInterval( 0 );
	EXPECT_EQ(midValue, reliableMcastChannelConfig.hsmInterval) << "Should be not 0, previous value: " << midValue;

	// setNdata
	reliableMcastChannelConfig.setNdata( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), reliableMcastChannelConfig.ndata) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	reliableMcastChannelConfig.setNdata( midValue );
	EXPECT_EQ(midValue, reliableMcastChannelConfig.ndata) << "Should be equal to " << midValue;
	reliableMcastChannelConfig.setNdata( 0 );
	EXPECT_EQ(DEFAULT_NDATA, reliableMcastChannelConfig.ndata) << "Should be not 0, default value: " << DEFAULT_NDATA;

	// setNmissing
	reliableMcastChannelConfig.setNmissing( (RWF_MAX_16) + 1ULL );
	EXPECT_EQ((RWF_MAX_16), reliableMcastChannelConfig.nmissing) << "Should be equal to RWF_MAX_16 - maximum allowed value";
	reliableMcastChannelConfig.setNmissing( midValue );
	EXPECT_EQ(midValue, reliableMcastChannelConfig.nmissing) << "Should be equal to " << midValue;
	reliableMcastChannelConfig.setNmissing( 0 );
	EXPECT_EQ(DEFAULT_NMISSING, reliableMcastChannelConfig.nmissing) << "Should be not 0, defualt value: " << DEFAULT_NMISSING;

	// setNrreq
	reliableMcastChannelConfig.setNrreq( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), reliableMcastChannelConfig.nrreq) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	reliableMcastChannelConfig.setNrreq( midValue );
	EXPECT_EQ(midValue, reliableMcastChannelConfig.nrreq) << "Should be equal to " << midValue;
	reliableMcastChannelConfig.setNrreq( 0 );
	EXPECT_EQ(DEFAULT_NREQ, reliableMcastChannelConfig.nrreq) << "Should be not 0, defualt value: " << DEFAULT_NREQ;

	// setTdata
	reliableMcastChannelConfig.setTdata( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), reliableMcastChannelConfig.tdata) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	reliableMcastChannelConfig.setTdata( midValue );
	EXPECT_EQ(midValue, reliableMcastChannelConfig.tdata) << "Should be equal to " << midValue;
	reliableMcastChannelConfig.setTdata( 0 );
	EXPECT_EQ(DEFAULT_TDATA, reliableMcastChannelConfig.tdata) << "Should be not 0, defualt value: " << DEFAULT_TDATA;

	// setTrreq
	reliableMcastChannelConfig.setTrreq( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), reliableMcastChannelConfig.trreq) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	reliableMcastChannelConfig.setTrreq( midValue );
	EXPECT_EQ(midValue, reliableMcastChannelConfig.trreq) << "Should be equal to " << midValue;
	reliableMcastChannelConfig.setTrreq( 0 );
	EXPECT_EQ(DEFAULT_TRREQ, reliableMcastChannelConfig.trreq) << "Should be not 0, defualt value: " << DEFAULT_TRREQ;

	// setPktPoolLimitHigh
	reliableMcastChannelConfig.setPktPoolLimitHigh( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), reliableMcastChannelConfig.pktPoolLimitHigh) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	reliableMcastChannelConfig.setPktPoolLimitHigh( DEFAULT_PKT_POOLLIMIT_HIGH + 1280 );
	EXPECT_EQ(DEFAULT_PKT_POOLLIMIT_HIGH + 1280, reliableMcastChannelConfig.pktPoolLimitHigh) << "Should be equal to " << (DEFAULT_PKT_POOLLIMIT_HIGH + 1280);
	reliableMcastChannelConfig.setPktPoolLimitHigh( midValue );
	EXPECT_EQ(DEFAULT_PKT_POOLLIMIT_HIGH, reliableMcastChannelConfig.pktPoolLimitHigh) << "Should be not 0, defualt value: " << DEFAULT_PKT_POOLLIMIT_HIGH;

	// setPktPoolLimitLow
	reliableMcastChannelConfig.setPktPoolLimitLow( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), reliableMcastChannelConfig.pktPoolLimitLow) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	reliableMcastChannelConfig.setPktPoolLimitLow( DEFAULT_PKT_POOLLIMIT_LOW + 1280 );
	EXPECT_EQ(DEFAULT_PKT_POOLLIMIT_LOW + 1280, reliableMcastChannelConfig.pktPoolLimitLow) << "Should be equal to " << (DEFAULT_PKT_POOLLIMIT_LOW + 1280);
	reliableMcastChannelConfig.setPktPoolLimitLow( midValue );
	EXPECT_EQ(DEFAULT_PKT_POOLLIMIT_LOW, reliableMcastChannelConfig.pktPoolLimitLow) << "Should be not 0, defualt value: " << DEFAULT_PKT_POOLLIMIT_LOW;

	// setTwait
	reliableMcastChannelConfig.setTwait( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), reliableMcastChannelConfig.twait) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	reliableMcastChannelConfig.setTwait( midValue );
	EXPECT_EQ(midValue, reliableMcastChannelConfig.twait) << "Should be equal to " << midValue;
	reliableMcastChannelConfig.setTwait( 0 );
	EXPECT_EQ(DEFAULT_TWAIT, reliableMcastChannelConfig.twait) << "Should be not 0, defualt value: " << DEFAULT_TWAIT;

	// setTbchold
	reliableMcastChannelConfig.setTbchold( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), reliableMcastChannelConfig.tbchold) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	reliableMcastChannelConfig.setTbchold( midValue );
	EXPECT_EQ(midValue, reliableMcastChannelConfig.tbchold) << "Should be equal to " << midValue;
	reliableMcastChannelConfig.setTbchold( 0 );
	EXPECT_EQ(DEFAULT_TBCHOLD, reliableMcastChannelConfig.tbchold) << "Should be not 0, defualt value: " << DEFAULT_TBCHOLD;

	// setTpphold
	reliableMcastChannelConfig.setTpphold( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), reliableMcastChannelConfig.tpphold) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	reliableMcastChannelConfig.setTpphold( midValue );
	EXPECT_EQ(midValue, reliableMcastChannelConfig.tpphold) << "Should be equal to " << midValue;
	reliableMcastChannelConfig.setTpphold( 0 );
	EXPECT_EQ(DEFAULT_TPPHOLD, reliableMcastChannelConfig.tpphold) << "Should be not 0, defualt value: " << DEFAULT_TPPHOLD;

	// setUserQLimit
	reliableMcastChannelConfig.setUserQLimit( (RWF_MAX_16) + 1ULL );
	EXPECT_EQ((RWF_MAX_16), reliableMcastChannelConfig.userQLimit) << "Should be equal to RWF_MAX_16 - maximum allowed value";
	reliableMcastChannelConfig.setUserQLimit( 10000ULL );
	EXPECT_EQ(10000ULL, reliableMcastChannelConfig.userQLimit) << "Should be equal to " << 10000ULL;
	reliableMcastChannelConfig.setUserQLimit( 0 );
	EXPECT_EQ(LOWLIMIT_USER_QLIMIT, reliableMcastChannelConfig.userQLimit) << "Should be not 0, defualt value: " << LOWLIMIT_USER_QLIMIT;

	// Make some changes...
	reliableMcastChannelConfig.recvAddress = "fhh22fhf";
	reliableMcastChannelConfig.recvServiceName = "fh9hfhf";
	reliableMcastChannelConfig.unicastServiceName = "fhh8fhf";
	reliableMcastChannelConfig.sendAddress = "fhh7fhf";
	reliableMcastChannelConfig.sendServiceName = "fhhf6hf";
	reliableMcastChannelConfig.hsmInterface = "fhhf6hf";
	reliableMcastChannelConfig.tcpControlPort = "fhh5fhf";
	reliableMcastChannelConfig.hsmMultAddress = "fhh4fhf";
	reliableMcastChannelConfig.hsmPort = "fhh3fhf";

	reliableMcastChannelConfig.hsmInterval = 11;
	reliableMcastChannelConfig.packetTTL = 35;
	reliableMcastChannelConfig.ndata = 33;
	reliableMcastChannelConfig.nmissing = 33;
	reliableMcastChannelConfig.nrreq = 33;
	reliableMcastChannelConfig.pktPoolLimitHigh = 33;
	reliableMcastChannelConfig.pktPoolLimitLow = 33;
	reliableMcastChannelConfig.tdata = 33;
	reliableMcastChannelConfig.trreq = 33;
	reliableMcastChannelConfig.twait = 33;
	reliableMcastChannelConfig.tbchold = 33;
	reliableMcastChannelConfig.tpphold = 33;
	reliableMcastChannelConfig.userQLimit = 33;
	reliableMcastChannelConfig.disconnectOnGap = RSSL_TRUE;

	// Tests clear method
	reliableMcastChannelConfig.clear();
	ReliableMcastChannelConfigTestDefaultValues(reliableMcastChannelConfig);
}


void EmaActiveConfigTest::BaseConfigTestDefaultValues(BaseConfig & baseConfig)
{
	// Tests default values
	EXPECT_TRUE(baseConfig.configuredName.empty());
	EXPECT_TRUE(baseConfig.instanceName.empty());
	EXPECT_EQ(baseConfig.itemCountHint, DEFAULT_ITEM_COUNT_HINT);
	EXPECT_EQ(baseConfig.serviceCountHint, DEFAULT_SERVICE_COUNT_HINT);
	EXPECT_EQ(baseConfig.dispatchTimeoutApiThread, DEFAULT_DISPATCH_TIMEOUT_API_THREAD);
	EXPECT_EQ(baseConfig.maxDispatchCountApiThread, DEFAULT_MAX_DISPATCH_COUNT_API_THREAD);
	EXPECT_EQ(baseConfig.maxDispatchCountUserThread, DEFAULT_MAX_DISPATCH_COUNT_USER_THREAD);
	EXPECT_EQ(baseConfig.maxEventsInPool, DEFAULT_MAX_EVENT_IN_POOL);
	EXPECT_EQ(baseConfig.requestTimeout, DEFAULT_REQUEST_TIMEOUT);

	EXPECT_EQ(baseConfig.xmlTraceMaxFileSize, DEFAULT_XML_TRACE_MAX_FILE_SIZE);
	EXPECT_EQ(baseConfig.xmlTraceToFile, DEFAULT_XML_TRACE_TO_FILE);
	EXPECT_EQ(baseConfig.xmlTraceToStdout, DEFAULT_XML_TRACE_TO_STDOUT);
	EXPECT_EQ(baseConfig.xmlTraceToMultipleFiles, DEFAULT_XML_TRACE_TO_MULTIPLE_FILE);
	EXPECT_EQ(baseConfig.xmlTraceWrite, DEFAULT_XML_TRACE_WRITE);
	EXPECT_EQ(baseConfig.xmlTraceRead, DEFAULT_XML_TRACE_READ);
	EXPECT_EQ(baseConfig.xmlTracePing, DEFAULT_XML_TRACE_PING);
	EXPECT_EQ(baseConfig.xmlTracePingOnly, DEFAULT_XML_TRACE_PING_ONLY);
	EXPECT_EQ(baseConfig.xmlTraceHex, DEFAULT_XML_TRACE_HEX);
	EXPECT_EQ(baseConfig.xmlTraceDump, DEFAULT_XML_TRACE_DUMP);
	EXPECT_EQ(baseConfig.xmlTraceFileName, DEFAULT_XML_TRACE_FILE_NAME);
	EXPECT_EQ(baseConfig.catchUnhandledException, DEFAULT_HANDLE_EXCEPTION);
	EXPECT_EQ(baseConfig.parameterConfigGroup, 1);

	EXPECT_TRUE(baseConfig.libSslName.empty());
	EXPECT_TRUE(baseConfig.libCryptoName.empty());
	EXPECT_TRUE(baseConfig.traceStr.empty());
	EXPECT_TRUE(baseConfig.restLogFileName.empty());

	EXPECT_EQ(baseConfig.tokenReissueRatio, DEFAULT_TOKEN_REISSUE_RATIO);
	EXPECT_EQ(baseConfig.defaultServiceIDForConverter, DEFAULT_SERVICE_ID_FOR_CONVERTER);
	EXPECT_EQ(baseConfig.jsonExpandedEnumFields, DEFAULT_JSON_EXPANDED_ENUM_FIELDS);
	EXPECT_EQ(baseConfig.catchUnknownJsonKeys, DEFAULT_CATCH_UNKNOWN_JSON_KEYS);
	EXPECT_EQ(baseConfig.catchUnknownJsonFids, DEFAULT_CATCH_UNKNOWN_JSON_FIDS);
	EXPECT_EQ(baseConfig.closeChannelFromFailure, DEFAULT_CLOSE_CHANNEL_FROM_FAILURE);
	EXPECT_EQ(baseConfig.outputBufferSize, DEFAULT_OUTPUT_BUFFER_SIZE);
	EXPECT_EQ(baseConfig.jsonTokenIncrementSize, DEFAULT_JSON_TOKEN_INCREMENT_SIZE);
	EXPECT_EQ(baseConfig.restEnableLog, DEFAULT_REST_ENABLE_LOG);
	EXPECT_EQ(baseConfig.restVerboseMode, DEFAULT_REST_VERBOSE_MODE);
	EXPECT_EQ(baseConfig.restEnableLogViaCallback, DEFAULT_REST_ENABLE_LOG_VIA_CALLBACK);
	EXPECT_EQ(baseConfig.sendJsonConvError, DEFAULT_SEND_JSON_CONV_ERROR);
	EXPECT_EQ(baseConfig.shouldInitializeCPUIDlib, DEFAULT_SHOULD_INIT_CPUID_LIB);
}

TEST_F(EmaActiveConfigTest, BaseConfigTest)
{
	ActiveConfig baseConfig("defaultServiceName");

	// Tests default values
	BaseConfigTestDefaultValues(baseConfig);

	const int midValue = 150;
	// setItemCountHint
	baseConfig.setItemCountHint( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), baseConfig.itemCountHint) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	baseConfig.setItemCountHint( midValue );
	EXPECT_EQ(midValue, baseConfig.itemCountHint) << "Should be equal to " << midValue;
	baseConfig.setItemCountHint( 0 );
	EXPECT_EQ(midValue, baseConfig.itemCountHint) << "Should be not 0, previous value: " << midValue;

	// setServiceCountHint
	baseConfig.setServiceCountHint( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), baseConfig.serviceCountHint) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	baseConfig.setServiceCountHint( midValue );
	EXPECT_EQ(midValue, baseConfig.serviceCountHint) << "Should be equal to " << midValue;
	baseConfig.setServiceCountHint( 0 );
	EXPECT_EQ(midValue, baseConfig.serviceCountHint) << "Should be not 0, previous value: " << midValue;

	// setCatchUnhandledException
	baseConfig.setCatchUnhandledException( 333 );
	EXPECT_TRUE(baseConfig.catchUnhandledException) << "Should be equal to true when set non 0";
	baseConfig.setCatchUnhandledException( midValue );
	EXPECT_TRUE(baseConfig.catchUnhandledException) << "Should be equal to true";
	baseConfig.setCatchUnhandledException( 0 );
	EXPECT_FALSE(baseConfig.catchUnhandledException) << "Should be equal to false";

	// setMaxDispatchCountApiThread
	baseConfig.setMaxDispatchCountApiThread( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), baseConfig.maxDispatchCountApiThread) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	baseConfig.setMaxDispatchCountApiThread( midValue );
	EXPECT_EQ(midValue, baseConfig.maxDispatchCountApiThread) << "Should be equal to " << midValue;
	baseConfig.setMaxDispatchCountApiThread( 0 );
	EXPECT_EQ(midValue, baseConfig.maxDispatchCountApiThread) << "Should be not 0, previous value: " << midValue;

	// setMaxDispatchCountUserThread
	baseConfig.setMaxDispatchCountUserThread( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), baseConfig.maxDispatchCountUserThread) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	baseConfig.setMaxDispatchCountUserThread( midValue );
	EXPECT_EQ(midValue, baseConfig.maxDispatchCountUserThread) << "Should be equal to " << midValue;
	baseConfig.setMaxDispatchCountUserThread( 0 );
	EXPECT_EQ(midValue, baseConfig.maxDispatchCountUserThread) << "Should be not 0, previous value: " << midValue;

	// setMaxEventsInPool
	baseConfig.setMaxEventsInPool( (RWF_MAX_U31) + 1ULL );
	EXPECT_EQ((RWF_MAX_U31), baseConfig.maxEventsInPool) << "Should be equal to RWF_MAX_U31 - maximum allowed value";
	baseConfig.setMaxEventsInPool( midValue );
	EXPECT_EQ(midValue, baseConfig.maxEventsInPool) << "Should be equal to " << midValue;
	baseConfig.setMaxEventsInPool( 0 );
	EXPECT_EQ(midValue, baseConfig.maxEventsInPool) << "Should be not 0, previous value: " << midValue;

	// setRequestTimeout
	baseConfig.setRequestTimeout( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), baseConfig.requestTimeout) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	baseConfig.setRequestTimeout( midValue );
	EXPECT_EQ(midValue, baseConfig.requestTimeout) << "Should be equal to " << midValue;
	baseConfig.setRequestTimeout( 0 );
	EXPECT_EQ(midValue, baseConfig.requestTimeout) << "Should be not 0, previous value: " << midValue;

	// Make some changes...
	baseConfig.configuredName = "s1dhgfjsdh";
	baseConfig.instanceName = "sdhgf5dh";
	baseConfig.itemCountHint = 23;
	baseConfig.serviceCountHint = 23;
	baseConfig.dispatchTimeoutApiThread = 23;
	baseConfig.maxDispatchCountApiThread = 23;
	baseConfig.maxDispatchCountUserThread = 23;
	baseConfig.maxEventsInPool = -23;
	baseConfig.requestTimeout = 23;

	baseConfig.xmlTraceMaxFileSize = -23;
	baseConfig.xmlTraceToFile = true;
	baseConfig.xmlTraceToStdout = true;
	baseConfig.xmlTraceToMultipleFiles = true;
	baseConfig.xmlTraceWrite = false;
	baseConfig.xmlTraceRead = false;
	baseConfig.xmlTracePing = true;
	baseConfig.xmlTracePingOnly = true;
	baseConfig.xmlTraceHex = true;
	baseConfig.xmlTraceDump = true;
	baseConfig.xmlTraceFileName = "23";
	baseConfig.catchUnhandledException = false;
	baseConfig.parameterConfigGroup = 23;

	baseConfig.libSslName = "sdhgf4sdh";
	baseConfig.libCryptoName = "sdh3gfjsdh";
	baseConfig.traceStr = "sdhgfjs2dh";
	baseConfig.restLogFileName = "dfgdgfewr3cd";

	baseConfig.tokenReissueRatio = 23.15;
	baseConfig.defaultServiceIDForConverter = 23;
	baseConfig.jsonExpandedEnumFields = true;
	baseConfig.catchUnknownJsonKeys = true;
	baseConfig.catchUnknownJsonFids = false;
	baseConfig.closeChannelFromFailure = false;
	baseConfig.outputBufferSize = 23;
	baseConfig.jsonTokenIncrementSize = 42;
	baseConfig.restEnableLog = true;
	baseConfig.restVerboseMode = true;
	baseConfig.restEnableLogViaCallback = true;
	baseConfig.sendJsonConvError = true;
	baseConfig.shouldInitializeCPUIDlib = false;

	// Tests clear method
	baseConfig.clear();
	BaseConfigTestDefaultValues(baseConfig);
}


void EmaActiveConfigTest::ActiveConfigTestDefaultValues(ActiveConfig & activeConfig)
{
	// Tests default values
	EXPECT_EQ(activeConfig.obeyOpenWindow, DEFAULT_OBEY_OPEN_WINDOW);
	EXPECT_EQ(activeConfig.postAckTimeout, DEFAULT_POST_ACK_TIMEOUT);
	EXPECT_EQ(activeConfig.maxOutstandingPosts, DEFAULT_MAX_OUTSTANDING_POSTS);
	EXPECT_EQ(activeConfig.loginRequestTimeOut, DEFAULT_LOGIN_REQUEST_TIMEOUT);
	EXPECT_EQ(activeConfig.directoryRequestTimeOut, DEFAULT_DIRECTORY_REQUEST_TIMEOUT);
	EXPECT_EQ(activeConfig.dictionaryRequestTimeOut, DEFAULT_DICTIONARY_REQUEST_TIMEOUT);
	EXPECT_EQ(activeConfig.reconnectAttemptLimit, DEFAULT_RECONNECT_ATTEMPT_LIMIT);
	EXPECT_EQ(activeConfig.reconnectMinDelay, DEFAULT_RECONNECT_MIN_DELAY);
	EXPECT_EQ(activeConfig.reconnectMaxDelay, DEFAULT_RECONNECT_MAX_DELAY);
	EXPECT_TRUE(activeConfig.pRsslRDMLoginReq == NULL);
	EXPECT_TRUE(activeConfig.pRsslDirectoryRequestMsg == NULL);
	EXPECT_TRUE(activeConfig.pRsslRdmFldRequestMsg == NULL);
	EXPECT_TRUE(activeConfig.pRsslEnumDefRequestMsg == NULL);
	EXPECT_TRUE(activeConfig.pDirectoryRefreshMsg == NULL);

	EXPECT_STREQ(activeConfig.defaultServiceName().c_str(), "defaultServiceName");
	EXPECT_EQ(activeConfig.reissueTokenAttemptLimit, DEFAULT_REISSUE_TOKEN_ATTEMP_LIMIT);
	EXPECT_EQ(activeConfig.reissueTokenAttemptInterval, DEFAULT_REISSUE_TOKEN_ATTEMP_INTERVAL);
	EXPECT_EQ(activeConfig.restRequestTimeOut, DEFAULT_REST_REQUEST_TIMEOUT);

	EXPECT_TRUE(activeConfig.restProxyHostName.empty());
	EXPECT_TRUE(activeConfig.restProxyPort.empty());
	EXPECT_TRUE(activeConfig.restProxyUserName.empty());
	EXPECT_TRUE(activeConfig.restProxyPasswd.empty());
	EXPECT_TRUE(activeConfig.restProxyDomain.empty());
	EXPECT_EQ(activeConfig.enablePreferredHostOptions, DEFAULT_ENABLE_PREFERRED_HOST);
	EXPECT_EQ(activeConfig.phDetectionTimeSchedule, DEFAULT_DETECTION_TIME_SCHEDULE);
	EXPECT_EQ(activeConfig.phDetectionTimeInterval, DEFAULT_DETECTION_TIME_INTERVAL);
	EXPECT_EQ(activeConfig.preferredChannelName, DEFAULT_CHANNEL_NAME);
	EXPECT_EQ(activeConfig.preferredWSBChannelName, DEFAULT_WSB_CHANNEL_NAME);
	EXPECT_EQ(activeConfig.phFallBackWithInWSBGroup, DEFAULT_FALL_BACK_WITH_IN_WSB_GROUP);
}

TEST_F(EmaActiveConfigTest, ActiveConfigTest)
{
	ActiveConfig activeConfig("defaultServiceName");

	// Tests default values
	ActiveConfigTestDefaultValues(activeConfig);

	const int midValue = 150;
	// setObeyOpenWindow
	activeConfig.setObeyOpenWindow( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ(1, activeConfig.obeyOpenWindow) << "Should be equal to 1 - maximum allowed value";
	activeConfig.setObeyOpenWindow( midValue );
	EXPECT_EQ(1, activeConfig.obeyOpenWindow) << "Should be equal to 1";
	activeConfig.setObeyOpenWindow( 0 );
	EXPECT_EQ(0, activeConfig.obeyOpenWindow) << "Should be equal to 0";

	// setPostAckTimeout
	activeConfig.setPostAckTimeout( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), activeConfig.postAckTimeout) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	activeConfig.setPostAckTimeout( midValue );
	EXPECT_EQ(midValue, activeConfig.postAckTimeout) << "Should be equal to " << midValue;
	activeConfig.setPostAckTimeout( 0 );
	EXPECT_EQ(midValue, activeConfig.postAckTimeout) << "Should be not 0, previous value: " << midValue;

	// setMaxOutstandingPosts
	activeConfig.setMaxOutstandingPosts( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), activeConfig.maxOutstandingPosts) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	activeConfig.setMaxOutstandingPosts( midValue );
	EXPECT_EQ(midValue, activeConfig.maxOutstandingPosts) << "Should be equal to " << midValue;
	activeConfig.setMaxOutstandingPosts( 0 );
	EXPECT_EQ(midValue, activeConfig.maxOutstandingPosts) << "Should be not 0, previous value: " << midValue;

	// setLoginRequestTimeOut
	activeConfig.setLoginRequestTimeOut( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), activeConfig.loginRequestTimeOut) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	activeConfig.setLoginRequestTimeOut( midValue );
	EXPECT_EQ(midValue, activeConfig.loginRequestTimeOut) << "Should be equal to " << midValue;
	activeConfig.setLoginRequestTimeOut( 0 );
	EXPECT_EQ(0, activeConfig.loginRequestTimeOut) << "Should be equal to 0";

	// setDirectoryRequestTimeOut
	activeConfig.setDirectoryRequestTimeOut( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), activeConfig.directoryRequestTimeOut) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	activeConfig.setDirectoryRequestTimeOut( midValue );
	EXPECT_EQ(midValue, activeConfig.directoryRequestTimeOut) << "Should be equal to " << midValue;
	activeConfig.setDirectoryRequestTimeOut( 0 );
	EXPECT_EQ(0, activeConfig.directoryRequestTimeOut) << "Should be equal to 0";

	// setDictionaryRequestTimeOut
	activeConfig.setDictionaryRequestTimeOut( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), activeConfig.dictionaryRequestTimeOut) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	activeConfig.setDictionaryRequestTimeOut( midValue );
	EXPECT_EQ(midValue, activeConfig.dictionaryRequestTimeOut) << "Should be equal to " << midValue;
	activeConfig.setDictionaryRequestTimeOut( 0 );
	EXPECT_EQ(0, activeConfig.dictionaryRequestTimeOut) << "Should be equal to 0";

	// setReconnectAttemptLimit
	activeConfig.setReconnectAttemptLimit( (RWF_MAX_U31) + 1ULL );
	EXPECT_EQ((RWF_MAX_U31), activeConfig.reconnectAttemptLimit) << "Should be equal to RWF_MAX_U31 - maximum allowed value";
	activeConfig.setReconnectAttemptLimit( midValue );
	EXPECT_EQ(midValue, activeConfig.reconnectAttemptLimit) << "Should be equal to " << midValue;
	activeConfig.setReconnectAttemptLimit( 0 );
	EXPECT_EQ(0, activeConfig.reconnectAttemptLimit) << "Should be equal to 0";

	// setReconnectMinDelay
	activeConfig.setReconnectMinDelay( (RWF_MAX_U31) + 1ULL );
	EXPECT_EQ((RWF_MAX_U31), activeConfig.reconnectMinDelay) << "Should be equal to RWF_MAX_U31 - maximum allowed value";
	activeConfig.setReconnectMinDelay( midValue );
	EXPECT_EQ(midValue, activeConfig.reconnectMinDelay) << "Should be equal to " << midValue;
	activeConfig.setReconnectMinDelay( 0 );
	EXPECT_EQ(midValue, activeConfig.reconnectMinDelay) << "Should be not 0, previous value: " << midValue;

	// setReconnectMaxDelay
	activeConfig.setReconnectMaxDelay( (RWF_MAX_U31) + 1ULL );
	EXPECT_EQ((RWF_MAX_U31), activeConfig.reconnectMaxDelay) << "Should be equal to RWF_MAX_U31 - maximum allowed value";
	activeConfig.setReconnectMaxDelay( midValue );
	EXPECT_EQ(midValue, activeConfig.reconnectMaxDelay) << "Should be equal to " << midValue;
	activeConfig.setReconnectMaxDelay( 0 );
	EXPECT_EQ(midValue, activeConfig.reconnectMaxDelay) << "Should be not 0, previous value: " << midValue;

	// setRestRequestTimeOut
	activeConfig.setRestRequestTimeOut( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), activeConfig.restRequestTimeOut) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	activeConfig.setRestRequestTimeOut( midValue );
	EXPECT_EQ(midValue, activeConfig.restRequestTimeOut) << "Should be equal to " << midValue;
	activeConfig.setRestRequestTimeOut( 0 );
	EXPECT_EQ(0, activeConfig.restRequestTimeOut) << "Should be equal to 0";

	// Make some changes...
	activeConfig.obeyOpenWindow = 0;
	activeConfig.postAckTimeout = 57;
	activeConfig.maxOutstandingPosts = 57;
	activeConfig.loginRequestTimeOut = 57;
	activeConfig.directoryRequestTimeOut = 57;
	activeConfig.dictionaryRequestTimeOut = 57;
	activeConfig.reconnectAttemptLimit = 57;
	activeConfig.reconnectMinDelay = 57;
	activeConfig.reconnectMaxDelay = 57;
	activeConfig.pRsslRDMLoginReq = NULL;
	activeConfig.pRsslDirectoryRequestMsg = NULL;
	activeConfig.pRsslRdmFldRequestMsg = NULL;
	activeConfig.pRsslEnumDefRequestMsg = NULL;
	activeConfig.pDirectoryRefreshMsg = NULL;

	//activeConfig.defaultServiceName().c_str(), "defaultServiceName");
	activeConfig.reissueTokenAttemptLimit = 57;
	activeConfig.reissueTokenAttemptInterval = 57;
	activeConfig.restRequestTimeOut = 57;

	activeConfig.restProxyHostName = "non def restProxyHostName";
	activeConfig.restProxyPort = "non def restProxyHostPort";
	activeConfig.restProxyUserName = "non def restProxyUserName";
	activeConfig.restProxyPasswd = "non def restProxyPasswd";
	activeConfig.restProxyDomain = "non def restProxyDomain";

	activeConfig.enablePreferredHostOptions = true;
	activeConfig.phDetectionTimeSchedule = "non def phDetectionTimeSchedule";
	activeConfig.phDetectionTimeInterval = 123;
	activeConfig.preferredChannelName = "non def preferredChannelName";
	activeConfig.preferredWSBChannelName = "non def preferredWSBChannelName";
	activeConfig.phFallBackWithInWSBGroup = true;

	// Tests clear method
	activeConfig.clear();
	ActiveConfigTestDefaultValues(activeConfig);
}


void EmaActiveConfigTest::ActiveServerConfigTestDefaultValues(OmmIProviderActiveConfig& activeServerConfig)
{
	// Tests default values
	EXPECT_EQ(activeServerConfig.acceptMessageWithoutBeingLogin, DEFAULT_ACCEPT_MSG_WITHOUT_BEING_LOGIN);
	EXPECT_EQ(activeServerConfig.acceptMessageWithoutAcceptingRequests, DEFAULT_ACCEPT_MSG_WITHOUT_ACCEPTING_REQUESTS);
	EXPECT_EQ(activeServerConfig.acceptDirMessageWithoutMinFilters, DEFAULT_ACCEPT_DIR_MSG_WITHOUT_MIN_FILTERS);
	EXPECT_EQ(activeServerConfig.acceptMessageWithoutQosInRange, DEFAULT_ACCEPT_MSG_WITHOUT_QOS_IN_RANGE);
	EXPECT_EQ(activeServerConfig.acceptMessageSameKeyButDiffStream, DEFAULT_ACCEPT_MSG_SAMEKEY_BUT_DIFF_STREAM);
	EXPECT_EQ(activeServerConfig.acceptMessageThatChangesService, DEFAULT_ACCEPT_MSG_THAT_CHANGES_SERVICE);

	EXPECT_STREQ(activeServerConfig.defaultServiceName().c_str(), "14002");
	EXPECT_TRUE(activeServerConfig.pDirectoryRefreshMsg == NULL);
}

TEST_F(EmaActiveConfigTest, ActiveServerConfigTest)
{
	OmmIProviderActiveConfig activeServerConfig;

	// Tests default values
	ActiveServerConfigTestDefaultValues(activeServerConfig);

	// Make some changes...
	activeServerConfig.acceptMessageWithoutBeingLogin = !DEFAULT_ACCEPT_MSG_WITHOUT_BEING_LOGIN;
	activeServerConfig.acceptMessageWithoutAcceptingRequests = !DEFAULT_ACCEPT_MSG_WITHOUT_ACCEPTING_REQUESTS;
	activeServerConfig.acceptDirMessageWithoutMinFilters = !DEFAULT_ACCEPT_DIR_MSG_WITHOUT_MIN_FILTERS;
	activeServerConfig.acceptMessageWithoutQosInRange = !DEFAULT_ACCEPT_MSG_WITHOUT_QOS_IN_RANGE;
	activeServerConfig.acceptMessageSameKeyButDiffStream = !DEFAULT_ACCEPT_MSG_SAMEKEY_BUT_DIFF_STREAM;
	activeServerConfig.acceptMessageThatChangesService = !DEFAULT_ACCEPT_MSG_THAT_CHANGES_SERVICE;

	// Tests clear method
	activeServerConfig.clear();
	ActiveServerConfigTestDefaultValues(activeServerConfig);
}


void EmaActiveConfigTest::OmmIProviderActiveConfigTestDefaultValues(OmmIProviderActiveConfig & ommIProvideractiveServerConfig)
{
	// Tests default values
	EXPECT_EQ(ommIProvideractiveServerConfig.getRefreshFirstRequired(), true);
	EXPECT_EQ(ommIProvideractiveServerConfig.getMaxFieldDictFragmentSize(), 8192);
	EXPECT_EQ(ommIProvideractiveServerConfig.getMaxEnumTypeFragmentSize(), 12800);
}

TEST_F(EmaActiveConfigTest, OmmIProviderActiveConfigTest)
{
	OmmIProviderActiveConfig ommIProvideractiveServerConfig;

	// Tests default values
	OmmIProviderActiveConfigTestDefaultValues(ommIProvideractiveServerConfig);

	const int midValue = 150;
	// setMaxFieldDictFragmentSize
	ommIProvideractiveServerConfig.setMaxFieldDictFragmentSize( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), ommIProvideractiveServerConfig.getMaxFieldDictFragmentSize()) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	ommIProvideractiveServerConfig.setMaxFieldDictFragmentSize( midValue );
	EXPECT_EQ(midValue, ommIProvideractiveServerConfig.getMaxFieldDictFragmentSize()) << "Should be equal to " << midValue;
	ommIProvideractiveServerConfig.setMaxFieldDictFragmentSize( 0 );
	EXPECT_EQ(midValue, ommIProvideractiveServerConfig.getMaxFieldDictFragmentSize()) << "Should be not 0, previous value: " << midValue;

	// setMaxEnumTypeFragmentSize
	ommIProvideractiveServerConfig.setMaxEnumTypeFragmentSize( (RWF_MAX_32) + 1ULL );
	EXPECT_EQ((RWF_MAX_32), ommIProvideractiveServerConfig.getMaxEnumTypeFragmentSize()) << "Should be equal to RWF_MAX_32 - maximum allowed value";
	ommIProvideractiveServerConfig.setMaxEnumTypeFragmentSize( midValue );
	EXPECT_EQ(midValue, ommIProvideractiveServerConfig.getMaxEnumTypeFragmentSize()) << "Should be equal to " << midValue;
	ommIProvideractiveServerConfig.setMaxEnumTypeFragmentSize( 0 );
	EXPECT_EQ(midValue, ommIProvideractiveServerConfig.getMaxEnumTypeFragmentSize()) << "Should be not 0, previous value: " << midValue;

	// setRefreshFirstRequired
	ommIProvideractiveServerConfig.setRefreshFirstRequired( (RWF_MAX_32) + 1ULL );
	EXPECT_TRUE(ommIProvideractiveServerConfig.getRefreshFirstRequired()) << "Should be equal to true";
	ommIProvideractiveServerConfig.setRefreshFirstRequired( midValue );
	EXPECT_TRUE(ommIProvideractiveServerConfig.getRefreshFirstRequired()) << "Should be equal to true";
	ommIProvideractiveServerConfig.setRefreshFirstRequired( 0 );
	EXPECT_FALSE(ommIProvideractiveServerConfig.getRefreshFirstRequired()) << "Should be equal to false";

	// Make some changes...
	ommIProvideractiveServerConfig.setRefreshFirstRequired(false);
	ommIProvideractiveServerConfig.setMaxFieldDictFragmentSize(1112);
	ommIProvideractiveServerConfig.setMaxEnumTypeFragmentSize(344567);

	// Tests clear method
	ommIProvideractiveServerConfig.clear();
	OmmIProviderActiveConfigTestDefaultValues(ommIProvideractiveServerConfig);
}
