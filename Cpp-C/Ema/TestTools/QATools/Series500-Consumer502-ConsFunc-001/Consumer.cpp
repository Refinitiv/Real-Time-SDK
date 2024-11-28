///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
#include <cstring>
#include <time.h>

using namespace refinitiv::ema::access;
using namespace std;

const UInt32 DEFAULT_IOCTL_INTERVAL = 0;
const time_t DEFAULT_IOCTL_TIME = 0;
const UInt32 DEFAULT_FALLBACK_INTERVAL = 0;
const UInt32 DEFAULT_FALLBACK_TIME = 0;
const bool DEFAULT_IOCTL_ENABLE_PH = false;
const EmaString DEFAULT_IOCTL_CHANNEL_NAME;
const EmaString DEFAULT_IOCTL_WSB_GROUPE;
const UInt32 DEFAULT_IOCTL_DETECTION_TIME_INTERVAL = 0;
const EmaString DEFAULT_IOCTL_DETECTION_TIME_SCHEDULE;
const bool DEFAULE_IOCTL_FALLBCK_WITHIN_WSB_GROUPE = false;

UInt32 ioctlInterval = DEFAULT_IOCTL_INTERVAL;
time_t ioctlTime = DEFAULT_IOCTL_TIME;
UInt32 fallBackInterval = DEFAULT_FALLBACK_INTERVAL;
time_t fallBackTime = DEFAULT_FALLBACK_TIME;
bool ioctlEnablePH = DEFAULT_IOCTL_ENABLE_PH;
EmaString ioctlChannelName = DEFAULT_IOCTL_CHANNEL_NAME;
EmaString ioctlWarmstandbyGroup = DEFAULT_IOCTL_WSB_GROUPE;
UInt32 ioctlDetectionTimeInterval = DEFAULT_IOCTL_DETECTION_TIME_INTERVAL;
EmaString ioctlDetectionTimeSchedule = DEFAULT_IOCTL_DETECTION_TIME_SCHEDULE;
bool ioctlFallBackWithinWSBGroup = DEFAULE_IOCTL_FALLBCK_WITHIN_WSB_GROUPE;

bool updateCalled = false;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& event) 
{
	cout << endl << refreshMsg << endl << "event channel info (refresh)"
		<< endl << event.getChannelInformation() << endl;
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& event)
{
	if (!updateCalled)
	{
		updateCalled = true;
		cout << endl << updateMsg << endl << "event channel info (update)"
			<< endl << event.getChannelInformation() << endl;
	}
	else
		cout << "skipped printing updateMsg" << endl;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& event) 
{
	cout << endl << statusMsg << endl << "event channel info (status)" << endl << event.getChannelInformation();
}

bool isPHChanged()
{
	if ((ioctlInterval != DEFAULT_IOCTL_INTERVAL) ||
		(ioctlChannelName != DEFAULT_IOCTL_CHANNEL_NAME) ||
		(ioctlWarmstandbyGroup != DEFAULT_IOCTL_WSB_GROUPE) ||
		(ioctlDetectionTimeInterval != DEFAULT_IOCTL_DETECTION_TIME_INTERVAL) ||
		(ioctlDetectionTimeSchedule != DEFAULT_IOCTL_DETECTION_TIME_SCHEDULE) ||
		(ioctlFallBackWithinWSBGroup != DEFAULE_IOCTL_FALLBCK_WITHIN_WSB_GROUPE))
		return true;
	else
		return false;
}

bool isPHInfoChanged(const PreferredHostInfo& phInfo)
{
	return (ioctlEnablePH != phInfo.getEnablePreferredHostOptions()) ||
		(ioctlChannelName != phInfo.getPreferredChannelName()) ||
		(ioctlWarmstandbyGroup != phInfo.getPreferredWSBChannelName()) ||
		(ioctlDetectionTimeInterval != phInfo.getPHDetectionTimeInterval()) ||
		(ioctlDetectionTimeSchedule != phInfo.getPHDetectionTimeSchedule()) ||
		(ioctlFallBackWithinWSBGroup = phInfo.getPHFallBackWithInWSBGroup());
}

void printIOCtlData()
{
	cout << "   Ioctl call to update PreferredHostOptions." << endl;
	cout << "   Time interval: " << ioctlInterval << endl;
	cout << "   Remaining time: " << ioctlTime - time(NULL) << endl;
}

void printDirectFallbackData()
{
	cout << "   Direct Fallback." << endl;
	cout << "   Time interval: " << fallBackInterval << endl;
	cout << "   Remaining time: "<< fallBackTime - time(NULL) << endl;
}

int main( int argc, char* argv[] )
{ 
	int i = 1;

	while (i < argc)
	{
		if (strcmp(argv[i], "-ioctlInterval") == 0)
		{
			i++;
			ioctlInterval = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "-fallBackInterval") == 0)
		{
			i++;
			fallBackInterval = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "-ioctlEnablePH") == 0)
		{
			i++;
			if (strcmp(argv[i], "true") == 0)
			    ioctlEnablePH = true;
			else if (strcmp(argv[i], "false") == 0)
				ioctlEnablePH = false;
			else
			{
				cout << "Invalid argument for ioctlEnablePH: " << argv[i] << endl;
				exit(-1);
			}
		}
		else if (strcmp(argv[i], "-ioctlChannelName") == 0)
		{
			i++;
			ioctlChannelName = argv[i];
		}
		else if (strcmp(argv[i], "-ioctlWarmstandbyGroup") == 0)
		{
			i++;
			ioctlWarmstandbyGroup = argv[i];
		}
		else if (strcmp(argv[i], "-ioctlDetectionTimeInterval") == 0)
		{
			i++;
			ioctlDetectionTimeInterval = atoi(argv[i]);
		}
		else if (strcmp(argv[i], "-ioctlDetectionTimeSchedule") == 0)
		{
			ioctlDetectionTimeSchedule.clear();
			i++;

			if (argv[i] == NULL || *argv[i] == '-') { continue; }

			do {

				ioctlDetectionTimeSchedule += argv[i];

				if (argv[i + 1] != NULL && *argv[i + 1] != '-') ioctlDetectionTimeSchedule += " ";
				else break;

				i++;
			} while (1);
		}
		else if (strcmp(argv[i], "-ioctlFallBackWithinWSBGroup") == 0)
		{
			i++;
			if (strcmp(argv[i], "true") == 0)
				ioctlFallBackWithinWSBGroup = true;
			else if (strcmp(argv[i], "false") == 0)
				ioctlFallBackWithinWSBGroup = false;
			else
			{
				cout << "Invalid argument for ioctlFallBackWithinWSBGroup: " << argv[i] << endl;
				exit(-1);
			}
		}
		else
		{
			cout << "Invalid argument: " << argv[i] << endl;
			exit(-1);
		}
		i++;
	}

	if (ioctlInterval <= 0 && isPHChanged())
	{
		cout << "Invalid input. ioctlInterval must be greater than zero." << endl;
		exit(-1);
	}
	try { 
		AppClient client;
		PreferredHostOptions phOptions;
		ChannelInformation channelInfo;
		OmmConsumer consumer( OmmConsumerConfig().username( "user" ).consumerName("Consumer_9"), client);
		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("IBM.N"), client);

		bool printChannelInfo = true;
		bool isModifyIOCtlDone = false;
		bool isFallbackDone = false;

		if (ioctlInterval)
			ioctlTime = time(NULL) + (time_t)ioctlInterval;
		if(fallBackInterval)
			fallBackTime = time(NULL) + (time_t)fallBackInterval;

		for (int i = 1; i < 60; i++)
		{
			if (ioctlInterval >=2 && i % (ioctlInterval/2) == 0 && printChannelInfo)
			{
				printChannelInfo = false;
				consumer.getChannelInformation(channelInfo);
				cout<< channelInfo  << endl;
			}

			if (ioctlInterval > 0 && !isModifyIOCtlDone)
				printIOCtlData();

			if (ioctlInterval && i % ioctlInterval == 0 && !isModifyIOCtlDone )
			{
				if (isPHInfoChanged(channelInfo.getPreferredHostInfo()))
				{
					phOptions.enablePreferredHostOptions(ioctlEnablePH);
					phOptions.preferredWSBChannelName(ioctlWarmstandbyGroup);
					phOptions.preferredChannelName(ioctlChannelName);
					phOptions.phDetectionTimeInterval(ioctlDetectionTimeInterval);
					phOptions.phDetectionTimeSchedule(ioctlDetectionTimeSchedule);
					phOptions.phFallBackWithInWSBGroup(ioctlFallBackWithinWSBGroup);
					consumer.modifyReactorChannelIOCtl(IOCtlReactorChannelCode::ReactorChannelPreferredHost, (void*)&phOptions);
				}
				isModifyIOCtlDone = true;
			}

			if (fallBackInterval > 0 && !isFallbackDone)
				printDirectFallbackData();

			if (fallBackInterval && i % fallBackInterval == 0 && !isFallbackDone )
			{
				isFallbackDone = true;
				consumer.fallbackPreferredHost();
			}

			sleep(1000);
		}
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
