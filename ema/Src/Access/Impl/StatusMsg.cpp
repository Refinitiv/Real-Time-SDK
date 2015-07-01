/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "StatusMsg.h"
#include "StatusMsgDecoder.h"
#include "StatusMsgEncoder.h"
#include "EmaBufferInt.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "RdmUtilities.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

StatusMsg::StatusMsg() :
 Msg(),
 _toString()
{
}

StatusMsg::~StatusMsg()
{
	if ( _pEncoder )
		g_pool._statusMsgEncoderPool.returnItem( static_cast<StatusMsgEncoder*>( _pEncoder ) );

	if ( _pDecoder )
		g_pool._statusMsgDecoderPool.returnItem( static_cast<StatusMsgDecoder*>( _pDecoder ) );
}

StatusMsg& StatusMsg::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

Data::DataCode StatusMsg::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum StatusMsg::getDataType() const
{
	return DataType::StatusMsgEnum;
}

const EmaString& StatusMsg::toString() const
{
	return toString( 0 );
}

const EmaString& StatusMsg::toString( UInt64 indent ) const
{
	const StatusMsgDecoder* pTempDecoder = static_cast<const StatusMsgDecoder*>( _pDecoder );

	addIndent( _toString.clear(), indent++ ).append( "StatusMsg" );
	addIndent( _toString, indent, true ).append( "streamId=\"" ).append( pTempDecoder->getStreamId() ).append( "\"" );
	addIndent( _toString, indent, true ).append( "domain=\"" ).append( rdmDomainToString( getDomainType() ) ).append( "\"" );			

	if ( pTempDecoder->hasState() )
		addIndent( _toString, indent, true ).append( "state=\"" ).append( pTempDecoder->getState().toString() ).append( "\"" );

	if ( pTempDecoder->hasItemGroup() )
	{
		EmaString temp;
		hexToString( temp, pTempDecoder->getItemGroup() );
		addIndent( _toString, indent, true ).append( "itemGroup=\"" ).append( temp ).append( "\"" );
	}

	if ( pTempDecoder->hasPermissionData() )
	{
		EmaString temp;
		hexToString( temp, pTempDecoder->getPermissionData() );
		addIndent( _toString, indent, true ).append( "permissionData=\"" ).append( temp ).append( "\"" );
	}

	indent--;
	if ( pTempDecoder->hasMsgKey() )
	{
		indent++;
		if ( pTempDecoder->hasName() )
			addIndent( _toString, indent, true ).append( "name=\"" ).append( pTempDecoder->getName() ).append( "\"" );

		if ( pTempDecoder->hasNameType() )
			addIndent( _toString, indent, true ).append( "nameType=\"" ).append( pTempDecoder->getNameType() ).append( "\"" );

		if ( pTempDecoder->hasServiceId() )
			addIndent( _toString, indent, true ).append( "serviceId=\"" ).append( pTempDecoder->getServiceId() ).append( "\"" );

		if ( pTempDecoder->hasServiceName() )
			addIndent( _toString, indent, true ).append( "serviceName=\"" ).append( pTempDecoder->getServiceName() ).append( "\"" );

		if ( pTempDecoder->hasFilter() )
			addIndent( _toString, indent, true ).append( "filter=\"" ).append( pTempDecoder->getFilter() ).append( "\"" );

		if ( pTempDecoder->hasId() )
			addIndent( _toString, indent, true ).append( "id=\"" ).append( pTempDecoder->getId() ).append( "\"" );

		indent--;

		if ( pTempDecoder->hasAttrib() )
		{
			indent++;
			addIndent( _toString, indent, true ).append( "Attrib dataType=\"" ).append( getDTypeAsString( pTempDecoder->getAttribData().getDataType() ) ).append( "\"\n" );

			indent++;
			_toString.append( pTempDecoder->getAttribData().toString( indent ) );
			indent--;

			addIndent( _toString, indent, true ).append( "AttribEnd" );
			indent--;
		}
	}
		
	if ( pTempDecoder->hasExtendedHeader() )
	{
		indent++;
		addIndent( _toString, indent, true ).append( "ExtendedHeader\n" );

		indent++;
		addIndent( _toString, indent );
		hexToString( _toString, pTempDecoder->getExtendedHeader() );
		indent--;

		addIndent( _toString, indent, true ).append( "ExtendedHeaderEnd" );
		indent--;
	}

	if ( _pDecoder->hasPayload() )
	{
		indent++;
		addIndent( _toString, indent, true ).append( "Payload dataType=\"" ).append( getDTypeAsString( pTempDecoder->getPayloadData().getDataType() ) ).append( "\"\n" );

		indent++;
		_toString.append( pTempDecoder->getPayloadData().toString( indent ) );
		indent--;

		addIndent( _toString, indent, true ).append( "PayloadEnd" );
		indent--;
	}
			
	addIndent( _toString, indent, true ).append( "StatusMsgEnd\n" );

	return _toString;
}

const EmaBuffer& StatusMsg::getAsHex() const
{
	return static_cast<const StatusMsgDecoder*>( _pDecoder )->getHexBuffer();
}

bool StatusMsg::hasItemGroup() const
{
	return static_cast<const StatusMsgDecoder*>(_pDecoder)->hasItemGroup();
}

bool StatusMsg::hasState() const
{
	return static_cast<const StatusMsgDecoder*>(_pDecoder)->hasState();
}

bool StatusMsg::hasPermissionData() const
{
	return static_cast<const StatusMsgDecoder*>(_pDecoder)->hasPermissionData();
}

bool StatusMsg::hasPublisherId() const
{
	return static_cast<const StatusMsgDecoder*>(_pDecoder)->hasPublisherId();
}

bool StatusMsg::hasServiceName() const
{
	return static_cast<const StatusMsgDecoder*>(_pDecoder)->hasServiceName();
}

const OmmState& StatusMsg::getState() const
{
	return static_cast<const StatusMsgDecoder*>(_pDecoder)->getState();
}

const EmaBuffer& StatusMsg::getItemGroup() const
{
	return static_cast<const StatusMsgDecoder*>(_pDecoder)->getItemGroup();
}

const EmaBuffer& StatusMsg::getPermissionData() const
{
	return static_cast<const StatusMsgDecoder*>(_pDecoder)->getPermissionData();
}

UInt32 StatusMsg::getPublisherIdUserId() const
{
	return static_cast<const StatusMsgDecoder*>(_pDecoder)->getPublisherIdUserId();
}

UInt32 StatusMsg::getPublisherIdUserAddress() const
{
	return static_cast<const StatusMsgDecoder*>(_pDecoder)->getPublisherIdUserAddress();
}

bool StatusMsg::getClearCache() const
{
	return static_cast<const StatusMsgDecoder*>(_pDecoder)->getClearCache();
}

bool StatusMsg::getPrivateStream() const
{
	return static_cast<const StatusMsgDecoder*>(_pDecoder)->getPrivateStream();
}

const EmaString& StatusMsg::getServiceName() const
{
	return static_cast<const StatusMsgDecoder*>(_pDecoder)->getServiceName();
}

Decoder& StatusMsg::getDecoder()
{
	if ( !_pDecoder )
		Msg::setDecoder( g_pool._statusMsgDecoderPool.getItem() );

	return *_pDecoder;
}

StatusMsg& StatusMsg::streamId( Int32 streamId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	_pEncoder->streamId( streamId );
	return *this;
}

StatusMsg& StatusMsg::domainType( UInt16 domainType )
{
	if ( domainType > 255 )
	{
		EmaString temp( "Passed in DomainType is out of range." );
		throwDtuException( domainType, temp );
		return *this;
	}

	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	_pEncoder->domainType( (UInt8)domainType );
	return *this;
}

StatusMsg& StatusMsg::name( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	_pEncoder->name( name );
	return *this;
}

StatusMsg& StatusMsg::nameType( UInt8 nameType )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	_pEncoder->nameType( nameType );
	return *this;
}

StatusMsg& StatusMsg::serviceName( const EmaString& serviceName )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	_pEncoder->serviceName( serviceName );
	return *this;
}

StatusMsg& StatusMsg::serviceId( UInt32 serviceId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	_pEncoder->serviceId( serviceId );
	return *this;
}

StatusMsg& StatusMsg::id( Int32 id )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	_pEncoder->identifier( id );
	return *this;
}

StatusMsg& StatusMsg::filter( UInt32 filter )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	_pEncoder->filter( filter );
	return *this;
}

StatusMsg& StatusMsg::state( OmmState::StreamState streamState, OmmState::DataState dataState,
						UInt8 statusCode, const EmaString& statusText )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	static_cast<StatusMsgEncoder*>(_pEncoder)->state( streamState, dataState, statusCode, statusText );
	return *this;
}

StatusMsg& StatusMsg::itemGroup( const EmaBuffer& itemGroup )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	static_cast<StatusMsgEncoder*>(_pEncoder)->itemGroup( itemGroup );
	return *this;
}

StatusMsg& StatusMsg::permissionData( const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	static_cast<StatusMsgEncoder*>(_pEncoder)->permissionData( permissionData );
	return *this;
}

StatusMsg& StatusMsg::publisherId( UInt32 userId, UInt32 userAddress )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	static_cast<StatusMsgEncoder*>(_pEncoder)->publisherId( userId, userAddress );
	return *this;
}

StatusMsg& StatusMsg::attrib( const ComplexType& data )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	_pEncoder->attrib( data );
	return *this;
}

StatusMsg& StatusMsg::payload( const ComplexType& data )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	_pEncoder->payload( data );
	return *this;
}

StatusMsg& StatusMsg::extendedHeader( const EmaBuffer& buffer )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	static_cast<StatusMsgEncoder*>(_pEncoder)->extendedHeader( buffer );
	return *this;
}

StatusMsg& StatusMsg::clearCache( bool clearCache )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	static_cast<StatusMsgEncoder*>(_pEncoder)->clearCache( clearCache );
	return *this;
}

StatusMsg& StatusMsg::privateStream( bool privateStream )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._statusMsgEncoderPool.getItem();

	static_cast<StatusMsgEncoder*>(_pEncoder)->privateStream( privateStream );
	return *this;
}
