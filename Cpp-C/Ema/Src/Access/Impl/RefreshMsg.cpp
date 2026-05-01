/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "RefreshMsg.h"
#include "RefreshMsgImpl.h"

#include "OmmState.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "RdmUtilities.h"
#include "StaticDecoder.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

RefreshMsg::RefreshMsg() :
 Msg(),
 _toString()
{
	setImpl( g_pool.getRefreshMsgImplItem() );
}

RefreshMsg::RefreshMsg(UInt32 size) :
 RefreshMsg()
{
	impl()->Arena().reserve(size);
}

RefreshMsg::RefreshMsg(const RefreshMsg& other) :
 Msg(),
 _toString()
{
	setImpl( g_pool.getRefreshMsgImplItem() );
	*impl() = *other.impl();
}

RefreshMsg::RefreshMsg( RefreshMsg&& other ) noexcept :
 Msg(),
 _toString()
{
	setImpl( other.impl() );
	other.setImpl( nullptr );
}

RefreshMsg& RefreshMsg::operator=( const RefreshMsg& other )
{
	if (this == &other)
		return *this;

	*impl() = *other.impl();

	return *this;
}

RefreshMsg& RefreshMsg::operator=( RefreshMsg&& other ) noexcept
{
	if (this == &other)
		return *this;

	if ( impl() )
	{
		impl()->deallocateBuffers<RefreshMsgImpl>();
		g_pool.returnItem(impl());
	}

	setImpl( other.impl() );
	other.setImpl( nullptr );

	return *this;
}

RefreshMsg::~RefreshMsg()
{
	if ( impl() )
	{
		impl()->deallocateBuffers<RefreshMsgImpl>();
		g_pool.returnItem( impl() );

		setImpl( nullptr );
	}
}

RefreshMsg& RefreshMsg::clear()
{
	EMA_ASSERT( impl() != nullptr, "Private implementation can't be missing");

	impl()->clear();

	return *this;
}

Data::DataCode RefreshMsg::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum RefreshMsg::getDataType() const
{
	return DataType::RefreshMsgEnum;
}

const EmaString& RefreshMsg::toString() const
{
	return toString( 0 );
}

const EmaString& RefreshMsg::toString( const refinitiv::ema::rdm::DataDictionary& dictionary ) const
{
	if (!dictionary.isEnumTypeDefLoaded() || !dictionary.isFieldDictionaryLoaded())
		return _toString.clear().append("\nDictionary is not loaded.\n");

	RefreshMsg refreshMsg;

	RsslBuffer& rsslBuffer = getEncoder().getEncodedBuffer();

	StaticDecoder::setRsslData( &refreshMsg, &rsslBuffer,
								RSSL_DT_MSG, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION,
								dictionary._pImpl->rsslDataDictionary() );

	_toString.clear().append(refreshMsg.toString());

	return _toString;
}

const EmaString& RefreshMsg::toString( UInt64 indent ) const
{
	_toString.clear();

	addIndent( _toString, indent++ ).append( "RefreshMsg" );
	addIndent( _toString, indent, true ).append( "streamId=\"" ).append( getStreamId() ).append( "\"" );
	addIndent( _toString, indent, true ).append( "domain=\"" ).append( rdmDomainToString( getDomainType() ) ).append( "\"" );

	if ( getSolicited() )
		addIndent( _toString, indent, true ).append( "Solicited" );

	if ( getComplete() )
		addIndent( _toString, indent, true ).append( "RefreshComplete" );

	if ( getPrivateStream() )
		addIndent( _toString, indent, true ).append( "PrivateStream" );

	if ( getDoNotCache() )
		addIndent( _toString, indent, true ).append( "DoNotCache" );

	if ( getClearCache() )
		addIndent( _toString, indent, true ).append( "ClearCache" );

	addIndent( _toString, indent, true ).append( "state=\"" ).append( getState().toString() ).append( "\"" );

	EmaString temp;
	hexToString( temp, getItemGroup() );
	addIndent( _toString, indent, true ).append( "itemGroup=\"" ).append( temp ).append( "\"" );

	if ( hasPermissionData() )
	{
		EmaString temp;
		hexToString( temp, getPermissionData() );
		addIndent( _toString, indent, true ).append( "permissionData=\"" ).append( temp ).append( "\"" );
	}

	if ( hasQos() )
		addIndent( _toString, indent, true ).append( "qos=\"" ).append( getQos().toString() ).append( "\"" );

	if ( hasPartNum() )
		addIndent( _toString, indent, true ).append( "partNum=\"" ).append( getPartNum() ).append( "\"" );

	if ( hasSeqNum() )
		addIndent( _toString, indent, true ).append( "seqNum=\"" ).append( getSeqNum() ).append( "\"" );

	if ( hasPublisherId() )
	{
		addIndent( _toString, indent, true ).append( "publisher user address=\"" ).append( getPublisherIdUserAddress() ).append( "\"" );
		addIndent( _toString, indent, true ).append( "publisher user id=\"" ).append( getPublisherIdUserId() ).append( "\"" );
	}

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

		if ( impl()->hasAttrib<const RefreshMsgImpl>() )
		{
			indent++;
			addIndent( _toString, indent, true ).append( "Attrib dataType=\"" ).append( getDTypeAsString( impl()->getAttribData().getDataType() ) ).append( "\"\n" );

			indent++;
			_toString.append( impl()->getAttribData().toString( indent ) );
			indent--;

			addIndent( _toString, indent, true ).append( "AttribEnd" );
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

		addIndent( _toString, indent, true ).append( "PayloadEnd" );
		indent--;
	}

	addIndent( _toString, indent, true ).append( "RefreshMsgEnd\n" );

	return _toString;
}

const EmaBuffer& RefreshMsg::getAsHex() const
{
	return impl()->getAsHex();
}

bool RefreshMsg::hasQos() const
{
	return impl()->hasQos();
}

bool RefreshMsg::hasSeqNum() const
{
	return impl()->hasSeqNum();
}

bool RefreshMsg::hasPartNum() const
{
	return impl()->hasPartNum();
}

bool RefreshMsg::hasPermissionData() const
{
	return impl()->hasPermissionData();
}

bool RefreshMsg::hasPublisherId() const
{
	return impl()->hasPublisherId();
}

bool RefreshMsg::hasServiceName() const
{
	return impl()->hasServiceName();
}

const OmmState& RefreshMsg::getState() const
{
	return impl()->getState();
}

const OmmQos& RefreshMsg::getQos() const
{
	return impl()->getQos();
}

UInt32 RefreshMsg::getSeqNum() const
{
	return impl()->getSeqNum();
}

UInt16 RefreshMsg::getPartNum() const
{
	return impl()->getPartNum();
}

const EmaBuffer& RefreshMsg::getItemGroup() const
{
	return impl()->getItemGroup();
}

const EmaBuffer& RefreshMsg::getPermissionData() const
{
	return impl()->getPermissionData();
}

UInt32 RefreshMsg::getPublisherIdUserId() const
{
	return impl()->getPublisherIdUserId();
}

UInt32 RefreshMsg::getPublisherIdUserAddress() const
{
	return impl()->getPublisherIdUserAddress();
}

bool RefreshMsg::getDoNotCache() const
{
	return impl()->getDoNotCache();
}

bool RefreshMsg::getSolicited() const
{
	return impl()->getSolicited();
}

bool RefreshMsg::getComplete() const
{
	return impl()->getComplete();
}

bool RefreshMsg::getClearCache() const
{
	return impl()->getClearCache();
}

bool RefreshMsg::getPrivateStream() const
{
	return impl()->getPrivateStream();
}

const EmaString& RefreshMsg::getServiceName() const
{
	return impl()->getServiceName();
}

Decoder& RefreshMsg::getDecoder()
{
	if ( !impl() )
	{
		setImpl( g_pool.getRefreshMsgImplItem() );
	}

	return impl()->_decoder;
}

RefreshMsg& RefreshMsg::streamId( Int32 streamId )
{
	impl()->setStreamId( streamId );
	return *this;
}

RefreshMsg& RefreshMsg::domainType( UInt16 domainType )
{
	if ( domainType > 255 )
	{
		EmaString temp( "Passed in DomainType is out of range." );
		throwDtuException( domainType, temp );
		return *this;
	}

	impl()->setDomainType( (UInt8)domainType );
	return *this;
}

RefreshMsg& RefreshMsg::name( const EmaString& name )
{
	impl()->setName( name );
	return *this;
}

RefreshMsg& RefreshMsg::nameType( UInt8 nameType )
{
	impl()->setNameType( nameType );
	return *this;
}

RefreshMsg& RefreshMsg::serviceName( const EmaString& serviceName )
{
	impl()->copyServiceName( serviceName );
	return *this;
}

RefreshMsg& RefreshMsg::serviceId( UInt32 serviceId )
{
	impl()->setServiceId( serviceId );
	return *this;
}

RefreshMsg& RefreshMsg::id( Int32 id )
{
	impl()->setId( id );
	return *this;
}

RefreshMsg& RefreshMsg::filter( UInt32 filter )
{
	impl()->setFilter( filter );
	return *this;
}

RefreshMsg& RefreshMsg::qos( UInt32 timeliness, UInt32 rate )
{

	impl()->setQos( timeliness, rate );
	return *this;
}

RefreshMsg& RefreshMsg::state( OmmState::StreamState streamState, OmmState::DataState dataState,
						UInt8 statusCode, const EmaString& statusText )
{

	impl()->setState( streamState, dataState, statusCode, statusText );
	return *this;
}

RefreshMsg& RefreshMsg::seqNum( UInt32 seqNum )
{

	impl()->setSeqNum( seqNum );
	return *this;
}

RefreshMsg& RefreshMsg::partNum( UInt16 partNum )
{

	impl()->setPartNum( partNum );
	return *this;
}

RefreshMsg& RefreshMsg::itemGroup( const EmaBuffer& itemGroup )
{

	impl()->setItemGroup( itemGroup );
	return *this;
}

RefreshMsg& RefreshMsg::permissionData( const EmaBuffer& permissionData )
{
	impl()->setPermissionData( permissionData );
	return *this;
}

RefreshMsg& RefreshMsg::publisherId( UInt32 userId, UInt32 userAddress )
{
	impl()->setPublisherId( userId, userAddress );
	return *this;
}

RefreshMsg& RefreshMsg::attrib( const ComplexType& data )
{
	impl()->setAttrib( data );
	return *this;
}

RefreshMsg& RefreshMsg::payload( const ComplexType& data )
{
	impl()->setPayload( data );
	return *this;
}

RefreshMsg& RefreshMsg::extendedHeader( const EmaBuffer& buffer )
{
	impl()->setExtendedHeader( buffer );
	return *this;
}

RefreshMsg& RefreshMsg::solicited( bool solicited )
{
	impl()->setSolicited( solicited );
	return *this;
}

RefreshMsg& RefreshMsg::doNotCache( bool doNotCache )
{

	impl()->setDoNotCache( doNotCache );
	return *this;
}

RefreshMsg& RefreshMsg::clearCache( bool clearCache )
{

	impl()->setClearCache( clearCache );
	return *this;
}

RefreshMsg& RefreshMsg::complete( bool complete )
{
	impl()->setComplete( complete );
	return *this;
}

RefreshMsg& RefreshMsg::privateStream( bool privateStream )
{
	impl()->setPrivateStream( privateStream );
	return *this;
}

RefreshMsgImpl* RefreshMsg::impl()
{
	return static_cast<RefreshMsgImpl*>(_pImpl);
}

const RefreshMsgImpl* RefreshMsg::impl() const
{
	return static_cast<const RefreshMsgImpl*>(_pImpl);
}
