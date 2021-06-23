///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2021 Refinitiv.      All rights reserved.          --
///*|-----------------------------------------------------------------------------

#include "AppUtil.h"
#include "NIProviderPerfClient.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

using namespace std;


NIProviderPerfClient::NIProviderPerfClient(NIProviderThread* _niProviderThread, NIProvPerfConfig& _niProvPerfConfig)
	: _bConnectionUp(false),
	niProviderThread(_niProviderThread),
	niProvPerfConfig(_niProvPerfConfig)
{}

NIProviderPerfClient::~NIProviderPerfClient() {}

bool NIProviderPerfClient::isConnectionUp() const
{
	return _bConnectionUp;
}

void NIProviderPerfClient::onRefreshMsg(const RefreshMsg& refreshMsg, const OmmProviderEvent& event)
{
	ChannelInformation channelInfo = event.getChannelInformation();

	cout << endl << "event channel info (refresh)" << endl << channelInfo
		<< endl << "Handle: " << event.getHandle();
	if (event.getClosure())
		cout << " Closure: " << event.getClosure() << endl;
	cout << endl << refreshMsg << endl;

	bool connectionUpOld = _bConnectionUp;

	if (channelInfo.getChannelState() == ChannelInformation::ActiveEnum)
	{
		_bConnectionUp = true;
	}
	else
	{
		_bConnectionUp = false;
	}

	if (_bConnectionUp != connectionUpOld)
		cout << "RefreshMsg. isConnectionUp = " << (connectionUpOld ? "True" : "False") << " -> " << (_bConnectionUp ? "True" : "False") << endl;
}

void NIProviderPerfClient::onStatusMsg(const StatusMsg& statusMsg, const OmmProviderEvent& event)
{
	niProviderThread->getProviderStats().statusCount.countStatIncr();

	ChannelInformation channelInfo = event.getChannelInformation();

	cout << endl << "event channel info (status)" << endl << channelInfo
		<< endl << "Handle: " << event.getHandle();
	if (event.getClosure())
		cout << " Closure: " << event.getClosure() << endl;
	cout << endl << statusMsg << endl;

	bool connectionUpOld = _bConnectionUp;

	if (channelInfo.getChannelState() == ChannelInformation::ActiveEnum)
	{
		_bConnectionUp = true;
	}
	else
	{
		_bConnectionUp = false;
	}

	if (_bConnectionUp != connectionUpOld)
		cout << "StatusMsg. isConnectionUp = " << (connectionUpOld ? "True" : "False") << " -> " << (_bConnectionUp ? "True" : "False") << endl;
}

// called when a client disconnects or when an item is unregistered.
void NIProviderPerfClient::onClose(const ReqMsg& reqMsg, const OmmProviderEvent& event)
{
	cout << endl << "event channel info (close)" << endl << event.getChannelInformation()
		<< endl << reqMsg << endl;
	_bConnectionUp = false;

	niProviderThread->getProviderStats().closeMsgCount.countStatIncr();
}
