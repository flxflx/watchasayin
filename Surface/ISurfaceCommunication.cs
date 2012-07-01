using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace watchasayin.Surface
{
    /// <summary>
    /// Signature of a Surface message handler.
    /// </summary>
    /// <param name="handleSender">The handle of the client who sent the message.</param>
    /// <param name="srfMsg">The received message.</param>
    delegate void DSurfaceMessageHandler(string handleSender, SurfaceMessage srfMsg);

    /// <summary>
    /// Definition of a Surface communication provider.
    /// </summary>
    interface ISurfaceCommunication
    {
        /// <summary>
        /// The registered message handlers. Simply add your own handler if you want to receive Surface messages.
        /// </summary>
        List<DSurfaceMessageHandler> surfaceMessageHandlers { get; }

        /// <summary>
        /// Sends a Surface message.
        /// </summary>
        /// <param name="handleRecepient">The handle of the client to send the message to.</param>
        /// <param name="srfMsg">The message itself.</param>
        void sendMessage(string handleRecepient, SurfaceMessage srfMsg);

        /// <summary>
        /// Returns a handle bound Surface communication provider.
        /// </summary>
        /// <param name="handleParticipant">The handle to bind the Surface communication provider to.</param>
        /// <returns>The requested bound Surface communication provider.</returns>
        ISurfaceCommunicationSpecific getSpecificInstance(string handleParticipant);
    }

    /// <summary>
    /// Signature of a bound Surface message handler.
    /// </summary>
    /// <param name="srfMsg">The received message.</param>
    delegate void DSurfaceSpecificMessageHandler(SurfaceMessage srfMsg);

    /// <summary>
    /// Definition of a bound Surface communication provier.
    /// </summary>
    interface ISurfaceCommunicationSpecific
    {
        /// <summary>
        /// The registered message handlers. Simply add your own handler if you want to receive Surface messages.
        /// </summary>
        DSurfaceSpecificMessageHandler surfaceSpecificMessageHandler { set; }

        /// <summary>
        /// Sends a Surface message.
        /// </summary>
        void sendMessage(SurfaceMessage srfMsg);
    }
}
