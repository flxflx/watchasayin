using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace watchasayin.Kryptonite
{
    /// <summary>
    /// Interface to the Kryptonite module. Each Kryptonite module is responsible for accomplishing the following tasks:
    /// 
    /// 1. Provide the designated Surface module with cryptographic services.
    ///  - Secure authenticated key-exchange (AKE) with a remote Kryptonite module.
    ///  - Message en/decryption.
    ///  - Assurance of freshness of messages.
    ///  - Assurance of authenticity of messages.
    /// 
    /// 2. Enable the user to manage their private keys. E.g. providing a GUI.
    /// 
    /// 3. Enable the user to validate public keys/fingerprints of communication partners.
    /// </summary>
    public interface IKryptonite : Surface.IModule
    {
    }

    /// <summary>
    /// Extends the IKryptonite interface with functionality to receive and send Kryptonite messages via an external provider.
    /// </summary>
    public interface IKryptoniteExternalCommunication : IKryptonite
    {
        /// <summary>
        /// Returns a communication-partner bound Kryptonite instance for usage with an external communication provider.
        /// </summary>
        /// <param name="handlePartner">Name of the communication-partner to bind the instance to.</param>
        /// <returns>The communication-partner bound instance.</returns>
        IKryptoniteExternalCommunicationSpecific getSpecificInstance(String handlePartner);
    }

    /// <summary>
    /// A session bound specific version of IKryptoniteExternalCommunication.
    /// </summary>
    public interface IKryptoniteExternalCommunicationSpecific : Surface.IModuleExternalCommunicationSpecific
    {
        /// <summary>
        /// Requests the establishment of a secure connection.
        /// </summary>
        /// <param name="primaryActor">Flag indicating, whether this side should act as primary actor during the Kryptonite protocol run.</param>
        /// <returns>A flag indicating success.</returns>
        bool establishSecureConnection(bool primaryActor);

        /// <summary>
        /// Requests the decryption of an encrypted message received from the respective communication-partner.
        /// </summary>
        /// <param name="cipherMessage">The message to decrypt.</param>
        /// <returns>the decrypted message.</returns>
        String decryptMessage(String cipherMessage);

        /// <summary>
        /// Requests the encryption of a message to be sent to the respective communication-partner.
        /// </summary>
        /// <param name="plainMessage">The message to encrypt.</param>
        /// <returns>The encrypted message.</returns>
        String encryptMessage(String plainMessage);

        /// <summary>
        /// A flag indicating whether the current session is secure.
        /// </summary>
        bool isSecure { get; }

        /// <summary>
        /// Returns the cryptographically secure, symmetric key that both parties agreed on.
        /// Throws an exception on failure.
        /// </summary>
        /// <param name="bits">Requested bit-strength of the key.</param>
        /// <param name="algorithm">Requested cryptographic algorithm of the key.</param>
        /// <returns>The key.</returns>
        CryptoKey getSessionKey(int bits, TCryptoAlgo algorithm);

        /// <summary>
        /// Closes the connection.
        /// </summary>
        /// <returns>A flag indicating success.</returns>
        bool close();
    }

    /// <summary>
    /// Exception is thrown when a severe error occurs during the connection-selection phase.
    /// </summary>
    public class ExceptionCryptoFailed : System.ApplicationException
    {
        /// <summary>
        /// The public constructor.
        /// </summary>
        public ExceptionCryptoFailed() { }
        /// <summary>
        /// The public constructor.
        /// </summary>
        /// <param name="message">A message describing the exception.</param>
        public ExceptionCryptoFailed(string message) { }
    }
}
