using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;


//The classes of this namespace define the Surface protocol with the help of a state-machine.
//State-machine layout is based on examples shown at http://msdn.microsoft.com/en-us/magazine/bb985799.aspx

namespace watchasayin.Surface.Protocol
{
    /// <summary>
    /// State base-class. All Surface protocol states are derived from this one.
    /// </summary>
    internal abstract class State
    {
        /// <summary>
        /// Starts a protocol run.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        internal virtual void startProtocol(Session session) { invalidMessage(session); }

        /// <summary>
        /// Ends a protocol run.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        internal void endProtocol(Session session) 
        {
            session.sendCLOSING();
            session._tearDown();
        }

        /// <summary>
        /// Called when a SYN message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        /// <param name="authentic">Flag indicating whether the received message was properly signed.</param>
        internal virtual void synReceived(Session session, bool authentic = false) { invalidMessage(session); }

        /// <summary>
        /// Called when a ACK message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        /// <param name="authentic">Flag indicating whether the received message was properly signed.</param>
        internal virtual void ackReceived(Session session, bool authentic = false) { invalidMessage(session); }

        /// <summary>
        /// Called when a NACK message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        /// <param name="authentic">Flag indicating whether the received message was properly signed.</param>
        internal virtual void nackReceived(Session session, bool authentic = false) { invalidMessage(session); }

        /// <summary>
        /// Called when a Btm2Btm message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        /// <param name="authentic">Flag indicating whether the received message was properly signed.</param>
        internal virtual void btm2btmReceived(Session session, bool authentic = false) { invalidMessage(session); }

        /// <summary>
        /// Called when a Krypt2Krypt message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        /// <param name="authentic">Flag indicating whether the received message was properly signed.</param>
        internal virtual void krypt2kryptReceived(Session session, bool authentic = false) { invalidMessage(session); }

        /// <summary>
        /// Called when a CLOSING message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        /// <param name="authentic">Flag indicating whether the received message was properly signed.</param>
        internal virtual void closingReceived(Session session, bool authentic = false) { session._tearDown(); }

        /// <summary>
        /// Called when an unknown message is received.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        /// <param name="authentic">Flag indicating whether the received message was properly signed.</param>
        internal virtual void unknownReceived(Session session, bool authentic = false) { invalidMessage(session); }

        /// <summary>
        /// Called whenever the security status of a session has changed.
        /// </summary>
        /// <param name="session">The corresponding session.</param>
        internal virtual void securityStatusChanged(Session session) { session._tearDown(); }
                
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
            session._tearDown();
        }
    }

    /// <summary>
    /// The initial state in a Surface protocol run.
    /// </summary>
    internal class Start : State
    {
        private static State state = new Start();

        private Start()
        {
        }

        internal static State Instance(Session session)
        {
            return state;
        }

        internal override void synReceived(Session session, bool authentic = false)
        {
            //TODO: version checks here
            session.sendACK();
            changeState(session, SessionConfirmed.Instance(session));
        }

        internal override void startProtocol(Session session)
        {
            session.sendSYN();
            changeState(session, WaitingForACK.Instance(session));
        }

    }

    /// <summary>
    /// The protocol participant who sent the initial SYN, 
    /// waits in this state for the corresponding ACK or NACK message. 
    /// </summary>
    class WaitingForACK : State
    {
        private static State state = new WaitingForACK();
        private WaitingForACK()
        {
        }

        internal static State Instance(Session session)
        {
            return state;
        }

        internal override void ackReceived(Session session, bool authentic = false)
        {
            changeState(session, SessionConfirmed.Instance(session));
        }

        //since NACK is al valid message for this state, it is implemented explicitly
        internal override void nackReceived(Session session, bool authentic = false)
        {
            session._tearDown();
        }
    }

    /// <summary>
    /// A solely transitional state. Decides whether a key-exchange is required before transitioning to the "Routing" state.
    /// </summary>
    class SessionConfirmed : State
    {
        private static State state = new SessionConfirmed();
        private SessionConfirmed()
        {
        }

        internal static State Instance(Session session)
        {
            if (session.isSecure == true)
            {
                return Routing.Instance(session);
            }
            else
            {
                //need to do a key-exchange before we can start routing 
                return ExchangingKeys.Instance(session);
            }
        }
    }

    /// <summary>
    /// Main state of the Surface protocol. Packets for Bottom and Kryptonite communication are routed until a CLOSING message is received or an internal abort occurs (e.g. end of call).
    /// Only fully signed messages are evaluated.
    /// </summary>
    class Routing : State
    {
        private static State state = new Routing();
        private Routing()
        {
        }

        internal static State Instance(Session session)
        {
            session.initBottomProtocol(session.startedProtocol);
            return state;
        }

        internal override void synReceived(Session session, bool authentic = false)
        {
            invalidMessage(session);
        }

        internal override void ackReceived(Session session, bool authentic = false)
        {
            invalidMessage(session);
        }

        internal override void nackReceived(Session session, bool authentic = false)
        {
            invalidMessage(session);
        }

        internal override void closingReceived(Session session, bool authentic = false)
        {
            session._tearDown();
        }

        internal override void btm2btmReceived(Session session, bool authentic = false)
        {
            if (authentic != true)
                return;

            session.deployBtm2Btm();

        }
        internal override void krypt2kryptReceived(Session session, bool authentic = false)
        {
            session.deployKrypt2Krypt();
        }

        internal override void unknownReceived(Session session, bool authentic = false)
        {
            invalidMessage(session);
        }
    }

    /// <summary>
    /// Before Bottom messages can be exchanged, we need to establish a secure channel.
    /// Kryptonite communication is deployed regardless of signatures.
    /// </summary>
    class ExchangingKeys : State
    {
        private static State state = new ExchangingKeys();
        private ExchangingKeys()
        {
        }

        internal static State Instance(Session session)
        {            
            session.initKryptoniteProtocol(session.startedProtocol);
            return state;
        }

        internal override void krypt2kryptReceived(Session session, bool authentic = false)
        {
            //no auth-check in this state of the protocol
            session.deployKrypt2Krypt();
        }

        internal override void securityStatusChanged(Session session)
        {
            //check if session has gone secure
            if (session.isSecure != true)
            {
                //this should never happen
                session._tearDown();
            }
            session.changeState(Routing.Instance(session));
        }
    }

    /// <summary>
    /// State-machine defining the Surface protocol. All classes that want to speak Surface, should inherit from this one.
    /// </summary>
    public abstract class Session
    {
        private State state;
        private bool _running;
        private bool _startedProtocol;

        /// <summary>
        /// Flag indicating whether the protocol run is active.
        /// </summary>
        public bool running { get { return _running; } }

        /// <summary>
        /// Flag indicating whether the this side started the protocol run.
        /// </summary>
        public bool startedProtocol { get { return _startedProtocol; } }

        /// <summary>
        /// The constructor.
        /// </summary>
        protected Session()
        {
            state = Start.Instance(this);
            _running = true;
            _startedProtocol = false;
        }

        /// <summary>
        /// Changes the state of the protocol run.
        /// </summary>
        /// <param name="to">State to transition to.</param>
        internal void changeState(State to)
        {
            state = to;
        }

        /// <summary>
        /// Starts the protocol run.
        /// </summary>
        public void start()
        {
            _startedProtocol = true;
            state.startProtocol(this);
        }

        /// <summary>
        /// Ends the protocol run.
        /// </summary>
        public void end()
        {
            _tearDown();
        }

        /// <summary>
        /// Stops the protocol run.
        /// </summary>
        internal void _tearDown()
        {
            _running = false;
            tearDown();
        }

        /// <summary>
        /// Tells the state-machine that a SYN message was received.
        /// </summary>
        /// <param name="authentic">Flag indicating whether the message was properly signed.</param>
        public void synReceived(bool authentic)
        {
            if (_running != true)
                return;

            state.synReceived(this, authentic);
        }

        /// <summary>
        /// Tells the state-machine that a ACK message was received.
        /// </summary>
        /// <param name="authentic">Flag indicating whether the message was properly signed.</param>
        public void ackReceived(bool authentic)
        {
            if (_running != true)
                return;

            state.ackReceived(this, authentic);    
        }

        /// <summary>
        /// Tells the state-machine that a NACK message was received.
        /// </summary>
        /// <param name="authentic">Flag indicating whether the message was properly signed.</param>
        public void nackReceived(bool authentic)
        {
            if (_running == true)
                return;

            state.nackReceived(this, authentic);
        }

        /// <summary>
        /// Tells the state-machine that a Btm2Btm message was received.
        /// </summary>
        /// <param name="authentic">Flag indicating whether the message was properly signed.</param>
        public void btm2btmReceived(bool authentic)
        {
            if (_running != true)
                return;

            state.btm2btmReceived(this, authentic);
            
        }

        /// <summary>
        /// Tells the state-machine that a Krypt2Krypt message was received.
        /// </summary>
        /// <param name="authentic">Flag indicating whether the message was properly signed.</param>
        public void krypt2kryptReceived(bool authentic)
        {
            if (_running != true)
                return;

            state.krypt2kryptReceived(this, authentic);
        }

        /// <summary>
        /// Tells the state-machine that a CLOSING message was received.
        /// </summary>
        /// <param name="authentic">Flag indicating whether the message was properly signed.</param>
        public void closingReceived(bool authentic)
        {
            if (_running != true)
                return;

            state.closingReceived(this, authentic);
        }

        /// <summary>
        /// Tells the state-machine that an unknown message was received.
        /// </summary>
        /// <param name="authentic">Flag indicating whether the message was properly signed.</param>
        public void unknownReceived(bool authentic)
        {
            if (_running != true)
                return;

            state.unknownReceived(this, authentic);
        }

        /// <summary>
        /// Tells the state-machine that the security status of the session changed.
        /// </summary>
        public void securityStatusChanged()
        {
            state.securityStatusChanged(this);
        }

        internal abstract void sendSYN(bool sign = false);
        internal abstract void sendACK(bool sign = false);
        internal abstract void sendNACK(bool sign = false);
        internal abstract void sendCLOSING(bool sign = false);

        /// <summary>
        /// Deploy the last Btm2Btm message.
        /// </summary>
        internal abstract void deployBtm2Btm();

        /// <summary>
        /// Deploy the last Krypt2Krypt message.
        /// </summary>
        internal abstract void deployKrypt2Krypt();

        /// <summary>
        /// Start the Kryptonite protocol for key-agreement as primary actor.
        /// <param name="primaryActor">Flag indicating whether to start the protocol as primary or secondary actor.</param>
        /// </summary>
        internal abstract void initKryptoniteProtocol(bool primaryActor);

        /// <summary>
        /// Flag indicating whether a public-key for the other protocol participant is available.
        /// </summary>
        internal abstract bool isSecure { get; }

        /// <summary>
        /// Start the underlying Bottom protocol to encrypt Skype traffic as primary actor (send first message).
        /// <param name="primaryActor">Flag indicating whether to start the protocol as primary or secondary actor.</param>
        /// </summary>
        internal abstract void initBottomProtocol(bool primaryActor);

        /// <summary>
        /// Clean-up function, that gets called once a protocol run has ended
        /// </summary>
        internal abstract void tearDown();
    }
}
