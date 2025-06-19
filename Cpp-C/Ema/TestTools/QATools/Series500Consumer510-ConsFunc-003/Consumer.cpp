/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Consumer.h"
#include <cstring>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& event) 
{
	//APIQA
	if (refreshMsg.getDomainType() == 1)
	{
		cout << refreshMsg << "\nevent session info (refresh)\n" << endl;		// defaults to refreshMsg.toString()
		printSessionStatus(event);
	}
	else
	{
		cout << refreshMsg << "\nevent channel info (refresh)\n" << event.getChannelInformation() << endl;		// defaults to refreshMsg.toString()
	}
	//END APIQA
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& event) 
{
	//APIQA
	if (updateMsg.getDomainType() == 1)
	{
		cout << updateMsg << "\nevent session info (update)\n" << endl;		// defaults to updateMsg.toString()
		printSessionStatus(event);
	}
	else
	{
		cout << updateMsg << "\nevent channel info (update)\n" << event.getChannelInformation() << endl;		// defaults to updateMsg.toString()
	}
	//END APIQA
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& event) 
{
	//APIQA
	if (statusMsg.getDomainType() == 1)
	{
		cout << statusMsg << "\nevent session info (status)\n" << endl;		// defaults to statusMsg.toString()
		printSessionStatus(event);
	}
	else
	{
		cout << statusMsg << "\nevent channel info (status)\n" << event.getChannelInformation() << endl;		// defaults to statusMsg.toString()
	}
	//END APIQA	
}

void AppClient::printSessionStatus(const refinitiv::ema::access::OmmConsumerEvent& event)
{
	EmaVector<ChannelInformation> statusVector; 
	
	event.getSessionInformation(statusVector);

	// Print out the channel information.
	for (UInt32 i = 0; i < statusVector.size(); ++i)
	{
		cout << statusVector[i] << endl;
	}
}
//API QA
void printHelp()
{
	cout << endl << "Options:\n" << " -?\tShows this usage" << endl
		<< " -test0 Testing default, no qos in ReqMsg." << endl
		<< " -test11 Testing ReqMsg.qos(Timeliness.REALTIME, Rate.TICK_BY_TICK)." << endl
		<< " -test12 Testing ReqMsg.qos(Timeliness.BEST_DELAYED_TIMELINESS, Rate.TICK_BY_TICK)." << endl
		<< " -test21 Testing ReqMsg.qos(qos(Timeliness.REALTIME, Rate.JIT_CONFLATED)." << endl;
}
//END API QA

int main( int argc, char* argv[] )
{ 
	try { 
		//API QA
		int _TEST = 0;
		bool test0 = false;
		bool test11 = false;
		bool test12 = false;
		bool test21 = false;
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-?") == 0)
			{
				printHelp();
				return 0;
			}
			else if (strcmp(argv[i], "-test0") == 0)
			{
				test0 = true;
				if (test0)
					_TEST = 0;
			}
			else if (strcmp(argv[i], "-test11") == 0)
			{
				test11 = true;
				if (test11)
					_TEST = 11;
			}
			else if (strcmp(argv[i], "-test12") == 0)
			{
				test12 = true;
				if (test12)
					_TEST = 12;
			}
			else if (strcmp(argv[i], "-test21") == 0)
			{
				test21 = true;
				if (test21)
					_TEST = 21;
			}
		}
		//END API QA

		AppClient client;
		ServiceList serviceList("SVG1");

		serviceList.concreteServiceList().push_back("DIRECT_FEED");
		serviceList.concreteServiceList().push_back("DIRECT_FEED_2");

		OmmConsumer consumer( OmmConsumerConfig().consumerName("Consumer_10").addServiceList(serviceList), client);

		//API QA
		void* closure1 = (void*)1;
		void* closure2 = (void*)2;
		ElementList view1; 
		OmmArray array1;

		array1.fixedWidth(2);
		array1.addInt(2).addInt(22).addInt(25).complete();

		view1.addUInt(ENAME_VIEW_TYPE, 1).addArray(ENAME_VIEW_DATA, array1).complete();

		ElementList view2;
		OmmArray array2;

		array2.fixedWidth(2);
		array2.addInt(2).addInt(30).addInt(31).complete();

		view2.addUInt(ENAME_VIEW_TYPE, 1).addArray(ENAME_VIEW_DATA, array2).complete();

		switch (_TEST) {
		case 0:
			cout << "***APIQA TEST 0 : ReqMsg does NOT set qos & view1(2,22,25) view2(2,30,31).***" << endl;
			consumer.registerClient(ReqMsg().serviceListName("SVG1").name("LSEG.L").payload(view1), client, closure1);
			consumer.registerClient(ReqMsg().serviceListName("SVG1").name("LSEG.L").payload(view2), client, closure2);
			break;
		case 11:
			cout << "***APIQA TEST 11 : ReqMsg.qos(Timeliness.REALTIME, Rate.TICK_BY_TICK) & view1(2,22,25) view2(2,30,31).***" << endl;
			consumer.registerClient(ReqMsg().serviceListName("SVG1").name("LSEG.L").payload(view1).qos(OmmQos::Timeliness::RealTimeEnum, OmmQos::Rate::TickByTickEnum), client, closure1);
			consumer.registerClient(ReqMsg().serviceListName("SVG1").name("LSEG.L").payload(view2).qos(OmmQos::Timeliness::RealTimeEnum, OmmQos::Rate::TickByTickEnum), client, closure2);
			break;
		case 12:
			cout << "***APIQA TEST 12 : ReqMsg.qos(Timeliness.BEST_DELAYED_TIMELINESS, Rate.TICK_BY_TICK) & view1(2,22,25) view2(2,30,31).***" << endl;
			consumer.registerClient(ReqMsg().serviceListName("SVG1").name("LSEG.L").payload(view1).qos(OmmQos::Timeliness::InexactDelayedEnum, OmmQos::Rate::TickByTickEnum), client, closure1);
			consumer.registerClient(ReqMsg().serviceListName("SVG1").name("LSEG.L").payload(view2).qos(OmmQos::Timeliness::InexactDelayedEnum, OmmQos::Rate::TickByTickEnum), client, closure2);
			break;
		case 21:
			cout << "***APIQA TEST 21 : ReqMsg.qos(Timeliness.REALTIME, Rate.JIT_CONFLATED) & view1(2,22,25) view2(2,30,31).***" << endl;
			consumer.registerClient(ReqMsg().serviceListName("SVG1").name("LSEG.L").payload(view1).qos(OmmQos::Timeliness::RealTimeEnum, OmmQos::Rate::JustInTimeConflatedEnum), client, closure1);
			consumer.registerClient(ReqMsg().serviceListName("SVG1").name("LSEG.L").payload(view2).qos(OmmQos::Timeliness::RealTimeEnum, OmmQos::Rate::JustInTimeConflatedEnum), client, closure2);
			break;		
		default:
			cout << "Invalid test";
		}
		//END API QA
		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
		
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
