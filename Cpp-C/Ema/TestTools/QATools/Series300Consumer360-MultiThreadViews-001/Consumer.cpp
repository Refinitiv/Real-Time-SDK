///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

#include "Consumer.h"
//APIQA
#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>  
#include <string>
//END APIQA

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

//APIQA
std::mutex mtx;

AppClient::AppClient()
{
	_hasUpdateMsg = false;
	_hasRefreshMsg = false;
}

AppClient::AppClient(bool debug)
{
	_debug = debug;
	_hasUpdateMsg = false;
	_hasRefreshMsg = false;
}
//END QA

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& )
{
	//APIQA
	if (!_debug)
		return;

	mtx.lock();
	cout << "Thread id: " << this_thread::get_id() << endl;
	//END APIQA

	cout << endl << "Item Name: " << ( refreshMsg.hasName() ? refreshMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "<not set>" ) );

	cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;
	//APIQA
	mtx.unlock();
	//END APIQA

	if ( DataType::FieldListEnum == refreshMsg.getPayload().getDataType() )
		decode( refreshMsg.getPayload().getFieldList() );

	_hasRefreshMsg = true;
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& )
{
	//APIQA
	if (!_debug)
		return;

	mtx.lock();
	cout << "Thread id: " << this_thread::get_id() << endl;
	//END APIQA

	cout << endl << "Item Name: " << ( updateMsg.hasName() ? updateMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (updateMsg.hasServiceName() ? updateMsg.getServiceName() : EmaString( "<not set>" ) ) << endl;

	//APIQA
	mtx.unlock();
	//END APIQA

	if ( DataType::FieldListEnum == updateMsg.getPayload().getDataType() )
		decode( updateMsg.getPayload().getFieldList() );

	_hasUpdateMsg = true;
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& )
{
	//APIQA
	if (!_debug)
		return;

	mtx.lock();
	cout << "Thread id: " << this_thread::get_id() << endl;
	//END APIQA

	cout << endl << "Item Name: " << ( statusMsg.hasName() ? statusMsg.getName() : EmaString( "<not set>" ) ) << endl
		<< "Service Name: " << (statusMsg.hasServiceName() ? statusMsg.getServiceName() : EmaString( "<not set>" ) );

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;

	//APIQA
	mtx.unlock();
	//END APIQA
}

void AppClient::decode( const FieldList& fl )
{
	//APIQA
	if (!_debug)
		return;

	mtx.lock();
	cout << "Thread id: " << this_thread::get_id() << endl;
	//END APIQA

	while ( fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();
	
		cout << "Name: " << fe.getName() << " Value: ";

		if ( fe.getCode() == Data::BlankEnum )
			cout << " blank" << endl;
		else
			switch ( fe.getLoadType() )
			{
			case DataType::RealEnum :
				cout << fe.getReal().getAsDouble() << endl;
				break;
			case DataType::DateEnum :
				cout << (UInt64)fe.getDate().getDay() << " / " << (UInt64)fe.getDate().getMonth() << " / " << (UInt64)fe.getDate().getYear() << endl;
				break;
			case DataType::TimeEnum :
				cout << (UInt64)fe.getTime().getHour() << ":" << (UInt64)fe.getTime().getMinute() << ":" << (UInt64)fe.getTime().getSecond() << ":" << (UInt64)fe.getTime().getMillisecond() << endl;
				break;
			case DataType::IntEnum :
				cout << fe.getInt() << endl;
				break;
			case DataType::UIntEnum :
				cout << fe.getUInt() << endl;
				break;
			case DataType::AsciiEnum :
				cout << fe.getAscii() << endl;
				break;
			case DataType::EnumEnum :
				fe.hasEnumDisplay() ? cout << fe.getEnumDisplay() << endl : cout << fe.getEnum() << endl;
				break;
			case DataType::ErrorEnum :
				cout << "( " << fe.getError().getErrorCodeAsString() << " )" << endl;
				break;
			case DataType::RmtesEnum:
				cout << fe.getRmtes().toString() << endl;
				break;
			default :
				cout << endl;
				break;
			}
		//APIQA
		cout << "Thread id: " << this_thread::get_id() << endl;
		//END APIQA
	}

	//APIQA
	mtx.unlock();
	//END APIQA
}

// APIQA
class ConsThread
{
	public:
		ConsThread(ElementList& view, string uniqueName, int debug = true):_view(view), _uniqueName(uniqueName), _debug(debug)
		{
			_appClient = std::unique_ptr<AppClient>(new AppClient(_debug));
			_consumer = std::unique_ptr<OmmConsumer>(new OmmConsumer(OmmConsumerConfig().username("user")));
			_thrd = NULL;
			_requestMsg.clear();
			_itemHandle = 0;
		}

		~ConsThread() 
		{
			_thrd->join();
		}

		void run() 
		{
			_thrd = unique_ptr<thread>(new thread(&ConsThread::_run, this));
		}

	private:
		ElementList &_view;
		std::string _uniqueName;
		unique_ptr<OmmConsumer> _consumer;
		unique_ptr<AppClient> _appClient;
		unique_ptr<thread> _thrd;
		int _debug;
		ReqMsg _requestMsg;
		UInt64 _itemHandle;

		void _run() 
		{
			for (int i = 0; i < 1000; ++i)
			{
				_itemHandle = _consumer->registerClient(_requestMsg.serviceName("DIRECT_FEED").name("IBM.N").payload(_view), *_appClient);
				
				mtx.lock();
				cout << "itemHandle: " << _itemHandle << endl;
				mtx.unlock();

				if (_debug)
				{
					mtx.lock();
					cout << _uniqueName << "   registered - Trial " << i <<endl;
					mtx.unlock();
				}
				try {
					this_thread::sleep_for(std::chrono::seconds(i));
				}
				catch (const std::exception& excp)
				{
					mtx.lock();
					cout << excp.what() << endl;
					mtx.unlock();
				}

				_requestMsg.clear();
				try {
					_consumer->reissue(_requestMsg, _itemHandle);
				}
				catch (const OmmException& excp)
				{
					if (_debug)
					{
						mtx.lock();
						cout << excp << endl;
						mtx.unlock();
					}
				}

				try {
					this_thread::sleep_for(std::chrono::seconds(i));
				}
				catch (const std::exception& excp) {
					mtx.lock();
					cout << excp.what() << endl;
					mtx.unlock();
				}

				while(!(_appClient->_hasUpdateMsg && _appClient->_hasRefreshMsg))
					this_thread::sleep_for(std::chrono::seconds(1));

				_consumer->unregister(_itemHandle);
				_itemHandle = 0;
				_appClient->_hasUpdateMsg = false;
				_appClient->_hasRefreshMsg = false;

				if (_debug)
				{
					mtx.lock();

					cout << _uniqueName << "   unregistered - Trial " << i << endl;
					mtx.unlock();
				}
			}
		}
};


int main()
{ 
	try {
		ElementList view; 
		OmmArray array;

		array.fixedWidth(2);
		array.addInt(22).addInt(25).complete();
		
		view.addUInt(ENAME_VIEW_TYPE, 1).addArray(ENAME_VIEW_DATA, array).complete();
		
		ConsThread consumerThread_One(view, "A");
		consumerThread_One.run();
		ConsThread consumerThread_Two(view, "B");
		consumerThread_Two.run();
		ConsThread consumerThread_Three(view, "C");
		consumerThread_Three.run();
		ConsThread consumerThread_Four(view, "D");
		consumerThread_Four.run();
		ConsThread consumerThread_Five(view, "E");
		consumerThread_Five.run();
		ConsThread consumerThread_Six(view, "F");
		consumerThread_Six.run();
		ConsThread consumerThread_Seven(view, "G");
		consumerThread_Seven.run();
		sleep( 60000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}
//END APIQA
