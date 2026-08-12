using LSEG.Ema.Domain.Internal;

namespace LSEG.Ema.Access.Tests.Domain.Internal
{
    public class OptionalFieldTests
    {
        [Fact]
        public void HasValue_should_be_false_by_default()
        {
            // Arrange
            var optionalField = new OptionalField<int>();
            // Act & Assert
            Assert.False(optionalField.HasValue);
        }

        [Fact]
        public void HasValue_should_be_true_after_setting_value()
        {
            // Arrange
            var optionalField = new OptionalField<int>();
            // Act
            optionalField.Value = 42;
            // Assert
            Assert.True(optionalField.HasValue);
        }

        [Fact]
        public void Value_should_throw_exception_if_value_is_not_set()
        {
            // Arrange
            var optionalField = new OptionalField<int>();
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => { var value = optionalField.Value; });
            Assert.Equal("Value is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void Value_should_throw_exception_with_name_if_value_is_not_set()
        {
            // Arrange
            var optionalField = new OptionalField<int>("Some property");
            // Act & Assert
            var exception = Assert.Throws<OmmInvalidUsageException>(() => { var value = optionalField.Value; });
            Assert.Equal("Some property is not set", exception.Message);
            Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION, exception.ErrorCode);
        }

        [Fact]
        public void Value_should_return_set_value()
        {
            // Arrange
            var optionalField = new OptionalField<string>();
            // Act
            optionalField.Value = "Test value";
            // Assert
            Assert.Equal("Test value", optionalField.Value);
        }

        [Fact]
        public void Clear_should_reset_HasValue()
        {
            // Arrange
            var optionalField = new OptionalField<double>();
            optionalField.Value = 3.14;
            // Act
            optionalField.Clear();
            // Assert
            Assert.False(optionalField.HasValue);
        }

        [Fact]
        public void Clear_should_not_throw_exception_when_value_is_not_set()
        {
            // Arrange
            var optionalField = new OptionalField<int>();
            // Act & Assert
            optionalField.Clear(); // Should not throw
            Assert.False(optionalField.HasValue);
        }

        [Fact]
        public void CopyFrom_should_copy_value_and_HasValue()
        {
            // Arrange
            var source = new OptionalField<string>();
            source.Value = "Source value";
            var target = new OptionalField<string>();
            // Act
            target.CopyFrom(source);
            // Assert
            Assert.True(target.HasValue);
            Assert.Equal("Source value", target.Value);
        }

        [Fact]
        public void CopyFrom_should_copy_HasValue_as_false_when_source_has_no_value()
        {
            // Arrange
            var source = new OptionalField<int>();
            var target = new OptionalField<int>();
            target.Value = 100; // Set some value before copying
            // Act
            target.CopyFrom(source);
            // Assert
            Assert.False(target.HasValue);
        }

        [Fact]
        public void ToString_should_return_expected_string_when_value_is_set()
        {
            // Arrange
            var optionalField = new OptionalField<int>();
            optionalField.Value = 123;
            // Act
            var result = optionalField.ToString();
            // Assert
            Assert.Equal("123", result);
        }

        [Fact]
        public void ToString_should_return_expected_string_when_value_is_not_set()
        {
            // Arrange
            var optionalField = new OptionalField<string>();
            // Act
            var result = optionalField.ToString();
            // Assert
            Assert.Equal("<no value>", result);
        }

        [Fact]
        public void ToString_should_return_expected_string_when_value_is_null()
        {
            // Arrange
            var optionalField = new OptionalField<string?>();
            optionalField.Value = null; // Setting value to null should be treated as having a value
            // Act
            var result = optionalField.ToString();
            // Assert
            Assert.Equal("<null>", result);
        }
    }
}
