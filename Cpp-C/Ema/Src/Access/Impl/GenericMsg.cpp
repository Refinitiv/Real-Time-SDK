/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "GenericMsg.h"
#include "GenericMsgImpl.h"

#include "Utilities.h"
#include "GlobalPool.h"
#include "RdmUtilities.h"
#include "StaticDecoder.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

GenericMsg::GenericMsg() :
 Msg(),
 _toString()
{
	setImpl( g_pool.getGenericMsgImplItem() );
}

GenericMsg::GenericMsg(UInt32 size) :
 GenericMsg()
{
	impl()->Arena().reserve(size);
}

GenericMsg::GenericMsg(const GenericMsg& other) :
 Msg(),
 _toString()
{
	setImpl( g_pool.getGenericMsgImplItem() );
	*impl() = *other.impl();
}

GenericMsg::GenericMsg( GenericMsg&& other ) noexcept :
 Msg(),
 _toString()
{
	setImpl(other.impl());
	other.setImpl( nullptr );
}

GenericMsg& GenericMsg::operator=( const GenericMsg& other )
{
	if (this == &other)
		return *this;

	*impl() = *other.impl();

	return *this;
}

GenericMsg& GenericMsg::operator=( GenericMsg&& other ) noexcept
{
	if (this == &other)
		return *this;

	if ( impl() )
	{
		impl()->clear();
		g_pool.returnItem( impl() );
	}

	setImpl( other.impl() );
	other.setImpl( nullptr );

	return *this;
}

GenericMsg::~GenericMsg()
{
	if ( impl() )
	{
		impl()->clear();
		g_pool.returnItem( impl() );

		setImpl( nullptr );
	}
}

GenericMsg& GenericMsg::clear()
{
	EMA_ASSERT( impl() != nullptr, "Private implementation can't be missing");

	impl()->clear();

	return *this;
}

Data::DataCode GenericMsg::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum GenericMsg::getDataType() const
{
	return DataType::GenericMsgEnum;
}

const EmaString& GenericMsg::toString() const
{
	return toString( 0 );
}

const EmaString& GenericMsg::toString( const refinitiv::ema::rdm::DataDictionary& dictionary ) const
{
	if (!dictionary.isEnumTypeDefLoaded() || !dictionary.isFieldDictionaryLoaded())
		return _toString.clear().append("\nDictionary is not loaded.\n");

	GenericMsg genericMsg;

	RsslBuffer& rsslBuffer = getEncoder().getEncodedBuffer();

	StaticDecoder::setRsslData( &genericMsg, &rsslBuffer,
								RSSL_DT_MSG, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION,
								dictionary._pImpl->rsslDataDictionary());

	_toString.clear().append(genericMsg.toString());

	return _toString;
}

const EmaString& GenericMsg::toString(  UInt64 indent ) const
{
	_toString.clear();

	addIndent( _toString, indent++ ).append( "GenericMsg" );
	addIndent( _toString, indent, true ).append( "streamId=\"" ).append( getStreamId() ).append( "\"" );
	addIndent( _toString, indent, true ).append( "domain=\"" ).append( rdmDomainToString( getDomainType() ) ).append( "\"" );

	if ( getComplete() )
		addIndent( _toString, indent, true ).append( "MessageComplete" );

	if ( hasSeqNum() )
		addIndent( _toString, indent, true ).append( "seqNum=\"" ).append( getSeqNum() ).append( "\"" );

	if ( hasSecondarySeqNum() )
		addIndent( _toString, indent, true ).append( "secondarySeqNum=\"" ).append( getSecondarySeqNum() ).append( "\"" );

	if ( hasPartNum() )
		addIndent( _toString, indent, true ).append( "partNum=\"" ).append( getPartNum() ).append( "\"" );

	if ( hasPermissionData() )
	{
		EmaString temp;
		hexToString( temp, getPermissionData() );
		addIndent( _toString, indent, true ).append( "permissionData=\"" ).append( temp ).append( "\"" );
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

		if ( hasFilter() )
			addIndent( _toString, indent, true ).append( "filter=\"" ).append( getFilter() ).append( "\"" );

		if ( hasId() )
			addIndent( _toString, indent, true ).append( "id=\"" ).append( getId() ).append( "\"" );

		indent--;
		if ( impl()->hasAttrib<const GenericMsgImpl>() )
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

	addIndent( _toString, indent, true ).append( "GenericMsgEnd\n" );

	return _toString;
}

const EmaBuffer& GenericMsg::getAsHex() const
{
	return impl()->getAsHex();
}

bool GenericMsg::hasSeqNum() const
{
	return impl()->hasSeqNum();
}

bool GenericMsg::hasSecondarySeqNum() const
{
	return impl()->hasSecondarySeqNum();
}

bool GenericMsg::hasPartNum() const
{
	return impl()->hasPartNum();
}

bool GenericMsg::hasPermissionData() const
{
	return impl()->hasPermissionData();
}

UInt32 GenericMsg::getSeqNum() const
{
	return impl()->getSeqNum();
}

UInt32 GenericMsg::getSecondarySeqNum() const
{
	return impl()->getSecondarySeqNum();
}

UInt16 GenericMsg::getPartNum() const
{
	return impl()->getPartNum();
}

const EmaBuffer& GenericMsg::getPermissionData() const
{
	return impl()->getPermissionData();
}

bool GenericMsg::getComplete() const
{
	return impl()->getComplete();
}

Decoder& GenericMsg::getDecoder()
{
	return impl()->_decoder;
}

GenericMsg& GenericMsg::name( const EmaString& name )
{
	impl()->setName( name );
	return *this;
}

GenericMsg& GenericMsg::nameType( UInt8 nameType )
{
	impl()->setNameType( nameType );
	return *this;
}

GenericMsg& GenericMsg::serviceId( UInt32 serviceId )
{
	impl()->setServiceId( serviceId );
	return *this;
}

GenericMsg& GenericMsg::id( Int32 id )
{
	impl()->setId( id );
	return *this;
}

GenericMsg& GenericMsg::filter( UInt32 filter )
{
	impl()->setFilter( filter );
	return *this;
}

GenericMsg& GenericMsg::streamId( Int32 streamId )
{
	impl()->setStreamId( streamId );
	return *this;
}

GenericMsg& GenericMsg::domainType( UInt16 domainType )
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

GenericMsg& GenericMsg::seqNum( UInt32 seqNum )
{
	impl()->setSeqNum( seqNum );
	return *this;
}

GenericMsg& GenericMsg::secondarySeqNum( UInt32 seqNum )
{
	impl()->setSecondarySeqNum( seqNum );
	return *this;
}

GenericMsg& GenericMsg::partNum( UInt16 partNum )
{
	impl()->setPartNum( partNum );
	return *this;
}

GenericMsg& GenericMsg::permissionData( const EmaBuffer& permissionData )
{
	impl()->setPermissionData( permissionData );
	return *this;
}

GenericMsg& GenericMsg::complete( bool complete )
{
	impl()->setComplete( complete );
	return *this;
}

GenericMsg& GenericMsg::attrib( const ComplexType& data )
{
	impl()->setAttrib( data );
	return *this;
}

GenericMsg& GenericMsg::payload( const ComplexType& data )
{
	impl()->setPayload( data );
	return *this;
}

GenericMsg& GenericMsg::extendedHeader( const EmaBuffer& Buffer )
{
	impl()->setExtendedHeader( Buffer );
	return *this;
}

GenericMsg& GenericMsg::providerDriven( bool providerDriven )
{
	impl()->setProviderDriven( providerDriven );
	return *this;
}

GenericMsgImpl* GenericMsg::impl()
{
	return static_cast<GenericMsgImpl*>(_pImpl);
}

const GenericMsgImpl* GenericMsg::impl() const
{
	return static_cast<const GenericMsgImpl*>(_pImpl);
}
