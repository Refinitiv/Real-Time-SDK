using LSEG.Ema.Domain.Common;
using LSEG.Ema.Domain.Directory;
using System;
using System.Collections.Generic;

namespace LSEG.Ema.Access.Tests.Domain.Directory
{
    public class DirectoryServiceTests
    {
        [Fact]
        public void GroupStateList_should_throw_exception_on_null_list()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.GroupStateList((IList<DirectoryServiceGroup>)null!));
            Assert.Equal("GroupStateList cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void GroupStateList_should_throw_exception_on_null_action()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.GroupStateList((Action<IFluentListBuilder<DirectoryServiceGroup>>)null!));
            Assert.Equal("buildAction cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Info_should_throw_exception_on_get_when_value_not_set()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.Info());
            Assert.Equal("Info element is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void Info_should_throw_exception_on_null_value()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.Info((DirectoryServiceInfo)null!));
            Assert.Equal("Info cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Info_should_throw_exception_on_null_action()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.Info((Action<DirectoryServiceInfo>)null!));
            Assert.Equal("buildAction cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void State_should_throw_exception_on_get_when_value_not_set()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.State());
            Assert.Equal("State element is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void State_should_throw_exception_on_null_value()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.State((DirectoryServiceState)null!));
            Assert.Equal("State cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void State_should_throw_exception_on_null_action()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.State((Action<DirectoryServiceState>)null!));
            Assert.Equal("buildAction cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Data_should_throw_exception_on_get_when_value_not_set()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.Data());
            Assert.Equal("Data element is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void Data_should_throw_exception_on_null_value()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.Data((DirectoryServiceData)null!));
            Assert.Equal("Data cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Data_should_throw_exception_on_null_action()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.Data((Action<DirectoryServiceData>)null!));
            Assert.Equal("buildAction cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Load_should_throw_exception_on_get_when_value_not_set()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.Load());
            Assert.Equal("Load element is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void Load_should_throw_exception_on_null_value()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.Load((DirectoryServiceLoad)null!));
            Assert.Equal("Load cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Load_should_throw_exception_on_null_action()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.Load((Action<DirectoryServiceLoad>)null!));
            Assert.Equal("buildAction cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Link_should_throw_exception_on_get_when_value_not_set()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.Link());
            Assert.Equal("Link element is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void Link_should_throw_exception_on_null_value()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.Link((DirectoryServiceLinkInfo)null!));
            Assert.Equal("Link cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Link_should_throw_exception_on_null_action()
        {
            // Arrange
            var service = new DirectoryService();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => service.Link((Action<DirectoryServiceLinkInfo>)null!));
            Assert.Equal("buildAction cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }
    }
}
