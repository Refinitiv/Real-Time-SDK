/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ChannelInformation_h
#define __refinitiv_ema_access_ChannelInformation_h

/**
   @class refinitiv::ema::access::ChannelInformation ChannelInformation.h "Access/Include/ChannelInformation.h"
   @brief ChannelInfo provides information on application channel(s).

   For IProvider applications, this channel information is about channels used by
   clients to connect to the IProvider application.

   For Consumer and NiProvider applications, this channel information is about the
   outbound channel (e.g., the channel used by a Consumer application to connect to
   an ADS) used to connect for receiving data (Consumer) or sending data (NiProvider).

   Examples of ChannelInformation usage are found in the examples:
   (Consumer) Consumer/100_Series/170__MarketPrice__ChannelInfo
   (IProvider) IProvider/100_Series/170__MarketPrice__ConnectedClientInfo
   (NiProvider) NiProvider/100_Series/170__MarketPrice__ChannelInfo
*/   

#include "Access/Include/EmaString.h"
#include "PreferredHostInfo.h"

namespace refinitiv {

namespace ema {

namespace access {

class EmaString;
class ChannelInfoImpl;

class EMA_ACCESS_API ChannelInformation
{
public:

  /** @enum ChannelState
	  An enumeration representing channel states.
  */
  enum ChannelState
  {
	ClosedEnum = -1,		/*!< (-1) Channel has been CLOSED. */
	InactiveEnum = 0,		/*!< (0) Channel is in the INACTIVE state. */
	InitializingEnum = 1,	/*!< (1) Channel is in the INITIALIZING state. */
	ActiveEnum = 2,			/*!< (2) Channel is in the ACTIVE state. */
  };

  /** @enum ConnectionType
	  An enumeration representing connection types.
  */
  enum ConnectionType
  {
	Unidentified = -1,         /*!< (-1) Unidentified */
	SocketEnum = 0,           /*!< (0) Channel is a standard TCP socket connection type */
	EncryptedEnum = 1,        /*!< (1) Channel is encrypted */
	HttpEnum = 2,             /*!< (2) Channel is an HTTP connection based tunneling type */
	Unidir_ShmemEnum = 3,     /*!< (3) Channel is using a shared memory connection */
	Reliable_McastEnum = 4,   /*!< (4) Channel is a reliable multicast based connection.
								This can be on a unified/mesh network where send and receive
								networks are the same or a segmented network where send and
								receive networks are different */
	Ext_Line_SocketEnum = 5,  /*!< (5) Channel is using an extended line socket transport */
	Seq_McastEnum = 6,        /*!< (6) Channel is an unreliable, sequenced multicast connection
								for reading from a LSEG Real-Time Direct Feed system. This is a
								client-only, read-only transport. This transport is supported on
								Linux only. */
	WebSocketEnum = 7,        /*!< (7) Channel is a WebSocket connection based tunneling type */
  };

  /** @enum ProtocolType
	  An enumeration representing protocol types.
  */
  enum ProtocolType
  {
	UnknownEnum = -1,		/*!< (-1) Unknown wireformat protocol */
	RwfEnum = 0,			/*!< (0) LSEG wireformat protocol */
	RsslJsonEnum = 2		/*!< (2) Rssl JSON protocol */
  };

  /** @enum CompressionType
	  An enumeration representing compression types.
  */
  enum CompressionType
  {
	  NoneEnum = 0x00,		 /*!< (0) No compression will be negotiated. */
	  ZLIBEnum = 0x01,		 /*!< (1) Will attempt to use Zlib compression. */
	  LZ4Enum = 0x02		 /*!< (2) Will attempt to use LZ4 compression */
  };

  ///@name Constructor
  //@{
  /** Constructs ChannelInformation.
   */
  ChannelInformation();
  //@}
	

  ///@name Constructor
  //@{
  /** Constructs ChannelInformation.
	  @param[in] connectedComponentInfo
	  @param[in] hostname
	  @param[in] ipAddress
	  @param[in] channelState
	  @param[in] connectionType
	  @param[in] protocolType
	  @param[in] majorVersion
	  @param[in] minorVersion
	  @param[in] pingTimeout
   */
  ChannelInformation( const EmaString& connectedComponentInfo,
					  const EmaString& hostname, const EmaString& ipAddress,
					  const ChannelState channelState, const ConnectionType connectionType,
					  const ProtocolType protocolType, const UInt32 majorVersion,
					  const UInt32 minorVersion, const UInt32 pingTimeout );
  //@}

  ///@name Destructor
  //@{
  /** Destructor.
   */
  virtual ~ChannelInformation();
  //@}

  ///@name Accessors
  //@{
  /** Clears the ChannelInformation
	  \remark invoking clear() resets all member variables to their default values
  */
  void clear();

  /** Gets the connected component info as a string
	  @return string representation of this object's connected component info
  */
  const EmaString& getConnectedComponentInfo() const { return _connectedComponentInfo; }

  /** Gets the hostname as a string
	  @return string representation of this object's hostname
	  \remark see discussion of this class at the top of this file for more information
  */
  const EmaString& getHostname() const { return _hostname; }

  /** Gets the port number that was used to connect to the server ADS/ADH (Consumer, NiProvider).
	  Valid for SOCKET connection type
	  @return port number
  */
  const UInt16 port() const { return _port; }

  /** Gets the IP address of the connected client
	  @return string representation of this object's IP address
	  \remark see discussion of this class at the top of this file for more information.
	  \remark this is set only for IProvider applications
  */
  const EmaString& getIPaddress() const { return _ipAddress; }

  /** Gets the channel state as a ChannelState enum
	  @return ChannelState enum representation of this object's channel state
  */
  ChannelState getChannelState() const { return _channelState; }

  /** Gets the connection type as a ConnectionType enum
	  @return ConnectionType enum representation of this object's connection type
  */
  ConnectionType getConnectionType() const { return _connectionType; }

  /** Gets the protocol type as a ProtocolType enum
	  @return ProtocolType enum representation of this object's protocol type
  */
  ProtocolType getProtocolType() const { return _protocolType; }

  /** Gets the major version
	  @return UInt32 representation of this object's major version
  */
  UInt32 getMajorVersion() const { return _majorVersion; }

  /** Gets the minor version
	  @return UInt32 representation of this object's minor version
  */
  UInt32 getMinorVersion() const { return _minorVersion; }

  /** Gets the ping timeout
	  @return UInt32 representation of this object's ping timeout
  */
  UInt32 getPingTimeout() const { return _pingTimeout; }

  /** Gets the max fragment size
	  @return The max fragment size before fragmentation and reassembly is necessary.
  */
  UInt32 getMaxFragmentSize() const { return _maxFragmentSize;  }

  /** Gets the maximum number of output buffers
	  @return The maximum number of output buffers available to the channel.
  */
  UInt32 getMaxOutputBuffers() const { return _maxOutputBuffers; }

  /** Gets the guaranteed number of output buffers
	  @return The guaranteed number of output buffers available to the channel.
  */
  UInt32 getGuaranteedOutputBuffers() const { return _guaranteedOutputBuffers; }

  /** Gets the number of input buffers
	  @return The number of input buffers available to the channel.
  */
  UInt32 getNumInputBuffers() const { return _numInputBuffers; }

  /** Gets the systems send Buffer size
	  @return The systems send buffer size respective to the transport type being used.
  */
  UInt32 getSysSendBufSize() const { return _sysSendBufSize; }

  /** Gets the systems receive Buffer size
	  @return The systems receive buffer size respective to the transport type being used.
  */
  UInt32 getSysRecvBufSize() const { return _sysRecvBufSize; }

  /** Gets the compression type
	  @return The type of compression being used, if it is enabled.
  */
  CompressionType getCompressionType() const { return _compressionType; }

  /** Gets the compression threshold
	  @return The compression threshold for compressing any message lager than this when compression is enabled.
  */
  UInt32 getCompressionThreshold() const { return _compressionThreshold; }

  /** Gets the encryption protocol
	  @return The current encryption protocol being used.
  */
  UInt64 getEncryptionProtocol() const { return _encryptionProtocol; }


	/** Gets configured name of the connection
	@return The name of the connection, as defined by the configuration.
	*/	
	EmaString getName() const { return _name; }

	/** Gets session name of the connection
	@return The name of the session that contains this connection
	*/
	EmaString getSessionName() const { return _sessionName; }

  /** Gets the preferred host information
	@return Preferred host information for current channel 
  */
  const PreferredHostInfo& getPreferredHostInfo() const { return _preferredHostInfo; }

  /** Gets a string representation of the class instance
	  @return string representation of the class instance.
  */
  const EmaString& toString() const;
  
  /** Operator const char* overload
	  \remark invokes toString.c_str()
	  @return a NULL terminated character string representation of this object
  */
  operator const char* () const;
  
  //@}

  ///@name Operations
  //@{

	/** Specifies name
		@param[in] name specifies name as a string
		@return reference to this object
	*/
	ChannelInformation& name(const EmaString& name);

	/** Specifies session name
		@param[in] sessionName specifies name as a string
		@return reference to this object
	*/
	ChannelInformation& sessionName(const EmaString& sessionName);

  /** Specifies hostname
	  @param[in] hostname specifies hostname as a string
	  @return reference to this object
  */
  ChannelInformation& hostname(const EmaString& hostname);

  /** Specifies port number that was used to connect to the server (Consumer, NiProvider)
	  @param[in] port specifies port number
	  @return reference to this object
   */
  ChannelInformation& port(const UInt16 port);

  /** Specifies the IP address of the connected client
	  @param[in] ipAddress specifies IP address as a string
	  @return reference to this object
  */
  ChannelInformation& ipAddress(const EmaString& ipAddress);

  /** Specifies connected component info
	  @param[in] connectedComponentInfo specifies connected component info as a string
	  @return reference to this object
  */
  ChannelInformation& connectedComponentInfo(const EmaString& connectedComponentInfo);

  /** Specifies channel state
	  @param[in] channelState specifies channel state as a ChannelState enum
	  @return reference to this object
  */
  ChannelInformation& channelState(ChannelState channelState);

  /** Specifies connection type
	  @param[in] connectionType specifies connection type as a ConnectionType enum
	  @return reference to this object
  */
  ChannelInformation& connectionType(ConnectionType connectionType);

  /** Specifies protocol type
	  @param[in] protocolType specifies protocol type as a ProtocolType enum
	  @return reference to this object
  */
  ChannelInformation& protocolType(ProtocolType protocolType);

  /** Specifies major version
	  @param[in] majorVersion specifies majorVersion
	  @return reference to this object
  */
  ChannelInformation& majorVersion(UInt32 majorVersion);

  /** Specifies minor version
	  @param[in] minorVersion specifies minorVersion
	  @return reference to this object
  */
  ChannelInformation& minorVersion(UInt32 minorVersion);

  /** Specifies ping timeout
	  @param[in] pingTimeout specifies ping timeout
	  @return reference to this object
  */
  ChannelInformation& pingTimeout(UInt32 pingTimeout);

  /** Specifies the max fragment size
	  @param[in] maxFragmentSize specifies max fragment size
	  @return reference to this object
  */
  ChannelInformation& maxFragmentSize(UInt32 maxFragmentSize);

  /** Specifies the maximum number of output buffers
	  @param[in] maxOutputBuffers specifies maximum number of output buffers
	  @return reference to this object
  */
  ChannelInformation& maxOutputBuffers(UInt32 maxOutputBuffers);

  /** Specifies the guaranteed number of output buffers
	  @param[in] guaranteedOutputBuffers specifies guaranteed number of output buffers
	  @return reference to this object
  */
  ChannelInformation& guaranteedOutputBuffers(UInt32 guaranteedOutputBuffers);

  /** Specifies the number of input buffers
	  @param[in] guaranteedOutputBuffers specifies number of input buffers
	  @return reference to this object
  */
  ChannelInformation& numInputBuffers(UInt32 numInputBuffers);

  /** Specifies the systems Send Buffer size
	  @param[in] sysSendBufSize specifies systems send Buffer size
	  @return reference to this object
  */
  ChannelInformation& sysSendBufSize(UInt32 sysSendBufSize);

  /** Specifies the systems Receive Buffer size
	 @param[in] sysRecvBufSize specifies systems receive Buffer size
	 @return reference to this object
  */
  ChannelInformation& sysRecvBufSize(UInt32 sysRecvBufSize);

  /** Specifies the compression type
	 @param[in] compressionType specifies compression type
	 @return reference to this object
  */
  ChannelInformation& compressionType(UInt32 compressionType);

  /** Specifies the compression threshold
	 @param[in] compressionThreshold specifies compression threshold
	 @return reference to this object
  */
  ChannelInformation& compressionThreshold(UInt32 compressionThreshold);

  /** Specifies the encryption protocol
	 @param[in] encryptionProtocol specifies the encryption protocol
	 @return reference to this object
  */
  ChannelInformation& encryptionProtocol(UInt64 encryptionProtocol);
  //@}

private:
  ChannelState _channelState;
  ConnectionType _connectionType;
  EmaString _name;
  EmaString _sessionName;
  EmaString _hostname;
  EmaString _ipAddress;
  UInt16 _port;
  EmaString _connectedComponentInfo;
  ProtocolType _protocolType;
  UInt32 _majorVersion;
  UInt32 _minorVersion;
  UInt32 _pingTimeout;
  UInt32 _maxFragmentSize;
  UInt32 _maxOutputBuffers;
  UInt32 _guaranteedOutputBuffers;
  UInt32 _numInputBuffers;
  UInt32 _sysSendBufSize;
  UInt32 _sysRecvBufSize;
  CompressionType _compressionType;
  UInt32 _compressionThreshold;
  UInt64 _encryptionProtocol;
  PreferredHostInfo _preferredHostInfo;
  mutable EmaString _toString;

  ChannelInformation& preferredHostInfo(void* preferredHostInfo, const void* channel);

  friend class ChannelInfoImpl;
};

}

}

}

#endif //__refinitiv_ema_access_ChannelInformation_h

