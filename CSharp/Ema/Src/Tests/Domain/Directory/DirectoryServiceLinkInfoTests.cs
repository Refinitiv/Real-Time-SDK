using LSEG.Ema.Domain.Common;
using LSEG.Ema.Domain.Directory;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LSEG.Ema.Access.Tests.Domain.Directory
{
    public class DirectoryServiceLinkInfoTests
    {
        [Fact]
        public void LinkList_should_throw_exception_on_null_list()
        {
            // Arrange
            var linkInfo = new DirectoryServiceLinkInfo();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => linkInfo.LinkList((IList<DirectoryServiceLink>)null!));
            Assert.Equal("linkList cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void LinkList_should_throw_exception_on_null_action()
        {
            // Arrange
            var linkInfo = new DirectoryServiceLinkInfo();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => linkInfo.LinkList((Action<IFluentListBuilder<DirectoryServiceLink>>)null!));
            Assert.Equal("buildAction cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }
    }
}
