/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ChannelInformation_h
#define __thomsonreuters_ema_access_ChannelInformation_h

/**
   @class thomsonreuters::ema::access::ChannelInformation ChannelInformation.h "Access/Include/ChannelInformation.h"
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

namespace thomsonreuters {

namespace ema {

namespace access {

class EmaString;

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
	UnknownChannelStateEnum = 99        /*!< (99) Channel state is unknown */
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
	Seq_McastEnum = 6         /*!< (6) Channel is an unreliable, sequenced multicast connection
								for reading from an Elektron Direct Feed system. This is a
								client-only, read-only transport. This transport is supported on
								Linux only. */
  };

  /** @enum ProtocolType
	  An enumeration representing protocol types.
  */
  enum ProtocolType
  {
	UnknownEnum = -1,		/*!< (-1) Unknown wireformat protocol */
	RwfEnum = 0				/*!< (0) Reuters wireformat protocol */
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
					  const UInt32 minorVersion, const UInt32 pingTimeout);
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

  /** Gets a string representation of the class instance.
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
  /** Specifies hostname
	  @param[in] hostname specifies hostname as a string
	  @return reference to this object
  */
  ChannelInformation& hostname(const EmaString& hostname);

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
  //@}

private:
  ChannelState _channelState;
  ConnectionType _connectionType;
  EmaString _hostname;
  EmaString _ipAddress;
  EmaString _connectedComponentInfo;
  ProtocolType _protocolType;
  UInt32 _majorVersion;
  UInt32 _minorVersion;
  UInt32 _pingTimeout;
  mutable EmaString _toString;
};

}

}

}

#endif //__thomsonreuters_ema_access_ChannelInformation_h

