///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2019-2021,2023-2024 LSEG. All rights reserved.     --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

/**
 * The ChannelInformation interface provides channel information to an application
 *
 * For IProvider applications, this channel information is about channels used by
 * clients to connect to the IProvider application.
 *
 * For Consumer and NiProvider applications, this channel information is about the
 * out bound channel (e.g., the channel used by a Consumer application to connect to
 * an ADS) used to connect for receiving data (Consumer) or sending data (NiProvider).
 *
 * Examples of ChannelInformation usage are found in the examples:
 * (Consumer) Consumer/100_Series/170__MarketPrice__ChannelInfo
 * (IProvider) IProvider/100_Series/170__MarketPrice__ConnectedClientInfo
 * (NiProvider) NiProvider/100_Series/170__MarketPrice__ChannelInfo
 */
public interface ChannelInformation
{
	/**
	 * Channel states.
	 */
	public static class ChannelState
	{
        /**
	     * Channel has been CLOSED.
	     * Channel is set in this state when any socket related operation failed
	     * because the far end connection has been closed.
	     */
	    public static final int CLOSED = -1;

	    /**
	     * Indicates that a channel is inactive. This channel cannot be used.
	     * This state typically occurs after a channel is closed by the user.
	     */
	    public static final int INACTIVE = 0;

	    /**
	     * Indicates that a channel requires additional initialization.
	     */
	    public static final int INITIALIZING = 1;

	    /**
	     * Indicates that a channel is active.
	     */
	    public static final int ACTIVE = 2;
	}

	/**
	 * Connection types.
	 */
	public static class ConnectionType
	{
	    /**
	    * Unidentified connection type
	    */
            public static final int UNIDENTIFIED = -1;

	    /**
	     * Indicates that the channel is using a standard TCP-based socket
	     * connection.
	     */
	    public static final int SOCKET = 0;

	    /**
	     * Indicates that the channel is using an SSL/TLS encrypted
	     * HTTP TCP-based socket connection.
	     */
	    public static final int ENCRYPTED = 1;

	    /**
	     * Indicates that the channel is using an HTTP TCP-based socket
	     * connection.
	     */
	    public static final int HTTP = 2;

	    /**
	     * Indicates that the channel is using a unidirectional shared
	     * memory connection.
	     */
	    public static final int UNIDIR_SHMEM = 3;

	    /**
	     * Indicates that the channel is using a reliable multicast based
	     * connection. This type can be used to connect on a unified/mesh network
	     * where send and receive networks are the same or a segmented network where
	     * send and receive networks are different.
	     */
	    public static final int RELIABLE_MCAST = 4;

	    /**
	     * Indicates that the channel is using unreliable, sequenced multicast connection
		 * for reading from Real-Time Direct system.
	     */
	    public static final int SEQUENCED_MCAST = 6;

		/**
		 * Indicates that the channel is using a standard TCP-based WebSocket
		 * connection. This type can be used to connect between any ETA Transport
		 * based applications.
		 */
	    public static final int WEBSOCKET = 7;
	}

	/**
	 * Protocol types.
	 */
	public static class ProtocolType
	{
		/**
		 * Unknown wire format protocol
		 */
		public static final int UNKNOWN = -1;

		/**
		 * Rssl wire format protocol
		 */
		public static final int RWF = 0;
		
		/**
		 * Rssl JSON protocol
		 */
		public static final int JSON = 2;
	}
	
	/**
	 * Compression types.
	 */
	public static class CompressionType
	{
		/**
		 * No compression will be negotiated
		 */
		public static final int NONE = 0x00;

		/**
		 * Will attempt to use Zlib compression
		 */
		public static final int ZLIB = 0x01;
		
		/**
		 * Will attempt to use LZ4 compression
		 */
		public static final int LZ4 = 0x02;
	}
	
	/**
	 * Gets the EMA's configuration channel name
	 * @return the channel name
	 */
	public String channelName();
	
	/**
	 * Gets the EMA's configuration session channel name
	 * @return the session channel name
	 */
	public String sessionChannelName();

	/** Clears the ChannelInformation
	 *  <p>invoking clear() resets all member variables to their default values</p>
	 */
	public void clear();

	/**
	 *  Converts the ChannelInformation to a string.
	 *
	 * @return the string
	 */
	public String toString();

	/**
	 *  Gets the connected component information as a string.
	 *
	 * @return the component information
	 */
	public String componentInformation();

	/**
	 *  Gets the host name as a string.
	 *
	 * @return the string host name 
	 */
	public String hostname();

	/**
	 * Gets port being used by channel. Valid for SOCKET connection type
	 * see @{@link ConnectionType}
	 *
	 * @return port number
	 */
	public int port();

	/**
	 *  Gets the IP address of the connected client
	 *  <p>This is set only for IProvider applications</p>
	 * @return the string IP address
	 */
	public String ipAddress();

	/**
	 *  Gets the connection type from the ConnectionTypes class.
	 *
	 * @return the connection type
	 */
	public int connectionType();
	
	/**
	 *  Gets the encrypted connection type from the ConnectionTypes class. This method is used when {@link #connectionType()} is {@link ConnectionType#ENCRYPTED}
	 *
	 * @return the connection type
	 */
	public int encryptedConnectionType();

	/**
	 *  Gets the reactor channel state from the ChannelState class.
	 *
	 * @return the reactor channel state
	 */
	public int channelState();

	/**
	 *  Gets the protocol type.
	 *
	 * @return the protocolType
	 */
	public int protocolType();

	/**
	 *  Gets the major version.
	 *
	 * @return the majorVersion
	 */
	public int majorVersion();

	/**
	 *  Gets the minor version.
	 *
	 * @return the minorVersion
	 */
	public int minorVersion();

	/**
	 *  Gets the ping timeout.
	 *
	 * @return the ping timeout
	 */
	public int pingTimeout();
	
	/**
	 * Gets the max fragment size
	 *
	 * @return the max fragment size
	 */
	public int maxFragmentSize();
	
	/**
	 * Gets the maximum number of output buffers
	 *
	 * @return the maximum number of output buffers available to the channel.
	 */
	public int maxOutputBuffers();
	
	/**
	 * Gets the guaranteed number of output buffers
	 *
	 * @return the guaranteed number of output buffers available to the channel.
	 */
	public int guaranteedOutputBuffers();
	
	/**
	 * Gets the number of input buffers
	 *
	 * @return the number of input buffers available to the channel.
	 */
	public int numInputBuffers();
	
	/**
	 * Gets the systems send Buffer size
	 *
	 * @return the systems send buffer size respective to the transport type being used.
	 */
	public int sysSendBufSize();
	
	/**
	 * Gets the systems receive Buffer size
	 *
	 * @return the systems receive buffer size respective to the transport type being used.
	 */
	public int sysRecvBufSize();
	
	/**
	 * Gets the compression type
	 *
	 * @return the type of compression being used, if it is enabled.
	 */
	public int compressionType();
	
	/**
	 * Gets the compression threshold
	 *
	 * @return the compression threshold for compressing any message lager than this when compression is enabled.
	 */
	public int compressionThreshold();

	/**
	 * Gets the security protocol
	 *
	 * @return the security protocol used in the connection.
	 */
	public String securityProtocol();
	
	/** Sets host name
	 *
	 * @param hostname is the host name associated with the channel
	 */
	public void hostname(String hostname);

	/**
	 * Sets port number
	 *
	 * @param port port to use by channel
	 */
	public void port(int port);

	/** Sets the IP address of the connected client
	 * <p>This is set only for IProvider applications</p>
	 * @param ipAddress is the ipAddress associated with the channel
	 */
	public void ipAddress(String ipAddress);

	/** Sets component info
	 *
	 * @param componentInfo is the component information associated with the channel
	 *
	 */
	public void componentInfo(String componentInfo);

	/** Set channel state
	 *
	 * @param channelState is the state associated with the channel
	 *
	 */
	public void channelState(int channelState);

	/** Sets connection type
	 *
	 * @param connectionType is the connection type associated with the channel
	 *
	 */
	public void connectionType(int connectionType);
	
	/** Sets encrypted connection type
	 *
	 * @param encryptedConnectionType is the encrypted connection type associated with the channel
	 *
	 */
	public void encryptedConnectionType(int encryptedConnectionType);

	/** Sets protocol type
	 *
	 * @param protocolType is the protocol type associated with the channel
	 *
	 */
	public void protocolType(int protocolType);

	/** Sets the major version of the channel
	 *
	 * @param majorVersion is the major version associated with the channel
	 *
	 */
	public void majorVersion(int majorVersion);

	/** Sets the minor version of the channel
	 *
	 * @param minorVersion is the minor version associated with the channel
	 *
	 */
	public void minorVersion(int minorVersion);

	/** Sets ping timeout
	 *
	 * @param pingTimeout is the ping timeout associated with the channel
	 *
	 */
	public void pingTimeout(int pingTimeout);
	
	/** Specifies the max fragment size
	 * 
	 * @param maxFragmentSize specifies max fragment size
	 */
	public void maxFragmentSize(int maxFragmentSize);

	/** Specifies the maximum number of output buffers
	 * 
	 * @param maxOutputBuffers specifies maximum number of output buffers
	 * 
	 */
	public void maxOutputBuffers(int maxOutputBuffers);

	/** Specifies the guaranteed number of output buffers
	 * 
	 * @param guaranteedOutputBuffers specifies guaranteed number of output buffers
	 * 
	 */
	public void guaranteedOutputBuffers(int guaranteedOutputBuffers);

	/** Specifies the number of input buffers
	 * 
	 * @param numInputBuffers specifies number of input buffers
	 * 
 	 */
	public void numInputBuffers(int numInputBuffers);

	/** Specifies the systems Send Buffer size
	 * 
	 * @param sysSendBufSize specifies systems send Buffer size
	 * 
 	 */
	public void sysSendBufSize(int sysSendBufSize);

	/** Specifies the systems Receive Buffer size
	 * 
	 * @param sysRecvBufSize specifies systems receive Buffer size
	 * 
	 */
	public void sysRecvBufSize(int sysRecvBufSize);

	/** Specifies the compression type
	 * 
	 * @param compressionType specifies compression type
	 * 
	 */
	public void compressionType(int compressionType);

	/** Specifies the compression threshold
	 * 
	 * @param compressionThreshold specifies compression threshold
	 * 
	 */
	public void compressionThreshold(int compressionThreshold);
	
	/** Specifies the security protocol
	 * 
	 * @param securityProtocol specifies security protocol
	 * 
	 */
	public void securityProtocol(String securityProtocol);
}
