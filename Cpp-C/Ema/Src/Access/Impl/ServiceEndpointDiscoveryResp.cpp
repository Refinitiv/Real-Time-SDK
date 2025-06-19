/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ServiceEndpointDiscoveryResp.h"
#include "Access/Include/ServiceEndpointDiscoveryInfo.h"
#include "ExceptionTranslator.h"
#include "Utilities.h"

#include <new>

using namespace refinitiv::ema::access;

ServiceEndpointDiscoveryResp::ServiceEndpointDiscoveryResp() :
_toString(0,4096)
{
	try
	{
		_pServiceEndpointDiscoveryInfoList = new EmaVector<ServiceEndpointDiscoveryInfo>(16);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for EmaVector<ServiceEndpointDiscoveryInfo> in ServiceEndpointDiscoveryResp().");
	}
}

ServiceEndpointDiscoveryResp::~ServiceEndpointDiscoveryResp()
{
	if (_pServiceEndpointDiscoveryInfoList)
	{
		delete _pServiceEndpointDiscoveryInfoList;
		_pServiceEndpointDiscoveryInfoList = 0;
	}
}

const EmaVector<ServiceEndpointDiscoveryInfo>& ServiceEndpointDiscoveryResp::getServiceEndpointInfoList() const
{
	return *_pServiceEndpointDiscoveryInfoList;
}

const EmaString& ServiceEndpointDiscoveryResp::toString() const
{
	UInt32 index;
	addIndent(_toString.clear().append("Services : "), 1, true);
	for (index = 0; index < _pServiceEndpointDiscoveryInfoList->size(); index++)
	{
		addIndent(_toString.append((*_pServiceEndpointDiscoveryInfoList)[index].toString(2)), 1, true);
	}

	return _toString;
}

ServiceEndpointDiscoveryResp::operator const char* () const
{
	return toString().c_str();
}

