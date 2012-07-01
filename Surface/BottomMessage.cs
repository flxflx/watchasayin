using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace watchasayin.Bottom
{
    /// <summary>
    /// The different valid messages of the Bottom protocol.
    /// </summary>
    public enum TBottomMessage
    {
        /// <summary>
        /// Requests the list of open connections from the participant. Contains a list of own connections.
        /// </summary>
        RequestConnections = 0,

        /// <summary>
        /// Contains a list of open connections.
        /// </summary>
        MyConnections,

        /// <summary>
        /// Contains a list of found matches between the participant's 
        /// list of open connections and own open connections.
        /// </summary>
        Matches,

        /// <summary>
        /// Confirms the received list of matches.
        /// </summary>
        Confirm,

        /// <summary>
        /// Aborts a session.
        /// </summary>
        Abort,

        /// <summary>
        /// Any unknown messages.
        /// </summary>
        Unknown,
    }

    /// <summary>
    /// Represents a message of the Bottom protocol.
    /// </summary>
    sealed class BottomMessage : WSMessage<TBottomMessage>
    {
        /// <inheritdoc />
        protected override string XML_MSG_IDENTIFIER { get { return "btmmsg"; } }

        /// <inheritdoc />
        public BottomMessage(string rawXml) : base(rawXml)
        {
        }

        /// <inheritdoc />
        public BottomMessage(XmlDocument xmlDoc) : base(xmlDoc)
        {
        }

        /// <inheritdoc />
        public BottomMessage(TBottomMessage type)
            : base((int)type)
        {
        }

        /// <inheritdoc />
        public BottomMessage(TBottomMessage type, string payload)
            : base((int)type, payload)
        {
        }

        protected override bool validType(int iType)
        {
            if (iType >= (int)TBottomMessage.Unknown)
            {
                return false;
            }
            return true;
        }
        protected override TBottomMessage intToMessageType(int iType)
        {
            return (TBottomMessage)iType;
        }
    }

}
