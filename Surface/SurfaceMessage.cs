using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using watchasayin;
using System.Xml;

namespace watchasayin.Surface
{
    /// <summary>
    /// The different valid messages of the Bottom protocol.
    /// </summary>
    public enum TSurfaceMessage
    {
        /// <summary>
        /// Message contains encapsulated communication between two Bottom components.
        /// </summary>
        Bottom2Bottom = 0,

        /// <summary>
        /// Message contains encapsulated communication between two Kryptonite components (e.g. OTR).
        /// </summary>
        Kryptonite2Kryptonite,

        /// <summary>
        /// Request to start Surface session.
        /// <para>No payload.</para>
        /// </summary>
        Syn, //request watchsayin session

        /// <summary>
        /// Previously received request to start Surface session is accepted.
        /// <para>No payload.</para>
        /// </summary>
        Ack,

        /// <summary>
        /// Previously received request to start Surface session is not accepted.
        /// This could be for example sent in case of a version conflict.
        /// <para>No payload.</para>
        /// </summary>
        Nack, //refuse 

        /// <summary>
        /// Other side has ended to protocol run.
        /// <para>No payload.</para>
        /// </summary>
        Closing,

        /// <summary>
        /// Message is unknown/invalid.
        /// </summary>
        Unknown,
    }

    /// <summary>
    /// Represents a message of the Surface protocol <see cref="Protocol.Session"/>.
    /// </summary>
    sealed class SurfaceMessage : WSMessage<TSurfaceMessage>
    {
        protected override string XML_MSG_IDENTIFIER { get { return "srfmsg"; } }

        /*
        public SurfaceMessage(TSurfaceMessage type, WSMessage<TGenericMessage> encapsulatedMsg) : base(type)
        {
            try
            {
                xmlDoc.DocumentElement.AppendChild(xmlDoc.ImportNode(encapsulatedMsg.xml.DocumentElement, true));
            }
            catch (XmlException)
            {
                _valid = false;
            }
        }
        */
        
        /// <inheritdoc />
        public SurfaceMessage(string rawXml) : base(rawXml)
        {
        }

        /// <inheritdoc />
        public SurfaceMessage(XmlDocument xmlDoc) : base(xmlDoc)
        {
        }

        /// <inheritdoc />
        public SurfaceMessage(TSurfaceMessage type) : base((int)type)
        {
        }

        /// <inheritdoc />
        public SurfaceMessage(TSurfaceMessage type, string payload) : base((int)type, payload)
        {
        }

        /// <inheritdoc />
        protected override bool validType(int iType)
        {
            if (iType >= (int)TSurfaceMessage.Unknown)
            {
                return false;
            }
            return true;
        }

        /// <inheritdoc />
        protected override TSurfaceMessage intToMessageType(int iType)
        {
            return (TSurfaceMessage)iType;
        }
    }
}
