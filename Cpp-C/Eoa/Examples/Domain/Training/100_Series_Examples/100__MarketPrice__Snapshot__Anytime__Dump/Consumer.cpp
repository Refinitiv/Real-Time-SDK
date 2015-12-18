///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

// 100__MarketPrice__Snapshot__Anytime__Dump
// Steps:
// - opens up MarketPrice service
// - requests MarketPrice item snapshot
// - waits till refresh complete
// - prints out info about entire received MarketPrice item (including Quote)

#include "Domain/MarketPrice/Include/Mp.h"
#include <iostream>

using namespace thomsonreuters::eoa::foundation;
using namespace thomsonreuters::eoa::domain::marketprice;
using namespace std;

int main( int argc, char* argv[] )
{
	try {
		Mp::ConsumerService mpService( "DIRECT_FEED" );

		Mp::ConsumerItem mpSnap = mpService.snap( Mp::ReqSpec( "TRI.N" ) );

		cout << mpSnap << endl;

	} catch ( const OmmException& excp ) {
		cout << excp << endl;
	}
	return 0;
}

/* Expected abridged output

Mp::ConsumerItem
    Symbol: TRI.N
    ServiceName: DIRECT_FEED
    is Active: no
    is Ok: no
    is Resync: no
    is Complete: yes
    StatusText: MessageComplete
    Mp::Quote
        RDNDISPLAY (2)    100
        RDN_EXCHID (4)    155
        DIVPAYDATE (38)    10 DEC 2005
        TRDPRC_1 (6)    41.00
        BID (22)    40.99
        ASK (25)    41.03
        ACVOL_1 (32)    100000
        NETCHNG_1 (11)    2.15
        ASK_TIME (267)    15:32:11:000:000:000
    Mp::QuoteEnd
Mp::ConsumerItemEnd

*/
