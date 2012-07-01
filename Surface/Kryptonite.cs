using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.ComponentModel;

namespace watchasayin.Kryptonite
{
    /// <summary>
    /// Definition of a user / Kryptonite interaction module.
    /// </summary>
    public interface IUserIO
    {
        //TODO: More sophisticated return values!

        /// <summary>
        /// Displays a notification to the user.
        /// </summary>
        /// <param name="notification">The notification to display.</param>
        /// <returns>A flag indicating whether Kryptonite should go on or not.</returns>
        bool displayNotification(String notification);

        /// <summary>
        /// Tell the user that no private key was found.
        /// </summary>
        /// <returns>A flag indicating whether Kryptonite should go on or not.</returns>
        bool noPrivateKeyFound();

        /// <summary>
        /// Tell the user that their private key is invalid.
        /// </summary>
        /// <returns>A flag indicating whether Kryptonite should go on or not.</returns>
        bool privateKeyInvalid();

        /// <summary>
        /// Tell the user that no fingerprints container was found.
        /// </summary>
        /// <returns>A flag indicating whether Kryptonite should go on or not.</returns>
        bool noFingerprintsFound();

        /// <summary>
        /// Tell the user that the fingerprint file is invalid.
        /// </summary>
        /// <returns>A flag indicating whether Kryptonite should go on or not.</returns>
        bool fingerprintsInvalid();

        //communication-partner specific
        /// <summary>
        /// Display the user the given fingerprint together with the given handle of the communication-partner and ask for confirmation
        /// </summary>
        /// <param name="handlePartner">The handle of the communication-partner.</param>
        /// <param name="fingerprint">The fingerprint to validate.</param>
        /// <returns>A flag indicating whether the user confirmed the fingerprint or not.</returns>
        bool confirmNewFingerprint(String handlePartner, String fingerprint);

        /// <summary>
        /// Displays their own fingerprint tot he user.
        /// </summary>
        /// <param name="fingerprint">The fingerprint to display.</param>
        void displayOwnFingerprint(String fingerprint);

        /// <summary>
        /// Notify the user of the successful completion of the AKE with the given user.
        /// </summary>
        /// <param name="handlePartner">The handle of the communication-partner.</param>
        /// <param name="fingerprint">The fingerprint of the communication-partner.</param>
        void akeSuccessful(String handlePartner, String fingerprint);
    }

    internal enum EventTypeSpecific 
    {
	    DLG_CONFIRM_NEW_FINGERPRINT = 20,
	    INFO_CONNECTION_SECURE = 21,
	    INFO_CONNECTION_STILL_SECURE = 22,
	    INFO_CONNECTION_INSECURE = 23
    };

    internal enum EventTypeGeneric
    {
	    UNKNOWN = 0, 
	    DLG_NO_PRIVATE_KEY_FOUND = 1,
	    DLG_PRIVATE_KEY_INVALID = 2,
	    DLG_NO_FINGERPRINTS_FOUND = 3,
	    DLG_FINGERPRINTS_INVALID = 4
    };

    internal enum ActionType
    {
        FATAL_ERROR = 0,
        NOTHING = 1,
        POLL_GENERIC = 2,
        POLL_SPECIFIC = 4
    };

    /// <summary>
    /// Class implementing the cryptographic functionality required by Kryptonite by loading an unspecified external library
    /// as In-Process COM server and using functionality supplied by that library over COM.  
    /// </summary>
    /// <remarks>
    /// The library has to be named Kryptonite.dll and has to be present in one of Window's default Dll search path.
    /// E.g. the dir the application runs in.           
    /// </remarks>
    public class SharedLibrary : IKryptoniteExternalCommunication
    {
        private KryptoniteCOM.Generic comGeneric;
        private KryptoniteCOM.Crypto comCrypto;
        private IUserIO userIO;

        private String handleSelf;
        private String workingDir;

        [DllImport("Kryptonite.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        static extern int DllInstall(bool bInstall, String cmd);

        /// <summary>
        /// The public constructor.
        /// </summary>
        /// <param name="handleSelf">The handle of the local user</param>
        /// <param name="userIO">An interface for communication with the user.</param>
        /// <param name="workingDir">Path of the directory to work in.</param>
        public SharedLibrary(String handleSelf, IUserIO userIO, String workingDir)
        {
            this.workingDir = workingDir;
            this.handleSelf = handleSelf;
            this.userIO = userIO;
        }

        /// <inheritdoc />
        public bool setUp()
        {
            int retVal;
            try
            {
                retVal = DllInstall(true, "user");
            }
            catch (DllNotFoundException)
            {
                Logger.log(TLogLevel.logUser, "Error: Kryptonite.dll or a runtime library required by it was not found, unable to proceed!");
                return false;
            }

            
            if (retVal != 0)
            {
                Logger.log(TLogLevel.logUser, "Error: Failed to load Kryptonite.dll.");
                return false;
            }
            
            //seems like Kryptonite.dll was loaded successfully
            comGeneric = new KryptoniteCOM.Generic();
            comCrypto = new KryptoniteCOM.Crypto();

            ActionType nextAction = (ActionType)comCrypto.init(handleSelf, workingDir);

            switch (nextAction)
            {
                case ActionType.NOTHING:
                    //everything's fine
                    break;

                case ActionType.POLL_GENERIC:
                    //further action needs to be done
                    Logger.log(TLogLevel.logDebug, "Info: Further action required to successfully init the crypto module.");
                    if (handleNextAction(nextAction) != true)
                    {
                        return false;
                    }
                    break;
                default:
                    Logger.log(TLogLevel.logUser, "Error: Failed to initialize crypto module.");
                    return false;
            }

            //display own fingerprint to the user
            String fingerprint;
            if (comCrypto.getOwnFingerprint(out fingerprint) != 1)
            {
                Logger.log(TLogLevel.logUser, "Fatal Error: Failed to get fingerprint of own private key.");
                Logger.log(TLogLevel.logDebug, "\tHere comes the log:");
                Logger.log(TLogLevel.logDebug, (String[])comGeneric.getLog());
                return false;
            }
            userIO.displayOwnFingerprint(fingerprint);

            return true;
        }

        /// <inheritdoc />
        public bool tearDown()
        {
            //Tell dll to unregister in-process COM server as user.
            return (DllInstall(false, "user") == 0);
        }

        /// <inheritdoc />
        public bool reset()
        {
            //TODO: what shall be done here?
            return false;
        }

        /// <inheritdoc />
        public IKryptoniteExternalCommunicationSpecific getSpecificInstance(String handlePartner)
        {
            return new SessionExternalComm(comGeneric, comCrypto, handlePartner, userIO);
        }

        private bool handleNextAction(ActionType nextAction)
        {
            while (nextAction == ActionType.POLL_GENERIC)
            {
                //first, check for unhandled events
                int eventType;
                string eventData;
                while (comCrypto.getNextEventGeneric(out eventType, out eventData) != 0)
                {
                    switch ((EventTypeGeneric)eventType)
                    {
                        case EventTypeGeneric.DLG_NO_PRIVATE_KEY_FOUND:
                            if (userIO.noPrivateKeyFound() != true)
                            {
                                return false;
                            }
                            nextAction = (ActionType)comCrypto.generatePrivateKey();
                           
                            break;

                        case EventTypeGeneric.DLG_PRIVATE_KEY_INVALID:
                            if (userIO.privateKeyInvalid() != true)
                            {
                                return false;
                            }
                            nextAction = (ActionType)comCrypto.generatePrivateKey();
                            break;

                        case EventTypeGeneric.DLG_NO_FINGERPRINTS_FOUND:
                            if (userIO.noFingerprintsFound() != true)
                            {
                                return false;
                            }
                            nextAction = (ActionType)comCrypto.createFingerprintStore();
                            break;

                        case EventTypeGeneric.DLG_FINGERPRINTS_INVALID:
                            if (userIO.fingerprintsInvalid() != true)
                            {
                                return false;
                            }
                            nextAction = (ActionType)comCrypto.createFingerprintStore();
                            break;

                        default:
                            Logger.log(TLogLevel.logDebug, "Error: Got unknown generic event-type.");
                            return false;
                    }
                }
            }

            if (nextAction == ActionType.FATAL_ERROR)
            {
                String[] lines = (String[])comGeneric.getLog();
                Logger.log(TLogLevel.logDebug, "Fatal error while initializing external Kryptonite module. Here comes the log:");
                Logger.log(TLogLevel.logDebug, lines);
                return false;
            }

            return true;
        }
    }

    internal class SessionExternalComm :  IKryptoniteExternalCommunicationSpecific
    {
        private KryptoniteCOM.Generic comGeneric;
        private KryptoniteCOM.Crypto comCrypto;
        private String handlePartner;
        private IUserIO userIO;

        private bool secure;


        private Surface.DSendMessageSpecific _sendMessage;

        /// <inheritdoc />
        public Surface.DSendMessageSpecific sendMessage { set { _sendMessage = value; } }

        /// <summary>
        /// The public constructor.
        /// </summary>
        /// <param name="comGeneric">An already connected COM object of type KryptoniteCOM.Generic.</param>
        /// <param name="comCrypto">An already connected COM object of type KryptoniteCOM.Crypto.</param>
        /// <param name="handlePartner">The name of the communication-partner to bind the instance to.</param>
        /// <param name="userIO">The interface used for communication with the user.</param>
        public SessionExternalComm(KryptoniteCOM.Generic comGeneric, KryptoniteCOM.Crypto comCrypto, String handlePartner, IUserIO userIO)
        {
            this.comGeneric = comGeneric;
            this.comCrypto = comCrypto;

            this.handlePartner = handlePartner;
            this.userIO = userIO;

            this.secure = false;
        }

        /// <inheritdoc />
        public void messageReceived(String rawMessage)
        {
            String plain;
            int deploy; //in this case deploy should be always zero

            ActionType nextAction;
            try
            {
                nextAction = (ActionType)comCrypto.messageReceived(handlePartner, rawMessage, out plain, out deploy);
            }
            catch
            {
                Logger.log(TLogLevel.logUser, "Fatal Error: An error occured while establishing a secure connection. Here comes the log:");
                String[] lines = (String[])comGeneric.getLog();
                foreach (String line in lines)
                {
                    Logger.log(TLogLevel.logDebug, "\t" + line);
                }
                this.secure = false;
                return;
            }

            if (deploy != 0 || nextAction == ActionType.FATAL_ERROR)
            {
                throw new ExceptionCryptoFailed("Processing Kryptonite message failed.");
            }

            handleNextAction(nextAction);
        }

        /// <inheritdoc />
        public bool establishSecureConnection(bool primaryActor)
        {
            if (primaryActor == false)
            {
                //we just wait for the other side to start
                return true;
            }

            if (handleNextAction((ActionType)comCrypto.startSession(handlePartner)) == false)
            {
                Logger.log(TLogLevel.logDebug, "Error: Could not initiate secure session. Here comes the log:");
                String[] lines = (String[])comGeneric.getLog();
                foreach (String line in lines)
                {
                    Logger.log(TLogLevel.logDebug, "\t" + line);
                }
                return false;
            }

            return true;
        }

        /// <inheritdoc />
        public String decryptMessage(String cipherMessage)
        {
            String plain;
            int deploy; //in this case deploy should be always one
            ActionType nextAction = (ActionType)comCrypto.messageReceived(handlePartner, cipherMessage, out plain, out deploy);

            if (deploy != 1 || nextAction == ActionType.FATAL_ERROR)
            {
                throw new ExceptionCryptoFailed("Decryption of message failed");
            }

            handleNextAction(nextAction);
            return plain;
        }

        /// <inheritdoc />
        public String encryptMessage(String plainMessage)
        {
            String cipher;
            ActionType nextAction = (ActionType)comCrypto.messageSending(handlePartner, plainMessage, out cipher);

            if (nextAction == ActionType.FATAL_ERROR)
            {
                throw new ExceptionCryptoFailed("Encryption of message failed");
            }

            handleNextAction(nextAction);
            return cipher;
        }

        /// <inheritdoc />
        public bool isSecure { get { return secure; } }

        private bool handleNextAction(ActionType nextAction)
        {
            while (nextAction == ActionType.POLL_SPECIFIC)
            {
                //first, check for unhandled events
                int eventType;
                string eventData;
                while (comCrypto.getNextEvent(handlePartner, out eventType, out eventData) != 0)
                {
                    switch ((EventTypeSpecific)eventType)
                    {
                        case EventTypeSpecific.DLG_CONFIRM_NEW_FINGERPRINT:
                            //eventData holds the fingerprint in this case
                            if (userIO.confirmNewFingerprint(handlePartner, eventData) == true)
                            {
                                if ((nextAction = (ActionType)comCrypto.confirmFingerprint(handlePartner, eventData)) == ActionType.FATAL_ERROR)
                                {
                                    Logger.log(TLogLevel.logUser, "Fatal Error: Failed to confirm the last new fingerprint.");
                                    Logger.log(TLogLevel.logDebug, "Here comes the log:");
                                    Logger.log(TLogLevel.logDebug, (String[])comGeneric.getLog());
                                    secure = false;
                                    return false;
                                }
                            }
                            else
                            {
                                secure = false;
                                return false;
                            }
                            break;

                        case EventTypeSpecific.INFO_CONNECTION_STILL_SECURE:
                        case EventTypeSpecific.INFO_CONNECTION_SECURE:
                            //we got a first time secure connection (new fingerprint)
                            String fingerprint;
                            if (comCrypto.getFingerprint(handlePartner, out fingerprint) != 1)
                            {
                                Logger.log(TLogLevel.logUser, "Error: Could not get fingerprint of partner: " + handlePartner);
                                return false;
                            }
                            Logger.log(TLogLevel.logUser, "Info: Successfully completed authenticated key-exchange with " + handlePartner + ".");
                            userIO.akeSuccessful(handlePartner, fingerprint);
                            secure = true;
                            return true;

                        case EventTypeSpecific.INFO_CONNECTION_INSECURE:
                            //we're insecure, leave!
                            //TODO: panic!
                            secure = false;
                            return false;

                        default:
                            Logger.log(TLogLevel.logDebug, "Error: Got unknown specific event-type.");
                            return false;
                    }
                }

                //second, check for notifications to display
                String notification;
                while (comCrypto.getNextNotification(handlePartner, out notification) != 0)
                {
                    userIO.displayNotification(notification);
                }

                //third, check for message to send
                String message;
                if (comCrypto.getNextMessage(handlePartner, out message) == 0)
                {
                    break;
                }
                _sendMessage(message);
            }

            return (nextAction != ActionType.FATAL_ERROR);
        }

        ///<inheritdoc />
        public CryptoKey getSessionKey(int bits, TCryptoAlgo algorithm)
        {
            if (this.isSecure != true)
            {
                throw new ExceptionCryptoFailed("Session is not secure!");
            }

            //we only support AES
            if (algorithm != TCryptoAlgo.AES)
            {
                throw new ExceptionCryptoFailed("Unsupported algorithm requested.");
            }

            Array key;
            if (comCrypto.getSessionKey(handlePartner, out key) != 1)
            {
                throw new ExceptionCryptoFailed("Failed to get session key!");
            }

            if (key.Length * 8 != bits)
            {
                throw new ExceptionCryptoFailed("Key is of wrong bit-strength!");
            }

            return new CryptoKey((byte[])key, TCryptoAlgo.AES);
        }

        ///<inheritdoc />
        public bool close()
        {
            //TODO: check if further actions are required!
            return ((ActionType)comCrypto.endSession(handlePartner) != ActionType.FATAL_ERROR);
        }
    }

}
