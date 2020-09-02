/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ServiceEndpointDiscoveryInfo.h"
#include "ExceptionTranslator.h"
#include "Utilities.h"

#include <new>

using namespace rtsdk::ema::access;

ServiceEndpointDiscoveryInfo::ServiceEndpointDiscoveryInfo() :
_toString(0,256)
{
	_pDataFormatList = 0;
	_pLocationList = 0;
	try
	{
		_pDataFormatList = new EmaVector<EmaString>(2);
		_pLocationList = new EmaVector<EmaString>(2);
	}
	catch (std::bad_alloc)
	{
		// it's safe to delete nullptr
		delete _pDataFormatList;
		_pDataFormatList = 0;
		delete _pLocationList;
		_pLocationList = 0;

		throwMeeException("Failed to allocate memory for EmaVector<EmaString> in ServiceEndpointDiscoveryInfo().");
	}
}

ServiceEndpointDiscoveryInfo::~ServiceEndpointDiscoveryInfo()
{
	if (_pDataFormatList)
	{
		delete _pDataFormatList;
		_pDataFormatList = 0;
	}

	if (_pLocationList)
	{
		delete _pLocationList;
		_pLocationList = 0;
	}
}

const EmaVector<EmaString>& ServiceEndpointDiscoveryInfo::getDataFormatList() const
{
	return *_pDataFormatList;
}

const EmaString&	ServiceEndpointDiscoveryInfo::getEndPoint() const
{
	return _endPoint;
}

const EmaVector<EmaString>& ServiceEndpointDiscoveryInfo::getLocationList() const
{
	return *_pLocationList;
}

const EmaString&	ServiceEndpointDiscoveryInfo::getPort() const
{
	return _port;
}

const EmaString&	ServiceEndpointDiscoveryInfo::getProvider() const
{
	return _provider;
}

const EmaString&	ServiceEndpointDiscoveryInfo::getTransport() const
{
	return _transport;
}

ServiceEndpointDiscoveryInfo& ServiceEndpointDiscoveryInfo::operator=(const ServiceEndpointDiscoveryInfo& other)
{
	UInt32 index;
	_pDataFormatList->clear();
	for (index = 0; index < other.getDataFormatList().size(); index++)
	{
		_pDataFormatList->push_back(other.getDataFormatList()[index]);
	}
	
	_endPoint = other.getEndPoint();

	_pLocationList->clear();
	for (index = 0; index < other.getLocationList().size(); index++)
	{
		_pLocationList->push_back(other.getLocationList()[index]);
	}

	_port = other.getPort();
	_provider = other.getProvider();
	_transport = other.getTransport();

	return *this;
}

const EmaString& ServiceEndpointDiscoveryInfo::toString() const
{
	return toString(0);
}

const EmaString& ServiceEndpointDiscoveryInfo::toString(UInt64 indent) const
{
	UInt32 index;
	addIndent(_toString.clear().append("Service : "), indent, true);
	addIndent(_toString.append("Provider : ").append(_provider), indent, true);
	addIndent(_toString.append("Transport : ").append(_transport), indent, true);
	addIndent(_toString.append("Endpoint : ").append(_endPoint), indent, true);
	addIndent(_toString.append("Port : ").append(_port), indent, true);

	_toString.append("Data Format : ");
	for (index = 0; index < _pDataFormatList->size(); index++)
		_toString.append((*_pDataFormatList)[index]).append("  ");

	addIndent(_toString, indent, true);
	_toString.append("Location : ");
	for (index = 0; index < _pLocationList->size(); index++)
		_toString.append((*_pLocationList)[index]).append(" ");

	return _toString;
}

ServiceEndpointDiscoveryInfo::operator const char* () const
{
	return _toString.c_str();
}
