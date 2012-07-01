using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace watchasayin
{
    /// <summary>
    /// The different log-levels.
    /// </summary>
    public enum TLogLevel
    {
        /// <summary>
        /// Nothing is logged.
        /// </summary>
        logNothing = 0,

        /// <summary>
        /// Only information intended for the user is logged.
        /// </summary>
        logUser,

        /// <summary>
        /// General debug information and user information are logged.
        /// </summary>
        logDebug,

        /// <summary>
        /// Everything is logged.
        /// </summary>
        logEverything,
    }

    /// <summary>
    /// Format of a log-writer callback.
    /// </summary>
    /// <param name="log">The string to log.</param>
    public delegate void DLogWriter (string log);

    /// <summary>
    /// The global logger.
    /// </summary>
    public static class Logger
    {
        private static List<DLogWriter> _logWriters = new List<DLogWriter>();

        /// <summary>
        /// List of registered log-writers. Each log-writer gets called on every new string to log.
        /// </summary>
        public static List<DLogWriter> logWriter { get { return _logWriters; } }

#if DEBUG
        /// <summary>
        /// The logging level.
        /// </summary>
        public static TLogLevel logLevel = TLogLevel.logDebug;
#else
        /// <summary>
        /// The logging level.
        /// </summary>
        public static TLogLevel logLevel = TLogLevel.logUser;
#endif

        /// <summary>
        /// Logs multiple lines of the same log-level.
        /// </summary>
        /// <param name="level">The log-level.</param>
        /// <param name="logLines">The lines to log.</param>
        public static void log(TLogLevel level, string[] logLines)
        {
            foreach (string line in logLines)
            {
                log(level, line);
            }
        }

        /// <summary>
        /// Logs one single line.
        /// </summary>
        /// <param name="level">The log-level.</param>
        /// <param name="log">The line to log.</param>
        public static void log(TLogLevel level, string log)
        {
            
            lock (_logWriters)
            {
                if (level <= logLevel)
                {
                    foreach (DLogWriter lw in _logWriters)
                    {
                        foreach (String line in log.Split('\n'))
                        {
                            lw(line);
                        }
                    }
                }
            }
        }
    }
}
