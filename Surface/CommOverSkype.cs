using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using SKYPE4COMLib;
using System.Threading;
using System.Security.Cryptography;

namespace watchasayin.Surface
{
    /// <summary>
    /// Implementation of ISurfaceCommunication using Skype built-in message-service for communication.
    /// </summary>
    class CommOverSkype : ISurfaceCommunication
    {
        private List<DSurfaceMessageHandler> _surfaceMessageHandlers;
        private Dictionary<String, DSurfaceSpecificMessageHandler> _surfaceSpecificMessageHandlers;

        private Queue<UndeployedMessage> undeployedMessages;
        private int maxUndeployedMessages;

        private List<int> knownMessages;
        private Dictionary<String, int> firstMessages;

        /// <inheritdoc />
        public List<DSurfaceMessageHandler> surfaceMessageHandlers
        {
            get { return _surfaceMessageHandlers; }
        }

        internal Dictionary<string, DSurfaceSpecificMessageHandler> surfaceSpecificMessageHandlers
        {
            get { return _surfaceSpecificMessageHandlers; }
        }

        private Skype skype;
        
        /// <summary>
        /// The public constructor.
        /// </summary>
        /// <param name="skype">A connected SKYPE4COMLib.Skype object.</param>
        /// <param name="sizeUnknownMessagesCache">Number of messages with no registered handlers to save. These messages will be deployed as soon as a matching handler gets registered.</param>
        public CommOverSkype(Skype skype, int sizeUnknownMessagesCache = 0)
        {
            this.skype = skype;
            this.maxUndeployedMessages = sizeUnknownMessagesCache;

            _surfaceMessageHandlers = new List<DSurfaceMessageHandler>();
            _surfaceSpecificMessageHandlers = new Dictionary<string, DSurfaceSpecificMessageHandler>();
     
            skype.MessageStatus += onSkypeMessageStatus;

            undeployedMessages = new Queue<UndeployedMessage>();
            knownMessages = new List<int>();
            firstMessages = new Dictionary<string, int>();
        }

        /// <inheritdoc />
        public void sendMessage(string handleRecepient, SurfaceMessage srfMsg)
        {
            lock (this)
            {
                skype.SendMessage(handleRecepient, srfMsg.text);
                Logger.log(TLogLevel.logEverything, "Info: Sent SrfMsg " + srfMsg.type + ": " + srfMsg.payload);
            }
        }

        /// <inheritdoc />
        public ISurfaceCommunicationSpecific getSpecificInstance(string handleParticipant)
        {
            return new CommOverSkypeSpecific(handleParticipant, this);
        }

        private void onSkypeMessageStatus(ChatMessage message, TChatMessageStatus status)
        {
            Logger.log(TLogLevel.logEverything, "Info: " + status + "Id: " + message.Id + " " + message.Body);
            //we're only processing received messages
            //note: when the chat messages window is open cmsReceived is skipped and the first state is cmsRead
            switch (status)
            {
                case TChatMessageStatus.cmsRead:
                case TChatMessageStatus.cmsReceived:
                    break;
                
                default:
                    return;
            }

            lock(this)
            {
                message.Seen = true;
                Stack<SurfaceMessage> srfMsgsToProcess = new Stack<SurfaceMessage>();
              
                //check if this is the first message of the current session
                if (firstMessages.ContainsKey(message.FromHandle) == false)
                {
                    //we got the first message in this session, just save its id
                    firstMessages[message.FromHandle] = message.Id;
                }
                    
                //check all message till the first one of this session if they were already processed

                /**
                 * Note: skype.Messages[message.FromHandle] is necesarry, since message.Chat.Messages 
                 * is never updated after once accessed.
                 **/
                Logger.log(TLogLevel.logEverything, "Info: Walking through messages:");
                foreach (ChatMessage m in skype.Messages[message.FromHandle])
                {
                    Logger.log(TLogLevel.logEverything, "\tId: " + m.Id );

                    if (m.Id < firstMessages[message.FromHandle])
                    {
                        break;
                    }

                    if (knownMessages.Contains(m.Id) == false)
                    {
                        //we got a new message
                        knownMessages.Add(m.Id);
                        SurfaceMessage srfMsg = new SurfaceMessage(m.Body);
                        if (srfMsg.valid == true)
                        {
                            Logger.log(TLogLevel.logEverything, "Info: Got Surface Message: " + srfMsg.payload);
                            srfMsgsToProcess.Push(srfMsg);
                        }
                    }

                    if (m.Id == firstMessages[message.FromHandle])
                    {
                        break;
                    }
                }

                foreach (SurfaceMessage srfMsg in srfMsgsToProcess)
                {
                    //call specific message handler
                    if (this._surfaceSpecificMessageHandlers.ContainsKey(message.FromHandle))
                    {
                        this._surfaceSpecificMessageHandlers[message.FromHandle](srfMsg);
                    }
                    else
                    {
                        //if no specific message handler is present, save the message for later deployment
                        Logger.log(TLogLevel.logDebug, "Info: No listener registered for SrfMsg: " + srfMsg.payload);

                        //lock access to undeployedMessages
                        Monitor.Enter(undeployedMessages);

                        if (undeployedMessages.Count >= maxUndeployedMessages)
                        {
                            undeployedMessages.Dequeue();
                        }
                        undeployedMessages.Enqueue(new UndeployedMessage(srfMsg, message.FromHandle));

                        Monitor.Exit(undeployedMessages);
                    }

                    //finally call generic message handlers
                    foreach (DSurfaceMessageHandler srfMsgHandler in this._surfaceMessageHandlers)
                    {
                        srfMsgHandler(message.FromHandle, srfMsg);
                    }
                }
            }
        }

        /// <summary>
        /// Tries to deploy yet undeployed messages from the given sender to the respective handler.
        /// </summary>
        /// <param name="handleSender">The handle of the sender to deploy the messages for.</param>
        /// <param name="messageType">The type of the messages to take into account.</param>
        internal void processUndeployedMessages(string handleSender, TSurfaceMessage messageType)
        {
            if (_surfaceSpecificMessageHandlers.ContainsKey(handleSender) == false)
            {
                return;
            }

            Monitor.Enter(undeployedMessages);
            Queue<UndeployedMessage> tempUm = new Queue<UndeployedMessage>();
            DSurfaceSpecificMessageHandler msgHandler = _surfaceSpecificMessageHandlers[handleSender];
            foreach (UndeployedMessage um in undeployedMessages)
            {
                if (um.fromHandle == handleSender && um.surfaceMessage.type == messageType)
                {
                    msgHandler(um.surfaceMessage);
                }
                else
                {
                    tempUm.Enqueue(um);
                }
            }
            undeployedMessages.Clear();
            foreach (UndeployedMessage um in tempUm)
            {
                undeployedMessages.Enqueue(um);
            }
            Monitor.Exit(undeployedMessages);
        }

        private class UndeployedMessage
        {
            private SurfaceMessage _surfaceMessage;
            public SurfaceMessage surfaceMessage { get { return _surfaceMessage; } }

            private String _fromHandle;
            public String fromHandle { get { return _fromHandle; } }

            public UndeployedMessage (SurfaceMessage surfaceMessage, String fromHandle)
            {
                this._fromHandle = fromHandle;
                this._surfaceMessage = surfaceMessage;
            }
        }
    }

    /// <summary>
    /// Implementation of ISurfaceCommunicationSpecific using Skype built-in message-service for communication.
    /// </summary>
    class CommOverSkypeSpecific : ISurfaceCommunicationSpecific
    {
        private string handleParticipant;
        private CommOverSkype commOverSkype;

        /// <inheritdoc />
        public DSurfaceSpecificMessageHandler surfaceSpecificMessageHandler 
        { 
            set 
            { 
                commOverSkype.surfaceSpecificMessageHandlers[handleParticipant] = value; 

                //check for peding messages
                commOverSkype.processUndeployedMessages(handleParticipant, TSurfaceMessage.Syn);
            } 
        }

        internal CommOverSkypeSpecific(string handleParticipant, CommOverSkype commOverSkype)
        {
            this.handleParticipant = handleParticipant;
            this.commOverSkype = commOverSkype;
        }

        /// <inheritdoc />
        public void sendMessage(SurfaceMessage srfMsg)
        {
            commOverSkype.sendMessage(handleParticipant, srfMsg);
        }
    }
}
