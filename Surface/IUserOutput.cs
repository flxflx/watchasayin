using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace watchasayin.Surface
{
    interface IUserOutput
    {
        /// <summary>
        /// Append a message to the log.
        /// </summary>
        /// <param name="msg">The message to append</param>
        void log(string msg);

        /// <summary>
        /// Add a new session to the GUI.
        /// </summary>
        /// <param name="handlePartner">The name of the other participant.</param>
        /// <param name="sessionType">The type of the session.</param>
        void addSession(String handlePartner, TSession sessionType);

        /// <summary>
        /// Remove a session from the GUI
        /// </summary>
        /// <param name="handlePartner">The handle of the other participant.</param>
        /// <param name="sessionType">The type of the session.</param>
        void removeSession(String handlePartner, TSession sessionType);
    }
}
