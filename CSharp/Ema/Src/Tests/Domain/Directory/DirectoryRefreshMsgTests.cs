/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Domain.Common;
using LSEG.Ema.Domain.Directory;
using LSEG.Ema.Rdm;
using System;
using System.Collections.Generic;
using static LSEG.Ema.Access.Tests.Domain.Directory.Utils;
using static LSEG.Ema.Access.Tests.TestUtilities;

namespace LSEG.Ema.Access.Tests.Domain.Directory
{
    public class DirectoryRefreshMsgTests
    {
        private DirectoryRefreshMsg CreateSampleDomainMessage() =>
            new DirectoryRefreshMsg()
                .Filter(DirectoryFilters.SERVICE_INFO_FILTER | DirectoryFilters.SERVICE_GROUP_FILTER | DirectoryFilters.SERVICE_LINK_FILTER)
                .ServiceList(PopulateDirectoryServices)
                .SequenceNumber(123)
                .State(OmmState.StreamStates.NON_STREAMING, OmmState.DataStates.NO_CHANGE, OmmState.StatusCodes.TIMEOUT, "Some status")
                .ClearCache(true)
                .DoNotCache(true)
                .Complete(true)
                .Solicited(true)
                .ServiceId(65000);

        private void AssertDomainMsgEqual(DirectoryRefreshMsg expected, DirectoryRefreshMsg actual)
        {
            AssertEqualBy(expected, actual, x => x.Filter());
            AssertEqualBy(expected, actual, x => x.SequenceNumber());
            AssertEqualBy(expected, actual, x => x.State().ToString());
            AssertEqualBy(expected, actual, x => x.ClearCache());
            AssertEqualBy(expected, actual, x => x.DoNotCache());
            AssertEqualBy(expected, actual, x => x.Complete());
            AssertEqualBy(expected, actual, x => x.Solicited());
            AssertEqualBy(expected, actual, x => x.ServiceId());
            AssertEqualByServiceList(expected, actual);
        }

        [Fact]
        public void Message_should_decode_previously_encoded()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();
            var emaMsg = domainMsg.Message().MarkForClear();

            var decodedEmaMsg = new RefreshMsg(emaMsg).MarkForClear();

            // Act
            var decodedDomainMsg = new DirectoryRefreshMsg().Message(decodedEmaMsg);

            // Assert
            AssertDomainMsgEqual(domainMsg, decodedDomainMsg);
        }

        [Fact]
        public void Message_should_throw_exception_on_decoding_message_with_wrong_domain_type()
        {
            // Arrange
            var domainMsg = new DirectoryRefreshMsg();
            var emaMsg = new RefreshMsg().MarkForClear();
            emaMsg.DomainType(EmaRdm.MMT_MARKET_PRICE);
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => domainMsg.Message(emaMsg));
            Assert.Equal("Domain type must be Directory instead of MarketPrice Domain.", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Message_should_throw_exception_on_decoding_null_message()
        {
            // Arrange
            var domainMsg = new DirectoryRefreshMsg();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => domainMsg.Message(null!));
            Assert.Equal("Message can not be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void ServiceList_should_throw_exception_on_setting_null_list()
        {
            // Arrange
            var domainMsg = new DirectoryRefreshMsg();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => domainMsg.ServiceList((IList<DirectoryService>)null!));
            Assert.Equal("ServiceList must be non-null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void ServiceList_should_throw_exception_on_setting_null_action()
        {
            // Arrange
            var domainMsg = new DirectoryRefreshMsg();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => domainMsg.ServiceList((Action<IFluentListBuilder<DirectoryService>>)null!));
            Assert.Equal("buildAction must be non-null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void CopyFrom_should_copy_properly()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();

            // Act
            var copiedDomainMsg = new DirectoryRefreshMsg().CopyFrom(domainMsg);

            // Assert
            AssertDomainMsgEqual(domainMsg, copiedDomainMsg);
        }

        [Fact]
        public void Clear_should_reset_properties()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();

            // Act
            domainMsg.Clear();

            // Assert
            Assert.Equal(DirectoryFilters.NONE, domainMsg.Filter());
            Assert.False(domainMsg.HasSequenceNumber);
            Assert.False(domainMsg.HasServiceId);
            Assert.Equal("Open / Ok / None / ''", domainMsg.State().ToString());
            Assert.False(domainMsg.ClearCache());
            Assert.False(domainMsg.DoNotCache());
            Assert.False(domainMsg.Complete());
            Assert.False(domainMsg.Solicited());
            Assert.Empty(domainMsg.ServiceList());
        }

        [Fact]
        public void ToString_should_output_expected_string_for_normal_instance()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();

            // Act
            var actualString = domainMsg.ToString();

            // Assert
            Assert.Equal(NormalizeLineEndings(@"Filter: SERVICE_INFO_FILTER, SERVICE_GROUP_FILTER, SERVICE_LINK_FILTER
DomainType: Directory Domain
ServiceList:
    [
        {
            Action: UPDATE
            ServiceId: 2
            Info:
                FilterId: 1
                Action: UPDATE
                ServiceName: Service #2
                Vendor: asdf
                IsSource: True
                SupportsQosRange: True
                SupportsOutOfBandSnapshots: True
                AcceptingConsumerStatus: True
                ItemList: aaaa
                CapabilitiesList:
                    [
                        1
                        2
                        3
                    ]
                DictionariesProvidedList:
                    [
                        Dict1
                        Dict2
                    ]
                DictionariesUsedList:
                    [
                        Dict1
                        Dict4
                    ]
                QosList:
                    [
                        InexactDelayed/JustInTimeConflated
                        Timeliness: 10/Rate: 15
                    ]
            State:
                FilterId: 2
                Action: UPDATE
                IsServiceUp: True
                AcceptingRequests: True
                Status: Open / Ok / AlreadyOpen / 'Statusssss'
            Load:
                FilterId: 4
                Action: UPDATE
                OpenLimit: 10987654321
                OpenWindow: 123456789
                LoadFactor: 3
            Data:
                FilterId: 5
                Action: UPDATE
                Type: HEADLINE
                Data:
                    123456789
                EndData
            Link:
                FilterId: 6
                Action: UPDATE
                LinkList:
                    [
                        FilterId: 6
                        Action: UPDATE
                        Name: Link_#1
                        Type: INTERACTIVE
                        IsLinkUp: True
                        LinkCode: RECOVERY_COMPLETED
                        Text: Some link
                    ]
            GroupStateList:
            [
                {
                    FilterId: 3
                    Action: UPDATE
                    Status: Open / Ok / AlreadyOpen / 'Some another status'
                    Group:
                        0102 03
                    MergedToGroup:
                        0302 01
                }
                {
                    FilterId: 3
                    Action: UPDATE
                    Status: Closed, Recoverable / Ok / NonUpdatingItem / 'Some statussss'
                    Group:
                        0102 03
                    MergedToGroup:
                        0302 01
                }
            ]
        }
    ]
State: Non-streaming / No Change / Timeout / 'Some status'
SequenceNumber: 123
ClearCache: True
DoNotCache: True
Complete: True
Solicited: True
ServiceId: 65000
"), NormalizeLineEndings(actualString));
        }

        [Fact]
        public void ToString_should_output_expected_string_for_clean_instance()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage().Clear();

            // Act
            var actualString = domainMsg.ToString();

            // Assert
            Assert.Equal(NormalizeLineEndings(@"Filter: NONE
DomainType: Directory Domain
ServiceList:
    [
    ]
State: Open / Ok / None / ''
ClearCache: False
DoNotCache: False
Complete: False
Solicited: False
ServiceId: <no value>
"), NormalizeLineEndings(actualString));
        }
    }
}
