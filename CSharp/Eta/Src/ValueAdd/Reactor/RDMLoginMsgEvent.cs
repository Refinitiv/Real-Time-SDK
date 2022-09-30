
using Refinitiv.Eta.ValueAdd.Rdm;

namespace Refinitiv.Eta.ValueAdd.Reactor
{

    /// <summary>
    /// Event provided to RDMLoginMsgCallback methods.
    /// </summary>
    ///
    /// <seealso cref="ReactorMsgEvent"/>
    public class RDMLoginMsgEvent : ReactorMsgEvent
    {
        internal RDMLoginMsgEvent() : base()
        {
        }

        public LoginMsg? LoginMsg;
    }
}
