/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "Ema.h"

class AppClient : public rtsdk::ema::access::OmmProviderClient
{
public:

	void processLoginRequest(const rtsdk::ema::access::ReqMsg&, const rtsdk::ema::access::OmmProviderEvent&);
protected:
	void onReqMsg(const rtsdk::ema::access::ReqMsg&, const rtsdk::ema::access::OmmProviderEvent&);

};