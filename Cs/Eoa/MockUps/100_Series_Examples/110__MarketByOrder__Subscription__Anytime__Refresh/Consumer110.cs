///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

// 110__MarketByOrder__Subscription__Anytime__Refresh
// Steps:
// - opens up MarketByOrder service
// - subscribes to a MarketByOrder item
// - refreshes the subscription every second
// - prints out all orders in the subscription

using ThomsonReuters.EOA.Domain.MarketByOrder;
using ThomsonReuters.EOA.Foundation;
using System.Threading;

namespace ThomsonReuters
{
    namespace EOA
    {
        namespace Example
        {
            namespace Domain
            {
                class Program
                {
                    static void Main(string[] args)
                    {
                        try
                        {
                            var mboService = new Mbo.ConsumerService("DIRECT_FEED");

                            var mboSubscription = mboService.Subscribe(new Mbo.ReqSpec("AAV.O"));

                            System.Console.WriteLine("Symbol: " + mboSubscription.Symbol + "\n");

                            for (int idx = 0; idx < 60; ++idx)
                            {
                                Thread.Sleep(1000);

                                mboService.Refresh(mboSubscription);

                                foreach (var order in mboSubscription.OrderBook)
                                    System.Console.WriteLine("Order id: " + order.GetIdAsString() + "\n"
                                                    + "Action: " + order.GetActionAsString() + "\n"
                                                    + "Price: " + order.GetPriceAsString() + "\n"
                                                    + "Side: " + order.GetPriceAsString() + "\n"
                                                    + "Size: " + order.GetSizeAsString() + "\n");
                            }
                        }
                        catch (OmmException excp)
                        {
                            System.Console.WriteLine(excp.Message);
                        }
                    }
                }
            }
        }
    }
}

/* Expected abridged output

Symbol: AAO.V

Order id:  1
Action:  Append
Price:  7.75
Side:  BID
Size:  100

Order id:  2
Action:  Append
Price:  7.75
Side:  BID
Size:  6400

Order id:  3
Action:  Append
Price:  7.75
Side:  BID
Size:  9200

Order id:  4
Action:  Append
Price:  7.75
Side:  BID
Size:  1500

*/
