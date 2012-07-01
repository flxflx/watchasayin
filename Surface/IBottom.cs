using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using watchasayin.Surface;

namespace watchasayin.Bottom
{
    /// <summary>
    /// Interface to the Bottom module. Each Bottom module is responsible for accomplishing the following tasks:
    /// 
    /// 1. Encryption/decryption of network-traffic (TCP and/or UDP) belonging to a certain session.
    /// <para>This is currently achieved through injecting a .dll into the Skype process. E.g. using LSP might be another fruitful approach.</para>
    /// 
    /// 2. Mapping network-traffic to sessions.
    /// <para>At the current state of development deep packet-inspection is only possible for a small portion of Skype-traffic.
    /// In order to be able to correlate traffic and sessions despite that, the Bottom modules of two participants in a watchasayin session need to communicate with each other.
    /// Therefore task 3 becomes necesarry</para>
    /// 
    /// 3. Communication with other Bottom modules using the Bottom protocol.
    /// <para>The current approach is to transmit Bottom protocol messages inside Surface protocol messages</para>
    /// </summary>
    public interface IBottom : Surface.IModule
    { 
    }

    /// <summary>
    /// Extends the IBottom interface with functionality to receive and send Bottom message via an external provider.
    /// </summary>
    public interface IBottomExternalCommunication : IBottom
    {
        /// <summary>
        /// Returns an communication-partner specific Bottom instance for usage with an external communication.
        /// </summary>
        /// <param name="handlePartner">Name of the communication-partner to bind the instance to.</param>
        /// <returns>The communication-partner speicifc instance</returns>
        IBottomExternalCommunicationSpecific getSpecificInstance(String handlePartner);
    }

    /// <summary>
    /// A session bound version of IBottomExternalComm.
    /// </summary>
    public interface IBottomExternalCommunicationSpecific : Surface.IModuleExternalCommunicationSpecific
    {
        /// <summary>
        /// Requests the encryption of a certain type of traffic.
        /// </summary>
        /// <param name="sessionId">Id of the session.</param>
        /// <param name="toEncrypt">The type of traffic that has to be encrypted.</param>
        /// <param name="key">The cryptographic key to be used.</param>
        /// <param name="primaryActor">Flag indicating, whether this side should act as primary actor during the Bottom protocol run.</param>
        /// <returns>Flag indicating success.</returns>
        bool encrypt(int sessionId, TSession toEncrypt, CryptoKey key, bool primaryActor);

        /// <summary>
        /// Gets the supported bit-strength for a cryptographic key.
        /// </summary>
        int getSupportedKeyBitStrength();

        /// <summary>
        /// Gets the supported algorithm for a cryptographic key.
        /// </summary>
        TCryptoAlgo getSupportedKeyAlgorithm();

        /// <summary>
        /// Closes the connection.
        /// </summary>
        /// <returns>A flag indicating success.</returns>
        bool close();
    }

    /// <summary>
    /// Exception is thrown when a severe error occurs during the encryption set-up phase.
    /// </summary>
    public class ExceptionEncryptionSetUpFailed : System.ApplicationException
    {
        ///
        public ExceptionEncryptionSetUpFailed() { }
        ///
        public ExceptionEncryptionSetUpFailed(string message) { }
        ///
        public ExceptionEncryptionSetUpFailed(string message, System.Exception inner) { }
        ///
        protected ExceptionEncryptionSetUpFailed(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }

    /// <summary>
    /// Exception is thrown when a severe error occurs during the connection-selection phase.
    /// </summary>
    public class ExceptionConnectionSelectionFailed : System.ApplicationException
    {
        ///
        public ExceptionConnectionSelectionFailed() { }
        ///
        public ExceptionConnectionSelectionFailed(string message) { }
        ///
        public ExceptionConnectionSelectionFailed(string message, System.Exception inner) { }
        ///
        protected ExceptionConnectionSelectionFailed(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }
}
