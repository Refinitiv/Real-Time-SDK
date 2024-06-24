/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#include "TunnelStreamLoginReqMsgImpl.h"
#include "ReqMsgEncoder.h"
#include "StaticDecoder.h"
#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

TunnelStreamLoginReqMsgImpl::TunnelStreamLoginReqMsgImpl() :
 _loginReqMsg(),
 _rsslBuffer(),
 _rsslMsg()
{
}

TunnelStreamLoginReqMsgImpl::TunnelStreamLoginReqMsgImpl( const TunnelStreamLoginReqMsgImpl& other ) :
 _loginReqMsg(),
 _rsslBuffer(),
 _rsslMsg()
{
	_rsslBuffer.length = other._rsslBuffer.length;

	_rsslBuffer.data = (char*)malloc( sizeof( char ) * _rsslBuffer.length );

	if ( !_rsslBuffer.data )
	{
		const char* text = "Failed to allocate memory in TunnelStreamLoginReqMsgImpl( const TunnelStreamLoginReqMsgImpl& ).";
		throwMeeException( text );
		return;
	}

	
	memcpy( _rsslBuffer.data, other._rsslBuffer.data, _rsslBuffer.length );

	StaticDecoder::setRsslData( &_loginReqMsg, &_rsslBuffer, RSSL_DT_MSG, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );
}

TunnelStreamLoginReqMsgImpl::~TunnelStreamLoginReqMsgImpl()
{
	if ( _rsslBuffer.data )
		free( _rsslBuffer.data );
}

TunnelStreamLoginReqMsgImpl& TunnelStreamLoginReqMsgImpl::operator=( const TunnelStreamLoginReqMsgImpl& other )
{
	if ( this == &other ) return *this;

	if ( _rsslBuffer.data )
		free( _rsslBuffer.data );

	_rsslBuffer.length = other._rsslBuffer.length;

	_rsslBuffer.data = (char*)malloc( sizeof( char ) * _rsslBuffer.length );

	if ( !_rsslBuffer.data )
	{
		const char* text = "Failed to allocate memory in TunnelStreamLoginReqMsgImpl::operator=( const TunnelStreamLoginReqMsgImpl& ).";
		throwMeeException( text );
		return *this;
	}

	
	memcpy( _rsslBuffer.data, other._rsslBuffer.data, _rsslBuffer.length );

	StaticDecoder::setRsslData( &_loginReqMsg, &_rsslBuffer, RSSL_DT_MSG, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

	return *this;
}

TunnelStreamLoginReqMsgImpl& TunnelStreamLoginReqMsgImpl::setLoginReqMsg( const ReqMsg& loginReqMsg )
{
	RsslEncodeIterator eIter;
	rsslClearEncodeIterator( &eIter );
	RsslRet retCode;
	if ( (retCode = rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION )) != RSSL_RET_SUCCESS )
	{
		EmaString temp( "Internal Error. Failed to set encode iterator version in TunnelStreamLoginReqMsgImpl::setLoginReqMsg( const ReqMsg& )." );
		throwIueException( temp, retCode );
		return *this;
	}

	if ( _rsslBuffer.data )
		free( _rsslBuffer.data );

	_rsslBuffer.length = 1024;
	_rsslBuffer.data = (char*)malloc( sizeof( char ) * _rsslBuffer.length );

	if ( !_rsslBuffer.data )
	{
		const char* temp = "Failed to allocate memory in TunnelStreamLoginReqMsgImpl::setLoginReqMsg( const ReqMsg& ).";
		throwMeeException( temp );
		return *this;
	}

	if ( (retCode = rsslSetEncodeIteratorBuffer( &eIter, &_rsslBuffer )) != RSSL_RET_SUCCESS )
	{
		EmaString temp( "Internal Error. Failed to set encode iterator buffer in TunnelStreamLoginReqMsgImpl::setLoginReqMsg( const ReqMsg& )." );
		throwIueException( temp, retCode );
		return *this;
	}

	RsslMsg* pRsslMsg = (RsslMsg*)static_cast<const ReqMsgEncoder&>( loginReqMsg.getEncoder() ).getRsslRequestMsg();

	while ( ( retCode = rsslEncodeMsg( &eIter, pRsslMsg ) ) == RSSL_RET_BUFFER_TOO_SMALL )
	{
		if ( _rsslBuffer.data )
			free( _rsslBuffer.data );

		_rsslBuffer.length += _rsslBuffer.length;
		_rsslBuffer.data = (char*)malloc( sizeof( char ) * _rsslBuffer.length );

		if ( !_rsslBuffer.data )
		{
			const char* temp = "Failed to allocate memory in TunnelStreamLoginReqMsgImpl::setLoginReqMsg( const ReqMsg& ).";
			throwMeeException( temp );
			return *this;
		}

		rsslClearEncodeIterator( &eIter );

		if ( (retCode = rsslSetEncodeIteratorRWFVersion( &eIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION )) != RSSL_RET_SUCCESS )
		{
			EmaString temp( "Internal Error. Failed to set encode iterator version in TunnelStreamLoginReqMsgImpl::setLoginReqMsg( const ReqMsg& )." );
			throwIueException( temp, retCode );
			return *this;
		}

		if ( (retCode = rsslSetEncodeIteratorBuffer( &eIter, &_rsslBuffer )) != RSSL_RET_SUCCESS )
		{
			EmaString temp( "Internal Error. Failed to set encode iterator buffer in TunnelStreamLoginReqMsgImpl::setLoginReqMsg( const ReqMsg& )." );
			throwIueException( temp, retCode );
			return *this;
		}
	}

	if ( retCode != RSSL_RET_SUCCESS )
	{
		if ( _rsslBuffer.data )
			free( _rsslBuffer.data );

		EmaString temp( "Internal Error. Failed to encode message in TunnelStreamLoginReqMsgImpl::setLoginReqMsg( const ReqMsg& )." );
		throwIueException( temp, retCode );
		return *this;
	}

	_rsslBuffer.length = rsslGetEncodedBufferLength( &eIter );

	return *this;
}

const ReqMsg& TunnelStreamLoginReqMsgImpl::getLoginReqMsg()
{
	StaticDecoder::setRsslData( &_loginReqMsg, &_rsslBuffer, RSSL_DT_MSG, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

	return _loginReqMsg;
}

RsslBuffer* TunnelStreamLoginReqMsgImpl::getRsslBuffer()
{
	return &_rsslBuffer;
}

RsslMsg* TunnelStreamLoginReqMsgImpl::getRsslMsg()
{
	RsslDecIterator dIter;
	rsslClearDecodeIterator( &dIter );
	RsslRet retCode;
	if ( (retCode = rsslSetDecodeIteratorRWFVersion( &dIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION )) != RSSL_RET_SUCCESS )
	{
		EmaString temp( "Internal Error. Failed to set decode iterator version in TunnelStreamLoginReqMsgImpl::getRsslMsg()." );
		throwIueException( temp, retCode );
		return 0;
	}

	if ( (retCode = rsslSetDecodeIteratorBuffer( &dIter, &_rsslBuffer )) != RSSL_RET_SUCCESS )
	{
		EmaString temp( "Internal Error. Failed to set decode iterator buffer in TunnelStreamLoginReqMsgImpl::getRsslMsg()." );
		throwIueException( temp, retCode );
		return 0;
	}

	if ( (retCode = rsslDecodeMsg( &dIter, &_rsslMsg )) != RSSL_RET_SUCCESS )
	{
		EmaString temp( "Internal Error. Failed to decode message in TunnelStreamLoginReqMsgImpl::getRsslMsg()." );
		throwIueException( temp, retCode );
		return 0;
	}

	return &_rsslMsg;
}
