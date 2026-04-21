using System.Text.Json;
using Xunit.Sdk;

namespace LSEG.Eta.Tests.Common.Xunit
{
    public class JsonXunitSerializer : IXunitSerializer
    {
        public object Deserialize(Type type, string serializedValue) => JsonSerializer.Deserialize(serializedValue, type)!;

        public string Serialize(object value) => JsonSerializer.Serialize(value);

        public bool IsSerializable(Type type, object? value, out string failureReason)
        {
            failureReason = null!;
            return true;
        }
    }
}
