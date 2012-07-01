using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.IO;

namespace watchasayin
{
    /// <summary>
    /// The valid types for a generic message.
    /// </summary>
    public enum TGenericMessage
    {
        /// <summary>
        /// A messsage of unknown type.
        /// </summary>
        unknown,
    }

    /// <summary>
    /// Base class for messages. Every message class should be derived from this one.
    /// </summary>
    /// <typeparam name="TMessage"></typeparam>
    public abstract class WSMessage<TMessage>
    {
        /// <summary>
        /// The entire message as XML document.
        /// </summary>
        protected XmlDocument xmlDoc;

        /// <summary>
        /// Flag indicating whether the message is valid.
        /// </summary>
        protected bool _valid;

        private TMessage _type;

        /// <summary>
        /// The type of the message.
        /// </summary>
        public TMessage type { get { return _type; } }

        /// <summary>
        /// String identifying the specific message type.
        /// </summary>
        protected abstract string XML_MSG_IDENTIFIER {get;}

        /// <summary>
        /// Checks whether the given type index is valid for the specific message type.
        /// </summary>
        /// <param name="iType">The type index.</param>
        /// <returns>Flag indicating the validness of the type index.</returns>
        protected abstract bool validType(int iType);

        /// <summary>
        /// Translates a certain type index to the coressponding type.
        /// </summary>
        /// <param name="iType">The type index.</param>
        /// <returns>The corresponding message type.</returns>
        protected abstract TMessage intToMessageType(int iType);


        private const string XML_TYPE = "type";

        /// <summary>
        /// Creates a message from a string in XML format.
        /// </summary>
        /// <param name="rawXml">The XML string.</param>
        protected WSMessage(string rawXml)
        {
            StringReader stringReader = new StringReader(rawXml);
            XmlTextReader xmlReader = new XmlTextReader(stringReader);
            xmlDoc = new XmlDocument();
            
            //try loading the xml data
            try
            {
                xmlDoc.Load(xmlReader);
                _valid = (xmlDoc.DocumentElement.Name == XML_MSG_IDENTIFIER);
            }
            catch (XmlException)
            {
                _valid = false;
            }
            
            //if data is has no valid xml format, stop here.
            if (_valid == false)
            {
                return;
            }
            
            //try getting the type of the message
            try
            {
                int iType = (int)(Convert.ToDecimal(xmlDoc.DocumentElement.GetAttribute(XML_TYPE)));
                if (validType(iType) != true)
                {
                    _valid = false;
                }
                else
                {
                    _type = intToMessageType(iType);
                }
            }
            catch (Exception)
            {
                _valid = false;
            }
        }

        /// <summary>
        /// Creates a message from an XML document.
        /// </summary>
        /// <param name="xmlDoc">The XML document.</param>
        protected WSMessage(XmlDocument xmlDoc)
        {
            _valid = (xmlDoc.DocumentElement.Name == XML_MSG_IDENTIFIER);
            this.xmlDoc = xmlDoc;
        }

        /// <summary>
        /// Creates a payloadless message of the given type.
        /// </summary>
        /// <param name="type">The message type</param>
        protected WSMessage(int type)
        {
            xmlDoc = new XmlDocument();
            xmlDoc.LoadXml("<" + XML_MSG_IDENTIFIER + "/>");
            xmlDoc.DocumentElement.SetAttribute(XML_TYPE, "" + type);
        }

        /// <summary>
        /// Creates a message of the given type and appends the given payload.
        /// </summary>
        /// <param name="type">The message type.</param>
        /// <param name="payload">The payload to append.</param>
        protected WSMessage(int type, string payload)
            : this(type)
        {
            xmlDoc.DocumentElement.InnerText = payload;
        }

        /// <summary>
        /// Flag indicating the validness of the message.
        /// </summary>
        public bool valid
        {
            get { return _valid; }
        }
        
        /// <summary>
        /// The message as an XML document.
        /// </summary>
        public XmlDocument xml
        {
            get { return xmlDoc; }
        }

        /// <summary>
        /// The message as text (XML format).
        /// </summary>
        public string text
        {
            get
            {
                StringWriter stringWriter = new StringWriter();
                XmlTextWriter xmlWriter = new XmlTextWriter(stringWriter);

                xmlDoc.WriteTo(xmlWriter);
                return stringWriter.ToString();
            }
        }

        /// <summary>
        /// The payload as raw text.
        /// </summary>
        public string payload
        {
            get
            {
                return xmlDoc.DocumentElement.InnerText;
            }
        }
    }
}
