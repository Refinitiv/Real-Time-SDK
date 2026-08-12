using LSEG.Ema.Domain.Directory;
using LSEG.Ema.Rdm;
using static LSEG.Ema.Access.Tests.TestUtilities;

namespace LSEG.Ema.Access.Tests.Domain.Directory
{
    public class DirectoryRequestMsgTests
    {
        private DirectoryRequestMsg CreateSampleDomainMessage() =>
            new DirectoryRequestMsg()
                .Filter(DirectoryFilters.SERVICE_INFO_FILTER | DirectoryFilters.SERVICE_GROUP_FILTER | DirectoryFilters.SERVICE_LINK_FILTER)
                .ServiceId(123)
                .ServiceName("TestService")
                .InitialImage(false)
                .InterestAfterRefresh(false);

        private void AssertDomainMsgEqual(DirectoryRequestMsg expected, DirectoryRequestMsg actual)
        {
            AssertEqualBy(expected, actual, x => x.Filter());
            AssertEqualBy(expected, actual, x => x.ServiceId());
            AssertEqualBy(expected, actual, x => x.ServiceName());
            AssertEqualBy(expected, actual, x => x.InitialImage());
            AssertEqualBy(expected, actual, x => x.InterestAfterRefresh());
        }

        [Fact]
        public void Message_should_decode_previously_encoded()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();
            var emaMsg = domainMsg.Message().MarkForClear();

            var decodedEmaMsg = new RequestMsg(emaMsg).MarkForClear();

            // Act
            var decodedDomainMsg = new DirectoryRequestMsg().Message(decodedEmaMsg);

            // Assert
            AssertDomainMsgEqual(domainMsg, decodedDomainMsg);
        }

        [Fact]
        public void Message_should_throw_exception_on_decoding_message_with_wrong_domain_type()
        {
            // Arrange
            var domainMsg = new DirectoryRequestMsg();
            var emaMsg = new RequestMsg().MarkForClear();
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
            var domainMsg = new DirectoryRequestMsg();
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
            var copiedDomainMsg = new DirectoryRequestMsg().CopyFrom(domainMsg);

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
            Assert.Equal(DirectoryFilters.ALL, domainMsg.Filter());
            Assert.False(domainMsg.HasServiceId);
            Assert.False(domainMsg.HasServiceName);
            Assert.True(domainMsg.InitialImage());
            Assert.True(domainMsg.InterestAfterRefresh());
        }

        [Fact]
        public void ToString_should_output_expected_string_for_normal_instance()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage();

            // Act
            var actualString = domainMsg.ToString();
            // Assert
            Assert.Equal(@"Filter: SERVICE_INFO_FILTER, SERVICE_GROUP_FILTER, SERVICE_LINK_FILTER
DomainType: Directory Domain
ServiceId: 123
ServiceName: TestService
InitialImage: False
InterestAfterRefresh: False
", actualString);
        }

        [Fact]
        public void ToString_should_output_expected_string_for_clean_instance()
        {
            // Arrange
            var domainMsg = CreateSampleDomainMessage().Clear();

            // Act
            var actualString = domainMsg.ToString();
            // Assert
            Assert.Equal(@"Filter: ALL
DomainType: Directory Domain
InitialImage: True
InterestAfterRefresh: True
", actualString);
        }

        [Theory]
        [InlineData(null)]
        [InlineData("")]
        public void ServiceName_should_throw_exception_if_value_is_null_or_empty(string? value)
        {
            // Arrange
            var domainMsg = new DirectoryRequestMsg();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => domainMsg.ServiceName(value!));
            Assert.StartsWith("ServiceName cannot be null or empty", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }
    }
}
