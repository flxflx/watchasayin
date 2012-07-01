using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using watchasayin.Bottom;
using watchasayin.Kryptonite;

namespace watchasayin.Surface
{
    /// <summary>
    /// An instance of this class is created for each incoming or outgoing call.
    /// Once the corresponding call is established, the two involved CallSession objects on both sides will communicate using the Surface protocol.
    /// Btm2Btm and Krypt2Krypt messages are directly deployed.
    /// </summary>
    class CallSession : Session
    {
        private int _id;
        /// <summary>
        /// The public constructor.
        /// </summary>
        /// <param name="handleParticipant">The unique handle of the other participant.</param>
        /// <param name="id">The ID of the call.</param>
        /// <param name="surfaceComm">An interface to a communication service provider.</param>
        /// <param name="cryptoProvider">An interface to a Kryptonite module.</param>
        /// <param name="trafficManipulator">An interface to a Bottom module.</param>
        public CallSession(string handleParticipant, int id, ISurfaceCommunicationSpecific surfaceComm, IKryptoniteExternalCommunicationSpecific cryptoProvider, IBottomExternalCommunicationSpecific trafficManipulator)
            : base(handleParticipant, TSession.call, surfaceComm, cryptoProvider, trafficManipulator)
        {
            this._id = id;
        }
        public int id { get { return _id; } }
   }
}
