/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2018-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Consumer.h"

#include <stdlib.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <string.h>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& ) 
{
	cout << refreshMsg << endl;		// defaults to refreshMsg.toString()
}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ) 
{
	cout << "updateMsg" << endl;		// defaults to updateMsg.toString()
}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& ) 
{
	cout << statusMsg << endl;		// defaults to statusMsg.toString()
}
  
vector<int> getView() {
  int size(rand() % 20 + 1);
  vector<int> view;
  for (int i = 0; i < size; ++i)
	view.push_back(rand() % 31 + 10);
  return view;
}

vector<string>getElementView() { // string of fid names
  static vector<string>stringFids;
  if (stringFids.empty()) {
	stringFids.push_back("BID");
	stringFids.push_back("ASK");
	stringFids.push_back("DSPLY_NAME");
	stringFids.push_back("RDN_EXCHD2");
	stringFids.push_back("TRDPRC_1");
	stringFids.push_back("TRDPRC_2");
	stringFids.push_back("TRDPRC_3");
	stringFids.push_back("TRDPRC_4");
	stringFids.push_back("TRDPRC_5");
	stringFids.push_back("HIGH_1");
	stringFids.push_back("LOW_1");
	stringFids.push_back("CURRENCY");
	stringFids.push_back("TRADE_DATE");
	stringFids.push_back("NEWS");
	stringFids.push_back("NEWS_TIME");
	stringFids.push_back("BIDSIZE");
	stringFids.push_back("ASKSIZE");
	stringFids.push_back("ACVOL_1");
	stringFids.push_back("BLKCOUNT");
	stringFids.push_back("BLKVOLUM");
	stringFids.push_back("OPEN_BID");
	stringFids.push_back("OPEN_ASK");
	stringFids.push_back("CLOSE_BID");
	stringFids.push_back("CLOSE_ASK");
	stringFids.push_back("YIELD");
	stringFids.push_back("EARNINGS");
	stringFids.push_back("PERATIO");
	stringFids.push_back("DIVIDENDTP");
	stringFids.push_back("DIVPAYDATE");
	stringFids.push_back("EXDIVDATE");
	stringFids.push_back("CTS_QUAL");
  }

  int size(rand() % 20 + 1);
  vector<string> view;
  int stringFidsSize(stringFids.size());
  for (int i = 0; i < size; ++i)
	view.push_back(stringFids.at(rand() % stringFidsSize));
  return view;
}

int main( int argc, char* argv[] ) {
  bool sendingFidsAsStrings(false);
  for (int i = 1; i < argc; ++i)
	if (strcmp(argv[i], "-e") == 0)
	  sendingFidsAsStrings = true;
	else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0) {
	  cout << "USAGE: " << argv[0] <<
		" [-eh?]\n\tspecify -e to send element lists; otherwise field lists will be sent" << endl;
	  return(0);
	}
	else {
	  cout << "unexpected argument: " << argv[i] << "\nUSAGE: " << argv[0]
		   << " [-eh?]\n\tspecify -e to send element lists; otherwise field lists will be sent" << endl;
	  return(1);
	}

  srand(time(0));

  try { 
		AppClient client;
		OmmConsumer consumer( OmmConsumerConfig("EmaConfig.xml").username( "user" ) );		

		ElementList view;
		OmmArray array;
		ReqMsg reqMsg;
		long handle;

		int requestCount = 0;
		int reissueCount = 0;

		vector<long> handles;
		vector<int> fids;
		vector<string> fidsAsStrings;
		int events(0);
		while (events < 60) {
		  int selector = rand() % 10; // 0, 1, ...9
		  if ( selector < 2) { // unregister
			if (handles.size() < 3)
			  continue;
			++events;
			int itemToUnregister = rand() % handles.size();
			cout<< "event: removing handle " << handles[itemToUnregister] << endl;
			consumer.unregister(handles[itemToUnregister]);
			vector<long>::iterator I = find<vector<long>::iterator, long>(handles.begin(), handles.end(),
																		  handles[itemToUnregister]);
			if ( I != handles.end() )
			  handles.erase(I);
			sleep(10000);
		  }

		  else if ( selector < 4) { // reissue
			if (handles.empty())
				continue;
			++events;
			array.clear();
			reqMsg.clear();

			if (sendingFidsAsStrings) {
			  fidsAsStrings = getElementView();
			  cout << "event: requesting fids for reissue " << ++requestCount << ": ";
			  for ( vector<string>::const_iterator I = fidsAsStrings.begin(); I != fidsAsStrings.end(); ++I ) {
				array.addAscii(static_cast<EmaString>(I->c_str()));
				cout << *I << ' ';
			  }
			  cout << endl;
			}
			else {
			  array.fixedWidth(2);
			  fids = getView();
			  cout << "event: requesting fids for reissue " << ++reissueCount << ": ";
			  for ( vector<int>::iterator I = fids.begin(); I != fids.end(); ++I ) {
				array.addInt(*I);
				cout << *I << ' ';
			  }
			  cout << endl;
			}
			array.complete();
			view.clear().addUInt( ENAME_VIEW_TYPE, (sendingFidsAsStrings ? 2 : 1) ).
			  addArray( ENAME_VIEW_DATA, array ).complete();
			reqMsg.clear().serviceName("ELEKTRON_DD").name("TRI.N").interestAfterRefresh(true).payload(view);
			int itemToReissue = rand() % handles.size();
			cout << "event: reissue for handle " << handles[itemToReissue] << endl;
			consumer.reissue( reqMsg, handles[itemToReissue] );
			sleep(10000);
		  }
		  else {				// registerClient
			++events;
			array.clear();
			if (sendingFidsAsStrings) {
			  fidsAsStrings = getElementView();
			  cout << "event: requesting fids for request " << ++requestCount << ": ";
			  for ( vector<string>::const_iterator I = fidsAsStrings.begin(); I != fidsAsStrings.end(); ++I ) {
				array.addAscii(static_cast<EmaString>(I->c_str()));
				cout << *I << ' ';
			  }
			}
			else {
			  array.fixedWidth(2);
			  fids = getView();
			  cout << "event: requesting fids for request " << ++requestCount << ": ";
			  for ( vector<int>::iterator I = fids.begin(); I != fids.end(); ++I ) {
				array.addInt(*I);
				cout << *I << ' ';
			  }
			}
			cout << endl;
			array.complete();
			view.clear().addUInt( ENAME_VIEW_TYPE, (sendingFidsAsStrings ? 2 : 1) ).
			  addArray( ENAME_VIEW_DATA, array ).complete();
			reqMsg.clear().serviceName("ELEKTRON_DD").name("TRI.N").interestAfterRefresh(true).payload(view);
			handle = consumer.registerClient(reqMsg, client);
			cout << "event: handle " << handle << " created" << endl;
			handles.push_back(handle);
			sleep(10000);
		  }
		}

		for (int i = 0; i < handles.size(); ++i) {
		  cout << "event: removing handle (end) " << handles[i] << endl;
		  consumer.unregister(handles[i]);
		  sleep(10000);
		}

		sleep( 10000 );				// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
  } catch ( const OmmException& excp ) {
	cout << excp << endl;
  }
  return 0;
}
