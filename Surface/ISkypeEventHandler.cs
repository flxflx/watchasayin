using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace watchasayin.Surface
{
    /// <summary>
    /// Signature of a "call started" event handler.
    /// </summary>
    /// <param name="callerHandle">The participant's handle.</param>
    /// <param name="id">The Id of the call.</param>
    /// <param name="outgoing">A flag indicating whether the call is outgoing or not.</param>
    public delegate void DCallStartedHandler(string callerHandle, int id, bool outgoing);

    /// <summary>
    /// Signature of a "call ended" event handler.
    /// </summary>
    /// <param name="callerHandle">The participant's handle.</param>
    /// <param name="id">The Id of the call.</param>
    public delegate void DCallEndedHandler(string calleeHandle, int id);

    /// <summary>
    /// Signature of a handler for received chat messages.
    /// </summary>
    /// <param name="senderHandle">The handle of the client which sent the message.</param>
    /// <param name="message">The received message.</param>
    public delegate void DChatMessageHandler(string senderHandle, string message);

    /// <summary>
    /// Definition of a Skype event dispatcher.
    /// </summary>
    interface ISkypeEventHandler
    {
        /// <summary>
        /// The registered "call started" handlers. Simply add your own handler if you want to be notified whenever a new call starts.
        /// </summary>
        List<DCallStartedHandler> callStartedHandlers { get; }

        /// <summary>
        /// The registered "call ended" handlers. Simply add your own handler if you want to be notified whenever a call ends.
        /// </summary>
        List<DCallEndedHandler> callEndedHandlers { get; }

        /// <summary>
        /// The registered handlers for received messages. Simply add your own handler if you want to be notified whenever a new chat message is received.
        /// </summary>
        List<DChatMessageHandler> chatMessageHandlers { get; }       
    }
}
