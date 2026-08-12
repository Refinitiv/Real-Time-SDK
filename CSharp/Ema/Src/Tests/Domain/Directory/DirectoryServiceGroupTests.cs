using LSEG.Ema.Domain.Directory;

namespace LSEG.Ema.Access.Tests.Domain.Directory
{
    public class DirectoryServiceGroupTests
    {
        [Fact]
        public void Group_should_throw_exception_on_null_value_passed()
        {
            // Arrange
            var serviceGroup = new DirectoryServiceGroup();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceGroup.Group((EmaBuffer)null!));
            Assert.Equal("Group cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void MergedToGroup_should_throw_exception_on_null_value_passed()
        {
            // Arrange
            var serviceGroup = new DirectoryServiceGroup();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceGroup.MergedToGroup((EmaBuffer)null!));
            Assert.Equal("MergedToGroup cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void MergedToGroup_should_throw_exception_on_get_when_value_not_set()
        {
            // Arrange
            var serviceGroup = new DirectoryServiceGroup();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceGroup.MergedToGroup());
            Assert.Equal("MergedToGroup element is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }
    }
}
