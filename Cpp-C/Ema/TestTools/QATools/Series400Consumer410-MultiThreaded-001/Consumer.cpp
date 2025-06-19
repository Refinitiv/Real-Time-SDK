/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Consumer.h"
#include "ConsumerThread.h"
#include "ConsumerResultValidation.h"

using namespace refinitiv::ema::access;
using namespace std;

extern void sleep(int millisecs);

int ResultValidation::_numRequestOpen = 0;
int ResultValidation::_numRefreshReceived = 0;
int ResultValidation::_numUpdateReceived = 0;
int ResultValidation::_numStatusReceived = 0;
int ResultValidation::_numInvalidClosure = 0;
int ResultValidation::_numValidClosure = 0;

bool ResultValidation::_SNAPSHOT = true;
int ResultValidation::_NUMOFITEMPERLOOP = 50;
bool ResultValidation::_USERDISPATCH = false;
int ResultValidation::_USERDISPATCHTIMEOUT = 1000;
int ResultValidation::_RUNTIME = 60000;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ommEvent )
{
	ResultValidation* closure = (ResultValidation*)(ommEvent.getClosure());

	closure->closureValidate(refreshMsg.getName());
	delete closure;

	++ResultValidation::_numRefreshReceived;
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
	++ResultValidation::_numUpdateReceived;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ommEvent )
{
	++ResultValidation::_numStatusReceived;
}

void printUsage()
{
	cout <<"\nOptions:\n"
		<< "  -?\tShows this usage\n\n"
		<< "  -snapshot \tSend item snapshot requests [default = true]\n"
		<< "  -numOfItemPerLoop \tSend the number of item request per loop [default = 50]\n"
		<< "  -userDispatch \tUse UserDispatch Operation Model [default = false]\n"
		<< "  -userDispatchTimeout \tSet dispatch timeout period in microseconds if UserDispatch Operation Model [default = 1000]\n"
		<< "  -runtime \tRun time for test case in milliseconds [default = 60000]\n"
		<< "\n";
	exit(-1);
}

void  readCommandlineArgs(int argc, char* argv[])
{
	int iargs = 1;

	while (iargs < argc)
	{
		if (0 == strcmp("-?", argv[iargs]))
		{
			printUsage();
		}
		else if (strcmp("-snapshot", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) printUsage();

			ResultValidation::_SNAPSHOT = (strcmp("true", argv[iargs++]) == 0 ? true : false);
		}
		else if (strcmp("-numOfItemPerLoop", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) printUsage();

			ResultValidation::_NUMOFITEMPERLOOP = atoi(argv[iargs++]);
		}
		else if (strcmp("-userDispatch", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) printUsage();

			ResultValidation::_USERDISPATCH = (strcmp("true", argv[iargs++]) == 0 ? true : false);
		}
		else if (strcmp("-userDispatchTimeout", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) printUsage();

			ResultValidation::_USERDISPATCHTIMEOUT = atoi(argv[iargs++]);
		}
		else if (strcmp("-runtime", argv[iargs]) == 0)
		{
			++iargs; if (iargs == argc) printUsage();

			ResultValidation::_RUNTIME = atoi(argv[iargs++]);
		}
		else
		{
			cout << "Invalid argument: " << argv[iargs] << endl;
			printUsage();
		}
	}
}

int main( int argc, char* argv[] )
{
	OmmConsumer* consumer = NULL;
	AppThread* userDispathThread = NULL;

	try {
		readCommandlineArgs(argc, argv);

		/*allow test with userDispatch or apiDispatch*/
		if (ResultValidation::_USERDISPATCH)
		{
			consumer = new OmmConsumer(OmmConsumerConfig().host("localhost:14002").username("user").operationModel(OmmConsumerConfig::UserDispatchEnum));
			userDispathThread = new AppThread(consumer);
			userDispathThread->start();
		}
		else
			consumer = new OmmConsumer(OmmConsumerConfig().host("localhost:14002").username("user"));

		AppClient appClient1;
		AppClient appClient2;
		AppClient appClient3;

		/*allow test with multiple threads on the same consumer instance*/
		ConsumerThread consumerThread1( consumer, &appClient1);
		ConsumerThread consumerThread2( consumer, &appClient2);
		ConsumerThread consumerThread3( consumer, &appClient3);
		
		cout << "The consumer application starts testing with "
			 << ( userDispathThread ? "UserDispatch..." : "ApiDispatch..." )
			 << endl;

		/*allow test with different item from different service name*/
		consumerThread1.openItem("IBM", "DIRECT_FEED");
		consumerThread2.openItem("TRI", "DIRECT_FEED");
		consumerThread3.openItem("YHOO", "DIRECT_FEED");

		consumerThread1.start();
		consumerThread2.start();
		consumerThread3.start();

		/*allow test with different long run period*/
		sleep(ResultValidation::_RUNTIME);

		consumerThread1.stop();
		consumerThread2.stop();
		consumerThread3.stop();

		cout << "The consumer application is waiting for all responses back before exit..." << endl;

		sleep(10000);

		/*allow to validate different specification*/
		ResultValidation::printTestResult();

		/*clean up*/
		if (userDispathThread)
		{
			userDispathThread->stop();
			delete userDispathThread;
		}
		if (consumer)
			delete consumer;
	}
	catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
