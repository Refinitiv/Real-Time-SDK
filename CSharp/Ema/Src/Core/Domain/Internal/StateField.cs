using LSEG.Ema.Access;
using LSEG.Eta.Codec;

namespace LSEG.Ema.Domain.Internal
{
    internal class StateField
    {
        private readonly OmmState m_OmmState = new();
        private readonly State m_RsslState = new();
        private readonly Eta.Codec.Buffer m_StateText = new();
        private bool m_IsDirty = false;

        public StateField()
        {
        }

        public bool HasValue { get; private set; } = false;

        public OmmState Value()
        {
            if (!HasValue)
                throw new OmmInvalidUsageException("State element is not set", OmmInvalidUsageException.ErrorCodes.INVALID_OPERATION);
            return GetOmmState();
        }

        public void Value(OmmState value)
        {
            if (value == null)
                throw new OmmInvalidUsageException("State cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            SetState(value.StreamState, value.DataState, value.StatusCode, value.StatusText);
        }

        public void Value(int streamState, int dataState, int statusCode, string statusText)
        {
            if (statusText == null)
                throw new OmmInvalidUsageException($"{nameof(statusText)} cannot be null", OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT);

            SetState(streamState, dataState, statusCode, statusText);
        }

        public void Clear()
        {
            m_RsslState.Clear();
            m_StateText.Clear();
            m_OmmState.Clear_All();
            HasValue = false;
            m_IsDirty = false;
        }

        public void CopyFrom(StateField source)
        {
            Clear();
            HasValue = source.HasValue;
            if (HasValue)
            {
                var state = source.Value();
                SetState(state.StreamState, state.DataState, state.StatusCode, state.StatusText);
            }
        }

        public override string ToString() => 
            HasValue
                ? GetOmmState().ToString()
                : "<no value>";

        private void SetState(int streamState, int dataState, int statusCode, string statusText)
        {
            m_RsslState.StreamState(streamState);
            m_RsslState.DataState(dataState);
            m_RsslState.Code(statusCode);
            m_StateText.Data(statusText);
            m_RsslState.Text(m_StateText);
            HasValue = true;
            m_IsDirty = true;
        }

        private OmmState GetOmmState()
        {
            if (m_IsDirty)
            {
                m_OmmState.Clear_All();
                m_OmmState.Decode(m_RsslState);
                m_IsDirty = false;
            }
            return m_OmmState;
        }
    }
}
