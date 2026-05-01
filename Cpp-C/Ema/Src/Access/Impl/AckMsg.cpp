/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "AckMsg.h"
#include "AckMsgImpl.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "RdmUtilities.h"
#include "StaticDecoder.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

const EmaString AccessDeniedString( "AccessDenied" );
const EmaString DeniedBySourceString( "DeniedBySource" );
const EmaString SourceDownString( "SourceDown" );
const EmaString SourceUnknownString( "SourceUnknown" );
const EmaString NoResourcesString( "NoResources" );
const EmaString NoResponseString( "NoResponse" );
const EmaString SymbolUnknownString( "SymbolUnknown" );
const EmaString NotOpenString( "NotOpen" );
const EmaString GatewayDownString( "GatewayDown" );
const EmaString NoneString( "None" );
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
	setImpl( g_pool.getAckMsgImplItem() );
}

AckMsg::AckMsg(UInt32 size) :
 AckMsg()
{
	impl()->Arena().reserve(size);
}

AckMsg::AckMsg(const AckMsg& other) :
 Msg(),
 _toString()
{
	AckMsgImpl* pAckImpl = g_pool.getAckMsgImplItem();
	*pAckImpl = *other.impl();

	setImpl( pAckImpl );
}

AckMsg::AckMsg(AckMsg&& other) noexcept :
 Msg(),
 _toString()
{
	setImpl( other.impl() );

	other.setImpl( nullptr );
}

AckMsg& AckMsg::operator=(const AckMsg& other)
{
	if (this == &other)
		return *this;

	*impl() = *other.impl();

	return *this;
}

AckMsg& AckMsg::operator=( AckMsg&& other ) noexcept
{
	if (this == &other)
		return *this;

	if ( impl() )
	{
		impl()->deallocateBuffers<AckMsgImpl>();
		g_pool.returnItem( impl() );
	}

	setImpl( other.impl() );
	other.setImpl( nullptr );

	return *this;
}

AckMsg::~AckMsg()
{
	if ( impl() )
	{
		impl()->deallocateBuffers<AckMsgImpl>();
		g_pool.returnItem( impl() );

		setImpl( nullptr );
	}
}

const EmaString& AckMsg::getNackCodeAsString() const
{
	return getNCodeAsString( getNackCode() );
}

AckMsg& AckMsg::clear()
{
	EMA_ASSERT( impl() != nullptr, "Private implementation can't be missing");

	impl()->clear();

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

const EmaString& AckMsg::toString( const refinitiv::ema::rdm::DataDictionary& dictionary ) const
{
	if (!dictionary.isEnumTypeDefLoaded() || !dictionary.isFieldDictionaryLoaded())
	{
		return _toString.clear().append("\nDictionary is not loaded.\n");
	}

	AckMsg ackMsg;

	RsslBuffer& rsslBuffer = getEncoder().getEncodedBuffer();

	StaticDecoder::setRsslData( &ackMsg, &rsslBuffer,
								RSSL_DT_MSG, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION,
								dictionary._pImpl->rsslDataDictionary() );

	_toString.clear().append(ackMsg.toString());

	return _toString;
}

const EmaString& AckMsg::toString( UInt64 indent ) const
{
	_toString.clear();

	addIndent( _toString, indent++ ).append( "AckMsg" );
	addIndent( _toString, indent, true ).append( "streamId=\"" ).append( getStreamId() ).append( "\"" );
	addIndent( _toString, indent, true ).append( "domain=\"" ).append( rdmDomainToString( getDomainType() ) ).append( "\"" );
	addIndent( _toString, indent, true ).append( "ackId=\"" ).append( getAckId() ).append( "\"" );

	if ( getPrivateStream() )
		addIndent( _toString, indent, true ).append( "PrivateStream" );

	if ( hasSeqNum() )
		addIndent( _toString, indent, true ).append( "seqNum=\"" ).append( getSeqNum() ).append( "\"" );

	if ( hasNackCode() )
		addIndent( _toString, indent, true ).append( "nackCode=\"" ).append( getNCodeAsString( getNackCode() ) ).append( "\"" );

	if ( hasText() )
		addIndent( _toString, indent, true ).append( "text=\"" ).append( getText() ).append( "\"" );

	indent--;
	if ( hasMsgKey() )
	{
		indent++;
		if ( hasName() )
			addIndent( _toString, indent, true ).append( "name=\"" ).append( getName() ).append( "\"" );

		if ( hasNameType() )
			addIndent( _toString, indent, true ).append( "nameType=\"" ).append( getNameType() ).append( "\"" );

		if ( hasServiceId() )
			addIndent( _toString, indent, true ).append( "serviceId=\"" ).append( getServiceId() ).append( "\"" );

		if ( hasServiceName() )
			addIndent( _toString, indent, true ).append( "serviceName=\"" ).append( getServiceName() ).append( "\"" );

		if ( hasFilter() )
			addIndent( _toString, indent, true ).append( "filter=\"" ).append( getFilter() ).append( "\"" );

		if ( hasId() )
			addIndent( _toString, indent, true ).append( "id=\"" ).append( getId() ).append( "\"" );

		indent--;

		if ( impl()->hasAttrib<const AckMsgImpl>() )
		{
			indent++;
			addIndent( _toString, indent, true ).append( "Attrib dataType=\"" ).append( getDTypeAsString( impl()->getAttribData().getDataType() ) ).append( "\"\n" );

			indent++;
			_toString.append( impl()->getAttribData().toString( indent ) );
			indent--;

			addIndent( _toString, indent ).append( "AttribEnd" );
			indent--;
		}
	}

	if ( hasExtendedHeader() )
	{
		indent++;
		addIndent( _toString, indent, true ).append( "ExtendedHeader\n" );

		indent++;

		addIndent( _toString, indent );
		hexToString( _toString, getExtendedHeader() );

		indent--;

		addIndent( _toString, indent, true ).append( "ExtendedHeaderEnd" );
		indent--;
	}

	if ( impl()->hasPayload() )
	{
		indent++;
		addIndent( _toString, indent, true ).append( "Payload dataType=\"" ).append( getDTypeAsString( impl()->getPayloadData().getDataType() ) ).append( "\"\n" );

		indent++;
		_toString.append( impl()->getPayloadData().toString( indent ) );
		indent--;

		addIndent( _toString, indent ).append( "PayloadEnd" );
		indent--;
	}

	addIndent( _toString, indent, true ).append( "AckMsgEnd\n" );

	return _toString;
}

const EmaBuffer& AckMsg::getAsHex() const
{
	return impl()->getAsHex();
}

bool AckMsg::hasSeqNum() const
{
	return impl()->hasSeqNum();
}

bool AckMsg::hasNackCode() const
{
	return impl()->hasNackCode();
}

bool AckMsg::hasText() const
{
	return impl()->hasText();
}

bool AckMsg::hasServiceName() const
{
	return impl()->hasServiceName();
}

UInt32 AckMsg::getAckId() const
{
	return impl()->getAckId();
}

UInt8 AckMsg::getNackCode() const
{
	return impl()->getNackCode();
}

UInt32 AckMsg::getSeqNum() const
{
	return impl()->getSeqNum();
}

const EmaString& AckMsg::getText() const
{
	return impl()->getText();
}

const EmaString& AckMsg::getServiceName() const
{
	return impl()->getServiceName();
}

bool AckMsg::getPrivateStream() const
{
	return impl()->getPrivateStream();
}

Decoder& AckMsg::getDecoder()
{
	return impl()->_decoder;
}

AckMsg& AckMsg::name( const EmaString& name )
{
	impl()->setName( name );
	return *this;
}

AckMsg& AckMsg::nameType( UInt8 nameType )
{
	impl()->setNameType( nameType );
	return *this;
}

AckMsg& AckMsg::serviceName( const EmaString& serviceName )
{
	impl()->copyServiceName( serviceName );
	return *this;
}

AckMsg& AckMsg::serviceId( UInt32 serviceId )
{
	impl()->setServiceId( serviceId );
	return *this;
}

AckMsg& AckMsg::id( Int32 id )
{
	impl()->setId( id );
	return *this;
}

AckMsg& AckMsg::filter( UInt32 filter )
{
	impl()->setFilter( filter );
	return *this;
}

AckMsg& AckMsg::streamId( Int32 streamId )
{
	impl()->setStreamId( streamId );
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

	impl()->setDomainType( ( UInt8 )domainType );
	return *this;
}

AckMsg& AckMsg::seqNum( UInt32 seqNum )
{
	impl()->setSeqNum( seqNum );
	return *this;
}

AckMsg& AckMsg::ackId( UInt32 postId )
{
	impl()->setAckId( postId );
	return *this;
}

AckMsg& AckMsg::nackCode( UInt8 nackCode )
{
	impl()->setNackCode( nackCode );
	return *this;
}

AckMsg& AckMsg::text( const EmaString& text )
{
	impl()->setText( text );
	return *this;
}

AckMsg& AckMsg::attrib( const ComplexType& data )
{
	impl()->setAttrib( data );
	return *this;
}

AckMsg& AckMsg::payload( const ComplexType& data )
{
	impl()->setPayload( data );
	return *this;
}

AckMsg& AckMsg::extendedHeader( const EmaBuffer& Buffer )
{
	impl()->setExtendedHeader( Buffer );
	return *this;
}

AckMsg& AckMsg::privateStream( bool privateStream )
{
	impl()->setPrivateStream( privateStream );
	return *this;
}

AckMsgImpl* AckMsg::impl()
{
	return static_cast<AckMsgImpl*>( _pImpl );
}

const AckMsgImpl* AckMsg::impl() const
{
	return static_cast<const AckMsgImpl*>( _pImpl );
}
