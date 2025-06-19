/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifdef WIN32
#include <windows.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif

#include "Ema.h"

#include "string.h"
#include <iostream>
#include <chrono>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;


int main()
{
	using namespace std::chrono;
	try
	{
		milliseconds beforeMS;
		milliseconds afterMS;

		FieldList fieldList;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			fieldList.clear().addReal(22, 3990, OmmReal::ExponentNeg2Enum).
				addReal(25, 3994, OmmReal::ExponentNeg2Enum).
				addReal(30, 9, OmmReal::Exponent0Enum).
				addReal(31, 19, OmmReal::Exponent0Enum).
				complete();
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run FieldList: " << (afterMS - beforeMS).count() << " MS" << "\n";

		ElementList elementList;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			elementList.clear().addAscii("QATest_addAscii", "Test_addAscii")
				.addInt("QATest_addInt", 50000000)
				.addUInt("QATest_addUInt", 1).complete();
		}

		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run ElementList: " << (afterMS - beforeMS).count() << " MS" << "\n";

		Map map;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			map.clear().addKeyAscii("QATestMap", MapEntry::AddEnum, elementList).complete();
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run Map: " << (afterMS - beforeMS).count() << " MS" << "\n";

		OmmArray array;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			array.clear().addUInt(123).addUInt(321).addUInt(567).complete();
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run OmmArray: " << (afterMS - beforeMS).count() << " MS" << "\n";

		FilterList filterList;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			filterList.clear().add(1, FilterEntry::UpdateEnum, fieldList);
			filterList.add(2, FilterEntry::UpdateEnum, elementList);
			filterList.add(3, FilterEntry::UpdateEnum, map);
			filterList.complete();
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run FilterList: " << (afterMS - beforeMS).count() << " MS" << "\n";

		Series series;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			series.clear().add(elementList);
			series.complete();
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run Series: " << (afterMS - beforeMS).count() << " MS" << "\n";

		Vector vector;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			vector.clear().add(1, VectorEntry::InsertEnum, filterList);
			vector.complete();
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run Vector: " << (afterMS - beforeMS).count() << " MS" << "\n";


		ReqMsg requestMsg;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			requestMsg.clear().serviceName("DIRECT_FEED").name("IBM.N");
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run ReqMsg: " << (afterMS - beforeMS).count() << " MS" << "\n";


		RefreshMsg refreshMsg;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			refreshMsg.clear().domainType(3).name("TEST").serviceId(1).
				state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Refresh Completed").solicited(true).privateStream(true).
				complete();
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run RefreshMsg: " << (afterMS - beforeMS).count() << " MS" << "\n";

		GenericMsg genericMsg;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			genericMsg.clear().domainType(2).name("genericMsg").payload(
				RefreshMsg().name("TEST").serviceName("DIRECT_FEED").
				state(OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "NestedMsg").
				payload(fieldList).
				complete());
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run GenericMsg: " << (afterMS - beforeMS).count() << " MS" << "\n";

		StatusMsg statusMsg;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			statusMsg.clear().name("TEST").serviceName("DIRECT_FEED").
				domainType(3).
				state(OmmState::ClosedEnum, OmmState::SuspectEnum, OmmState::NotFoundEnum, "Item not found");
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run StatusMsg: " << (afterMS - beforeMS).count() << " MS" << "\n";

		UpdateMsg updateMsg;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			updateMsg.clear().payload(fieldList);
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run UpdateMsg: " << (afterMS - beforeMS).count() << " MS" << "\n";


		PostMsg postMsg;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			postMsg.clear().postId(1).serviceId(2).name("TEST").solicitAck(true).payload(fieldList).complete();
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run PostMsg: " << (afterMS - beforeMS).count() << " MS" << "\n";

		AckMsg ackMsg;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			ackMsg.clear().domainType(1);
			ackMsg.ackId(1);
			ackMsg.seqNum(1);
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run AckMsg: " << (afterMS - beforeMS).count() << " MS" << "\n";

		OmmXml xml;
		const char* data = "TEST_TEST_TEST_TEST";
		UInt32 length = (UInt32)strlen(data);
		EmaBuffer buff(data, length);
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			xml.clear().set(buff);
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run OmmXml: " << (afterMS - beforeMS).count() << " MS" << "\n";

		OmmAnsiPage ansiPage;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			ansiPage.clear().set(buff);
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run OmmAnsiPage: " << (afterMS - beforeMS).count() << " MS" << "\n";

		OmmOpaque opaque;
		beforeMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		for (unsigned int i = 0; i < 10000000; i++)
		{
			opaque.clear().set(buff);
		}
		afterMS = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		std::cout << "Time of run OmmOpaque: " << (afterMS - beforeMS).count() << " MS" << "\n";

	}
	catch ( const OmmException& excp )
	{
		cout << excp << endl;
	}
	
	return 0;
}
