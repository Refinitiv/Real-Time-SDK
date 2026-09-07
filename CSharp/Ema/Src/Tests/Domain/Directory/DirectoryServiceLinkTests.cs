/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Domain.Directory;
using LSEG.Ema.Rdm;
using static LSEG.Ema.Access.Tests.TestUtilities;

namespace LSEG.Ema.Access.Tests.Domain.Directory
{
    public class DirectoryServiceLinkTests
    {
        [Fact]
        public void Name_should_throw_exception_on_null_value()
        {
            // Arrange
            var serviceLink = new DirectoryServiceLink();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceLink.Name(null!));
            Assert.Equal("name cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Type_should_throw_exception_on_invalid_value()
        {
            // Arrange
            var serviceLink = new DirectoryServiceLink();
            var invalidValue = GetInvalidEnumValue<UpstreamSourceType>();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceLink.Type(invalidValue));
            Assert.Equal($"Invalid element value {invalidValue} of {EmaRdm.ENAME_TYPE}", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void LinkCode_should_throw_exception_on_invalid_value()
        {
            // Arrange
            var serviceLink = new DirectoryServiceLink();
            var invalidValue = GetInvalidEnumValue<LinkCode>();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceLink.LinkCode(invalidValue));
            Assert.Equal($"Invalid element value {invalidValue} of {EmaRdm.ENAME_LINK_CODE}", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Text_should_throw_exception_on_null_value()
        {
            // Arrange
            var serviceLink = new DirectoryServiceLink();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceLink.Text(null!));
            Assert.Equal("text can not be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }
    }
}
