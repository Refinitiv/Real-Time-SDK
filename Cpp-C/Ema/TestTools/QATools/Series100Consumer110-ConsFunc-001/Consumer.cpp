///*|-----------------------------------------------------------------------------A
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------
//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Consumer.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;
using namespace std;

void AppClient::onRefreshMsg( const RefreshMsg& refreshMsg, const OmmConsumerEvent& )
{
    if ( refreshMsg.hasMsgKey() )
	cout << "Item Name: " << refreshMsg.getName() << endl << "Service Name: " << ( refreshMsg.hasServiceName() ? refreshMsg.getServiceName() : EmaString( "not set" ) ) << endl;

    cout << endl << "Item State: " << refreshMsg.getState().toString() << endl;

    if ( DataType::FieldListEnum == refreshMsg.getPayload().getDataType() )
        decode( refreshMsg.getPayload().getFieldList() );
	if ( DataType::ElementListEnum == refreshMsg.getPayload().getDataType() )
		decodeEList( refreshMsg.getPayload().getElementList() );
	if ( DataType::MapEnum == refreshMsg.getPayload().getDataType() )
		decodeM( refreshMsg.getPayload().getMap() );
	if ( DataType::OpaqueEnum == refreshMsg.getPayload().getDataType() )
		         decode( refreshMsg.getPayload().getOpaque() );

}

void AppClient::onUpdateMsg( const UpdateMsg& updateMsg, const OmmConsumerEvent& ommEvent )
{
    if ( updateMsg.hasMsgKey() )
	        cout << endl << "Item Name: " << updateMsg.getName() << endl << "Service Name: " << updateMsg.getServiceName() << endl;

			    if ( DataType::FieldListEnum == updateMsg.getPayload().getDataType() )
				        decode( updateMsg.getPayload().getFieldList() );
	            if ( DataType::ElementListEnum == updateMsg.getPayload().getDataType() )
						decodeEList( updateMsg.getPayload().getElementList() );
				if (  DataType::MapEnum == updateMsg.getPayload().getDataType() )
		                decodeM( updateMsg.getPayload().getMap() );
                if ( DataType::OpaqueEnum == updateMsg.getPayload().getDataType() )
				        decode( updateMsg.getPayload().getOpaque() );

}

void AppClient::onStatusMsg( const StatusMsg& statusMsg, const OmmConsumerEvent& )
{
	if ( statusMsg.hasMsgKey() )
		cout << endl << "Item Name: " << statusMsg.getName() << endl << "Service Name: " << statusMsg.getServiceName();

	if ( statusMsg.hasState() )
		cout << endl << "Item State: " << statusMsg.getState().toString() << endl;
}

void AppClient::decodeM( const Map& map )
{
	if (  map.getSummaryData().getDataType() == DataType::FieldListEnum )
	{
		cout << "Map Summary data:" << endl;
		decode( map.getSummaryData().getFieldList() );
	}
		
	while ( map.forth() )
	{
		const MapEntry& me = map.getEntry();

        if ( me.getKey().getDataType() == DataType::BufferEnum )
            cout << "Action: " << me.getMapActionAsString() << " key value: " << me.getKey().getBuffer() << endl;

		if ( me.getLoadType() == DataType::FieldListEnum )
		{
			cout << "Entry data:" << endl;
			decode( me.getFieldList() );
		}
	}
}
void AppClient::decode( const OmmOpaque& oq )
{
    cout <<  "OmmOpaque data: " << oq.getAsHex() << endl;
}

void AppClient::decode( const FieldList& fl )
{
	while ( fl.forth() )
	{
		const FieldEntry& fe = fl.getEntry();

		cout << "Fid: " << fe.getFieldId() << " Name = " << fe.getName() << " DataType: " << fe.getLoad().getDataType()  << " Value: ";

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
			case DataType::ErrorEnum :
			  cout << fe.getError().getErrorCode() << "( " << fe.getError().getErrorCodeAsString() << " )" << endl;
				break;
			case DataType::EnumEnum :
				cout << fe.getEnum() << endl;
				break;
			default :
				cout << endl;
				break;
			}
	}
}


void AppClient::decodeEList( const ElementList& el )
{
	while ( el.forth() )
	{
		const ElementEntry& ee = el.getEntry();
	     cout << "ElementEntry Data  " << ee.getAscii() << endl;
	}
}




int main( int argc, char* argv[] )
{ 
	if(argc != 2 && argc != 5)
	{
		cout<<"Error: Invalid number of arguments"<<endl;
		cout<<"Usage: "<<argv[0]<<" -m 1-99(testcase 1 or 2 or 3 or 4 ...) -user userName"<<endl;
		return -1;
	}
 
	try { 
		AppClient client;
		AppClient client2;
		OmmConsumerConfig cc;
		OmmConsumerConfig cc1;
		EmaString sName( "DIRECT_FEED" );
		EmaString YSName( "ATS201_1" );
        EmaString loginuser1("user");
        EmaString loginuser2("user2");
		cout << "YSName is caseInsensitiveCompare is with ATS201_1 ats201_1 " << YSName.caseInsensitiveCompare("ays201_1") <<endl;
		
		
		if(strcmp(argv[3], "-user") == 0)
		{
			loginuser1.clear();
			loginuser1.append(argv[4]);
		    cc.host("localhost:14002");
		    cc1.host("localhost:14002");
			cc.username(loginuser1);
			cc1.username(loginuser2);
			cc.applicationId("256");
			sName.clear();
			sName.append("DIRECT_FEED");
			cout << "serviceName use will be" <<sName.c_str() << endl;
		}
		else
		{
		    sleep(1000);
		    cc.host("localhost:14002");
			sName.clear();
			sName.append("DIRECT_FEED");
			cout << "serviceName use will be" <<sName.c_str() << endl;
		}
		OmmConsumer * consumer = new OmmConsumer(cc);
		void* closure4 = (void*)1;
		void* closure1 = (void*)1;
		void* closure2 = (void*)1;
		void* closure3 = (void*)1;
		void* closure5 = (void*)1;
		void* closure6 = (void*)1;
		void* closure7 = (void*)1;
		int temp = 0; 
		if(strcmp(argv[1], "-m") == 0 || strcmp(argv[1], "-M") == 0)
		{
			temp = atoi(argv[2]);
			if(temp < 0 )
			UInt64 h3 = consumer->registerClient(ReqMsg().serviceName( sName.c_str() ).name( "IBM.N" ), client,  closure3 );
			else if(temp == 1 )
	        { 
		     UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" )
   		     .payload( ElementList().addUInt( ":ViewType", 1 ).
   		     addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 22 ).addInt( 25 ).complete() ).complete() ), client,  closure1 );
   		     UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure2 );
        	 UInt64 h3 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "IBM.N" ), client,  closure3 );
        	 sleep(600);
        	 consumer->unregister(h2);
			}
		else if(temp == 2 )
	    { 
		    UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" )
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 22 ).addInt( 25 ).complete() ).complete() ), client,  closure1 );
		    UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" )
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 25 ).addInt( 22 ).complete() ).complete() ), client,  closure2 );
        	UInt64 h3 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "IBM.N" ), client,  closure3 );
        	sleep(600);
        }
		else if(temp == 3 )
	    { 
		    UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" )
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 22 ).addInt( 6 ).complete() ).complete() ), client,  closure1 );
		    UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" )
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 25 ).addInt( 32 ).complete() ).complete() ), client,  closure2 );
        	UInt64 h3 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "IBM.N" ), client,  closure3 );
		    UInt64 h4 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" )
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 6 ).addInt( 11 ).addInt( 25 ).complete() ).complete() ), client,  closure4 );
        }
		else if(temp == 4 )
	    { 
		    UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" )
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 22 ).addInt( 6 ).complete() ).complete() ), client,  closure1 );
		    UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" )
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 25 ).addInt( 32 ).complete() ).complete() ), client,  closure2 );
        	UInt64 h3 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "IBM.N" ), client,  closure3 );
		    UInt64 h4 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ).interestAfterRefresh(false), client,  closure4);

        }
		else if(temp == 5 )
	    { 
		    UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" )
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 22 ).addInt( 25 ).complete() ).complete() ), client,  closure1 );
        	sleep(600);
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N").initialImage(false)
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 22 ).addInt( 25 ).complete() ).complete() ), h1 );
        }
		else if(temp == 6 )
	    { 
		    UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "CSCO.O" )
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 22 ).addInt( 25 ).complete() ).complete() ), client,  closure1 );
        	UInt64 h3 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "SPOT" ), client,  closure3 );
        	sleep(600);
			cout << "PAUSE NOW" << endl;
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "SPOT").pause(true).initialImage(false),h3);
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "CSCO.O").pause(true).initialImage(false)
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 22 ).addInt( 25 ).complete() ).complete() ), h1 );
			sleep(1600);
			cout << "RESUME  NOW" << endl;
	        
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "SPOT").pause(false).initialImage(false),h3);
	
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "CSCO.O").pause(false).initialImage(true),h1);
		}
		else if(temp == 7 )
	    { 
		    UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TALK" ).domainType(200)
			.payload( ElementList().addUInt( ":ViewType", 2 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 4 ).addAscii( "Data" ).addAscii( "Lata" ).complete() ).complete() ), client,  closure1 );
			sleep(1600);
            cout << "Reissue with  full view after 1.6 seconds"<<endl;
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TALK" ).domainType(200), h1);
        }
		else if(temp == 8 )
	    {
		   cout << "Snap TRI.N full, view TRI.N(22,25), after 1.6 seocnds snap request withview TRI.N(6,25),  Final shoud be view with 22,25 only on h1" << endl;
		    UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ).interestAfterRefresh(false), client,  closure2 );
		    UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" )
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 22 ).addInt( 25 ).complete() ).complete() ), client,  closure1 );
             
			sleep(1600);
		    UInt64 h3 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ).interestAfterRefresh(false)
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 6 ).addInt( 25 ).complete() ).complete() ), client,  closure3 );
			  
        }
		else if(temp == 9 )
	    { 
            cout << "SEND REQUEST FOR two TRI.N different Handles" << endl;
		    UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure1 );
        	UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure2 );
        	sleep(1600);
			cout << "PAUSE NOW h1" << endl;
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N").pause(true).initialImage(false),h1);
			cout << "wait for some time" << endl;
			sleep(1600);
			cout << "PAUSE NOW h2" << endl;
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N").pause(true).initialImage(false),h2);
			cout << "STOP recieve updates" << endl;
			sleep(1600);
			cout << "RESUME  NOW h1" << endl;
			
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N").pause(false).initialImage(false),h1);
	        sleep(1600);
			cout << "RESUME  NOW h2" << endl;
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N").pause(false).initialImage(false),h2);
		}

		else if(temp == 10 )
	    { 
            cout << "SEND REQUEST FOR two TRI.N different Handles" << endl;
			
		    UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure1 );
        	UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure2 );
        	sleep(1600);
			cout << "PAUSE NOW h1" << endl;
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N").pause(true).initialImage(false),h1);
			cout << "wait for some time" << endl;
			sleep(1600);
			cout << "PAUSE NOW h2" << endl;
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N").pause(true).initialImage(false),h2);
			cout << "STOP recieve updates" << endl;
			sleep(1600);
			cout << "RESUME  NOW h1" << endl;
			
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N").pause(false).initialImage(true),h1);
	        sleep(1600);
			cout << "RESUME  NOW h2" << endl;
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N").pause(false).initialImage(false),h2);
		}

		else if(temp == 11 )
	    { 
            cout << "SEND REQUEST FOR two TRI.N different Handles" << endl;
		    UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure1 );
        	UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure2 );
        	sleep(20000);
		}
		else if(temp == 12 )
	    { 
            cout << "SEND REQUEST FOR two TRI.N different Handles" << endl;
		    UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure1 );
        	UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client2,  closure2 );
        	cout << "unregister handle h2 after 10 seconds" << endl;
			sleep(10000);
			cout << "unregister handle h2 after now" << endl;
			consumer->unregister(h2);
		}
		else if(temp == 13 )
	    { 
            cout << "SEND REQUEST FOR RES-DS needs to run directConnect rsslProvider" << endl;
		    UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "RES-DS" ), client,  closure1 );
			sleep (1000);
			cout << "SEND REQUEST FOR RES-DS as private" << endl;

		    UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "RES-DS" ).privateStream(true), client,  closure2 );
		}
		else if(temp == 16 )
	    { 
            cout << "SEND REQUEST FOR Different DOMAIN needs to do with live ELEKTRON FEED "<< endl;
			UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure1 );
			UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "AGG.V" ).domainType(7), client,  closure2 );
			UInt64 h3 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "BBH.ITC" ).domainType(8), client,  closure3 );
			UInt64 h4 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRBI.PQ" ).domainType(9), client,  closure4 );
			UInt64 h5 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "_ADS_CACHE_LIST" ).domainType(10), client,  closure5 );
            UInt64 h6 = consumer->registerClient( ReqMsg().serviceName( YSName.c_str() ).name( "BASIC" ).domainType(22), client,  closure6 );
            UInt64 h7 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "CSCO.O" ).interestAfterRefresh(false), client,  closure7 );
		}	
		else if(temp == 17 )
	    { 
            cout << "SEND REQUEST FOR TRI.N, TRI.N , A.N live ELEKTRON FEED "<< endl;
			UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure1 );
            UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "A.N" ).interestAfterRefresh(true), client,  closure2 );
			UInt64 h3 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure3 );
		}
		else if(temp == 18 )
	    { 
            cout << "SEND REQUEST FOR SPOT live ELEKTRON FEED with request for MSGKey in update flag set"<< endl;
			UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "SPOT" ), client,  closure1 );
		}
		else if(temp == 19 )
	    { 
            cout << "SEND REQUEST FOR TRI.N, TRI.N , A.N live ELEKTRON FEED "<< endl;
			UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure1 );
            UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "A.N" ).interestAfterRefresh(true), client,  closure2 );
			UInt64 h3 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure3 );
			sleep(5000);
            cout << "Unreigster handle h1 should send priority change on network  "<< endl;
			consumer->unregister(h1);
		}
		else if(temp == 20 )
	    { 
            cout << "SEND REQUEST FOR TRI.N, TRI.N , A.N live ELEKTRON FEED  Reissue on TRI.N h1 with refresh request"<< endl;
			UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure1 );
            UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "A.N" ).interestAfterRefresh(true), client,  closure2 );
			UInt64 h3 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure3 );
			sleep(5000);
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N").initialImage(true),h1);
            cout << "Reissue on handle h1 should refresh for h1  "<< endl;
		}
		else if(temp == 21 )
	    { 
            cout << "SEND REQUEST FOR TRI.N MarkteByPrice from sds direct connect use option -direct  Reissue on TRI.N h1 with refresh request"<< endl;
			UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ).domainType(8), client,  closure1 );
            UInt64 h2 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ).domainType(8), client,  closure2 );
			sleep(1000);
			cout << "REISSUE ONE H1 now" << endl;
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N").initialImage(true),h1);
		}
	    else if(temp == 22 )
	    { 
            cout << "SEND REQUEST FOR SPOT live ELEKTRON FEED with request for MSGKey in update flag set channel config in EMACOnfig.xml should had  MsgKeyInUpdates  "<< endl;
			UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "SPOT" ), client,  closure1 );
		}
		else if(temp == 23 )
	    { 
		     UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure1 );
        	cout<< "wait" << endl;
			sleep(6000);
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N").initialImage(true)
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 22 ).addInt( 25 ).complete() ).complete() ), h1 );
		}
		else if(temp == 24 )
	    { 
			cout << "LOGIN REISSUE WITH PAUSE ALL and RESUMEALL " << endl;
		    UInt64 login_handle = consumer->registerClient( ReqMsg().domainType( MMT_LOGIN ), client, closure2 );
			UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName ).name( "SPOT" ), client,  closure1 );
			cout<< "wait 6 seconds" << endl;
			sleep(6000);
			cout<< "PAUSE ALL" << endl;
		    consumer->reissue( ReqMsg().domainType( MMT_LOGIN ).initialImage( false ).pause( true ), login_handle );
		    cout<< "wait 20 seconds" << endl;
		    sleep(20000);
		    cout<< "RESUME ALL" << endl;
			consumer->reissue( ReqMsg().domainType( MMT_LOGIN ).initialImage( false ).pause( false ), login_handle );
         }
		else if(temp == 25 )
	    { 
		    OmmConsumer * consumer1 = new OmmConsumer(cc1);
		    cout << "LOGIN Multiple handles with dacs enable ads user1 has permission user2 don't  " << endl;
		    UInt64 login_handle1 = consumer->registerClient( ReqMsg().domainType( MMT_LOGIN ), client, closure2 );
		    UInt64 login_handle2 = consumer1->registerClient( ReqMsg().domainType( MMT_LOGIN ), client2, closure3 );
			UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName ).name( "SPOT" ), client,  closure1 );
			UInt64 h2 = consumer1->registerClient( ReqMsg().serviceName( sName ).name( "TRI.N" ), client2,  closure4 );
         }
		else if(temp == 26 )
	    { 
		    OmmConsumer * consumer1 = new OmmConsumer(cc1);
			cout << "LOGIN Multiple handles with dacs enable ads user1 has permission EFG don't  " << endl;
		    UInt64 login_handle1 = consumer->registerClient( ReqMsg().domainType( MMT_LOGIN ), client, closure2 );
		    UInt64 login_handle2 = consumer1->registerClient( ReqMsg().domainType( MMT_LOGIN ), client2, closure3 );
			UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName ).name( "CSCO.O" ), client,  closure1 );
			UInt64 h2 = consumer1->registerClient( ReqMsg().serviceName( sName ).name( "TRI.N" ), client2,  closure4 );
         }
         else if(temp == 27 )
        {
            cout << "LOGIN handles unregister  unregister client will not clean login to ADS login will be clean when consumer is unregister " << endl;
            UInt64 login_handle1 = consumer->registerClient( ReqMsg().domainType( MMT_LOGIN ), client, closure2 );
            UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName ).name( "SPOT" ), client,  closure1 );
            sleep(2000);
            cout << "UNREGISTER H1 which is using open for SPOT" << endl;
            consumer->unregister(h1);
            sleep(1000);
            cout << endl << "UNREGISTER login handle which is using " << loginuser1.c_str() << endl;
            consumer->unregister(login_handle1);
            sleep(1000);
         } 
         else if(temp == 28 )
        {
            cout << "Two LOGIN handles  test item fanout" << endl;
            UInt64 login_handle1 = consumer->registerClient( ReqMsg().domainType( MMT_LOGIN ).name( loginuser2 ), client, closure2 );
            UInt64 login_handle2 = consumer->registerClient( ReqMsg().domainType( MMT_LOGIN ), client, closure3 );
            UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName ).name( "SPOT" ), client,  closure1);
			sleep(2000);
            cout << "UNREGISTER login handle which is using " << loginuser1.c_str() << endl;
            consumer->unregister(login_handle2);
			sleep(2000);
            cout << "UNREGISTER login handle which is using " << loginuser2.c_str() << endl;
            consumer->unregister(login_handle1);
         } 
           
		else if(temp == 29 )
	    { 
		     UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" ), client,  closure1 );
        	cout<< "wait" << endl;
			sleep(6000);
			consumer->reissue( ReqMsg().serviceName( sName.c_str() ).name( "TRI.NN").initialImage(true)
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 22 ).addInt( 25 ).complete() ).complete() ), h1 );
		}
        else if(temp == 30 )
        {
		    UInt64 login_handle = consumer->registerClient( ReqMsg().domainType( MMT_LOGIN ).name( loginuser1 ), client, closure2 );
            UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "TRI.N" )
            .payload( ElementList().addUInt( ":ViewType", 1 ).
            addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 22 ).addInt( 25 ).complete() ).complete() ), client,  closure1 );

            cout << "LOGIN REISSUE WITH PAUSE ALL and RESUMEALL " << endl;
            cout<< "wait 2 seconds" << endl;
            sleep(2000);
		    consumer->reissue( ReqMsg().domainType( MMT_LOGIN ).initialImage( false ).pause( true ), login_handle );
        cout<< "wait 5 seconds" << endl;
        sleep(5000);
        consumer->reissue( ReqMsg().domainType( MMT_LOGIN ).initialImage( false ).pause( false ).name( "apiqa" ), login_handle );
         }
		else if(temp == 99 )
	    { 
		    UInt64 h1 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "IBM.N" )
   		    .payload( ElementList().addUInt( ":ViewType", 1 ).
   		    addArray( ":ViewData", OmmArray().fixedWidth( 2 ).addInt( 2 ).addInt( 3 ).addInt( 4 ).addInt( 6 ).addInt( 7 ).addInt( 8 ).addInt( 9 ).addInt( 10 ).addInt( 11 ).addInt( 12 ).addInt( 13 ).addInt( 14 ).addInt( 15 ).addInt( 16 ).addInt( 18 ).addInt( 19 ).addInt( 21 ).addInt( 22 ).addInt( 23 ).addInt( 24 ).addInt( 25 ).addInt( 26 ).addInt( 27 ).addInt( 28 ).addInt( 29 ).addInt( 30 ).addInt( 31 ).addInt( 32 ).addInt( 33 ).addInt( 34 ).addInt( 35 ).addInt( 36 ).addInt( 37 ).addInt( 38 ).addInt( 39 ).addInt( 40 ).addInt( 42 ).addInt( 43 ).addInt( 53 ).addInt( 56 ).addInt( 58 ).addInt( 60 ).addInt( 61 ).addInt( 71 ).addInt( 75 ).addInt( 76 ).addInt( 77 ).addInt( 78 ).addInt( 79 ).addInt( 90 ).addInt( 91 ).addInt( 100 ).addInt( 104 ).addInt( 105 ).addInt( 110 ).addInt( 111 ).addInt( 117 ).addInt( 118 ).addInt( 131 ).addInt( 178 ).addInt( 198 ).addInt( 259 ).addInt( 293 ).addInt( 296 ).addInt( 340 ).addInt( 350 ).addInt( 351 ).addInt( 372 ).addInt( 373 ).addInt( 374 ).addInt( 375 ).addInt( 376 ).addInt( 377 ).addInt( 378 ).addInt( 379 ).addInt( 728 ).addInt( 869 ).addInt( 998 ).addInt( 999 ).addInt( 1000 ).addInt( 1001 ).addInt( 1002 ).addInt( 1003 ).addInt( 1021 ).addInt( 1023 ).addInt( 1025 ).addInt( 1041 ).addInt( 1042 ).addInt( 1043 ).addInt( 1044 ).addInt( 1055 ).addInt( 1056 ).addInt( 1067 ).addInt( 1075 ).addInt( 1076 ).addInt( 1080 ).addInt( 1379 ).addInt( 1383 ).addInt( 1392 ).addInt( 1404 ).addInt( 1465 ).addInt( 1501 ).addInt( 1642 ).addInt( 1709 ).addInt( 2326 ).complete() ).complete() ), client,  closure1 );

        	sleep(600);
        }
		else
			UInt64 h3 = consumer->registerClient( ReqMsg().serviceName( sName.c_str() ).name( "IBM.N" ), client,  closure3 );
	    }
		sleep( 360000 );				// API calls processRespMSg() with received messages
	} catch ( const OmmException& excp  ) {
		cout << excp << endl;
	}
	return 0;
}
// END APIQA
