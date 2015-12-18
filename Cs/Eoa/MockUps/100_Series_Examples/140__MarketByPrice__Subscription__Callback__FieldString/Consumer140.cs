///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

// 140__MarketByPrice__Subscription__Callback__FieldString
// Steps:
// - opens up MarketByPrice service
// - subscribes to a MarketByPrice item and passes callback client
// - when a network event happens, retrieves the PricePoint and prints their individual
//   field values from the received Mbp::OrderBook

using ThomsonReuters.EOA.Foundation;
using ThomsonReuters.EOA.Domain.MarketByPrice;
using System.Threading;

namespace ThomsonReuters
{
    namespace EOA
    {
        namespace Example
        {
            namespace Domain
            {
                class Program : Mbp.ConsumerItemClient
                {
                    static void Main(string[] args)
                    {
                        try
                        {
                           var mbpConsumerService = new Mbp.ConsumerService("DIRECT_FEED");

                            mbpConsumerService.Subscribe(new Mbp.ReqSpec("BBH.ITC"), new Program());

                            Thread.Sleep(60000);
                        }
                        catch (OmmException excp)
                        {
                            System.Console.WriteLine(excp.Message);
                        }
                    }

                    public void OnConsumerItemSync(Mbp.ConsumerItem current, Mbp.RefreshInfo ri, object closure)
                    {
                        System.Console.WriteLine("Symbol: " + current.Symbol + "\n");
                        Print(ri.OrderBook);
                    }

                    public void OnConsumerItemPartial(Mbp.ConsumerItem current, Mbp.RefreshInfo ri, object closure)
                    {
                        Print(ri.OrderBook);
                    }

                    public void OnConsumerItemStatus(Mbp.ConsumerItem current, Mbp.StatusInfo change, object closure)
                    {
                        if (current.OK)
                            System.Console.WriteLine("Subscription is Ok\n");
                        else
                            System.Console.WriteLine("Subscription is not Ok\n");
                    }

                    public void OnConsumerItemUpdate(Mbp.ConsumerItem current, Mbp.UpdateInfo ui, object closure)
                    {
                        Print(ui.OrderBook);
                    }

                    void Print(Mbp.OrderBook orderBook)
                    {
                        foreach (var pricePoint in orderBook)
                            System.Console.WriteLine("Action: " + pricePoint.GetActionAsString() + "\n"
                                                    + "Price: " + pricePoint.GetPriceAsString() + "\n"
                                                    + "Size: " + pricePoint.GetSizeAsString() + "\n"
                                                    + "Side: " + pricePoint.GetSideAsString() + "\n");
                    }
                }
            }
        }
    }
}

/* Expected abridged outpot 

Symbol: BBH.ITC

Action: Append
Price: 77.00
Size: 8000
Side: BID

Action: Append
Price: 76.99
Size: 2500
Side: BID

Action: Append
Price: 77.54
Size: 1700
Side: ASK

*/
