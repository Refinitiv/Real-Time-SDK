/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Domain.Common;
using LSEG.Ema.Domain.Directory;
using System;
using System.Collections.Generic;

namespace LSEG.Ema.Access.Tests.Domain.Directory
{
    public class DirectoryServiceInfoTests
    {
        [Fact]
        public void CapabilitiesList_should_throw_exception_on_null_list()
        {
            // Arrange
            var serviceInfo = new DirectoryServiceInfo();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceInfo.CapabilitiesList((IList<ulong>)null!));
            Assert.Equal("CapabilitiesList cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void CapabilitiesList_should_throw_exception_on_null_action()
        {
            // Arrange
            var serviceInfo = new DirectoryServiceInfo();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceInfo.CapabilitiesList((Action<IFluentListBuilder<ulong>>)null!));
            Assert.Equal("buildAction cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void DictionariesProvidedList_should_throw_exception_on_null_list()
        {
            // Arrange
            var serviceInfo = new DirectoryServiceInfo();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceInfo.DictionariesProvidedList((IList<string>)null!));
            Assert.Equal("DictionariesProvidedList cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void DictionariesProvidedList_should_throw_exception_on_null_action()
        {
            // Arrange
            var serviceInfo = new DirectoryServiceInfo();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceInfo.DictionariesProvidedList((Action<IFluentListBuilder<string>>)null!));
            Assert.Equal("buildAction cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void DictionariesProvidedList_should_throw_exception_when_not_set()
        {
            // Arrange
            var serviceInfo = new DirectoryServiceInfo();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceInfo.DictionariesProvidedList());
            Assert.Equal("DictionariesProvidedList element is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void DictionariesUsedList_should_throw_exception_on_null_list()
        {
            // Arrange
            var serviceInfo = new DirectoryServiceInfo();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceInfo.DictionariesUsedList((IList<string>)null!));
            Assert.Equal("DictionariesUsedList cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void DictionariesUsedList_should_throw_exception_on_null_action()
        {
            // Arrange
            var serviceInfo = new DirectoryServiceInfo();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceInfo.DictionariesUsedList((Action<IFluentListBuilder<string>>)null!));
            Assert.Equal("buildAction cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void DictionariesUsedList_should_throw_exception_when_not_set()
        {
            // Arrange
            var serviceInfo = new DirectoryServiceInfo();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceInfo.DictionariesUsedList());
            Assert.Equal("DictionariesUsedList element is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void QosList_should_throw_exception_on_null_list()
        {
            // Arrange
            var serviceInfo = new DirectoryServiceInfo();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceInfo.QosList((IList<DirectoryQos>)null!));
            Assert.Equal("QosList cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void QosList_should_throw_exception_on_null_action()
        {
            // Arrange
            var serviceInfo = new DirectoryServiceInfo();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceInfo.QosList((Action<IFluentListBuilder<DirectoryQos>>)null!));
            Assert.Equal("buildAction cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void QosList_should_throw_exception_when_not_set()
        {
            // Arrange
            var serviceInfo = new DirectoryServiceInfo();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => serviceInfo.QosList());
            Assert.Equal("QosList element is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }
    }
}
