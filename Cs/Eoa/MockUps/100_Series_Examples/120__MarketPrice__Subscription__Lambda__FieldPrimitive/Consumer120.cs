///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

// 120__MarketPrice__Subscription__Lambda__FieldPrimitive
// Steps:
// - opens up MarketPrice service
// - subscribes to a MarketPrice item and passes lambda expression
// - outputs a received Bid value for the item

using ThomsonReuters.EOA.Domain.MarketPrice;
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
                            var mpService = new Mp.ConsumerService("DIRECT_FEED");

                            mpService.Subscribe(new Mp.ReqSpec("TRI.N"), (mpSubscription) =>
                            { System.Console.WriteLine("Symbol: " + mpSubscription.Symbol + " : Bid: " + mpSubscription.Qoute.GetBidAsString() + "\n"); });

                            Thread.Sleep(60000);
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

Symbol: TRI.N  :  Bid : 40.99
Symbol: TRI.N  :  Bid : 41.00
Symbol: TRI.N  :  Bid : 41.01
Symbol: TRI.N  :  Bid : 41.02
Symbol: TRI.N  :  Bid : 41.03
Symbol: TRI.N  :  Bid : 41.04

*/
