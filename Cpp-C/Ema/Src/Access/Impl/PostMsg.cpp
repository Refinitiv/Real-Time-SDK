/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "PostMsg.h"
#include "PostMsgImpl.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "RdmUtilities.h"
#include "StaticDecoder.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

const EmaString CreateName( "Create" );
const EmaString DeleteName( " | Delete" );
const EmaString ModifyName( " | ModifyPermission" );

PostMsg::PostMsg() :
 Msg(),
 _toString(),
 _postUserRightsString()
{
	setImpl( g_pool.getPostMsgImplItem() );
}

PostMsg::PostMsg(UInt32 size) :
 PostMsg()
{
	impl()->Arena().reserve(size);
}

PostMsg::PostMsg(const PostMsg& other) :
 Msg(),
 _toString(),
 _postUserRightsString()
{
	setImpl( g_pool.getPostMsgImplItem() );
	*impl() = *other.impl();
}

PostMsg::PostMsg( PostMsg&& other ) noexcept :
 Msg(),
 _toString(),
 _postUserRightsString()
{
	setImpl( other.impl() );
	other.setImpl( nullptr );
}

PostMsg& PostMsg::operator=( const PostMsg& other )
{
	if (this == &other)
		return *this;

	*impl() = *other.impl();

	return *this;
}

PostMsg& PostMsg::operator=( PostMsg&& other ) noexcept
{
	if (this == &other)
		return *this;

	if (_pImpl)
	{
		impl()->deallocateBuffers<PostMsgImpl>();
		g_pool.returnItem( impl() );
	}

	setImpl( other.impl() );
	other.setImpl( nullptr );

	return *this;
}

PostMsg::~PostMsg()
{
	if ( impl() )
	{
		impl()->deallocateBuffers<PostMsgImpl>();
		g_pool.returnItem( impl() );

		setImpl( nullptr );
	}
}

PostMsg& PostMsg::clear()
{
	EMA_ASSERT( impl() != nullptr, "Private implementation can't be missing");

	impl()->clear();
	
	return *this;
}

const EmaString& PostMsg::getPostUserRightsAsString() const
{
	UInt16 postUserRights = getPostUserRights();

	_postUserRightsString.set( "PostUserRights are: " );

	if ( postUserRights & CreateEnum )
		_postUserRightsString += CreateName;

	if ( postUserRights & DeleteEnum )
		_postUserRightsString += DeleteName;

	if ( postUserRights & ModifyPermissionEnum )
		_postUserRightsString += ModifyName;

	return _postUserRightsString;
}

Data::DataCode PostMsg::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum PostMsg::getDataType() const
{
	return DataType::PostMsgEnum;
}

const EmaString& PostMsg::toString() const
{
	return toString( 0 );
}

const EmaString& PostMsg::toString( const refinitiv::ema::rdm::DataDictionary& dictionary ) const
{
	if (!dictionary.isEnumTypeDefLoaded() || !dictionary.isFieldDictionaryLoaded())
		return _toString.clear().append("\nDictionary is not loaded.\n");

	PostMsg refreshMsg;

	RsslBuffer& rsslBuffer = getEncoder().getEncodedBuffer();

	StaticDecoder::setRsslData( &refreshMsg, &rsslBuffer,
								RSSL_DT_MSG, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION,
								dictionary._pImpl->rsslDataDictionary() );

	_toString.clear().append(refreshMsg.toString());

	return _toString;
}

const EmaString& PostMsg::toString(UInt64 indent) const
{
	_toString.clear();

	addIndent( _toString, indent++ ).append( "PostMsg" );
	addIndent( _toString, indent, true ).append( "streamId=\"" ).append( getStreamId() ).append( "\"" );
	addIndent( _toString, indent, true ).append( "domain=\"" ).append( rdmDomainToString( getDomainType() ) ).append( "\"" );

	if ( getComplete() )
		addIndent( _toString, indent, true ).append( "Complete" );

	if ( getSolicitAck() )
		addIndent( _toString, indent, true ).append( "Ack Requested" );

	if ( hasSeqNum() )
		addIndent( _toString, indent, true ).append( "seqNum=\"" ).append( getSeqNum() ).append( "\"" );

	if ( hasPartNum() )
		addIndent( _toString, indent, true ).append( "partNum=\"" ).append( getPartNum() ).append( "\"" );

	if ( hasPostId() )
		addIndent( _toString, indent, true ).append( "postId=\"" ).append( getPostId() ).append( "\"" );

	if ( hasPostUserRights() )
		addIndent( _toString, indent, true ).append( "postUserRights=\"" ).append( getPostUserRights() ).append( "\"" );

	if ( hasPermissionData() )
	{
		EmaString temp;
		hexToString( temp, getPermissionData() );
		addIndent( _toString, indent, true ).append( "permissionData=\"" ).append( temp ).append( "\"" );
	}

	addIndent( _toString, indent, true ).append( "publisherIdUserId=\"" ).append( getPublisherIdUserId() ).append( "\"" );
	addIndent( _toString, indent, true ).append( "publisherIdUserAddress=\"" ).append( getPublisherIdUserAddress() ).append( "\"" );

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
		if ( impl()->hasAttrib<const PostMsgImpl>() )
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

	addIndent( _toString, indent, true ).append( "PostMsgEnd\n" );

	return _toString;
}

const EmaBuffer& PostMsg::getAsHex() const
{
	return impl()->getAsHex();
}

bool PostMsg::hasSeqNum() const
{
	return impl()->hasSeqNum();
}

bool PostMsg::hasPostId() const
{
	return impl()->hasPostId();
}

bool PostMsg::hasPartNum() const
{
	return impl()->hasPartNum();
}

bool PostMsg::hasPostUserRights() const
{
	return impl()->hasPostUserRights();
}

bool PostMsg::hasPermissionData() const
{
	return impl()->hasPermissionData();
}

bool PostMsg::hasServiceName() const
{
	return impl()->hasServiceName();
}

UInt32 PostMsg::getSeqNum() const
{
	return impl()->getSeqNum();
}

UInt32 PostMsg::getPostId() const
{
	return impl()->getPostId();
}

UInt16 PostMsg::getPartNum() const
{
	return impl()->getPartNum();
}

UInt16 PostMsg::getPostUserRights() const
{
	return impl()->getPostUserRights();
}

const EmaBuffer& PostMsg::getPermissionData() const
{
	return impl()->getPermissionData();
}

UInt32 PostMsg::getPublisherIdUserId() const
{
	return impl()->getPublisherIdUserId();
}

UInt32 PostMsg::getPublisherIdUserAddress() const
{
	return impl()->getPublisherIdUserAddress();
}

bool PostMsg::getSolicitAck() const
{
	return impl()->getSolicitAck();
}

bool PostMsg::getComplete() const
{
	return impl()->getComplete();
}

const EmaString& PostMsg::getServiceName() const
{
	return impl()->getServiceName();
}

Decoder& PostMsg::getDecoder()
{
	if ( !impl() )
	{
		setImpl( g_pool.getPostMsgImplItem() );
	}

	return impl()->_decoder;
}

PostMsg& PostMsg::streamId( Int32 id )
{
	impl()->setStreamId( id );
	return *this;
}

PostMsg& PostMsg::domainType( UInt16 domainType )
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

PostMsg& PostMsg::name( const EmaString& name )
{
	impl()->setName( name );
	return *this;
}

PostMsg& PostMsg::nameType( UInt8 nameType )
{
	impl()->setNameType( nameType );
	return *this;
}

PostMsg& PostMsg::serviceName( const EmaString& name )
{
	impl()->copyServiceName( name );
	return *this;
}

PostMsg& PostMsg::serviceId( UInt32 serviceId )
{
	impl()->setServiceId( serviceId );
	return *this;
}

PostMsg& PostMsg::id( Int32 id )
{
	impl()->setId( id );
	return *this;
}

PostMsg& PostMsg::filter( UInt32 filter )
{
	impl()->setFilter( filter );
	return *this;
}

PostMsg& PostMsg::seqNum( UInt32 seqNum )
{
	impl()->setSeqNum( seqNum );
	return *this;
}

PostMsg& PostMsg::postId( UInt32 postId )
{
	impl()->setPostId( postId );
	return *this;
}

PostMsg& PostMsg::partNum( UInt16 partNum )
{
	impl()->setPartNum( partNum );
	return *this;
}

PostMsg& PostMsg::postUserRights( UInt16 postUserRights )
{
	impl()->setPostUserRights( postUserRights );
	return *this;
}

PostMsg& PostMsg::permissionData( const EmaBuffer& permissionData )
{
	impl()->setPermissionData( permissionData );
	return *this;
}

PostMsg& PostMsg::publisherId( UInt32 UserId, UInt32 UserAddress )
{

	impl()->setPublisherId( UserId, UserAddress );
	return *this;
}

PostMsg& PostMsg::attrib( const ComplexType& data )
{
	impl()->setAttrib( data );
	return *this;
}

PostMsg& PostMsg::payload( const ComplexType& data )
{
	impl()->setPayload( data );
	return *this;
}

PostMsg& PostMsg::extendedHeader( const EmaBuffer& Buffer )
{
	impl()->setExtendedHeader( Buffer );
	return *this;
}

PostMsg& PostMsg::solicitAck( bool ack )
{
	impl()->setSolicitAck( ack );
	return *this;
}

PostMsg& PostMsg::complete( bool complete )
{
	impl()->setComplete( complete );
	return *this;
}

PostMsgImpl* PostMsg::impl()
{
	return static_cast<PostMsgImpl*>(_pImpl);
}

const PostMsgImpl* PostMsg::impl() const
{
	return static_cast<const PostMsgImpl*>(_pImpl);
}
