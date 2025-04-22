/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|         Copyright (C) 2024 LSEG. All rights reserved.             --
 *|-----------------------------------------------------------------------------
 */

#include "Access/Include/ServiceList.h"
#include "Access/Include/EmaString.h"
#include "Access/Include/EmaVector.h"

using namespace refinitiv::ema::access;

ServiceList::ServiceList(const EmaString& name)
{
	_name = name;
	_serviceList.clear();
}

ServiceList::ServiceList(const ServiceList& other) :
	_name(other._name),
	_serviceList(other._serviceList)
{
}

ServiceList::~ServiceList()
{
	_name.clear();
	_serviceList.clear();
}


void ServiceList::clear()
{
	_name.clear();
	_serviceList.clear();
}

ServiceList& ServiceList::addService(const EmaString& serviceName)
{
	_serviceList.push_back(serviceName);

	return *this;
}

ServiceList& ServiceList::removeService(const EmaString& serviceName)
{
	_serviceList.removeValue(serviceName);

	return *this;
}

EmaVector<EmaString>& ServiceList::concreteServiceList()
{
	return _serviceList;
}

const EmaString& ServiceList::name() const
{
	return _name;
}

ServiceList& refinitiv::ema::access::ServiceList::name(const EmaString& name)
{
	_name = name;
	return *this;
}


