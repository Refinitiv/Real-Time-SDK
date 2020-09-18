///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

#include "NiProvider.h"

using namespace rtsdk::ema::access;
using namespace std;

int main( int argc, char* argv[] )
{
	try
	{
        //APIQA
		// NIProvider only publishes for 2 seconds
		for (int i = 0; i < 1000000; i++)
		{
			cout << "!!! createOmmProvider() " << i << " !!!" << endl;
			OmmProvider* provider = new OmmProvider(OmmNiProviderConfig());
			UInt64 itemHandle = 5;

			provider->submit(RefreshMsg().serviceName("NI_PUB").name("IBM.N")
				.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed")
				.payload(FieldList()
					.addReal(22, 3990, OmmReal::ExponentNeg2Enum)
					.addReal(25, 3994, OmmReal::ExponentNeg2Enum)
					.addReal(30, 9, OmmReal::Exponent0Enum)
					.addReal(31, 19, OmmReal::Exponent0Enum)
					.complete())
				.complete(), itemHandle);

			sleep(1000);

			for (Int32 j = 0; j < 2; j++)
			{
				provider->submit(UpdateMsg().serviceName("NI_PUB").name("IBM.N")
					.payload(FieldList()
						.addReal(22, 3391 + j, OmmReal::ExponentNeg2Enum)
						.addReal(30, 10 + j, OmmReal::Exponent0Enum)
						.complete()), itemHandle);
				sleep(1000);
			}
			cout << "!!! ~OmmProvider() " << i << " !!!" << endl;
			delete provider;
		}
		//END APIQA
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
