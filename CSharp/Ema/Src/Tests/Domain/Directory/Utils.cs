using LSEG.Ema.Domain.Common;
using LSEG.Ema.Domain.Internal;
using LSEG.Ema.Domain.Directory;
using System.Linq;
using static LSEG.Ema.Access.Tests.TestUtilities;

namespace LSEG.Ema.Access.Tests.Domain.Directory
{
    internal static class Utils
    {
        public static void PopulateDirectoryServices(IFluentListBuilder<DirectoryService> builder)
        {
            builder
                .Add(new DirectoryService()
                    .Action(DirectoryMapAction.UPDATE)
                    .ServiceId(2)
                    .Data(d => d
                        .Action(DirectoryFilterAction.UPDATE)
                        .DataAsUInt(123456789ul)
                        .Type(DirectoryServiceData.ContentType.HEADLINE))
                    .GroupStateList(gsl => gsl
                        .Add(new DirectoryServiceGroup()
                            .Action(DirectoryFilterAction.UPDATE)
                            .Status(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.ALREADY_OPEN, "Some another status")
                            .Group(new EmaBuffer(new byte[] { 1, 2, 3 }))
                            .MergedToGroup(new EmaBuffer(new byte[] { 3, 2, 1 })))
                        .Add(new DirectoryServiceGroup()
                            .Action(DirectoryFilterAction.UPDATE)
                            .Status(OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.OK, OmmState.StatusCodes.NON_UPDATING_ITEM, "Some statussss")
                            .Group(new EmaBuffer(new byte[] { 1, 2, 3 }))
                            .MergedToGroup(new EmaBuffer(new byte[] { 3, 2, 1 }))))
                    .Info(i => i
                        .Action(DirectoryFilterAction.UPDATE)
                        .AcceptingConsumerStatus(true)
                        .CapabilitiesList(cl => cl.Add(1ul).Add(2ul).Add(3ul))
                        .DictionariesProvidedList(dpl => dpl.Add("Dict1").Add("Dict2"))
                        .DictionariesUsedList(dul => dul.Add("Dict1").Add("Dict4"))
                        .IsSource(true)
                        .ItemList("aaaa")
                        .QosList(ql => ql
                            .Add(new DirectoryQos().Timeliness(OmmQos.Timelinesses.INEXACT_DELAYED).Rate(OmmQos.Rates.JUST_IN_TIME_CONFLATED))
                            .Add(new DirectoryQos().Timeliness(10).Rate(15)))
                        .ServiceName("Service #2")
                        .SupportsQosRange(true)
                        .SupportsOutOfBandSnapshots(true)
                        .Vendor("asdf"))
                    .Load(l => l
                        .Action(DirectoryFilterAction.UPDATE)
                        .LoadFactor(3)
                        .OpenLimit(10987654321ul)
                        .OpenWindow(123456789ul))
                    .State(s => s
                        .Action(DirectoryFilterAction.UPDATE)
                        .AcceptingRequests(true)
                        .IsServiceUp(true)
                        .Status(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.ALREADY_OPEN, "Statusssss"))
                    .Link(l => l
                        .Action(DirectoryFilterAction.UPDATE)
                        .LinkList(ll => ll
                            .Add(new DirectoryServiceLink()
                                .Action(DirectoryFilterAction.UPDATE)
                                .IsLinkUp(true)
                                .LinkCode(LinkCode.RECOVERY_COMPLETED)
                                .Name("Link_#1")
                                .Text("Some link")
                                .Type(UpstreamSourceType.INTERACTIVE)))));
        }

        public static void AssertEqualByServiceList<TMessage, TDomainMessage>(
            DirectoryMsgWithPayload<TMessage, TDomainMessage> expected,
            DirectoryMsgWithPayload<TMessage, TDomainMessage> actual)
            where TMessage : Msg, new()
            where TDomainMessage : DirectoryMsgWithPayload<TMessage, TDomainMessage>
        {
            AssertEqualBy(expected, actual, x => x.ServiceList().Count);
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Action());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].ServiceId());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Data().Action());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Data().Type());
            AssertEqualBy(expected, actual, x => ((OmmUInt)x.ServiceList()[0].Data().Data()).Value);
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].GroupStateList().Count);
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].GroupStateList()[0].Action());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].GroupStateList()[0].Status().ToString());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].GroupStateList()[0].Group().AsRawHexString());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].GroupStateList()[0].MergedToGroup().AsRawHexString());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].GroupStateList()[1].Action());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].GroupStateList()[1].Status().ToString());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].GroupStateList()[1].Group().AsRawHexString());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].GroupStateList()[1].MergedToGroup().AsRawHexString());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Info().Action());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Info().AcceptingConsumerStatus());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Info().CapabilitiesList());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Info().DictionariesProvidedList());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Info().DictionariesUsedList());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Info().IsSource());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Info().ItemList());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Info().QosList().Select(x => x.ToString()).ToArray());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Info().ServiceName());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Info().SupportsQosRange());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Info().SupportsOutOfBandSnapshots());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Info().Vendor());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Load().Action());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Load().LoadFactor());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Load().OpenLimit());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Load().OpenWindow());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].State().Action());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].State().AcceptingRequests());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].State().IsServiceUp());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].State().Status().ToString());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Link().Action());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Link().LinkList().Count);
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Link().LinkList()[0].Action());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Link().LinkList()[0].IsLinkUp());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Link().LinkList()[0].LinkCode());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Link().LinkList()[0].Name());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Link().LinkList()[0].Text());
            AssertEqualBy(expected, actual, x => x.ServiceList()[0].Link().LinkList()[0].Type());
        }
    }
}
