///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
///*|-----------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

#include "Consumer.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

EmaString ric1("IBM.N");
EmaString ric2("MSFT.O");
EmaString ric3("GOOG.O");
EmaString ric4("TRI.N");
EmaString ric5("GAZP.MM");
EmaString ric6(".09IY");
EmaString ric7(".TRXFLDESP");
EmaString ric8("A3M.MC");
EmaString ric9("ABE.MC");
EmaString ric10("ACS.MC");
OmmArray* batchReqPayload = new OmmArray();
ReqMsg* batchReqMsg = new ReqMsg();

void AppClient::onAllMsg(const Msg& msg, const OmmConsumerEvent& ev)
{
}



void iter(OmmConsumer& consumer, AppClient& client) {

	ReqMsg req;
	req.serviceName("DIRECT_FEED")
		.payload(ElementList().addArray(ENAME_BATCH_ITEM_LIST, OmmArray()
			.addAscii("IBM.N").addAscii("MSFT.O").addAscii("GOOG.O").addAscii("TRI.N").addAscii("GAZP.MM").addAscii(".09IY").addAscii(".TRXFLDESP").addAscii("A3M.MC").addAscii("ABE.MC").addAscii("ACS.MC")
			.complete())
			.complete())
		.interestAfterRefresh(false);

	UInt64 handle = consumer.registerClient(req, client, (void*)1);
}

void iter1(OmmConsumer& consumer, AppClient& client) {

	//reqMsg reuse, OmmArray reuse, EmaString reuse
	batchReqMsg->clear();
	batchReqPayload->clear();
	batchReqMsg->serviceName("DIRECT_FEED")
		.payload(ElementList().addArray(ENAME_BATCH_ITEM_LIST, batchReqPayload->
			addAscii(ric1).addAscii(ric2).addAscii(ric3).addAscii(ric4).addAscii(ric5).addAscii(ric6).addAscii(ric7).addAscii(ric8).addAscii(ric9).addAscii(ric10)
			.complete())
			.complete())
		.interestAfterRefresh(false);

	UInt64 handle1 = consumer.registerClient(*batchReqMsg, client, (void*)1);
}

void iter2(OmmConsumer& consumer, AppClient& client) {

	//reqMsg reuse, EmaString reuse, not use batch
	batchReqMsg->clear();
	batchReqPayload->clear();
	batchReqMsg->serviceName("DIRECT_FEED").name(ric1).interestAfterRefresh(false);
	consumer.registerClient(*batchReqMsg, client, (void*)1);
	batchReqMsg->serviceName("DIRECT_FEED").name(ric2).interestAfterRefresh(false);
	consumer.registerClient(*batchReqMsg, client, (void*)1);
	batchReqMsg->serviceName("DIRECT_FEED").name(ric3).interestAfterRefresh(false);
	consumer.registerClient(*batchReqMsg, client, (void*)1);
	batchReqMsg->serviceName("DIRECT_FEED").name(ric4).interestAfterRefresh(false);
	consumer.registerClient(*batchReqMsg, client, (void*)1);
	batchReqMsg->serviceName("DIRECT_FEED").name(ric5).interestAfterRefresh(false);
	consumer.registerClient(*batchReqMsg, client, (void*)1);
	batchReqMsg->serviceName("DIRECT_FEED").name(ric6).interestAfterRefresh(false);
	consumer.registerClient(*batchReqMsg, client, (void*)1);
	batchReqMsg->serviceName("DIRECT_FEED").name(ric7).interestAfterRefresh(false);
	consumer.registerClient(*batchReqMsg, client, (void*)1);
	batchReqMsg->serviceName("DIRECT_FEED").name(ric8).interestAfterRefresh(false);
	consumer.registerClient(*batchReqMsg, client, (void*)1);
	batchReqMsg->serviceName("DIRECT_FEED").name(ric9).interestAfterRefresh(false);
	consumer.registerClient(*batchReqMsg, client, (void*)1);
	batchReqMsg->serviceName("DIRECT_FEED").name(ric10).interestAfterRefresh(false);
	consumer.registerClient(*batchReqMsg, client, (void*)1);
}

void test()
{
	try {
		AppClient client;
        OmmConsumer consumer( OmmConsumerConfig().username( "rmds" ) );
		cout << "********************************" << endl;
		
		unsigned long long startTime = getCurrentTime();
		long int cnt = 0;
		while ( startTime + 600000000000 > getCurrentTime() ) 
		{
			cnt++;
			iter1(consumer, client);
			sleep(300);
			cout << "*** cnt= " << cnt << endl;
		}

		cout << "done cnt= " << cnt << endl;
		
		sleep(50000);

	}
	catch (const OmmException& excp) {
		cout << excp << endl;
	}
}

int main(int argc, char* argv[]) {
	test();
}

//END APIQA
