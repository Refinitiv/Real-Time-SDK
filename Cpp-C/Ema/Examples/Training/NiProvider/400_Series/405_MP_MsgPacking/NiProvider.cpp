/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "NiProvider.h"

using namespace refinitiv::ema::access;
using namespace std;

int main()
{
	try
	{
		OmmProvider provider( OmmNiProviderConfig().username( "user" ) );
		UInt64 itemHandle = 5;
		bool sentRefreshMsg = false; // Keeps track if we packed and sent our first refresh

		provider.submit(RefreshMsg().serviceName("NI_PUB").name("IBM.N")
			.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed")
			.payload(FieldList()
				.addReal(22, 3990, OmmReal::ExponentNeg2Enum)
				.addReal(25, 3994, OmmReal::ExponentNeg2Enum)
				.addReal(30, 9, OmmReal::Exponent0Enum)
				.addReal(31, 19, OmmReal::Exponent0Enum)
				.complete())
			.complete(), itemHandle);


		UpdateMsg updMsg;
		PackedMsg packedMsg(provider);
		FieldList fieldList;

		for (Int32 a = 0; a < 60; a++)
		{
			packedMsg.initBuffer();

			for (Int32 i = 0; i < 10; i++)
			{
				if (!sentRefreshMsg)
				{
					// Add RefreshMsg one time for the first PackedMsg
					fieldList.addReal(22, 3990, OmmReal::ExponentNeg2Enum);
					fieldList.addReal(25, 3994, OmmReal::ExponentNeg2Enum);
					fieldList.addReal(30, 9, OmmReal::Exponent0Enum);
					fieldList.addReal(31, 19, OmmReal::Exponent0Enum);
					fieldList.complete();

					packedMsg.addMsg(RefreshMsg().serviceName("NI_PUB").name("IBM.N")
						.state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "UnSolicited Refresh Completed")
						.payload(fieldList)
						.complete(), itemHandle);

					sentRefreshMsg = true;
				}

				updMsg.clear();
				fieldList.clear();
				fieldList.addReal(22, 3391 + i, OmmReal::ExponentNeg2Enum);
				fieldList.addReal(30, 10 + i, OmmReal::Exponent0Enum);
				fieldList.complete();

				updMsg.serviceName("NI_PUB").name("IBM.N").payload(fieldList);

				packedMsg.addMsg(updMsg, itemHandle);
			}

			if (packedMsg.packedMsgCount() > 0)
			{
				provider.submit(packedMsg);
				packedMsg.clear();
			}
			else
			{
				cerr << "No one message was added to the packed buffer" << endl;
				return -1;
			}

			sleep(1000);
		}
	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	return 0;
}
