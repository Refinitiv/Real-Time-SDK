/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Domain.Directory;
using LSEG.Eta.Codec;
using System.Linq;

namespace LSEG.Ema.Access.Tests.Domain.Directory
{
    public class DirectoryServiceDataTests
    {
        [Fact]
        public void DataType_should_throw_exception_on_get_when_Data_not_set()
        {
            // Arrange
            var serviceData = new DirectoryServiceData();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceData.DataType);
            Assert.Equal("Data element is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Theory]
        [InlineData(-1)]
        [InlineData(1024)]
        public void Type_should_throw_exception_on_invalid_value(int invalidValue)
        {
            // Arrange
            var serviceData = new DirectoryServiceData();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceData.Type((DirectoryServiceData.ContentType)invalidValue));
            Assert.Equal($"Invalid element value {invalidValue} of Type", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Data_should_throw_exception_when_Data_not_set()
        {
            // Arrange
            var serviceData = new DirectoryServiceData();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceData.Data());
            Assert.Equal("Data element is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void DataAsBuffer_should_throw_exception_on_null_value_passed()
        {
            // Arrange
            var serviceData = new DirectoryServiceData();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceData.DataAsBuffer(null!));
            Assert.Equal("data can not be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void DataAsAscii_should_throw_exception_on_null_value_passed()
        {
            // Arrange
            var serviceData = new DirectoryServiceData();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceData.DataAsAscii(null!));
            Assert.Equal("data can not be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void DataAsUtf8_should_throw_exception_on_null_value_passed()
        {
            // Arrange
            var serviceData = new DirectoryServiceData();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceData.DataAsUtf8(null!));
            Assert.Equal("data can not be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void DataAsRmtes_should_throw_exception_on_null_value_passed()
        {
            // Arrange
            var serviceData = new DirectoryServiceData();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceData.DataAsRmtes(null!));
            Assert.Equal("data can not be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Theory]
        [InlineData(RealHints.EXPONENT_14 - 1)]
        [InlineData(RealHints.NOT_A_NUMBER + 1)]
        public void DataAsReal_should_throw_exception_on_invalid_magnitude_type_passed(int invalidMagnitudeType)
        {
            // Arrange
            var serviceData = new DirectoryServiceData();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceData.DataAsReal(1, invalidMagnitudeType));
            Assert.Equal("magnitudeType should be in range of MagnitudeType values", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void DataAsState_should_throw_exception_on_null_statusText_passed()
        {
            // Arrange
            var serviceData = new DirectoryServiceData();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceData.DataAsState(1, 2, 3, null!));
            Assert.Equal("statusText cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void DataAsComplexType_should_throw_exception_on_null_value_passed()
        {
            // Arrange
            var serviceData = new DirectoryServiceData();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceData.DataAsComplexType(null!));
            Assert.Equal("data cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }
    }
}
