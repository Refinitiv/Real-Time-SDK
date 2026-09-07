/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Domain.Internal;
using LSEG.Eta.Codec;

namespace LSEG.Ema.Access.Tests.Domain.Internal
{
    public class StateFieldTests
    {
        [Fact]
        public void HasValue_should_be_false_by_default()
        {
            // Arrange
            var stateField = new StateField();
            // Act & Assert
            Assert.False(stateField.HasValue);
        }

        [Fact]
        public void HasValue_should_be_true_after_setting_value()
        {
            // Arrange
            var stateField = new StateField();
            // Act
            stateField.Value(1, 2, 3, "Test status");
            // Assert
            Assert.True(stateField.HasValue);
        }

        [Fact]
        public void Value_should_throw_exception_if_state_is_not_set()
        {
            // Arrange
            var stateField = new StateField();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => stateField.Value());
            Assert.Equal("State element is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void Value_should_throw_exception_when_OmmState_is_null()
        {
            // Arrange
            var stateField = new StateField();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => stateField.Value(null!));
            Assert.Equal("State cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Value_should_throw_exception_when_status_text_is_null()
        {
            // Arrange
            var stateField = new StateField();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => stateField.Value(1, 2, 3, null!));
            Assert.Equal("statusText cannot be null", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT, exception.ErrorCode);
        }

        [Fact]
        public void Value_should_set_and_get_state_properly()
        {
            // Arrange
            var stateField = new StateField();
            // Act
            stateField.Value(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NOT_FOUND, "Test status");
            // Assert
            Assert.True(stateField.HasValue);
            var actualState = stateField.Value();
            Assert.Equal(OmmState.StreamStates.OPEN, actualState.StreamState);
            Assert.Equal(OmmState.DataStates.OK, actualState.DataState);
            Assert.Equal(OmmState.StatusCodes.NOT_FOUND, actualState.StatusCode);
            Assert.Equal("Test status", actualState.StatusText);
        }

        [Fact]
        public void Value_should_set_state_properly_more_than_once()
        {
            // Arrange
            var stateField = new StateField();
            // Act
            stateField.Value(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "another status");
            var actualState = stateField.Value();
            stateField.Value(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.ALREADY_OPEN, "Test status");
            actualState = stateField.Value();
            // Assert
            Assert.Equal(OmmState.StreamStates.OPEN, actualState.StreamState);
            Assert.Equal(OmmState.DataStates.OK, actualState.DataState);
            Assert.Equal(OmmState.StatusCodes.ALREADY_OPEN, actualState.StatusCode);
            Assert.Equal("Test status", actualState.StatusText);
        }

        [Fact]
        public void Clear_should_reset_state_and_HasValue()
        {
            // Arrange
            var stateField = new StateField();
            stateField.Value(1, 2, 3, "Test status");
            // Act
            stateField.Clear();
            // Assert
            Assert.False(stateField.HasValue);
            var exception = Assert.Throws<OmmInvalidUsageException>(() => stateField.Value());
            Assert.Equal("State element is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void CopyFrom_should_copy_state_properly()
        {
            // Arrange
            var source = new StateField();
            source.Value(1, 2, 3, "Test status");
            var target = new StateField();
            // Act
            target.CopyFrom(source);
            // Assert
            Assert.True(target.HasValue);
            var actualState = target.Value();
            var expectedState = source.Value();
            Assert.Equal(expectedState.StreamState, actualState.StreamState);
            Assert.Equal(expectedState.DataState, actualState.DataState);
            Assert.Equal(expectedState.StatusCode, actualState.StatusCode);
            Assert.Equal(expectedState.StatusText, actualState.StatusText);
        }

        [Fact]
        public void ToString_should_return_expected_string_when_state_is_set()
        {
            // Arrange
            var stateField = new StateField();
            // Act
            stateField.Value(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.TIMEOUT, "Timeout occurred");
            // Assert
            var expectedString = "Closed / Suspect / Timeout / 'Timeout occurred'";
            Assert.Equal(expectedString, stateField.ToString());
        }

        [Fact]
        public void ToString_should_return_expected_string_when_state_is_not_set()
        {
            // Arrange
            var stateField = new StateField();
            // Act & Assert
            Assert.Equal("<no value>", stateField.ToString());
        }
    }
}
