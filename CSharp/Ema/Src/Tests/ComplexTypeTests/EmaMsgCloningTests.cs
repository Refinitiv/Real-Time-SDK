/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;

using static LSEG.Ema.Access.Tests.TestUtilities;

namespace LSEG.Ema.Access.Tests.ComplexTypeTests
{
    public class EmaMsgCloningTests
    {
        #region Utilities

        private record CloningPropertyDef<TMsg>(
            Func<TMsg, object> Getter,
            Action<TMsg, object> Setter,
            object TestValue,
            Action<TMsg>? CustomAssert = null)
            where TMsg : Msg;

        private CloningPropertyDef<TMsg> Prop<TMsg, TProp>(
            Func<TMsg, TProp> getter,
            Action<TMsg, TProp> setter,
            TProp testValue,
            Action<TMsg, TProp>? customAssert = null)
            where TMsg : Msg
        {
            return new(msg => getter(msg)!, (msg, val) => setter(msg, (TProp)val), testValue!,
                customAssert != null
                    ? ((msg) => customAssert(msg, testValue))
                    : null);
        }

        private CloningPropertyDef<TMsg> PropEmaBuffer<TMsg>(
            Func<TMsg, EmaBuffer> getter,
            Action<TMsg, EmaBuffer> setter,
            byte[] testValue)
            where TMsg : Msg
            => Prop(
                getter,
                setter,
                new EmaBuffer(testValue),
                (clonedMsg, v) => Assert.Equal(v.Contents, getter(clonedMsg).Contents));

        private CloningPropertyDef<TMsg> PropPayload<TMsg>(Action<TMsg, ComplexType> setPayload)
            where TMsg : Msg
        {
            return Prop<TMsg, FieldList>(
                        msg => msg.Payload().FieldList(),
                        setPayload,
                        new FieldList()
                            .AddAscii(3, "ABC")
                            .AddEnumValue(15, 840)
                            .AddReal(21, 3900, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                            .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                            .AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                            .AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0)
                            .AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0)
                            .Complete()
                            .MarkForClear(),
                        (clonedMsg, testValue) =>
                        {
                            var clonedMsgString = clonedMsg.ToString(m_DataDictionary);
                            const string PayloadString = @"
    Payload dataType=""FieldList""
        FieldList
            FieldEntry fid=""3"" name=""DSPLY_NAME"" dataType=""Rmtes"" value=""ABC""
            FieldEntry fid=""15"" name=""CURRENCY"" dataType=""Enum"" value=""840""
            FieldEntry fid=""21"" name=""HST_CLOSE"" dataType=""Real"" value=""39""
            FieldEntry fid=""22"" name=""BID"" dataType=""Real"" value=""39.9""
            FieldEntry fid=""25"" name=""ASK"" dataType=""Real"" value=""39.94""
            FieldEntry fid=""30"" name=""BIDSIZE"" dataType=""Real"" value=""9""
            FieldEntry fid=""31"" name=""ASKSIZE"" dataType=""Real"" value=""19""
        FieldListEnd
    PayloadEnd";
                            Assert.Contains(
                                NormalizeLineEndings(PayloadString),
                                NormalizeLineEndings(clonedMsgString));
                        });

            static string NormalizeLineEndings(string text) =>
                System.Text.RegularExpressions.Regex.Replace(text, @"(\r\n|\r|\n)", "\n");
        }

        private CloningPropertyDef<TMsg> PropEncodedAttrib<TMsg>()
            where TMsg : Msg
            => Prop<TMsg, (int AttribContainerType, Eta.Codec.Buffer EncodedAttrib)>(
                msg => (msg.m_rsslMsg.MsgKey.AttribContainerType, msg.m_rsslMsg.MsgKey.EncodedAttrib),
                (msg, v) =>
                {
                    msg.m_rsslMsg.MsgKey.AttribContainerType = v.AttribContainerType;
                    msg.m_rsslMsg.MsgKey.EncodedAttrib = v.EncodedAttrib;
                },
                (AttribContainerType: DataTypes.OPAQUE, EncodedAttrib: CreateEtaBuffer(new byte[] { 3, 2, 1 })),
                (clonedMsg, v) =>
                {
                    Assert.Equal(v.AttribContainerType, clonedMsg.m_rsslMsg.MsgKey.AttribContainerType);
                    Assert.Equal(v.EncodedAttrib.Data().Contents, clonedMsg.m_rsslMsg.MsgKey.EncodedAttrib.Data().Contents);
                });

        private void TestCloning<TMsg>(
            TMsg msg,
            Func<TMsg, TMsg> clone,
            params CloningPropertyDef<TMsg>[] propertyDefs)
            where TMsg : Msg
        {
            // Arrange
            foreach (var p in propertyDefs)
            {
                p.Setter(msg, p.TestValue);
            }
            // Act
            var clonedMsg = clone(msg).MarkForClear();
            msg.Clear_All(); // ensure that we have a deep copy so that data in destination persists even after the source is cleared
            // Assert
            foreach (var p in propertyDefs)
            {
                if (p.CustomAssert == null)
                    Assert.Equal(p.TestValue, p.Getter(clonedMsg));
                else
                    p.CustomAssert(clonedMsg);
            }
        }
        
        private Eta.Codec.Buffer CreateEtaBuffer(byte[] bytes)
        {
            var result = new Eta.Codec.Buffer();
            result.Data(new Eta.Common.ByteBuffer(bytes).Flip());
            return result;
        }

        #endregion

        private static Rdm.DataDictionary m_DataDictionary = new();     // Create DataDicitonary and load them from the current location

        static EmaMsgCloningTests()
        {
            LoadEmaFieldDictionary(m_DataDictionary);
            LoadEmaEnumTypeDictionary(m_DataDictionary);
        }

        private void TestRefreshMsgCloning(Func<RefreshMsg, RefreshMsg> clone)
        {
            TestCloning(new RefreshMsg().MarkForClear(), clone,
                Prop<RefreshMsg, string>(msg => msg.Name(), (msg, v) => msg.SetName(v), "DUMMY.D"),
                Prop<RefreshMsg, int>(msg => msg.NameType(), (msg, v) => msg.NameType(v), EmaRdm.USER_NAME),
                Prop<RefreshMsg, int>(msg => msg.ServiceId(), (msg, v) => msg.ServiceId(v), 3),
                Prop<RefreshMsg, int>(msg => msg.Id(), (msg, v) => msg.Id(v), 4),
                Prop<RefreshMsg, long>(msg => msg.Filter(), (msg, v) => msg.Filter(v), 123),
                PropEncodedAttrib<RefreshMsg>(),
                Prop<RefreshMsg, int>(msg => msg.DomainType(), (msg, v) => msg.DomainType(v), EmaRdm.MMT_LOGIN),
                Prop<RefreshMsg, int>(msg => msg.StreamId(), (msg, v) => msg.StreamId(v), 5),
                Prop<RefreshMsg, bool>(msg => msg.Complete(), (msg, v) => msg.Complete(v), true),
                PropEmaBuffer<RefreshMsg>(
                    msg => msg.ExtendedHeader(),
                    (msg, v) => msg.ExtendedHeader(v),
                    new byte[]{4,5,6}),
                Prop<RefreshMsg, (uint Timeliness, uint Rate)>(
                    msg => (msg.Qos().Timeliness, msg.Qos().Rate),
                    (msg, v) => msg.Qos(v.Timeliness, v.Rate),
                    (Timeliness: Eta.Codec.QosTimeliness.DELAYED, Rate: Eta.Codec.QosRates.TICK_BY_TICK)),
                Prop<RefreshMsg, string>(msg => msg.ServiceName(), (msg, v) => msg.ServiceName(v), "slkdjf"),
                Prop<RefreshMsg, long>(msg => msg.SeqNum(), (msg, v) => msg.SeqNum(v), 234L),
                PropEmaBuffer<RefreshMsg>(
                    msg => msg.PermissionData(),
                    (msg, v) => msg.PermissionData(v),
                    new byte[]{1,2,3}),
                Prop<RefreshMsg, int>(msg => msg.PartNum(), (msg, v) => msg.PartNum(v), 3),
                Prop<RefreshMsg, (long PublisherIdUserId, long PublisherIdUserAddress)>(
                    msg => (PublisherIdUserId: msg.PublisherIdUserId(), PublisherIdUserAddress: msg.PublisherIdUserAddress()),
                    (msg, v) => msg.PublisherId(v.PublisherIdUserId, v.PublisherIdUserAddress),
                    (PublisherIdUserId: 4, PublisherIdUserAddress: 8)),
                Prop<RefreshMsg, (int StreamState, int DataState, int StatusCode, string StatusText)>(
                    msg => (msg.State().StreamState, msg.State().DataState, msg.State().StatusCode, msg.State().StatusText),
                    (msg, v) => msg.State(v.StreamState, v.DataState, v.StatusCode, v.StatusText),
                    (StreamState: 2, DataState: 1, StatusCode: 3, StatusText: "aaassas")),
                PropEmaBuffer<RefreshMsg>(
                    msg => msg.ItemGroup(),
                    (msg, v) => msg.ItemGroup(v),
                    new byte[]{5,2,3}),
                Prop<RefreshMsg, bool>(msg => msg.Solicited(), (msg, v) => msg.Solicited(v), true),
                Prop<RefreshMsg, bool>(msg => msg.Complete(), (msg, v) => msg.Complete(v), true),
                Prop<RefreshMsg, bool>(msg => msg.ClearCache(), (msg, v) => msg.ClearCache(v), true),
                Prop<RefreshMsg, bool>(msg => msg.PrivateStream(), (msg, v) => msg.PrivateStream(v), true),
                Prop<RefreshMsg, bool>(msg => msg.DoNotCache(), (msg, v) => msg.DoNotCache(v), true),
                PropPayload<RefreshMsg>((msg, v) => msg.Payload(v))
                );
        }

        [Fact]
        public void RefreshMsgCopyCtorTest() =>
            TestRefreshMsgCloning(msg => new(msg));

        [Fact]
        public void RefreshMsgCloneTest() =>
            TestRefreshMsgCloning(msg => msg.Clone());

        [Fact]
        public void RefreshMsgCopyTest() =>
            TestRefreshMsgCloning(msg =>
            {
                var clone = new RefreshMsg();
                msg.Copy(clone);
                return clone;
            });

        private void TestUpdateMsgCloning(Func<UpdateMsg, UpdateMsg> clone)
        {
            TestCloning(new UpdateMsg().MarkForClear(), clone,
                Prop<UpdateMsg, string>(msg => msg.Name(), (msg, v) => msg.SetName(v), "DUMMY.D"),
                Prop<UpdateMsg, int>(msg => msg.NameType(), (msg, v) => msg.NameType(v), EmaRdm.USER_NAME),
                Prop<UpdateMsg, int>(msg => msg.ServiceId(), (msg, v) => msg.ServiceId(v), 3),
                Prop<UpdateMsg, int>(msg => msg.Id(), (msg, v) => msg.Id(v), 4),
                Prop<UpdateMsg, long>(msg => msg.Filter(), (msg, v) => msg.Filter(v), 123),
                PropEncodedAttrib<UpdateMsg>(),
                Prop<UpdateMsg, int>(msg => msg.DomainType(), (msg, v) => msg.DomainType(v), EmaRdm.MMT_LOGIN),
                Prop<UpdateMsg, int>(msg => msg.StreamId(), (msg, v) => msg.StreamId(v), 5),
                PropEmaBuffer<UpdateMsg>(msg => msg.ExtendedHeader(), (msg, v) => msg.ExtendedHeader(v), new byte[]{4,5,6}),
                Prop<UpdateMsg, string>(msg => msg.ServiceName(), (msg, v) => msg.ServiceName(v), "slkdjf"),
                Prop<UpdateMsg, long>(msg => msg.SeqNum(), (msg, v) => msg.SeqNum(v), 234L),
                PropEmaBuffer<UpdateMsg>(msg => msg.PermissionData(), (msg, v) => msg.PermissionData(v), new byte[]{1,2,3}),
                Prop<UpdateMsg, (int Count, int Time)>(
                    msg => (msg.ConflatedCount(), msg.ConflatedTime()),
                    (msg, v) => msg.Conflated(v.Count, v.Time),
                    (Count: 3, Time: 10)),
                Prop<UpdateMsg, (long PublisherIdUserId, long PublisherIdUserAddress)>(
                    msg => (PublisherIdUserId: msg.PublisherIdUserId(), PublisherIdUserAddress: msg.PublisherIdUserAddress()),
                    (msg, v) => msg.PublisherId(v.PublisherIdUserId, v.PublisherIdUserAddress),
                    (PublisherIdUserId: 4, PublisherIdUserAddress: 8)),
                Prop<UpdateMsg, int>(msg => msg.UpdateTypeNum(), (msg, v) => msg.UpdateTypeNum(v), 23),
                Prop<UpdateMsg, bool>(msg => msg.DoNotCache(), (msg, v) => msg.DoNotCache(v), true),
                Prop<UpdateMsg, bool>(msg => msg.DoNotConflate(), (msg, v) => msg.DoNotConflate(v), true),
                Prop<UpdateMsg, bool>(msg => msg.DoNotRipple(), (msg, v) => msg.DoNotRipple(v), true),
                PropPayload<UpdateMsg>((msg, v) => msg.Payload(v))
                );
        }

        [Fact]
        public void UpdateMsgCopyCtorTest() =>
            TestUpdateMsgCloning(msg => new(msg));

        [Fact]
        public void UpdateMsgCloneTest() =>
            TestUpdateMsgCloning(msg => msg.Clone());

        [Fact]
        public void UpdateMsgCopyTest() =>
            TestUpdateMsgCloning(msg =>
            {
                var clone = new UpdateMsg();
                msg.Copy(clone);
                return clone;
            });

        private void TestStatusMsgCloning(Func<StatusMsg, StatusMsg> clone)
        {
            TestCloning(new StatusMsg().MarkForClear(), clone,
                Prop<StatusMsg, string>(msg => msg.Name(), (msg, v) => msg.SetName(v), "DUMMY.D"),
                Prop<StatusMsg, int>(msg => msg.NameType(), (msg, v) => msg.NameType(v), EmaRdm.USER_NAME),
                Prop<StatusMsg, int>(msg => msg.ServiceId(), (msg, v) => msg.ServiceId(v), 3),
                Prop<StatusMsg, int>(msg => msg.Id(), (msg, v) => msg.Id(v), 4),
                Prop<StatusMsg, long>(msg => msg.Filter(), (msg, v) => msg.Filter(v), 123),
                PropEncodedAttrib<StatusMsg>(),
                Prop<StatusMsg, int>(msg => msg.DomainType(), (msg, v) => msg.DomainType(v), EmaRdm.MMT_LOGIN),
                PropEmaBuffer<StatusMsg>(msg => msg.ExtendedHeader(), (msg, v) => msg.ExtendedHeader(v), new byte[] { 4, 5, 6 }),
                Prop<StatusMsg, string>(msg => msg.ServiceName(), (msg, v) => msg.ServiceName(v), "slkdjf"),
                PropEmaBuffer<StatusMsg>(msg => msg.PermissionData(), (msg, v) => msg.PermissionData(v), new byte[] { 1, 2, 3 }),
                PropEmaBuffer<StatusMsg>(msg => msg.ItemGroup(), (msg, v) => msg.ItemGroup(v), new byte[] { 1, 2, 3 }),
                Prop<StatusMsg, (int StreamState, int DataState, int StatusCode, string StatusText) >(
                    msg => (msg.State().StreamState, msg.State().DataState, msg.State().StatusCode, msg.State().StatusText),
                    (msg, v) => msg.State(v.StreamState, v.DataState, v.StatusCode, v.StatusText),
                    (StreamState: 1, DataState: 2, StatusCode: 3, StatusText: "weoiur")),
                PropPayload<StatusMsg>((msg, v) => msg.Payload(v))
                );
        }

        [Fact]
        public void StatusMsgCopyCtorTest() =>
            TestStatusMsgCloning(msg => new(msg));

        [Fact]
        public void StatusMsgCloneTest() =>
            TestStatusMsgCloning(msg => msg.Clone());

        [Fact]
        public void StatusMsgCopyTest() =>
            TestStatusMsgCloning(msg =>
            {
                var clone = new StatusMsg();
                msg.Copy(clone);
                return clone;
            });

        private void TestAckMsgCloning(Func<AckMsg, AckMsg> clone)
        {
            TestCloning(new AckMsg().MarkForClear(), clone,
                Prop<AckMsg, string>(msg => msg.Name(), (msg, v) => msg.SetName(v), "DUMMY.D"),
                Prop<AckMsg, int>(msg => msg.NameType(), (msg, v) => msg.NameType(v), EmaRdm.USER_NAME),
                Prop<AckMsg, int>(msg => msg.ServiceId(), (msg, v) => msg.ServiceId(v), 3),
                Prop<AckMsg, int>(msg => msg.Id(), (msg, v) => msg.Id(v), 4),
                Prop<AckMsg, long>(msg => msg.Filter(), (msg, v) => msg.Filter(v), 123),
                PropEncodedAttrib<AckMsg>(),
                Prop<AckMsg, int>(msg => msg.DomainType(), (msg, v) => msg.DomainType(v), EmaRdm.MMT_LOGIN),
                PropEmaBuffer<AckMsg>(msg => msg.ExtendedHeader(), (msg, v) => msg.ExtendedHeader(v), new byte[] { 4, 5, 6 }),
                Prop<AckMsg, string>(msg => msg.ServiceName(), (msg, v) => msg.ServiceName(v), "slkdjf"),
                Prop<AckMsg, string>(msg => msg.Text(), (msg, v) => msg.Text(v), "xmcnv"),
                PropPayload<AckMsg>((msg, v) => msg.Payload(v))
                );
        }

        [Fact]
        public void AckMsgCopyCtorTest() =>
            TestAckMsgCloning(msg => new(msg));

        [Fact]
        public void AckMsgCloneTest() =>
            TestAckMsgCloning(msg => msg.Clone());

        [Fact]
        public void AckMsgCopyTest() =>
            TestAckMsgCloning(msg =>
            {
                var clone = new AckMsg();
                msg.Copy(clone);
                return clone;
            });

        private void TestPostMsgCloning(Func<PostMsg, PostMsg> clone)
        {
            TestCloning(new PostMsg().MarkForClear(), clone,
                Prop<PostMsg, string>(msg => msg.Name(), (msg, v) => msg.SetName(v), "DUMMY.D"),
                Prop<PostMsg, int>(msg => msg.NameType(), (msg, v) => msg.NameType(v), EmaRdm.USER_NAME),
                Prop<PostMsg, int>(msg => msg.ServiceId(), (msg, v) => msg.ServiceId(v), 3),
                Prop<PostMsg, int>(msg => msg.Id(), (msg, v) => msg.Id(v), 4),
                Prop<PostMsg, long>(msg => msg.Filter(), (msg, v) => msg.Filter(v), 123),
                PropEncodedAttrib<PostMsg>(),
                Prop<PostMsg, int>(msg => msg.DomainType(), (msg, v) => msg.DomainType(v), EmaRdm.MMT_LOGIN),
                PropEmaBuffer<PostMsg>(msg => msg.ExtendedHeader(), (msg, v) => msg.ExtendedHeader(v), new byte[] { 4, 5, 6 }),
                Prop<PostMsg, string>(msg => msg.ServiceName(), (msg, v) => msg.ServiceName(v), "slkdjf"),
                PropEmaBuffer<PostMsg>(msg => msg.PermissionData(), (msg, v) => msg.PermissionData(v), new byte[] { 2, 4, 3 }),
                PropPayload<PostMsg>((msg, v) => msg.Payload(v))
                );
        }

        [Fact]
        public void PostMsgCopyCtorTest() =>
            TestPostMsgCloning(msg => new(msg));

        [Fact]
        public void PostMsgCloneTest() =>
            TestPostMsgCloning(msg => msg.Clone());

        [Fact]
        public void PostMsgCopyTest() =>
            TestPostMsgCloning(msg =>
            {
                var clone = new PostMsg();
                msg.Copy(clone);
                return clone;
            });

        private void TestRequestMsgCloning(Func<RequestMsg, RequestMsg> clone)
        {
            TestCloning(new RequestMsg().MarkForClear(), clone,
                Prop<RequestMsg, string>(msg => msg.Name(), (msg, v) => msg.SetName(v), "DUMMY.D"),
                Prop<RequestMsg, int>(msg => msg.NameType(), (msg, v) => msg.NameType(v), EmaRdm.USER_NAME),
                Prop<RequestMsg, int>(msg => msg.ServiceId(), (msg, v) => msg.ServiceId(v), 3),
                Prop<RequestMsg, int>(msg => msg.Id(), (msg, v) => msg.Id(v), 4),
                Prop<RequestMsg, long>(msg => msg.Filter(), (msg, v) => msg.Filter(v), 123),
                PropEncodedAttrib<RequestMsg>(),
                Prop<RequestMsg, int>(msg => msg.DomainType(), (msg, v) => msg.DomainType(v), EmaRdm.MMT_LOGIN),
                PropEmaBuffer<RequestMsg>(msg => msg.ExtendedHeader(), (msg, v) => msg.ExtendedHeader(v), new byte[] { 4, 5, 6 }),
                Prop<RequestMsg, string>(msg => msg.ServiceName(), (msg, v) => msg.ServiceName(v), "slkdjf"),
                PropPayload<RequestMsg>((msg, v) => msg.Payload(v))
                );
        }

        [Fact]
        public void RequestMsgCopyCtorTest() =>
            TestRequestMsgCloning(msg => new(msg));

        [Fact]
        public void RequestMsgCloneTest() =>
            TestRequestMsgCloning(msg => msg.Clone());

        [Fact]
        public void RequestMsgCopyTest() =>
            TestRequestMsgCloning(msg =>
            {
                var clone = new RequestMsg();
                msg.Copy(clone);
                return clone;
            });
    }
}
