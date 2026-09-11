/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Domain.Directory;
using LSEG.Ema.Rdm;
using System;
using static LSEG.Ema.Access.Tests.TestUtilities;

namespace LSEG.Ema.Access.Tests.Domain.Directory
{
    public class DirectoryStatusMsgTests
    {
        private DirectoryStatusMsg CreateSampleDomainMessage() =>
            new DirectoryStatusMsg()
                .Filter(DirectoryFilters.SERVICE_INFO_FILTER | DirectoryFilters.SERVICE_GROUP_FILTER | DirectoryFilters.SERVICE_LINK_FILTER)
                .ClearCache(true)
                .State(OmmState.StreamStates.NON_STREAMING, OmmState.DataStates.NO_CHANGE, OmmState.StatusCodes.TIMEOUT, "Some status")
                .PermissionData(new EmaBuffer(new byte[] { 1, 2, 3 }))
                .ServiceId(65000);

        private void AssertDomainMsgEqual(DirectoryStatusMsg expected, DirectoryStatusMsg actual)
        {
            AssertEqualBy(expected, actual, x => x.Filter());
            AssertEqualBy(expected, actual, x => x.ClearCache());
            AssertEqualBy(expected, actual, x => x.State().ToString());
            AssertEqualBy(expected, actual, x => x.PermissionData());
            AssertEqualBy(expected, actual, x => x.ServiceId());
        }

        [Fact]
        public void Message_should_decode_previously_encoded()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();
            var emaMsg = domainMsg.Message().MarkForClear();

            var decodedEmaMsg = new StatusMsg(emaMsg).MarkForClear();

            // Act
            var decodedDomainMsg = new DirectoryStatusMsg().Message(decodedEmaMsg);

            // Assert
            AssertDomainMsgEqual(domainMsg, decodedDomainMsg);
        }

        [Fact]
        public void Message_should_throw_exception_on_decoding_message_with_wrong_domain_type()
        {
            // Arrange
            var domainMsg = new DirectoryStatusMsg();
            var emaMsg = new StatusMsg().MarkForClear();
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
            var domainMsg = new DirectoryStatusMsg();
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
            var copiedDomainMsg = new DirectoryStatusMsg().CopyFrom(domainMsg);

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
            Assert.False(domainMsg.HasFilter);
            Assert.False(domainMsg.ClearCache());
            Assert.False(domainMsg.HasState);
            Assert.False(domainMsg.HasPermissionData);
            Assert.False(domainMsg.HasServiceId);
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
State: Non-streaming / No Change / Timeout / 'Some status'
PermissionData:
    0102 03
ClearCache: True
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
ClearCache: False
ServiceId: <no value>
"), NormalizeLineEndings(actualString));
        }

        [Fact]
        public void PermissionData_should_throw_when_not_set()
        {
            // Arrange
            var domainMsg = new DirectoryStatusMsg();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => domainMsg.PermissionData());
            Assert.Equal("PermissionData element is not set", exception.Message);
        }

        [Fact]
        public void PermissionData_should_throw_when_set_to_null()
        {
            // Arrange
            var domainMsg = new DirectoryStatusMsg();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => domainMsg.PermissionData(null!));
            Assert.Equal("PermissionData cannot be null", exception.Message);
        }

        [Fact]
        public void PermissionData_should_return_value_when_properly_set()
        {
            // Arrange
            var domainMsg = new DirectoryStatusMsg();
            var buffer = new EmaBuffer(new byte[] { 1, 2, 3 });
            // Act
            domainMsg.PermissionData(buffer);
            var result = domainMsg.PermissionData();
            // Assert
            Assert.Equal(buffer, result);
        }

        [Fact]
        public void Filter_should_throw_when_value_not_set()
        {
            // Arrange
            var domainMsg = new DirectoryStatusMsg();
            Assert.False(domainMsg.HasFilter);
            // Act & Assert
            var e = Assert.Throws<OmmInvalidUsageException>(() => domainMsg.Filter());
            Assert.StartsWith("Filter element is not set", e.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, e.ErrorCode);
        }
    }
}
