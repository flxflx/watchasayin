using System;
using System.Threading;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using watchasayin.Bottom;
using watchasayin.Kryptonite;

namespace watchasayin.Surface
{
    /// <summary>
    /// The different types of communication sessions.
    /// </summary>
    public enum TSession
    {
        /// <summary>
        /// A simple call.
        /// </summary>
        /// <remarks>Currently not supported.</remarks>
        call = 0,

        /// <summary>
        /// A conference-call, possibly involving more than two participants.
        /// </summary>
        /// <remarks>Currently not supported.</remarks>
        callConference,

        /// <summary>
        /// A simple text-chat.
        /// </summary>
        /// <remarks>Currently not supported.</remarks>
        chat,

        /// <summary>
        /// A text-chat involving more than two participants.
        /// </summary>
        /// <remarks>Currently not supported.</remarks>
        chatConference,

        /// <summary>
        /// A video-call.
        /// </summary>
        /// <remarks>Currently not supported.</remarks>
        video,

        /// <summary>
        /// A conference-video-call, possibly involving more than two participants.
        /// </summary>
        /// <remarks>Currently not supported.</remarks>
        vidoeConference,

        /// <summary>
        /// A file-transfer.
        /// </summary>
        /// <remarks>Currently not supported.</remarks>
        fileTransfer,

        /// <summary>
        /// All traffic between two users.
        /// </summary>
        /// <remarks>Currently the only supported modus.</remarks>
        all,
    }

    class Core
    {
        private IUserOutput ui;
        private IEventDispatcher eventDispatcher;
        private ISurfaceCommunication surfaceComm;
        private IBottomExternalCommunication bottom;
        private IKryptoniteExternalCommunication kryptonite;

        private Dictionary<String, CallSession> activeCalls;

        /// <summary>
        /// The public constructor.
        /// </summary>
        /// <param name="ui">The UI module to use.</param>
        /// <param name="eventDispatcher">The event dispatcher to use.</param>
        /// <param name="surfaceComm">The Surface protocol communication module to use.</param>
        /// <param name="kryptonite">The Kryptonite module to use for an authenticated key-exchange and secure deploying of Bottom protocol messages.</param>
        /// <param name="bottom">The Bottom module to use for encrypting communication sessions.</param>
        public Core(IUserOutput ui, IEventDispatcher eventDispatcher, ISurfaceCommunication surfaceComm, IKryptoniteExternalCommunication kryptonite, IBottomExternalCommunication bottom)
        {
            this.ui = ui;
            this.eventDispatcher = eventDispatcher;
            this.surfaceComm = surfaceComm;
            this.kryptonite = kryptonite;
            this.bottom = bottom;

            this.activeCalls = new Dictionary<string, CallSession>();

            //set callbacks
            eventDispatcher.callStartedHandlers.Add(onNewCall);
            eventDispatcher.callEndedHandlers.Add(onEndCall);
            surfaceComm.surfaceMessageHandlers.Add(onSurfaceMessage);

            //init modules
            if (bottom.setUp() != true)
            {
                Logger.log(TLogLevel.logUser, "Fatal Error: Failed to initialize in-process module.");
            }

            if (kryptonite.setUp() != true)
            {
                Logger.log(TLogLevel.logUser, "Fatal Error: Failed to initialize crypto module.");
            }
        }

        /// <summary>
        /// Call this just before the termintation of the application. Unloads Kryptonite and Bottom modules.
        /// </summary>
        public void close()
        {
            bottom.tearDown();
            kryptonite.tearDown();
        }

        private void onNewCall(string callerHandle, int id, bool outgoing)
        {
            lock (activeCalls)
            {
                //check if there's already an ongoing call with the given handle
                if (activeCalls.ContainsKey(callerHandle))
                {
                    Logger.log(TLogLevel.logDebug, "Info: Got notification of a new call while there is still an ongoing call with the same partner.");
                    return;
                }

                //notify the GUI
                ui.addSession(callerHandle, TSession.call);

                //create new Bottom, Kryptonite and SurfaceComm instances
                ISurfaceCommunicationSpecific surfaceCommInstance = surfaceComm.getSpecificInstance(callerHandle);
                IBottomExternalCommunicationSpecific bottomInstance = bottom.getSpecificInstance(callerHandle);
                IKryptoniteExternalCommunicationSpecific kryptoniteInstance = kryptonite.getSpecificInstance(callerHandle);

                //create new call object
                CallSession callSession = new CallSession(callerHandle, id, surfaceCommInstance, kryptoniteInstance, bottomInstance);

                //if this side started the call, start the Surface protocol.
                if (outgoing == true)
                {
                    callSession.start();
                    Logger.log(TLogLevel.logDebug, "Info: Started Surface protocol run.");
                }
                activeCalls[callerHandle] = callSession;
            }
        }

        private void onEndCall(string callerHandle, int id)
        {
            lock (activeCalls)
            {
                //tell the gui
                ui.removeSession(callerHandle, TSession.call);

                //remove call object
                activeCalls[callerHandle].end();
                activeCalls.Remove(callerHandle);

                /*
                if (activeCalls.Count == 0)
                {
                    if (bottom.reset() == false)
                    {
                        Logger.log(TLogLevel.logDebug, "Error: Failed to reset Bottom module.");
                    }
                }
                 * */
            }
        }

        
        private void onSurfaceMessage(string senderHandle, SurfaceMessage srfMsg)
        {
            Logger.log(TLogLevel.logEverything,"Got srfMsg: " + srfMsg.payload);
        }
        
        public bool encryptConversation(int Id)
        {
            //TODO: implement, currently all calls get automatically encrypted.
            return false;
        }
    }
}
