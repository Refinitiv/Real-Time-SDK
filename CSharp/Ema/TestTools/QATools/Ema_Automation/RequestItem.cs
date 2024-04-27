using LSEG.Ema.Access;
using System.Collections.Concurrent;
using System.Management.Automation;
using Msg = LSEG.Ema.Access.Msg;

namespace LSEG.Ema.Automation;

[Cmdlet(VerbsLifecycle.Request, "Item")]
public class RequestItem : Cmdlet
{
    [Parameter(ValueFromPipeline = true, Mandatory = true)]
    public OmmConsumer Consumer { get; set; }

    [Parameter(Mandatory = true)]
    public string Item { get; set; }

    [Parameter(Mandatory = true)]
    public string Service { get; set; }

    [Parameter(Mandatory = false)]
    public TimeSpan? Timeout { get; set; }

    [Parameter(Mandatory = false)]
    public int? Limit { get; set; }

    internal class AppClient : IOmmConsumerClient
    {
        private readonly BlockingCollection<Msg> collection;
        private readonly int? limit;
        private int received; 

        public AppClient(BlockingCollection<Msg> collection, int? limit)
        {
            this.collection = collection;
            this.limit = limit;
        }

        public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent) =>
            Add(refreshMsg);

        public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent) =>
            Add(updateMsg);

        public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent) =>
            Add(statusMsg);

        private void Add(Msg msg)
        {
            if((limit is null || received < limit) && !collection.IsCompleted)
            {
                collection.Add(msg);
                received++;
            }
            else
            {
                if(!collection.IsCompleted)
                {
                    collection.CompleteAdding();
                }
            }
        }
    }

    protected override void ProcessRecord()
    {
        var collection = new BlockingCollection<Msg>();
        var appClient = new AppClient(collection, Limit);
        RequestMsg reqMsg = new();
        Consumer.RegisterClient(reqMsg.ServiceName(Service).Name(Item), appClient);
        if(Timeout is not null)
        {
            Task.Delay(Timeout.Value).ContinueWith(t => 
            {
                if (!collection.IsCompleted)
                {
                    collection.CompleteAdding();
                }
            });
        }
        foreach (var msg in collection.GetConsumingEnumerable())
        {
            if(msg is not null)
            {
                WriteObject(msg);
            }
        }
    }
}
