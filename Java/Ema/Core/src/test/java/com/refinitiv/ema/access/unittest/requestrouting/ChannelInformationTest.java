///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|              Copyright (C) 2024 LSEG. All rights reserved.                --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access.unittest.requestrouting;

import com.refinitiv.ema.access.ChannelInformation;
import com.refinitiv.ema.access.PreferredHostInfo;

public class ChannelInformationTest implements ChannelInformation {

	private int _channelState;
	private int _connectionType;
	private String _hostname;
	private String _ipAddress;
	private int _port;
	private String _componentInfo;
	private int _protocolType;
	private int _majorVersion;
	private int _minorVersion;
	private int _pingTimeout;
	private int _maxFragmentSize;
	private int _maxOutputBuffers;
	private int _guaranteedOutputBuffers;
	private int _numInputBuffers;
	private int _sysSendBufSize;
	private int _sysRecvBufSize;
	private int _compressionType;
	private int _compressionThreshold;
	private int _encryptedConnectionType;
	private String _securityProtocol;
	public String _confChannelName;
	public String _confSessionChannelName;
	private PreferredHostInfo _preferredHostInfo;
	
	public ChannelInformationTest(ChannelInformation channelInformation)
	{
		_channelState = channelInformation.channelState();
		_connectionType = channelInformation.connectionType();
		_protocolType = channelInformation.protocolType();
		_pingTimeout = channelInformation.pingTimeout();
		_majorVersion = channelInformation.majorVersion();
		_minorVersion = channelInformation.minorVersion();
		_ipAddress = channelInformation.ipAddress();
		_hostname = channelInformation.hostname();
		_componentInfo = channelInformation.componentInformation();
		_port = channelInformation.port();
		_maxFragmentSize = channelInformation.maxFragmentSize();
		_maxOutputBuffers = channelInformation.maxOutputBuffers();
		_guaranteedOutputBuffers = channelInformation.guaranteedOutputBuffers();
		_numInputBuffers = channelInformation.numInputBuffers();
		_sysSendBufSize = channelInformation.sysSendBufSize();
		_sysRecvBufSize = channelInformation.sysRecvBufSize();
		_compressionType = channelInformation.compressionType();
		_compressionThreshold = channelInformation.compressionThreshold();
		_encryptedConnectionType = channelInformation.encryptedConnectionType();
		_securityProtocol = channelInformation.securityProtocol();
		_confChannelName = channelInformation.channelName();
		_confSessionChannelName = channelInformation.sessionChannelName();
		_preferredHostInfo = channelInformation.preferredHostInfo();
	}
	
	@Override
	public String channelName() {
		return _confChannelName;
	}

	@Override
	public String sessionChannelName() {
		return _confSessionChannelName;
	}

	@Override
	public void clear() {
		_channelState = ChannelState.CLOSED;
		_connectionType = ConnectionType.UNIDENTIFIED;
		_protocolType = ProtocolType.UNKNOWN;
		_pingTimeout = _majorVersion = _minorVersion = 0;
		_ipAddress = _hostname = _componentInfo = null;
		_port = 0;
		_maxFragmentSize = 0;
		_maxOutputBuffers = 0;
		_guaranteedOutputBuffers = 0;
		_numInputBuffers = 0;
		_sysSendBufSize = 0;
		_sysRecvBufSize = 0;
		_compressionType = 0;
		_compressionThreshold = 0;
		_encryptedConnectionType = -1;
		_securityProtocol = null;
		_confChannelName = "";
		_confSessionChannelName = "";
		_preferredHostInfo = null;
	}

	@Override
	public String componentInformation() {
		return _componentInfo;
	}

	@Override
	public String hostname() {
		return _hostname;
	}

	@Override
	public int port() {
		return _port;
	}

	@Override
	public String ipAddress() {
		return _ipAddress;
	}

	@Override
	public int connectionType() {
		return _connectionType;
	}

	@Override
	public int encryptedConnectionType() {
		return _encryptedConnectionType;
	}

	@Override
	public int channelState() {
		return _channelState;
	}

	@Override
	public int protocolType() {
		return _protocolType;
	}

	@Override
	public int majorVersion() {
		return _majorVersion;
	}

	@Override
	public int minorVersion() {
		return _minorVersion;
	}

	@Override
	public int pingTimeout() {
		return _pingTimeout;
	}

	@Override
	public int maxFragmentSize() {
		return _maxFragmentSize;
	}

	@Override
	public int maxOutputBuffers() {
		return _maxOutputBuffers;
	}

	@Override
	public int guaranteedOutputBuffers() {
		return _guaranteedOutputBuffers;
	}

	@Override
	public int numInputBuffers() {
		return _numInputBuffers;
	}

	@Override
	public int sysSendBufSize() {
		return _sysSendBufSize;
	}

	@Override
	public int sysRecvBufSize() {
		return _sysRecvBufSize;
	}

	@Override
	public int compressionType() {
		return _compressionType;
	}

	@Override
	public int compressionThreshold() {
		return _compressionThreshold;
	}

	@Override
	public String securityProtocol() {
		return _securityProtocol;
	}

	@Override
	public PreferredHostInfo preferredHostInfo() {
		return _preferredHostInfo;
	}

	@Override
	public void hostname(String hostname) {
		_hostname = hostname;
	}

	@Override
	public void port(int port) {
		_port = port;
	}

	@Override
	public void ipAddress(String ipAddress) {
		_ipAddress = ipAddress;
	}

	@Override
	public void componentInfo(String componentInfo) {
		_componentInfo = componentInfo;
	}

	@Override
	public void channelState(int channelState) {
		_channelState = channelState;		
	}

	@Override
	public void connectionType(int connectionType) {
		_connectionType = connectionType;
	}

	@Override
	public void encryptedConnectionType(int encryptedConnectionType) {
		_encryptedConnectionType = encryptedConnectionType;
	}

	@Override
	public void protocolType(int protocolType) {
		_protocolType = protocolType;
		
	}

	@Override
	public void majorVersion(int majorVersion) {
		_majorVersion = majorVersion;	
	}

	@Override
	public void minorVersion(int minorVersion) {
		_minorVersion = minorVersion;
	}

	@Override
	public void pingTimeout(int pingTimeout) {
		_pingTimeout = pingTimeout;
	}

	@Override
	public void maxFragmentSize(int maxFragmentSize) {
		_maxFragmentSize = maxFragmentSize;
	}

	@Override
	public void maxOutputBuffers(int maxOutputBuffers) {
		_maxOutputBuffers = maxOutputBuffers;
	}

	@Override
	public void guaranteedOutputBuffers(int guaranteedOutputBuffers) {
		_guaranteedOutputBuffers = guaranteedOutputBuffers;
	}

	@Override
	public void numInputBuffers(int numInputBuffers) {
		_numInputBuffers = numInputBuffers;
	}

	@Override
	public void sysSendBufSize(int sysSendBufSize) {
		_sysSendBufSize = sysSendBufSize;
	}

	@Override
	public void sysRecvBufSize(int sysRecvBufSize) {
		_sysRecvBufSize = sysRecvBufSize;
	}

	@Override
	public void compressionType(int compressionType) {
		_compressionType = compressionType;
	}

	@Override
	public void compressionThreshold(int compressionThreshold) {
		_compressionThreshold = compressionThreshold;
	}

	@Override
	public void securityProtocol(String securityProtocol) {
		_securityProtocol = securityProtocol;
	}

	@Override
	public void preferredHostInfo(PreferredHostInfo preferredHostInfo) {
		_preferredHostInfo = preferredHostInfo;
	}
}
