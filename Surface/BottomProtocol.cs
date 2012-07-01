using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;


//The classes of this namespace define the Bottom protocol with the help of a state-machine.
//Assumptions:
//I. Both participants are mutually authenticated. 
//II. The integrety and authenticity of all messages is assured by an unknown, encapsulating protocol.
//State-machine layout is based on examples shown at http://msdn.microsoft.com/en-us/magazine/bb985799.aspx

namespace watchasayin.Bottom.Protocol
{
    /// <summary>
    /// State base-class. All Bottom protocol states are derived from this one.
    /// </summary>
    internal abstract class State
    {
        internal abstract String name { get; }

        /// <summary>
        /// Starts a protocol run.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        internal virtual void startProtocol(Session session) { invalidMessage(session); }

        /// <summary>
        /// Ends a protocol run.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        internal static void endProtocol(Session session)
        {
            session.sendAbort();
            session.tearDown();
        }

        /// <summary>
        /// Called when a REQUEST_CONNECTIONS message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        internal virtual void requestConnectionsReceived(Session session) { invalidMessage(session); }

        /// <summary>
        /// Called when a MY_CONNECTIONS message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        internal virtual void myConnectionsReceived(Session session) { invalidMessage(session); }

        /// <summary>
        /// Called when a MATCHES message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        internal virtual void matchesReceived(Session session) { invalidMessage(session); }

        /// <summary>
        /// Called when a CONFIRM message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        internal virtual void confirmReceived(Session session) { invalidMessage(session); }

        /// <summary>
        /// Called when a ABORT message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        internal virtual void abortReceived(Session session) { invalidMessage(session); }

        /// <summary>
        /// Called when a CLOSING message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        internal virtual void closingReceived(Session session) { session.tearDown(); }

        /// <summary>
        /// Called when an unknown message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        internal virtual void unknownReceived(Session session) { invalidMessage(session); }

        /// <summary>
        /// Default function for a state-transition.
        /// </summary>
        /// <param name="session">Corresponding Surface protocol run.</param>
        /// <param name="state">Next state.</param>
        internal virtual void changeState(Session session, State state)
        {
            session.changeState(state);
        }

        /// <summary>
        /// Default handler for non protocol-conform events.
        /// Ends the respective protocol run.
        /// </summary>
        /// <param name="session">Corresponding Surface protocol run.</param>
        internal virtual void invalidMessage(Session session)
        {
            session.tearDown();
        }
    }

    /// <summary>
    /// The initial state in a Bottom protocol run.
    /// </summary>
    internal class Start : State
    {
        private static State state = new Start();
        internal override String name { get { return "Start"; } }

        private Start()
        {
        }

        internal static State Instance(Session session)
        {
            return state;
        }

        internal override void requestConnectionsReceived(Session session) 
        {
            session.sendMyConnections();
            changeState(session, FindMatches.Instance(session));
        }

        internal override void startProtocol(Session session)
        {
            session.sendRequestConnections();
            changeState(session, WaitingForConnectionsList.Instance(session));
        }
    }

    /// <summary>
    /// The protocol participant who sent the initial REQUEST_CONNECTIONS 
    /// waits in this state for the corresponding MY_CONNECTIONS message. 
    /// </summary>
    class WaitingForConnectionsList : State
    {
        private static State state = new WaitingForConnectionsList();
        internal override String name { get { return "Waiting For Connections List"; } }

        private WaitingForConnectionsList()
        {
        }

        internal static State Instance(Session session)
        {
            return state;
        }

        internal override void myConnectionsReceived(Session session)
        {
            changeState(session, FindMatches.Instance(session));
        }
    }

    /// <summary>
    /// Waits for the list of matches found by the other protocol participant.
    /// </summary>
    class WaitForMatches : State
    {
        private static State state = new WaitForMatches();
        internal override String name { get { return "Wait For Matches"; } }

        private WaitForMatches()
        {
        }

        internal static State Instance(Session session)
        {
            return state;
        }

        internal override void matchesReceived(Session session)
        {
            changeState(session, CompareMatches.Instance(session));
        }
    }


    /// <summary>
    /// A solely transitional state. Looks for matches between the received connections list and own connections list.
    /// </summary>
    class FindMatches : State
    {
        private static State state = new FindMatches();
        internal override String name { get { return "Find Matches"; } }

        private FindMatches()
        {
        }

        internal static State Instance(Session session)
        {
            if (session.evaluateReceivedConnections() == true)
            {
                Logger.log(TLogLevel.logDebug, "Info: Found proper matches between own connections and peer's connection.");
                //proper matches found, go on
                session.sendMatches();
                return WaitForMatches.Instance(session);
            }
            else
            {
                Logger.log(TLogLevel.logDebug, "Info: Did not find proper matches between own connections and peer's connection.");
                //no proper matches found, close protocol
                endProtocol(session);
                return state;
            }
        }
    }

    /// <summary>
    /// Waits for the list of matches found by the other protocol participant.
    /// </summary>
    class WaitForConfirmation : State
    {
        private static State state = new WaitForConfirmation();
        internal override String name { get { return "Wait For Confirmation"; } }

        private WaitForConfirmation()
        {
        }

        internal static State Instance(Session session)
        {
            return state;
        }

        internal override void confirmReceived(Session session)
        {
            //if we end up here, everything went fine -> end protocol and start encryption!
            session.tearDown();
            Logger.log(TLogLevel.logDebug, "Info: Bottom protocol run completed successfully.");
            session.startSecureChannel();
        }
    }

    /// <summary>
    /// A solely transitional state. Compares own matches with the matches received from the other party.
    /// </summary>
    class CompareMatches : State
    {
        private static State state = new CompareMatches();
        internal override String name { get { return "Compare Matches"; } }

        private CompareMatches()
        {
        }

        internal static State Instance(Session session)
        {
            
            if (session.evaluateReceivedMatches() == true)
            {
                //received matches and own matches fit together, confirm
                Logger.log(TLogLevel.logDebug, "Info: Matches from peer were successfully validated."); 
                session.sendConfirm();
                return WaitForConfirmation.Instance(session);
            }
            else
            {
                //received matches and own matches do not fit together, abort
                Logger.log(TLogLevel.logDebug, "Info: Matches from peer could not be validated."); 
                endProtocol(session);
                return state;
            }
        }
    }

    /// <summary>
    /// State-machine defining the Bottom protocol. All classes that want to speak Bottom, should inherit from this one.
    /// </summary>
    public abstract class Session
    {
        private State state;
        private bool _running;

        /// <summary>
        /// Flag indicating whether the protocol run is active.
        /// </summary>
        public bool running { get { return _running; } }

        /// <summary>
        /// The constructor.
        /// </summary>
        protected Session()
        {
            state = Start.Instance(this);
            _running = true;
        }

        /// <summary>
        /// Changes the state of the protocol run.
        /// </summary>
        /// <param name="to">State to transition to.</param>
        internal void changeState(State to)
        {
            Logger.log(TLogLevel.logEverything, "Info: Bottom protocol state: " + to.name);
            state = to;
        }

        /// <summary>
        /// Starts the protocol run.
        /// </summary>
        public void start()
        {
            state.startProtocol(this);
        }

        /// <summary>
        /// Ends the protocol run.
        /// </summary>
        public void end()
        {
            tearDown();
        }

        /// <summary>
        /// Stops the protocol run.
        /// </summary>
        internal void tearDown()
        {
            _running = false;
        }

        /// <summary>
        /// Tells the state-machine that a REQUEST_CONNECTIONS message was received.
        /// </summary>
        public void requestConnectionsReceived()
        {
            if (_running != true)
                return;

            state.requestConnectionsReceived(this);
        }

        /// <summary>
        /// Tells the state-machine that a REQUEST_CONNECTIONS message was received.
        /// </summary>
        public void myConnectionsReceived()
        {
            if (_running != true)
                return;

            state.myConnectionsReceived(this);
        }

        /// <summary>
        /// Tells the state-machine that a MATCHES message was received.
        /// </summary>
        public void matchesReceived()
        {
            if (_running != true)
                return;

            state.matchesReceived(this);
        }

        /// <summary>
        /// Tells the state-machine that a CONFIRM message was received.
        /// </summary>
        public void confirmReceived()
        {
            if (_running != true)
                return;

            state.confirmReceived(this);
        }

        /// <summary>
        /// Tells the state-machine that a ABORT message was received.
        /// </summary>
        public void abortReceived()
        {
            if (_running != true)
                return;

            state.abortReceived(this);
        }

        /// <summary>
        /// Tells the state-machine that an unknown message was received.
        /// </summary>
        public void unknownReceived()
        {
            if (_running != true)
                return;

            state.unknownReceived(this);
        }

        internal abstract void sendRequestConnections();
        internal abstract void sendMyConnections();
        internal abstract void sendMatches();
        internal abstract void sendConfirm();
        internal abstract void sendAbort();

        /// <summary>
        /// Starts a secure channel after the Bottom protocol completed successfully
        /// </summary>
        internal abstract void startSecureChannel();

        /// <summary>
        /// Evaluates the connection matches received from the other participant.
        /// </summary>
        /// <returns>A flag indicating success.</returns>
        internal abstract bool evaluateReceivedMatches();

        /// <summary>
        /// Evaluates the connections list received from the other participant.
        /// </summary>
        /// <returns>A flag indicating success.</returns>
        internal abstract bool evaluateReceivedConnections();
    }
}
