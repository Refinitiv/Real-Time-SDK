/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2018. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ChannelInfoImpl.h"
#include "ChannelInformation.h"

using namespace thomsonreuters::ema::access;

ChannelInformation::ChannelInformation()
{
  clear();
}

ChannelInformation::ChannelInformation( const EmaString& connectedComponentInfo,
						  const EmaString& hostname, const EmaString& ipAddress,
						  const ChannelState channelState, const ConnectionType connectionType,
						  const ProtocolType protocolType, const UInt32 majorVersion,
						  const UInt32 minorVersion, const UInt32 pingTimeout) :
  _connectedComponentInfo( connectedComponentInfo ),  _hostname( hostname ), _ipAddress( ipAddress ),
  _channelState( channelState ), _connectionType( connectionType ), _protocolType( protocolType ),
  _majorVersion( majorVersion ), _minorVersion( minorVersion ), _pingTimeout( pingTimeout ) {}

ChannelInformation::~ChannelInformation() {}

const EmaString& ChannelInformation::toString() const {
  _toString.clear().append( "hostname: " ).append( _hostname )
	.append( "\n\tIP address: " ).append( _ipAddress )
	.append( "\n\tconnected component info: " ).append( _connectedComponentInfo )
	.append( "\n\tchannel state: " );
  switch( _channelState ) {
	case ClosedEnum: _toString.append( "closed" ); break;
	case InactiveEnum: _toString.append( "inactive" ); break;
	case InitializingEnum: _toString.append( "initializing" ); break;
	case ActiveEnum: _toString.append( "active" ); break;
	default: _toString.append( "unknown: " + _channelState ); break;
  }
  _toString.append( "\n\tconnection type: " );
  switch( _connectionType ) {
	case ConnectionType::Unidentified: _toString.append("unknown"); break;
	case SocketEnum: _toString.append( "socket" ); break;
	case EncryptedEnum: _toString.append( "encrypted" ); break;
	case HttpEnum: _toString.append( "http" ); break;
	case Unidir_ShmemEnum: _toString.append( "shmem" ); break;
	case Reliable_McastEnum: _toString.append( "reliableMCast" ); break;
	case Ext_Line_SocketEnum: _toString.append( "extended line socket" ); break;
	case Seq_McastEnum: _toString.append( "seqMCast" ); break;
	default: _toString.append( "unknown: " + _connectionType ); break;
  }
  _toString.append( "\n\tprotocol type: " );
  switch( _protocolType ) {
	case RwfEnum: _toString.append( "Reuters wire format" ); break;
	case UnknownEnum:
	default:
	_toString.append( "unknown wire format: " + _protocolType ); break;
  }
  _toString.append( "\n\tmajor version: " ).append( _majorVersion )
	.append( "\n\tminor version: " ).append( _minorVersion )
	.append( "\n\tping timeout: " ).append( _pingTimeout );

  return _toString;
}

ChannelInformation::operator const char*() const
{
    return toString().c_str();
}

void ChannelInformation::clear() {
  _channelState = UnknownChannelStateEnum;
  _connectionType = ConnectionType::Unidentified;
  _hostname.clear();
  _ipAddress.clear();
  _connectedComponentInfo.clear();
  _protocolType = UnknownEnum;
  _majorVersion = _minorVersion = _pingTimeout = 0;
  _toString.clear();
}

ChannelInformation& ChannelInformation::hostname(const EmaString& value) {
  _hostname = value;
  return *this;
}
ChannelInformation& ChannelInformation::ipAddress(const EmaString& value) {
  _ipAddress = value;
  return *this;
}
ChannelInformation& ChannelInformation::connectedComponentInfo(const EmaString& value) {
  _connectedComponentInfo = value;
  return *this;
}
ChannelInformation& ChannelInformation::channelState(ChannelState value) {
  _channelState = value;
  return *this;
}
ChannelInformation& ChannelInformation::connectionType(ConnectionType value) {
  _connectionType = value;
  return *this;
}
ChannelInformation& ChannelInformation::protocolType(ProtocolType value) {
  _protocolType = value;
  return *this;
}
ChannelInformation& ChannelInformation::majorVersion(UInt32 value) {
  _majorVersion = value;
  return *this;
}
ChannelInformation& ChannelInformation::minorVersion(UInt32 value) {
  _minorVersion = value;
  return *this;
}
ChannelInformation& ChannelInformation::pingTimeout(UInt32 value) {
  _pingTimeout = value;
  return *this;
}

void thomsonreuters::ema::access::getChannelInformationImpl(const RsslReactorChannel* rsslReactorChannel,
															OmmCommonImpl::ImplementationType implType,
															ChannelInformation& ci) 
{
  RsslChannel* rsslChannel;  
  ci.clear();
  if (rsslReactorChannel == 0 || (rsslChannel = rsslReactorChannel->pRsslChannel) == 0)
	return;

  if (implType == OmmCommonImpl::IProviderEnum) {
	ci.hostname(rsslChannel->clientHostname).ipAddress(rsslChannel->clientIP);
  }
  else {
	ci.hostname(rsslChannel->hostname);
	if (implType == OmmCommonImpl::NiProviderEnum)
	  ci.ipAddress("not available for OmmNiProvider connections");
	else
	  ci.ipAddress("not available for OmmConsumer connections");
  }
  return getChannelInformation(rsslReactorChannel, rsslChannel, ci);
}

/* ci has been cleared and calling function has verified that channel arguments are non-null.
 * Calling function has also updated hostname and ipAddress
 */
void thomsonreuters::ema::access::getChannelInformation(const RsslReactorChannel* rsslReactorChannel,
												 const RsslChannel* rsslChannel,
												 ChannelInformation& ci) {
  // create channel info
  EmaString componentInfo;
  RsslErrorInfo rsslErrorInfo;
  RsslReactorChannelInfo rsslReactorChannelInfo;

  // if channel is closed, rsslReactorGetChannelInfo does not succeed
  if (rsslReactorGetChannelInfo(const_cast<RsslReactorChannel*>(rsslReactorChannel),
								&rsslReactorChannelInfo, &rsslErrorInfo) == RSSL_RET_SUCCESS) {
	for (unsigned int i = 0; i < rsslReactorChannelInfo.rsslChannelInfo.componentInfoCount; ++i) {
	  componentInfo.append(rsslReactorChannelInfo.rsslChannelInfo.componentInfo[i]->componentVersion.data);
	  if (i < (rsslReactorChannelInfo.rsslChannelInfo.componentInfoCount - 1))
		componentInfo.append(", ");
	}
  }
  else
	componentInfo.append("unavailable");

  ci.connectedComponentInfo(componentInfo)
	.channelState(static_cast<ChannelInformation::ChannelState>(rsslChannel->state))
	.connectionType(static_cast<ChannelInformation::ConnectionType>(rsslChannel->connectionType))
	.protocolType(static_cast<ChannelInformation::ProtocolType>(rsslChannel->protocolType))
	.majorVersion(rsslChannel->majorVersion)
	.minorVersion(rsslChannel->minorVersion)
	.pingTimeout(rsslChannel->pingTimeout);
}
