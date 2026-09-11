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
using static LSEG.Ema.Access.Tests.TestUtilities;

namespace LSEG.Ema.Access.Tests.Domain.Directory
{
    public class DirectoryConsumerStatusMsgTests
    {
        private DirectoryConsumerStatusMsg CreateSampleDomainMessage() =>
            new DirectoryConsumerStatusMsg()
                .ConsumerServiceStatusList((Action<IFluentListBuilder<DirectoryConsumerStatusService>>)(csl => csl
                    .Add(new DirectoryConsumerStatusService()
                        .ServiceId(3)
                        .Action(DirectoryMapAction.ADD)
                        .SourceMirroringMode(SourceMirroringMode.STANDBY)
                        .WarmStandbyMode(WarmStandbyDirectoryServiceType.STANDBY)
                    )
                    .Add(new DirectoryConsumerStatusService()
                        .ServiceId(10)
                        .Action(DirectoryMapAction.UPDATE)
                        .SourceMirroringMode(SourceMirroringMode.ACTIVE_WITH_STANDBY)
                        .WarmStandbyMode(WarmStandbyDirectoryServiceType.ACTIVE)
                    ))
                )
                .SequenceNumber(123);

        private void AssertDomainMsgEqual(DirectoryConsumerStatusMsg expected, DirectoryConsumerStatusMsg actual)
        {
            AssertEqualBy(expected, actual, x => x.SequenceNumber());
            AssertEqualBy(expected, actual, x => x.ConsumerServiceStatusList().Count);
            AssertEqualBy(expected, actual, x => x.ConsumerServiceStatusList()[0].ServiceId());
            AssertEqualBy(expected, actual, x => x.ConsumerServiceStatusList()[0].Action());
            AssertEqualBy(expected, actual, x => x.ConsumerServiceStatusList()[0].SourceMirroringMode());
            AssertEqualBy(expected, actual, x => x.ConsumerServiceStatusList()[0].WarmStandbyMode());
            AssertEqualBy(expected, actual, x => x.ConsumerServiceStatusList()[1].ServiceId());
            AssertEqualBy(expected, actual, x => x.ConsumerServiceStatusList()[1].Action());
            AssertEqualBy(expected, actual, x => x.ConsumerServiceStatusList()[1].SourceMirroringMode());
            AssertEqualBy(expected, actual, x => x.ConsumerServiceStatusList()[1].WarmStandbyMode());
        }

        [Fact]
        public void Message_should_decode_previously_encoded()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();
            var emaMsg = domainMsg.Message().MarkForClear();

            var decodedEmaMsg = new GenericMsg(emaMsg).MarkForClear();

            // Act
            var decodedDomainMsg = new DirectoryConsumerStatusMsg().Message(decodedEmaMsg);

            // Assert
            AssertDomainMsgEqual(domainMsg, decodedDomainMsg);
        }

        [Fact]
        public void Message_should_throw_exception_on_decoding_message_with_wrong_domain_type()
        {
            // Arrange
            var domainMsg = new DirectoryConsumerStatusMsg();
            var emaMsg = new GenericMsg().MarkForClear();
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
            var domainMsg = new DirectoryConsumerStatusMsg();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => domainMsg.Message(null!));
            Assert.Equal("Message can not be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void CopyFrom_should_copy_properly()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();

            // Act
            var copiedDomainMsg = new DirectoryConsumerStatusMsg().CopyFrom(domainMsg);

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
            Assert.Empty(domainMsg.ConsumerServiceStatusList());
            Assert.False(domainMsg.HasSequenceNumber);
        }

        [Fact]
        public void ToString_should_output_expected_string_for_normal_instance()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();

            // Act
            var actualString = domainMsg.ToString();

            // Assert
            Assert.Equal(NormalizeLineEndings(@"DomainType: Directory Domain
ConsumerServiceStatusList:
    [
        ConsumerStatusService:
            ServiceId: 3
            Action: ADD
            SourceMirroringMode: STANDBY
            WarmStandbyMode: STANDBY
        EndConsumerStatusService
        ConsumerStatusService:
            ServiceId: 10
            Action: UPDATE
            SourceMirroringMode: ACTIVE_WITH_STANDBY
            WarmStandbyMode: ACTIVE
        EndConsumerStatusService
    ]
SequenceNumber: 123
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
            Assert.Equal(NormalizeLineEndings(@"DomainType: Directory Domain
ConsumerServiceStatusList:
    [
    ]
"), NormalizeLineEndings(actualString));
        }

        [Fact]
        public void Filter_should_throw_exception_on_get()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => domainMsg.Filter());
            Assert.Equal("Filter isn't used for this type of message", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void Filter_should_throw_exception_on_set()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => domainMsg.Filter(DirectoryFilters.SERVICE_GROUP_FILTER));
            Assert.Equal("Filter isn't used for this type of message", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void ConsumerServiceStatusList_should_throw_exception_on_list_set_to_null()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() =>
                domainMsg.ConsumerServiceStatusList((IList<DirectoryConsumerStatusService>)null!));
            Assert.Equal("consumerServiceStatusList can not be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void ConsumerServiceStatusList_should_throw_exception_on_builder_set_to_null()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() =>
                domainMsg.ConsumerServiceStatusList((Action<IFluentListBuilder<DirectoryConsumerStatusService>>)null!));
            Assert.Equal("buildAction can not be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }
    }
}
