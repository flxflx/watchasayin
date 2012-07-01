using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using watchasayin.Bottom;
using watchasayin.Kryptonite;


namespace watchasayin.Surface.SkypeSpecific
{
    /// <summary>
    /// Will be thrown when functionality is called that has not been implemented yet.
    /// <para>
    /// Based on code from http://msdn.microsoft.com/en-us/library/ms173163(v=VS.80).aspx
    /// </para>
    /// </summary>
    public class ExceptionNotImplemented : System.ApplicationException
    {
        public ExceptionNotImplemented() { }
        public ExceptionNotImplemented(string message) { }
        public ExceptionNotImplemented(string message, System.Exception inner) { }

        // Constructor needed for serialization 
        // when exception propagates from a remoting server to the client.
        protected ExceptionNotImplemented(System.Runtime.Serialization.SerializationInfo info,
            System.Runtime.Serialization.StreamingContext context) { }
    }

    /// <summary>
    /// This class represents a Surface-protocol-enabled communication session in Skype (e.g. a call, a video-conference...).
    /// Every class implementing a specific communication session should inherit from this class, e.g. <see cref="CallSession"/>.
    /// <para>Btm2Btm and Krypt2Krypt messages are directly deployed.</para>
    /// </summary>
    abstract class Session : Surface.Protocol.Session
    {
        private string _handleParticipant;
       
        protected bool _encrypted;
        protected IKryptoniteExternalCommunicationSpecific kryptonite;
        protected IBottomExternalCommunicationSpecific bottom;
        protected ISurfaceCommunicationSpecific surfaceComm;

        private bool _deployBtm2Btm;
        private bool _deployKrypt2Krypt;

        private TSession type;


        public Session(string handleParticipant, TSession type, ISurfaceCommunicationSpecific surfaceComm, IKryptoniteExternalCommunicationSpecific kryptonite, IBottomExternalCommunicationSpecific bottom)
        {
            this._handleParticipant = handleParticipant;
            this.kryptonite = kryptonite;
            this.bottom = bottom;
            this.surfaceComm = surfaceComm;
            this.type = type;

            //set-up surface message received call-back
            surfaceComm.surfaceSpecificMessageHandler = this.surfaceMessageReceived;

            //set-up send message call-backs
            bottom.sendMessage = this.sendBottomMessage;
            kryptonite.sendMessage = this.sendKryptoniteMessage;

            this._deployBtm2Btm = false;
            this._deployKrypt2Krypt = false;
            this._encrypted = false;
        }

        public string handleParticipant { get { return _handleParticipant; } }
        public bool encrypted { get { return _encrypted; } }

        /// <summary>
        /// Sends a Bottom message encapsulated into a Surface message.
        /// </summary>
        /// <param name="rawMessage">The message to be send.</param>
        private void sendBottomMessage(String rawMessage)
        {
            String encryptedMessage;
            try
            {
                encryptedMessage = kryptonite.encryptMessage(rawMessage);
            }
            catch
            {
                Logger.log(TLogLevel.logUser, "Error: Encryption of Bottom protocol message failed.");
                return;
            }
            
            SurfaceMessage srfMsg = new SurfaceMessage(TSurfaceMessage.Bottom2Bottom, encryptedMessage);
            surfaceComm.sendMessage(srfMsg);
        }

        /// <summary>
        /// Sends a Kryptonite message encapsulated into a Surface message.
        /// </summary>
        /// <param name="rawMessage">The message to be send.</param>
        private void sendKryptoniteMessage(String rawMessage)
        {
            SurfaceMessage srfMsg = new SurfaceMessage(TSurfaceMessage.Kryptonite2Kryptonite, rawMessage);
            surfaceComm.sendMessage(srfMsg);
        }

        /// <summary>
        /// This should be called whenever a Surface message is received from the corresponding participant.
        /// </summary>
        /// <param name="surfaceMessage">The message that was received.</param>
        private void surfaceMessageReceived(SurfaceMessage surfaceMessage)
        {
            Logger.log(TLogLevel.logEverything, "Info: Received Surface message: " + surfaceMessage.type);
            // We don't want any race-condition here, aight?
            lock(this)
            {
                //reset deploy flags
                this._deployBtm2Btm = false;
                this._deployKrypt2Krypt = false;

                bool authentic = false;

                switch (surfaceMessage.type)
                {
                    case TSurfaceMessage.Syn:
                        //todo: authentication
                        synReceived(authentic);
                        break;

                    case TSurfaceMessage.Ack:
                        ackReceived(authentic);
                        break;

                    case TSurfaceMessage.Nack:
                        //todo: authentication
                        nackReceived(authentic);
                        break;

                    case TSurfaceMessage.Bottom2Bottom:
                        
                        String btmMsg;
                        //decrypt
                        try
                        {
                            btmMsg = kryptonite.decryptMessage(surfaceMessage.payload);
                            authentic = true;
                        }
                        catch (ExceptionCryptoFailed)
                        {
                            Logger.log(TLogLevel.logDebug, "Error: Decrypting of Bottom message failed.");
                            break;
                        }
                        
                        btm2btmReceived(authentic);

                        //check if deploy has been set by the underlying state-machine
                        if (this._deployBtm2Btm == true)
                        {
                            //deploy
                            bottom.messageReceived(btmMsg);
                        }
                        break;

                    case TSurfaceMessage.Kryptonite2Kryptonite:
                        krypt2kryptReceived(true);

                        //check if deploy has been set by the underlying state-machine
                        if (this._deployKrypt2Krypt == true)
                        {
                            //deploy

                            bool oldSecure = kryptonite.isSecure;
                            try
                            {
                                kryptonite.messageReceived(surfaceMessage.payload);
                            }
                            catch (ExceptionCryptoFailed)
                            {
                                Logger.log(TLogLevel.logDebug, "Error: Deploying of Kryptonite message failed.");
                            }

                            //check if the security status of this session changed
                            if (oldSecure != kryptonite.isSecure)
                            {
                                securityStatusChanged();
                            }
                        }
                        break;

                    case TSurfaceMessage.Closing:
                        closingReceived(authentic);
                        break;

                    case TSurfaceMessage.Unknown:
                        unknownReceived(authentic);
                        break;

                    default:
                        //drop
                        break;
                }
            }
        }

        internal override void sendSYN(bool sign = false)
        {
            if (sign == false)
            {
                surfaceComm.sendMessage(new SurfaceMessage(TSurfaceMessage.Syn));
            }
            else
            {
                throw new ExceptionNotImplemented("Trying to sign a message in sendSYN()");
            }
        }

        internal override void sendACK(bool sign = false)
        {
            if (sign == false)
            {
                surfaceComm.sendMessage(new SurfaceMessage(TSurfaceMessage.Ack));
            }
            else
            {
                throw new ExceptionNotImplemented("Trying to sign a message in sendACK()");
            }
        }
        internal override void sendNACK(bool sign = false)
        {
            if (sign == false)
            {
                surfaceComm.sendMessage(new SurfaceMessage(TSurfaceMessage.Nack));
            }
            else
            {
                throw new ExceptionNotImplemented("Trying to sign a message in sendNACK()");
            }
        }

        internal override void sendCLOSING(bool sign = false)
        {
            if (sign == false)
            {
                surfaceComm.sendMessage(new SurfaceMessage(TSurfaceMessage.Closing));
            }
            else
            {
                throw new ExceptionNotImplemented("Trying to sign a message in sendCLOSING()");
            }
        }

        internal override void deployBtm2Btm()
        {
            _deployBtm2Btm = true;
        }

        internal override void deployKrypt2Krypt()
        {
            _deployKrypt2Krypt = true;
        }

        internal override bool isSecure 
        {
            get
            {
                return kryptonite.isSecure;
            }
        }

        internal override void initBottomProtocol(bool primaryActor)
        {
            CryptoKey key;
            try
            {
                key = kryptonite.getSessionKey(bottom.getSupportedKeyBitStrength(), bottom.getSupportedKeyAlgorithm());
            }
            catch
            {
                Logger.log(TLogLevel.logUser, "Fatal Error: Failed to get secure key.");
                return;
            }

            if (bottom.encrypt(0, TSession.call, key, primaryActor) != true)
            {
                Logger.log(TLogLevel.logUser, "Error: Something went wrong while encrypting the call.");
            }
        }

        internal override void initKryptoniteProtocol(bool primaryActor)
        {
            if (kryptonite.establishSecureConnection(primaryActor) != true)
            {
                Logger.log(TLogLevel.logUser, "Error: Something went wrong while exchanging keys.");
                end();
            }
        }

        internal override void tearDown()
        {
            if (bottom.close() != true)
            {
                Logger.log(TLogLevel.logDebug, "Error: Failed to close Bottom connection.");
            }

            if (kryptonite.close() != true)
            {
                Logger.log(TLogLevel.logDebug, "Error: Failed to close Kryptonite connection.");
            }
        }
    }
}
