/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ConsumerResultValidation.h"

using namespace refinitiv::ema::access;

ResultValidation::ResultValidation(const refinitiv::ema::access::EmaString& itemName)
{
	_itemName = itemName;
}

bool ResultValidation::closureValidate(const refinitiv::ema::access::EmaString& receivedItemName)
{
	bool result = (receivedItemName == _itemName) ? true : false;
	if (result)
		++ResultValidation::_numValidClosure;
	else
		++ResultValidation::_numInvalidClosure;

	return result;
}

void ResultValidation::printTestResult()
{
	EmaString _testResultPrint;
	_testResultPrint.append("\n\n***ResultValidation Test Result*** \n\n");

	if (_numRequestOpen == _numRefreshReceived)
		_testResultPrint.append(">>Request/Refresh validation succeeded \n");
	else
		_testResultPrint.append(">>Request/Refresh validation failed \n");

	_testResultPrint.append("_numRequestOpen = ").append(_numRequestOpen).append(" \n_numRefreshReceived = ").append(_numRefreshReceived);

	if (ResultValidation::_numInvalidClosure > 0)
		_testResultPrint.append("\n\n>>Closure validation failed \n").append("_numInvalidClosure = ").append(_numInvalidClosure);
	else
		_testResultPrint.append("\n\n>>Closure validation succeeded \n").append("_numValidClosure = ").append(_numValidClosure);

	_testResultPrint.append("\n\n>>Update msg validation\n").append("_numUpdateReceived = ").append(_numUpdateReceived);

	_testResultPrint.append("\n\n>>Status msg validation\n").append("_numStatusReceived = ").append(_numStatusReceived);

	cout << _testResultPrint.c_str() << endl;

	for (std::list<void*>::iterator it = closuresList.begin(); it != closuresList.end(); ++it)
	{
		delete(*it);
	}
}
