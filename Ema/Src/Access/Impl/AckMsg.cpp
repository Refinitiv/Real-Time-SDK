/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "AckMsg.h"
#include "AckMsgDecoder.h"
#include "AckMsgEncoder.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "RdmUtilities.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

const EmaString AccessDeniedString ( "AccessDenied" );
const EmaString DeniedBySourceString ( "DeniedBySource" );
const EmaString SourceDownString ( "SourceDown" );
const EmaString SourceUnknownString ( "SourceUnknown" );
const EmaString NoResourcesString ( "NoResources" );
const EmaString NoResponseString ( "NoResponse" );
const EmaString SymbolUnknownString ( "SymbolUnknown" );
const EmaString NotOpenString ( "NotOpen" );
const EmaString GatewayDownString( "GatewayDown" );
const EmaString NoneString ( "None" );
const EmaString InvalidContentString( "InvalidContent" );
EmaString TempNCString;

const EmaString& getNCodeAsString( UInt16 nCode )
{
	switch ( nCode )
	{
	case AckMsg::NoneEnum :
		return NoneString;
	case AckMsg::AccessDeniedEnum:
		return AccessDeniedString;
	case AckMsg::DeniedBySourceEnum:
		return DeniedBySourceString;
	case AckMsg::SourceDownEnum:
		return SourceDownString;
	case AckMsg::SourceUnknownEnum:
		return SourceUnknownString;
	case AckMsg::NoResourcesEnum:
		return NoResourcesString;
	case AckMsg::NoResponseEnum:
		return NoResponseString;
	case AckMsg::GatewayDownEnum :
		return GatewayDownString;
	case AckMsg::SymbolUnknownEnum:
		return SymbolUnknownString;
	case AckMsg::NotOpenEnum:
		return NotOpenString;
	case AckMsg::InvalidContentEnum :
		return InvalidContentString;
	default :
		return TempNCString.set( "Unknown NackCode value " ).append( nCode );
	}
}

AckMsg::AckMsg() :
 Msg(),
 _toString()
{
}

AckMsg::~AckMsg()
{
	if ( _pEncoder )
		g_pool._ackMsgEncoderPool.returnItem( static_cast<AckMsgEncoder*>( _pEncoder ) );

	if ( _pDecoder )
		g_pool._ackMsgDecoderPool.returnItem( static_cast<AckMsgDecoder*>( _pDecoder ) );
}

const EmaString& AckMsg::getNackCodeAsString() const
{
	return getNCodeAsString( getNackCode() );
}

AckMsg& AckMsg::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

Data::DataCode AckMsg::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum AckMsg::getDataType() const
{
	return DataType::AckMsgEnum;
}

const EmaString& AckMsg::toString() const
{
	return toString( 0 );
}

const EmaString& AckMsg::toString(  UInt64 indent ) const
{
	const AckMsgDecoder* pTempDecoder = static_cast<const AckMsgDecoder*>( _pDecoder );
		
	addIndent( _toString.clear(), indent++ ).append( "AckMsg" );
	addIndent( _toString, indent, true ).append( "streamId=\"" ).append( pTempDecoder->getStreamId() ).append( "\"" );
	addIndent( _toString, indent, true ).append( "domain=\"" ).append( rdmDomainToString( getDomainType() ) ).append( "\"" );
	addIndent( _toString, indent, true ).append( "ackId=\"" ).append( pTempDecoder->getAckId() ).append( "\"" );

	if ( pTempDecoder->hasSeqNum() )
		addIndent( _toString, indent, true ).append( "seqNum=\"" ).append( pTempDecoder->getSeqNum() ).append( "\"" );

	if ( pTempDecoder->hasNackCode() )
		addIndent( _toString, indent, true ).append( "nackCode=\"" ).append( getNCodeAsString( pTempDecoder->getNackCode() ) ).append( "\"" );

	if ( pTempDecoder->hasText() )
		addIndent( _toString, indent, true ).append( "text=\"" ).append( pTempDecoder->getText() ).append( "\"" );

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

			addIndent( _toString, indent ).append( "AttribEnd" );
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

	if ( pTempDecoder->hasPayload() )
	{
		indent++;
		addIndent( _toString, indent, true ).append( "Payload dataType=\"" ).append( getDTypeAsString( pTempDecoder->getPayloadData().getDataType() ) ).append( "\"\n" );

		indent++;
		_toString.append( pTempDecoder->getPayloadData().toString( indent ) );
		indent--;

		addIndent( _toString, indent ).append( "PayloadEnd" );
		indent--;
	}
			
	addIndent( _toString, indent, true ).append( "AckMsgEnd\n" );

	return _toString;
}

const EmaBuffer& AckMsg::getAsHex() const
{
	return static_cast<const AckMsgDecoder*>( _pDecoder )->getHexBuffer();
}

bool AckMsg::hasSeqNum() const
{
	return static_cast<const AckMsgDecoder*>(_pDecoder)->hasSeqNum();
}

bool AckMsg::hasNackCode() const
{
	return static_cast<const AckMsgDecoder*>(_pDecoder)->hasNackCode();
}

bool AckMsg::hasText() const
{
	return static_cast<const AckMsgDecoder*>(_pDecoder)->hasText();
}

bool AckMsg::hasServiceName() const
{
	return static_cast<const AckMsgDecoder*>(_pDecoder)->hasServiceName();
}

UInt32 AckMsg::getAckId() const
{
	return static_cast<const AckMsgDecoder*>(_pDecoder)->getAckId();
}

UInt8 AckMsg::getNackCode() const
{
	return static_cast<const AckMsgDecoder*>(_pDecoder)->getNackCode();
}

UInt32 AckMsg::getSeqNum() const
{
	return static_cast<const AckMsgDecoder*>(_pDecoder)->getSeqNum();
}

const EmaString& AckMsg::getText() const
{
	return static_cast<const AckMsgDecoder*>(_pDecoder)->getText();
}

const EmaString& AckMsg::getServiceName() const
{
	return static_cast<const AckMsgDecoder*>(_pDecoder)->getServiceName();
}

bool AckMsg::getPrivateStream() const
{
	return static_cast<const AckMsgDecoder*>(_pDecoder)->getPrivateStream();
}

Decoder& AckMsg::getDecoder()
{
	if ( !_pDecoder )
		setDecoder( g_pool._ackMsgDecoderPool.getItem() );

	return *_pDecoder;
}

AckMsg& AckMsg::name( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	_pEncoder->name( name );
	return *this;
}

AckMsg& AckMsg::nameType( UInt8 nameType )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	_pEncoder->nameType( nameType );
	return *this;
}

AckMsg& AckMsg::serviceName( const EmaString& serviceName )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	_pEncoder->serviceName( serviceName );
	return *this;
}

AckMsg& AckMsg::serviceId( UInt32 serviceId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	_pEncoder->serviceId( serviceId );
	return *this;
}

AckMsg& AckMsg::id( Int32 id )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	_pEncoder->identifier( id );
	return *this;
}

AckMsg& AckMsg::filter( UInt32 filter )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	_pEncoder->filter( filter );
	return *this;
}

AckMsg& AckMsg::streamId( Int32 streamId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	_pEncoder->streamId( streamId );
	return *this;
}

AckMsg& AckMsg::domainType( UInt16 domainType )
{
	if ( domainType > 255 )
	{
		EmaString temp( "Passed in DomainType is out of range." );
		throwDtuException( domainType, temp );
		return *this;
	}

	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	_pEncoder->domainType( (UInt8)domainType );
	return *this;
}

AckMsg& AckMsg::seqNum( UInt32 seqNum )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	static_cast<AckMsgEncoder*>(_pEncoder)->seqNum( seqNum );
	return *this;
}

AckMsg& AckMsg::ackId( UInt32 postId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	static_cast<AckMsgEncoder*>(_pEncoder)->ackId( postId );
	return *this;
}

AckMsg& AckMsg::nackCode( UInt8 nackCode )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	static_cast<AckMsgEncoder*>(_pEncoder)->nackCode( nackCode );
	return *this;
}

AckMsg& AckMsg::text( const EmaString& text )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	static_cast<AckMsgEncoder*>(_pEncoder)->text( text );
	return *this;
}

AckMsg& AckMsg::attrib( const ComplexType& data )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	_pEncoder->attrib( data );
	return *this;
}

AckMsg& AckMsg::payload( const ComplexType& data )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	_pEncoder->payload( data );
	return *this;
}

AckMsg& AckMsg::extendedHeader( const EmaBuffer& Buffer )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	static_cast<AckMsgEncoder*>(_pEncoder)->extendedHeader( Buffer );
	return *this;
}

AckMsg& AckMsg::privateStream( bool privateStream )
{
	if ( !_pEncoder )
		_pEncoder = g_pool._ackMsgEncoderPool.getItem();

	static_cast<AckMsgEncoder*>(_pEncoder)->privateStream( privateStream );
	return *this;
}
