/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2018-2020,2022-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "UpdateMsg.h"
#include "UpdateMsgDecoder.h"
#include "UpdateMsgEncoder.h"
#include "EmaBufferInt.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "RdmUtilities.h"
#include "StaticDecoder.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

UpdateMsg::UpdateMsg() :
 Msg(),
 _toString()
{
}

UpdateMsg::UpdateMsg(const UpdateMsg& other) :
 Msg(),
 _toString()
{
	const char* functionName = "UpdateMsg::UpdateMsg(const UpdateMsg& other)";

	// Get a decoder for this instance
	getDecoder();

	_pDecoder->cloneBufferToMsg(&const_cast<UpdateMsg&>(other), functionName);

	if (other.hasMsgKey()) // Override with the message key generated by the Watchlist
	{
		UpdateMsgDecoder* upDecoder = static_cast<UpdateMsgDecoder*>(_pDecoder);

		upDecoder->cloneMsgKey(other);
	}

	if (hasMsgKey()) // Set the decoded values from the clone buffer to the encoder
	{
		if (hasName())
		{
			name(getName());
		}

		if (hasNameType())
		{
			nameType(getNameType());
		}

		if (hasServiceId())
		{
			serviceId(getServiceId());
		}

		if (hasId())
		{
			id(getId());
		}

		if (hasFilter())
		{
			filter(getFilter());
		}

		if (_pDecoder->hasAttrib())
		{
			attrib(getAttrib().getData());
		}
	}

	domainType(getDomainType());

	if (hasExtendedHeader())
	{
		extendedHeader(getExtendedHeader());
	}

	if (other.hasServiceName())
	{
		static_cast<UpdateMsgDecoder*>(_pDecoder)->setServiceName(other.getServiceName());
	}

	if (other.hasSeqNum())
	{
		seqNum(getSeqNum());
	}

	if (other.hasPermissionData())
	{
		permissionData(getPermissionData());
	}

	if (other.hasConflated())
	{
		conflated(getConflatedCount(), getConflatedTime());
	}

	if (other.hasPublisherId())
	{
		publisherId(getPublisherIdUserId(), getPublisherIdUserAddress());
	}

	updateTypeNum(getUpdateTypeNum());

	doNotCache(getDoNotCache());

	doNotConflate(getDoNotConflate());

	doNotRipple(getDoNotRipple());

	if (_pDecoder->hasPayload())
	{
		payload(getPayload().getData());
	}
}

UpdateMsg::~UpdateMsg()
{
	if ( _pEncoder )
	{
		g_pool.returnItem( static_cast<UpdateMsgEncoder*>( _pEncoder ) );
		_pEncoder = nullptr;
	}

	if ( _pDecoder )
	{
		// Free memory from cloning the message if any
		_pDecoder->deallocateCopiedBuffer();

		g_pool.returnItem( static_cast<UpdateMsgDecoder*>( _pDecoder) );

		_pDecoder = nullptr;
	}

}

UpdateMsg& UpdateMsg::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

Data::DataCode UpdateMsg::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum UpdateMsg::getDataType() const
{
	return DataType::UpdateMsgEnum;
}

const EmaString& UpdateMsg::toString() const
{
	return toString( 0 );
}

const EmaString& UpdateMsg::toString( const refinitiv::ema::rdm::DataDictionary& dictionary ) const
{
	UpdateMsg updateMsg;

	if (!dictionary.isEnumTypeDefLoaded() || !dictionary.isFieldDictionaryLoaded())
		return _toString.clear().append("\nDictionary is not loaded.\n");

	if (!_pEncoder)
	{
		_pEncoder = g_pool.getUpdateMsgEncoderItem();
		static_cast<Encoder*>(_pEncoder)->acquireEncIterator();
	}
	else if (!_pEncoder->ownsIterator())
		static_cast<Encoder*>(_pEncoder)->acquireEncIterator();

	RsslBuffer& rsslBuffer = _pEncoder->getRsslBuffer();

	StaticDecoder::setRsslData(&updateMsg, &rsslBuffer, RSSL_DT_MSG, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, dictionary._pImpl->rsslDataDictionary());
	_toString.clear().append(updateMsg.toString());

	return _toString;
}

const EmaString& UpdateMsg::toString( UInt64 indent ) const
{
	if (!_pDecoder)
		return _toString.clear().append("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n");

	const UpdateMsgDecoder* pTempDecoder = static_cast<const UpdateMsgDecoder*>(_pDecoder);

	addIndent( _toString.clear(), indent++ ).append( "UpdateMsg" );
	addIndent( _toString, indent, true ).append( "streamId=\"" ).append( pTempDecoder->getStreamId() ).append( "\"" );
	addIndent( _toString, indent, true ).append( "domain=\"" ).append( rdmDomainToString( getDomainType() ) ).append( "\"" );			
	addIndent( _toString, indent, true ).append( "updateTypeNum=\"" ).append( pTempDecoder->getUpdateTypeNum() ).append( "\"" );

	if ( pTempDecoder->getDoNotCache() )
		addIndent( _toString, indent, true ).append( "DoNotCache" );

	if ( pTempDecoder->getDoNotConflate() )
		addIndent( _toString, indent, true ).append( "DoNotConflate" );

	if ( pTempDecoder->getDoNotRipple() )
		addIndent( _toString, indent, true ).append( "DoNotRipple" );

	if ( pTempDecoder->hasPermissionData() )
	{
		EmaString temp;
		hexToString( temp, pTempDecoder->getPermissionData() );
		addIndent( _toString, indent, true ).append( "permissionData=\"" ).append( temp ).append( "\"" );
	}

	if ( pTempDecoder->hasConflated() )
	{
		addIndent( _toString, indent, true ).append( "conflatedCount=\"" ).append( pTempDecoder->getConflatedCount() ).append( "\"" );
		addIndent( _toString, indent, true ).append( "conflatedTime=\"" ).append( pTempDecoder->getConflatedTime() ).append( "\"" );
	}

	if ( pTempDecoder->hasSeqNum() )
		addIndent( _toString, indent, true ).append( "seqNum=\"" ).append( pTempDecoder->getSeqNum() ).append( "\"" );			

	if ( pTempDecoder->hasPublisherId() )
	{
		addIndent( _toString, indent, true ).append( "publisher user address=\"" ).append( pTempDecoder->getPublisherIdUserAddress() ).append( "\"" );			
		addIndent( _toString, indent, true ).append( "publisher user id=\"" ).append( pTempDecoder->getPublisherIdUserId() ).append( "\"" );			
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
			
	addIndent( _toString, indent, true ).append( "UpdateMsgEnd\n" );

	return _toString;
}

const EmaBuffer& UpdateMsg::getAsHex() const
{
	return static_cast<const UpdateMsgDecoder*>( _pDecoder )->getHexBuffer();
}

bool UpdateMsg::hasSeqNum() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->hasSeqNum();
}

bool UpdateMsg::hasPermissionData() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->hasPermissionData();
}

bool UpdateMsg::hasConflated() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->hasConflated();
}

bool UpdateMsg::hasPublisherId() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->hasPublisherId();
}

bool UpdateMsg::hasServiceName() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->hasServiceName();
}

UInt8 UpdateMsg::getUpdateTypeNum() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->getUpdateTypeNum();
}

UInt32 UpdateMsg::getSeqNum() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->getSeqNum();
}

const EmaBuffer& UpdateMsg::getPermissionData() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->getPermissionData();
}

UInt16 UpdateMsg::getConflatedTime() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->getConflatedTime();
}

UInt16 UpdateMsg::getConflatedCount() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->getConflatedCount();
}

UInt32 UpdateMsg::getPublisherIdUserId() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->getPublisherIdUserId();
}

UInt32 UpdateMsg::getPublisherIdUserAddress() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->getPublisherIdUserAddress();
}

bool UpdateMsg::getDoNotCache() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->getDoNotCache();
}

bool UpdateMsg::getDoNotConflate() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->getDoNotConflate();
}

bool UpdateMsg::getDoNotRipple() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->getDoNotRipple();
}

const EmaString& UpdateMsg::getServiceName() const
{
	return static_cast<const UpdateMsgDecoder*>(_pDecoder)->getServiceName();
}

Decoder& UpdateMsg::getDecoder()
{
	if ( !_pDecoder )
		setDecoder( g_pool.getUpdateMsgDecoderItem() );

	return *_pDecoder;
}

UpdateMsg& UpdateMsg::streamId( Int32 streamId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	_pEncoder->streamId( streamId );
	return *this;
}

UpdateMsg& UpdateMsg::domainType( UInt16 domainType )
{
	if ( domainType > 255 )
	{
		EmaString temp( "Passed in DomainType is out of range." );
		throwDtuException( domainType, temp );
		return *this;
	}

	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	_pEncoder->domainType( (UInt8)domainType );
	return *this;
}

UpdateMsg& UpdateMsg::name( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	_pEncoder->name( name );
	return *this;
}

UpdateMsg& UpdateMsg::nameType( UInt8 nameType )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	_pEncoder->nameType( nameType );
	return *this;
}

UpdateMsg& UpdateMsg::serviceName( const EmaString& serviceName )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	_pEncoder->serviceName( serviceName );
	return *this;
}

UpdateMsg& UpdateMsg::serviceId( UInt32 serviceId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	_pEncoder->serviceId( serviceId );
	return *this;
}

UpdateMsg& UpdateMsg::id( Int32 id )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	_pEncoder->identifier( id );
	return *this;
}

UpdateMsg& UpdateMsg::filter( UInt32 filter )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	_pEncoder->filter( filter );
	return *this;
}

UpdateMsg& UpdateMsg::updateTypeNum( UInt8 updateTypeNum )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	static_cast<UpdateMsgEncoder*>(_pEncoder)->updateTypeNum( updateTypeNum );
	return *this;
}

UpdateMsg& UpdateMsg::seqNum( UInt32 seqNum )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	static_cast<UpdateMsgEncoder*>(_pEncoder)->seqNum( seqNum );
	return *this;
}

UpdateMsg& UpdateMsg::permissionData( const EmaBuffer& permissionData )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	static_cast<UpdateMsgEncoder*>(_pEncoder)->permissionData( permissionData );
	return *this;
}

UpdateMsg& UpdateMsg::conflated( UInt16 count, UInt16 time )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	static_cast<UpdateMsgEncoder*>(_pEncoder)->conflated( count, time );
	return *this;
}

UpdateMsg& UpdateMsg::publisherId( UInt32 userId, UInt32 userAddress )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	static_cast<UpdateMsgEncoder*>(_pEncoder)->publisherId( userId, userAddress );
	return *this;
}

UpdateMsg& UpdateMsg::attrib( const ComplexType& data )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	_pEncoder->attrib( data );
	return *this;
}

UpdateMsg& UpdateMsg::payload( const ComplexType& data )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	_pEncoder->payload( data );
	return *this;
}

UpdateMsg& UpdateMsg::extendedHeader( const EmaBuffer& buffer )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	static_cast<UpdateMsgEncoder*>(_pEncoder)->extendedHeader( buffer );
	return *this;
}

UpdateMsg& UpdateMsg::doNotCache( bool doNotCache )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	static_cast<UpdateMsgEncoder*>(_pEncoder)->doNotCache( doNotCache );
	return *this;
}

UpdateMsg& UpdateMsg::doNotConflate( bool doNotConflate )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	static_cast<UpdateMsgEncoder*>(_pEncoder)->doNotConflate( doNotConflate );
	return *this;
}

UpdateMsg& UpdateMsg::doNotRipple( bool doNotRipple )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getUpdateMsgEncoderItem();

	static_cast<UpdateMsgEncoder*>(_pEncoder)->doNotRipple( doNotRipple );
	return *this;
}
