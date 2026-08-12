using LSEG.Ema.Access;

namespace LSEG.Ema.Domain.Internal
{
    /// <summary>
    /// Automatically handles presence and value of an optional field.
    /// </summary>
    /// <remarks>
    /// Works properly only for immutable values.
    /// </remarks>
    /// <typeparam name="T">Type of the optional field value.</typeparam>
    internal struct OptionalField<T>
    {
        private T m_Value;
        private readonly string m_Name;

        public OptionalField(string name)
        {
            m_Name = !string.IsNullOrEmpty(name) ? name : "Value";
            m_Value = default!;
        }

        public OptionalField()
        {
            m_Name = "Value";
            m_Value = default!;
        }

        public T Value
        {
            get => HasValue
                ? m_Value
                : throw new OmmInvalidUsageException($"{m_Name} is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            set
            {
                m_Value = value;
                HasValue = true;
            }
        }

        public bool HasValue { get; private set; }

        public void Clear()
        {
            HasValue = false;
        }

        public void CopyFrom(OptionalField<T> value)
        {
            m_Value = value.m_Value;
            HasValue = value.HasValue;
        }

        public override string ToString() =>
            HasValue
                ? (m_Value?.ToString() ?? "<null>")
                : "<no value>";
    }
}
