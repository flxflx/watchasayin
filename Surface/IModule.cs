using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace watchasayin.Surface
{
    /// <summary>
    /// Definition of a module extending the Surface funtionality.
    /// </summary>
    public interface IModule
    {
        /// <summary>
        /// Tells the module to install itself/do initial setups.
        /// </summary>
        /// <returns>A flag indicating success.</returns>
        bool setUp();

        /// <summary>
        /// Tells the module to uninstall.
        /// </summary>
        /// <returns>A flag indicating success.</returns>
        bool tearDown();

        /// <summary>
        /// Resets the module.
        /// </summary>
        /// <returns>A flag indicating success.</returns>
        bool reset();
    }

    /// <summary>
    /// Definition of a callback for sending raw messages.
    /// </summary>
    /// <param name="rawMessage"></param>
    public delegate void DSendMessageSpecific(String rawMessage);

    /// <summary>
    /// A user specific version of IModuleExternalCommunication.
    /// </summary>
    public interface IModuleExternalCommunicationSpecific
    {
        /// <summary>
        /// This should be called whenever a Bottom protocol message is received.
        /// </summary>
        /// <param name="rawMessage">The message that was received.</param>
        void messageReceived(String rawMessage);

        /// <summary>
        /// Callback is called whenever the Bottom module wishes to send a Bottom protocol message.
        /// </summary>
        DSendMessageSpecific sendMessage { set; }
    }
}
