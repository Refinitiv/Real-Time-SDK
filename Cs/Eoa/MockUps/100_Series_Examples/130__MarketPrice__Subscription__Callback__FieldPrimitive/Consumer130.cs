///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

// 130__MarketPrice__Subscription__Callback__FieldPrimitive
// Steps:
// - opens up MarketPrice service
// - subscribes to a MarketPrice item and passes callback client
// - when a network event happens, retrieves the Bid, Ask and TradePrice values
//   from the received Mp::Quote

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
                class Program : Mp.ConsumerItemClient
                {
                    static void Main(string[] args)
                    {
                        try
                        {
                            var mpService = new Mp.ConsumerService("DIRECT_FEED");

                            mpService.Subscribe(new Mp.ReqSpec("TRI.N"), new Program());

                            Thread.Sleep(60000);
                        }
                        catch (OmmException excp)
                        {
                            System.Console.WriteLine(excp.Message);
                        }
                    }

                    public void OnConsumerItemSync(Mp.ConsumerItem current, Mp.RefreshInfo ri, object closure)
                    {
                        try
                        {
                            System.Console.WriteLine("Symbol: " + current.Symbol + "\n");
                            print(current.Qoute);
                        }
                        catch (OmmException excp)
                        {
                            System.Console.WriteLine(excp.Message);
                        }
                    }

                    public void OnConsumerItemUpdate(Mp.ConsumerItem current, Mp.UpdateInfo ui, object closure)
                    {
                        try
                        {
                            print(ui.Qoute);
                        }
                        catch (OmmException excp)
                        {
                            System.Console.WriteLine(excp.Message);
                        }
                    }

                    public void OnConsumerItemStatus(Mp.ConsumerItem current, Mp.StatusInfo si, object closure)
                    {
                        if (current.OK)
                            System.Console.WriteLine("Subscription is Ok\n");
                        else
                            System.Console.WriteLine("Subscription is not Ok\n");
                    }

                    public void OnConsumerItemPartial(Mp.ConsumerItem current, Mp.RefreshInfo ri, object closure)
                    { }

                    void print(Mp.Qoute qoute)
                    {
                        System.Console.WriteLine("Ask: " + qoute.GetAsk() + "\n"
                                   + "Bid: " + qoute.GetBid() + "\n"
                                   + "Trade Price: " + qoute.GetTradePrice() + "\n");
                    }
                }
            }
        }
    }
}

/* Abridged Output

Symbol: TRI.N

Ask: 40.23
Bid: 40.19
Trade Price: 40.2

Ask: 40.24
Bid: 40.2
Trade Price: 40.21

*/