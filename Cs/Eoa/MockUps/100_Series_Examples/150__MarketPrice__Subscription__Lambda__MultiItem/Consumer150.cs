///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

// 150__MarketPrice__Subscription__Lambda__MultiItem
// Steps:
// - opens up MarketPrice service
// - subscribes to two MarketPrice items and passes labda expressions
// - outputs received Bid values for the items

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

                            mpService.Subscribe(new Mp.ReqSpec("MSFT.O"), (mpSubscription) =>
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

/* Abridged Output

Symbol: TRI.N  :  Bid : 40.31
Symbol: MSFT.O  :  Bid : 23.91
Symbol: TRI.N  :  Bid : 40.32
Symbol: MSFT.O  :  Bid : 23.92

*/
