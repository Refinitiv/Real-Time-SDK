/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "TunnelStreamRequest.h"
#include "ReqMsg.h"
#include "ExceptionTranslator.h"
#include "TunnelStreamLoginReqMsgImpl.h"
#include "OmmInvalidUsageException.h"

#include <limits.h>
#include <new>

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

CosCommon::CosCommon() :
 _maxMsgSize( 6144 )
{
}

CosCommon::~CosCommon()
{
}

CosCommon::CosCommon( const CosCommon& other ) :
 _maxMsgSize( other._maxMsgSize )
{
}

CosCommon& CosCommon::operator=( const CosCommon& other )
{
	if ( this != &other )
		_maxMsgSize = other._maxMsgSize;

	return *this;
}

CosCommon& CosCommon::clear()
{
	_maxMsgSize = 6144;
	return *this;
}

CosCommon& CosCommon::maxMsgSize( UInt64 maxMsgSize )
{
	if ( maxMsgSize == 0 ||
		maxMsgSize > LONG_MAX )
	{
		EmaString temp( "Passed in maximum message size is out of valid range. Passed in value is " );
		temp.append( maxMsgSize );

		throwOorException( temp );
	}

	_maxMsgSize = maxMsgSize;
	return *this;
}

UInt64 CosCommon::getMaxMsgSize() const
{
	return _maxMsgSize;
}


CosAuthentication::CosAuthentication() :
 _type( CosAuthentication::NotRequiredEnum )
{
}

CosAuthentication::~CosAuthentication()
{
}

CosAuthentication::CosAuthentication( const CosAuthentication& other ) :
 _type( other._type )
{
}

CosAuthentication& CosAuthentication::operator=( const CosAuthentication& other )
{
	if ( this != &other )
		_type = other._type;

	return *this;
}

CosAuthentication& CosAuthentication::clear()
{
	_type = CosAuthentication::NotRequiredEnum;
	return *this;
}

CosAuthentication& CosAuthentication::type( CosAuthentication::CosAuthenticationType type )
{
	if ( type < CosAuthentication::NotRequiredEnum ||
		type > CosAuthentication::OmmLoginEnum )
	{
		EmaString temp( "Passed in CosAuthenticationType is out of range. Passed in value is " );
		temp.append( (Int64)type );
		throwOorException( temp );
	}

	_type = type;
	return *this;
}

CosAuthentication::CosAuthenticationType CosAuthentication::getType() const
{
	return _type;
}


CosFlowControl::CosFlowControl() :
 _type( CosFlowControl::NoneEnum ),
 _recvWindowSize( -1 ),
 _sendWindowSize( -1 )
{
}

CosFlowControl::CosFlowControl( const CosFlowControl& other ) :
 _type( other._type ),
 _recvWindowSize( other._recvWindowSize ),
 _sendWindowSize( other._sendWindowSize )
{
}

CosFlowControl::~CosFlowControl()
{
}

CosFlowControl& CosFlowControl::operator=( const CosFlowControl& other )
{
	if ( this == &other ) return *this;

	_type = other._type;
	_recvWindowSize = other._recvWindowSize;
	_sendWindowSize = other._sendWindowSize;

	return *this;
}

CosFlowControl& CosFlowControl::clear()
{
	_type = CosFlowControl::NoneEnum;
	_recvWindowSize = -1;
	_sendWindowSize = -1;

	return *this;
}

CosFlowControl& CosFlowControl::type( CosFlowControl::CosFlowControlType type )
{
	if ( type < CosFlowControl::NoneEnum ||
		type > CosFlowControl::BidirectionalEnum )
	{
		EmaString temp( "Passed in CosFlowControlType is out of range. Passed in value is " );
		temp.append( (Int64)type );
		throwOorException( temp );
	}

	_type = type;
	return *this;
}

CosFlowControl& CosFlowControl::recvWindowSize( Int64 size )
{
	if ( size < -1 || 
		size > LONG_MAX )
	{
		EmaString temp( "Passed in received window size is out of valid range. Passed in value is " );
		temp.append( size );

		throwOorException( temp );
	}

	_recvWindowSize = size;
	return *this;
}

CosFlowControl& CosFlowControl::sendWindowSize( Int64 size )
{
	if ( size < 0 || 
		size > LONG_MAX )
	{
		EmaString temp( "Passed in send window size is out of valid range. Passed in value is " );
		temp.append( size );

		throwOorException( temp );
	}

	_sendWindowSize = size;
	return *this;
}

CosFlowControl::CosFlowControlType CosFlowControl::getType() const
{
	return _type;
}

Int64 CosFlowControl::getRecvWindowSize() const
{
	return _recvWindowSize;
}

Int64 CosFlowControl::getSendWindowSize() const
{
	return _sendWindowSize;
}


CosDataIntegrity::CosDataIntegrity() :
 _type( CosDataIntegrity::BestEffortEnum )
{
}

CosDataIntegrity::CosDataIntegrity( const CosDataIntegrity& other ) :
 _type( other._type )
{
}

CosDataIntegrity::~CosDataIntegrity()
{
}

CosDataIntegrity& CosDataIntegrity::operator=( const CosDataIntegrity& other )
{
	if ( this != &other )
		_type = other._type;

	return *this;
}

CosDataIntegrity& CosDataIntegrity::clear()
{
	_type = CosDataIntegrity::BestEffortEnum;
	return *this;
}

CosDataIntegrity& CosDataIntegrity::type( CosDataIntegrity::CosDataIntegrityType type )
{
	if ( type < CosDataIntegrity::BestEffortEnum ||
		type > CosDataIntegrity::ReliableEnum )
	{
		EmaString temp( "Passed in CosDataIntegrity is out of range. Passed in value is " );
		temp.append( (Int64)type );
		throwOorException( temp );
	}

	_type = type;
	return *this;
}

CosDataIntegrity::CosDataIntegrityType CosDataIntegrity::getType() const
{
	return _type;
}


CosGuarantee::CosGuarantee() :
 _type( CosGuarantee::NoneEnum ),
 _persistLocally( true ),
 _filePath()
{
}

CosGuarantee::CosGuarantee( const CosGuarantee& other ) :
 _type( other._type ),
 _persistLocally( other._persistLocally ),
 _filePath( other._filePath )
{
}

CosGuarantee::~CosGuarantee()
{
}

CosGuarantee& CosGuarantee::operator=( const CosGuarantee& other )
{
	if ( this == &other ) return *this;

	_type = other._type;
	_persistLocally = other._persistLocally;
	_filePath = other._filePath;

	return *this;
}

CosGuarantee& CosGuarantee::clear()
{
	_type = CosGuarantee::NoneEnum;
	_persistLocally = true;
	_filePath.clear();

	return *this;
}

CosGuarantee& CosGuarantee::type( CosGuarantee::CosGuaranteeType type )
{
	if ( type < CosGuarantee::NoneEnum ||
		type > CosGuarantee::PersistentQueueEnum )
	{
		EmaString temp( "Passed in CosGuarantee is out of range. Passed in value is " );
		temp.append( (Int64)type );
		throwOorException( temp );
	}

	_type = type;
	return *this;
}

CosGuarantee& CosGuarantee::persistLocally( bool persistLocally )
{
	_persistLocally = persistLocally;
	return *this;
}

CosGuarantee& CosGuarantee::persistenceFilePath( const EmaString& filePath )
{
	_filePath = filePath;
	return *this;
}

CosGuarantee::CosGuaranteeType CosGuarantee::getType() const
{
	return _type;
}

bool CosGuarantee::getPersistLocally() const
{
	return _persistLocally;
}

const EmaString& CosGuarantee::getPersistenceFilePath() const
{
	return _filePath;
}


ClassOfService::ClassOfService() :
 _common(),
 _authentication(),
 _flowControl(),
 _dataIntegrity(),
 _guarantee()
{
}

ClassOfService::ClassOfService( const ClassOfService& other ) :
 _common( other._common ),
 _authentication( other._authentication ),
 _flowControl( other._flowControl ),
 _dataIntegrity( other._dataIntegrity ),
 _guarantee( other._guarantee )
{
}

ClassOfService::~ClassOfService()
{
}

ClassOfService& ClassOfService::operator=( const ClassOfService& other )
{
	if ( this == &other ) return *this;

	_common = other._common;
	_authentication = other._authentication;
	_flowControl = other._flowControl;
	_dataIntegrity = other._dataIntegrity;
	_guarantee = other._guarantee;

	return *this;
}

ClassOfService& ClassOfService::clear()
{
	_common.clear();
	_authentication.clear();
	_flowControl.clear();
	_dataIntegrity.clear();
	_guarantee.clear();

	return *this;
}

ClassOfService& ClassOfService::common( const CosCommon& cosCommon )
{
	_common = cosCommon;
	return *this;
}

ClassOfService& ClassOfService::authentication( const CosAuthentication& cosAuthentication )
{
	_authentication = cosAuthentication;
	return *this;
}

ClassOfService& ClassOfService::flowControl( const CosFlowControl& cosFlowControl )
{
	_flowControl = cosFlowControl;
	return *this;
}

ClassOfService& ClassOfService::dataIntegrity( const CosDataIntegrity& cosDataIntegrity )
{
	_dataIntegrity = cosDataIntegrity;
	return *this;
}

ClassOfService& ClassOfService::guarantee( const CosGuarantee& cosGuarantee )
{
	_guarantee = cosGuarantee;
	return *this;
}

const CosCommon& ClassOfService::getCommon() const
{
	return _common;
}

const CosAuthentication& ClassOfService::getAuthentication() const
{
	return _authentication;
}

const CosFlowControl& ClassOfService::getFlowControl() const
{
	return _flowControl;
}

const CosDataIntegrity& ClassOfService::getDataIntegrity() const
{
	return _dataIntegrity;
}

const CosGuarantee& ClassOfService::getGuarantee() const
{
	return _guarantee;
}


TunnelStreamRequest::TunnelStreamRequest() :
 _domainType( 0 ),
 _serviceId( 0 ),
 _responseTimeout( 60 ),
 _guaranteedOutputBuffers( 50 ),
 _serviceIdSet( false ),
 _serviceNameSet( false ),
 _serviceName(),
 _name(),
 _cos(),
 _pImpl( 0 )
{
}

TunnelStreamRequest::TunnelStreamRequest( const TunnelStreamRequest& other ) :
 _domainType( other._domainType ),
 _serviceId( other._serviceId ),
 _responseTimeout( other._responseTimeout ),
 _guaranteedOutputBuffers( other._guaranteedOutputBuffers ),
 _serviceIdSet( other._serviceIdSet ),
 _serviceNameSet( other._serviceNameSet ),
 _serviceName( other._serviceName ),
 _name( other._name ),
 _cos( other._cos ),
 _pImpl( 0 )
{
	if ( other._pImpl )
	{
		try {
			_pImpl = new TunnelStreamLoginReqMsgImpl( *other._pImpl );
		} catch ( std::bad_alloc& ) {}

		if ( !_pImpl )
		{
			const char* text = "Failed to allocate memory in TunnelStreamRequest( const TunnelStreamRequest& ).";
			throwMeeException( text );
		}
	}
}

TunnelStreamRequest::~TunnelStreamRequest()
{
	if ( _pImpl )
		delete _pImpl;
}

TunnelStreamRequest& TunnelStreamRequest::operator=( const TunnelStreamRequest& other )
{
	if ( this == &other ) return *this;

	_domainType = other._domainType;
	_serviceId = other._serviceId;
	_responseTimeout = other._responseTimeout;
	_guaranteedOutputBuffers = other._guaranteedOutputBuffers;
	_serviceIdSet = other._serviceIdSet;
	_serviceNameSet = other._serviceNameSet;
	_serviceName = other._serviceName;
	_name = other._name;
	_cos = other._cos;

	if ( other._pImpl )
	{
		if ( _pImpl )
			*_pImpl = *other._pImpl;
		else
		{
			try {
				_pImpl = new TunnelStreamLoginReqMsgImpl( *other._pImpl );
			} catch ( std::bad_alloc& ) {}

			if ( !_pImpl )
			{
				const char* text = "Failed to allocate memory in TunnelStreamRequest::operator=( const TunnelStreamRequest& ).";
				throwMeeException( text );
			}
		}
	}
	else
	{
		if ( _pImpl )
		{
			delete _pImpl;
			_pImpl = 0;
		}
	}

	return *this;
}

TunnelStreamRequest& TunnelStreamRequest::clear()
{
	_domainType = 0;
	_serviceId = 0;
	_responseTimeout = 60;
	_guaranteedOutputBuffers = 50;
	_serviceIdSet = false;
	_serviceNameSet = false;
	_serviceName.clear();
	_name.clear();
	_cos.clear();
	
	if ( _pImpl )
	{
		delete _pImpl;
		_pImpl = 0;
	}

	return *this;
}

TunnelStreamRequest& TunnelStreamRequest::domainType( UInt8 domainType )
{
	if ( domainType > 255 )
	{
		EmaString temp( "Passed in domain type is not supported." );
		temp.append( (UInt32)domainType );
		throwDtuException( domainType, temp );
	}

	_domainType = domainType;
	return *this;
}

TunnelStreamRequest& TunnelStreamRequest::serviceId( UInt32 serviceId )
{
	if ( _serviceNameSet )
	{
		EmaString temp( "Attempt to set serviceId while serviceName is already set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_serviceIdSet = true;
	_serviceId = serviceId;

	return *this;
}

TunnelStreamRequest& TunnelStreamRequest::serviceName( const EmaString& serviceName )
{
	if ( _serviceIdSet )
	{
		EmaString temp( "Attempt to set serviceName while serviceId is already set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_serviceNameSet = true;
	_serviceName = serviceName;

	return *this;
}

TunnelStreamRequest& TunnelStreamRequest::name( const EmaString& name )
{
	_nameSet = true;
	_name = name;

	return *this;
}

TunnelStreamRequest& TunnelStreamRequest::responseTimeout( UInt32 timeout )
{
	_responseTimeout = timeout;
	return *this;
}

TunnelStreamRequest& TunnelStreamRequest::guaranteedOutputBuffers( UInt32 outputBuffers )
{
	_guaranteedOutputBuffers = outputBuffers;
	return *this;
}

TunnelStreamRequest& TunnelStreamRequest::classOfService( const ClassOfService& cos )
{
	_cos = cos;
	return *this;
}

TunnelStreamRequest& TunnelStreamRequest::loginReqMsg( const ReqMsg& loginReqMsg )
{
	if ( !_pImpl )
	{
		try {
			_pImpl = new TunnelStreamLoginReqMsgImpl();
		} catch ( std::bad_alloc& ) {}

		if ( !_pImpl )
		{
			const char* text = "Failed to allocate memory in TunnelStreamRequest::loginReqMsg( const ReqMsg& ).";
			throwMeeException( text );
		}
	}

	_pImpl->setLoginReqMsg( loginReqMsg );

	return *this;
}

bool TunnelStreamRequest::hasServiceId() const
{
	return _serviceIdSet;
}

bool TunnelStreamRequest::hasServiceName() const
{
	return _serviceNameSet;
}

bool TunnelStreamRequest::hasName() const
{
	return _nameSet;
}

bool TunnelStreamRequest::hasLoginReqMsg() const
{
	return _pImpl ? true : false;
}

UInt16 TunnelStreamRequest::getDomainType() const
{
	return _domainType;
}

UInt32 TunnelStreamRequest::getServiceId() const
{
	if ( !_serviceIdSet )
	{
		EmaString temp( "Attempt to get serviceId while it is not set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _serviceId;
}

const EmaString& TunnelStreamRequest::getServiceName() const
{
	if ( !_serviceNameSet )
	{
		EmaString temp( "Attempt to get serviceName while it is not set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _serviceName;
}

const EmaString& TunnelStreamRequest::getName() const
{
	if ( !_nameSet )
	{
		EmaString temp( "Attempt to get Name while it is not set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _name;
}

UInt32 TunnelStreamRequest::getResponseTimeOut() const
{
	return _responseTimeout;
}

UInt32 TunnelStreamRequest::getGuaranteedOutputBuffers() const
{
	return _guaranteedOutputBuffers;
}

const ReqMsg& TunnelStreamRequest::getLoginReqMsg() const
{
	if ( !_pImpl )
	{
		EmaString temp( "Attempt to get loginReqMsg while it is not set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pImpl->getLoginReqMsg();
}

const ClassOfService& TunnelStreamRequest::getClassOfService() const
{
	return _cos;
}
