using System;
using System.Collections.Specialized;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SKYPE4COMLib;
using System.Threading;


namespace watchasayin.Surface.SkypeSpecific
{
    /// <summary>
    /// Implementation of IEventDispatcher using SKYPE4COMLib.
    /// </summary>
    class APIEventHandler : IEventDispatcher
    {
        private List<DCallStartedHandler> _callStartedHandlers;
        private List<DCallEndedHandler> _callEndedHandlers;
        private List<DChatMessageHandler> _chatMessageHandlers;

        /// <inheritdoc />
        public List<DCallStartedHandler> callStartedHandlers
        {
            get {return _callStartedHandlers;}
            set {}
        }
        /// <inheritdoc />
        public List<DCallEndedHandler> callEndedHandlers
        {
            get { return _callEndedHandlers; }
            set {}
        }
        /// <inheritdoc />
        public List<DChatMessageHandler> chatMessageHandlers
        {
            get { return _chatMessageHandlers; }
            set {}
        }

        private Skype skype;

        /// <summary>
        /// The public constructor.
        /// </summary>
        /// <param name="skype">A connected Skype4COM.Skype object.</param>
        public APIEventHandler(Skype skype)
        {
            this.skype = skype;

            _callStartedHandlers = new List<DCallStartedHandler>();
            _callEndedHandlers = new List<DCallEndedHandler>();
            _chatMessageHandlers = new List<DChatMessageHandler>();

            skype.MessageStatus += onSkypeMessageStatus;
            skype.CallStatus += onSkypeCallStatus;
        }

        private void onSkypeCallStatus(Call call, TCallStatus status)
        {
         
            
            switch (status)
            {
                    //new call
                    //TODO: check why clsInprogress doesnt work here!
                case TCallStatus.clsRinging:
                    TCallType type = call.Type;

                    if ((type == TCallType.cltIncomingP2P) || (type == TCallType.cltOutgoingP2P))
                    {
                        foreach (DCallStartedHandler callStartedHandler in callStartedHandlers)
                        {
                            callStartedHandler(call.PartnerHandle, call.Id, type == TCallType.cltOutgoingP2P);
                        }
                    }
                    break;

                    //call closed
                case TCallStatus.clsMissed:
                case TCallStatus.clsFinished:
                case TCallStatus.clsRefused:
                case TCallStatus.clsCancelled:
                    foreach (DCallEndedHandler callEndedHandler in callEndedHandlers)
                    {
                        callEndedHandler(call.PartnerHandle, call.Id);
                    }
                    break;

                default:
                    Logger.log(TLogLevel.logEverything, "Info: Call status changed to: " + status);
                    break;

            }
        }

        private void onSkypeMessageStatus(ChatMessage message, TChatMessageStatus status)
        {
            //we're only processing received messages
            if (status != TChatMessageStatus.cmsReceived)
            {
                return;
            }

            //finally call registered WSMessage handlers
            foreach (DChatMessageHandler msgHandler in chatMessageHandlers)
            {
                msgHandler(message.FromHandle, message.Body);
            }
        }
    }
}
