using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.ComponentModel;
using System.IO;
using System.Threading;
using watchasayin.Surface;


namespace watchasayin.Bottom
{
    //typedefs
    using HWND = UInt32;
    using DWORD = UInt32;
    using LPCTSTR = String;
    using LPTSTR = String;
    using HANDLE = UInt32;
    using BOOL = UInt32;
    using SIZE_T = UInt32;
    using LPVOID = UInt32;
    using HMODULE = UInt32;
    using FARPROC = UInt32;

    /// <summary>
    /// Definition of a user / Session notification module.
    /// </summary>
    public interface IUserIO
    {
        /// <summary>
        /// Tell the user that the given session is now secure.
        /// </summary>
        /// <param name="handlePartner">The partner of the session.</param>
        /// <param name="type">The type of the session.</param>
        void goneSecure(string handlePartner, TSession type);

        /// <summary>
        /// Tell the user that the given session is no longer secure.
        /// </summary>
        /// <param name="handlePartner">The partner of the session.</param>
        /// <param name="type">The type of the session.</param>
        void goneInsecure(string handlePartner, TSession type);
    }

    /// <summary>
    /// The different IP protocols.
    /// </summary>
    /// <remarks>Identifiers taken from struct IPPROTO in ws2def.h.</remarks>
    public enum TIpProtocol
    {
        /// <summary>
        /// The ICMP protocol.
        /// </summary>
        ICMP = 1,
        // ...
        /// <summary>
        /// The TCP protocol.
        /// </summary>
        TCP = 6,
        /// <summary>
        /// The UDP protocol.
        /// </summary>
        UDP = 17,
        // ...
        /// <summary>
        /// An unknown protocol.
        /// </summary>
        unknown = -1,
    };

    /// <summary>
    /// The different security policies of a Bottom module.
    /// </summary>
    public enum TSecurityPolicy
    {
        /// <summary>
        /// No connections are blocked at any time.
        /// </summary>
        VERY_LOOSE = 1,

        /// <summary>
        /// All new outgoing connections are blocked as long as a secure session is active. 
        /// </summary>
        LOOSE = 2,

        /// <summary>
        /// All unencrypted outgoing connections are blocked as long as a secure session is active.
        /// </summary>
        TIGHT = 3,

        /// <summary>
        /// An unknown security policy.
        /// </summary>
        unknown = -1
    };

    /// <summary>
    /// Class implementing the required Bottom functionality by injecting a Dll into the target process.
    /// <remarks>Requires an external provider for sending and receiving Bottom protocol messages.</remarks>
    /// </summary>
    public class InProcess : IBottomExternalCommunication
    {
        
        private const String FILENAME_BOTTOM_DLL = "Bottom.dll";
       
        private HANDLE hTarget;

        //Win32 api-function required for finding the target process and injecting a Dll.
        [DllImport("User32.dll", SetLastError = true)]
        static extern DWORD GetWindowThreadProcessId(HWND hWnd, ref DWORD lpdwProcessId);
        [DllImport("User32.dll", SetLastError = true)]
        static extern HWND FindWindowA(LPCTSTR lpClassName, DWORD lpWindowName);

        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern HANDLE OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern BOOL WriteProcessMemory(HANDLE hProcess, LPVOID lpBaseAddress, string lpBuffer, SIZE_T nSize, ref SIZE_T lpNumberOfBytesWritten);
        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern LPVOID VirtualAllocEx(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern HMODULE GetModuleHandleA(LPCTSTR lpModuleName);
        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern FARPROC GetProcAddress(HMODULE hModule, LPCTSTR lpProcName);
        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern HANDLE CreateRemoteThread(HANDLE hProcess, LPVOID lpThreadAttributes, SIZE_T dwStackSize, LPVOID lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, DWORD lpThreadId);
        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern BOOL CloseHandle(HANDLE hObject);
        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern int ResumeThread(HANDLE hThread);
        [DllImport("Kernel32.dll", SetLastError = true)]
        static extern BOOL CreateProcessA(LPCTSTR lpApplicationName, LPTSTR lpCommandLine, LPVOID lpProcessAttributes, LPVOID lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
            LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, ref STARTUPINFO lpStartupInfo, out PROCESS_INFORMATION lpProcessInformation);

        const int CREATE_SUSPENDED = 0x00000004;

        //NOTE: Following two structures were taken from http://blogs.msdn.com/b/thottams/archive/2006/08/11/696013.aspx and are needed for calling CreateProcessA()
        private struct PROCESS_INFORMATION
        {
            public IntPtr hProcess;
            public IntPtr hThread;
            public uint dwProcessId;
            public uint dwThreadId;
        }
 
        private struct STARTUPINFO
        {
            public uint cb;
            public string lpReserved;
            public string lpDesktop;
            public string lpTitle;
            public uint dwX;
            public uint dwY;
            public uint dwXSize;
            public uint dwYSize;
            public uint dwXCountChars;
            public uint dwYCountChars;
            public uint dwFillAttribute;
            public uint dwFlags;
            public short wShowWindow;
            public short cbReserved2;
            public IntPtr lpReserved2;
            public IntPtr hStdInput;
            public IntPtr hStdOutput;
            public IntPtr hStdError;
        }

        //winapi consts
        const DWORD PROCESS_VM_WRITE = 0x0020;
        const DWORD PROCESS_VM_OPERATION = 0x0008;
        const DWORD PROCESS_CREATE_THREAD = 0x0002;
        const DWORD PROCESS_QUERY_INFORMATION = 0x0400; 
        const DWORD PROCESS_VM_READ = 0x0010;

        const DWORD MEM_COMMIT = 0x1000;
        const DWORD PAGE_READWRITE = 0x04;

        private BottomCOM.Generic comGeneric;
        private BottomCOM.HookingEngine comHookEngine;
        private bool initSuccessful;

        private IUserIO userIO;
        private String workingDir;
        private String targetClassname;
        private String targetPath;
        private String targetName;


        /// <summary>
        /// Public constructor.
        /// </summary>
        /// <param name="userIO">An interface for communication with the user.</param>
        /// <param name="workingDir">The path of the dir to save/load files.</param>
        /// <param name="targetClassname">A characteristic classname registered by the target process. The classname is used to identify the target's process.</param>
        /// <param name="targetName">The name of the target - only used for logging.</param>
        /// <param name="targetPath">The path to the target executable.</param>
        /// <param name="targetStartIfNeeded">A flag indicating whether the target should be launched if it is not running already.</param>
        public InProcess(IUserIO userIO, String workingDir, String targetName, String targetClassname, String targetPath = "", bool targetStartIfNeeded = false)
        {
            this.userIO = userIO;
            this.workingDir = workingDir;
            this.targetClassname = targetClassname;
            this.targetPath = targetPath;
            this.targetName = targetName;

            hTarget = new HANDLE();
            initSuccessful = false;

            hookTarget(targetStartIfNeeded);
        }

        private bool hookTarget(bool startIfNeeded)
        {
            //1. Write Bottom.dll to hdd
            System.IO.FileStream bottomDll;
            try
            {
                bottomDll = new System.IO.FileStream(FILENAME_BOTTOM_DLL, FileMode.Create);
            }
            catch //Pokemon style - gotta catch 'em all :>
            {
                Logger.log(TLogLevel.logUser, "Error: Failed to create Bottom.dll");
                return false;
            }

            try
            {
                bottomDll.Write(BottomResources.BottomDll, 0, BottomResources.BottomDll.Length);
            }
            catch (Exception e)
            {
                Logger.log(TLogLevel.logUser, "Error: Failed to write to Bottom.dll, exception: " + e.Message);
                bottomDll.Close();
                File.Delete(FILENAME_BOTTOM_DLL);
                return false;
            }
            bottomDll.Close();

            //2. Get target's process handle
            bool needToResume = false;
            HANDLE hThreadToResume = 0;

            //get window handle
            HWND hwndTarget = FindWindowA(targetClassname, 0);
            if (hwndTarget == 0)
            {
                //looks like the process isn't running (or the classname is wrong)
                if (startIfNeeded != true)
                {
                    Logger.log(TLogLevel.logUser, "Fatal Error: " + targetName + " appears not to be running.");
                    File.Delete(FILENAME_BOTTOM_DLL);
                    return false;
                }
                Logger.log(TLogLevel.logUser, "Info: " + targetName + " appears not to be running. Trying to start it...");

                //start process suspended
                int lastBs = targetPath.LastIndexOf("\\");
                if (lastBs == -1)
                {
                    Logger.log(TLogLevel.logUser, "Fatal Error: Could not start " + targetName + ".");
                    File.Delete(FILENAME_BOTTOM_DLL);
                    return false;
                }
                String targetDir = targetPath.Substring(0, lastBs);
                STARTUPINFO si = new STARTUPINFO();
                PROCESS_INFORMATION pi = new PROCESS_INFORMATION();
                if (CreateProcessA(targetPath, null, 0, 0, 0, CREATE_SUSPENDED, 0, targetDir, ref si, out pi) != 1)
                {
                    Logger.log(TLogLevel.logUser, "Fatal Error: Could not start " + targetName + ".");
                    File.Delete(FILENAME_BOTTOM_DLL);
                    return false;
                }

                hTarget = (uint)pi.hProcess;
                hThreadToResume = (uint)pi.hThread;
                needToResume = true;
            }
            else
            {
                //looks like the process is running
                Logger.log(TLogLevel.logUser, "Info: " + targetName + " is already running.");

                //get the target's process id
                DWORD pidTarget = new DWORD();
                GetWindowThreadProcessId(hwndTarget, ref pidTarget);

                Logger.log(TLogLevel.logDebug, "Info: Identified process with id " + pidTarget + " as " + targetName + " process.");

                //finally get the corresponding process handle
                hTarget = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD, 0, pidTarget);
                if (hTarget == 0)
                {
                    Logger.log(TLogLevel.logDebug, "Error: Could not get Skype's process handle. Insufficient rights?");
                    File.Delete(FILENAME_BOTTOM_DLL);
                    return false;
                }
            }

            //3. Write path of Bottom.dll to Skype process
            string pathBottomDll = AppDomain.CurrentDomain.BaseDirectory + FILENAME_BOTTOM_DLL + "\0";
            Logger.log(TLogLevel.logEverything, "Info: Path of Bottom module is: " + pathBottomDll);
            LPVOID addrPath = VirtualAllocEx(hTarget, 0, (SIZE_T)(pathBottomDll.Length), MEM_COMMIT, PAGE_READWRITE);
            if (addrPath == 0)
            {
                Logger.log(TLogLevel.logDebug, "Error: Could not allocate memory in target process.");
                File.Delete(FILENAME_BOTTOM_DLL);
                return false;
            }
            DWORD written = new DWORD();
            if (WriteProcessMemory(hTarget, addrPath, pathBottomDll, (SIZE_T)pathBottomDll.Length, ref written) == 0)
            {
                Logger.log(TLogLevel.logDebug, "Error: Could not write path of Bottom.dll to target process.");
                File.Delete(FILENAME_BOTTOM_DLL);
                return false;
            }
            //4. Get address of LoadLibraryA
            HANDLE hKernel32 = GetModuleHandleA("Kernel32.dll");
            if (hKernel32 == 0)
            {
                Logger.log(TLogLevel.logDebug, "Error: Could not get handle to kernel32.dll.");
                File.Delete(FILENAME_BOTTOM_DLL);
                return false;
            }
            FARPROC addrLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryA");
            if (addrLoadLibrary == 0)
            {
                Logger.log(TLogLevel.logDebug, "Error: Could not get address kernel32.LoadLibraryA.");
                File.Delete(FILENAME_BOTTOM_DLL);
                return false;
            }

            //5. Call CreateRemoteThread(PID, LoadLibraryA, pPathBottomDll)
            if (CreateRemoteThread(hTarget, 0, 0, addrLoadLibrary, addrPath, 0, 0) == 0)
            {
                Logger.log(TLogLevel.logDebug, "Error: Could not create a new thread in the target process. Last error: " + Marshal.GetLastWin32Error().ToString());
                File.Delete(FILENAME_BOTTOM_DLL);
                return false;
            }

            //6. Clean-up
            ////too lazy for VirtualFreeEx()
            CloseHandle(hTarget);

            //7. Try to connect to COM object
            int i;
            for (i = 0; i < 100; i++)
            {
                try
                {
                    comGeneric = new BottomCOM.Generic();
                    Logger.log(TLogLevel.logUser, "Successfully attached to " + targetName + " process with id: " + comGeneric.getPid());
                    break;
                }
                catch
                {
                    System.Threading.Thread.Sleep(20 + i * 10);
                }
            }

            if (i == 100)
            {
                Logger.log(TLogLevel.logUser, "Fatal Error: Failed to attach to " + targetName +  " process.");
                return false;
            }

            //connect to hook engine
            comHookEngine = new BottomCOM.HookingEngine();
            if (comHookEngine.init(workingDir) != 1)
            {
                Logger.log(TLogLevel.logUser, "Fatal Error: Could not init in-process module.");
                return false;
            }

            //8. Set-up hooks
            if (comHookEngine.hookNetworkIO() != 1)
            {
                Logger.log(TLogLevel.logUser, "Fatal Error: Could not init in-process module.");
                return false;
            }

            //9. Resume process in case it was created suspended
            if (needToResume == true)
            {
                if (ResumeThread(hThreadToResume) == -1)
                {
                    Logger.log(TLogLevel.logUser, "Fatal Error: Could not resume " + targetName + " process.");
                    return false;
                }

                //wait for characteristic class to become registered
                for (i = 0; i < 100; i++)
                {
                    if (FindWindowA(targetClassname, 0) != 0)
                    {
                        break;
                    }
                    System.Threading.Thread.Sleep(20 + i * 10);
                }
                if (i == 100)
                {
                    Logger.log(TLogLevel.logUser, "Fatal Error: " + targetName + " did not load as expected.");
                    return false;
                }
            }

            initSuccessful = true;
            return true;
        }

        /// <summary>
        /// Sets up the traffic encryptor/decryptor inside the target process.
        /// </summary>
        /// <returns>A flag indicating success.</returns>
        public bool setUp()
        {
            return initSuccessful;
        }

        /// <summary>
        /// Unloads the traffic encryptor/decryptor.
        /// </summary>
        /// <returns>A flag indicating success.</returns>
        public bool tearDown()
        {
            if (initSuccessful == false)
            {
                return true;
            }


            try
            {
                if (comHookEngine.unhookNetworkIO() != 1)
                {
                    return false;
                }

                comGeneric.unload(FILENAME_BOTTOM_DLL);
            }
            catch
            {
            }
            
            return true;
        }

        /// <summary>
        /// Resets the Bottom module.
        /// </summary>
        /// <remarks>Effectively only clears the incoming and outgoing connections lists.</remarks>
        /// <returns>A flag indicating success.</returns>
        public bool reset()
        {
            return (comHookEngine.resetConnections() == 1);
        }

        /// <summary>
        /// Creates a new Bottom session object.
        /// </summary>
        /// <returns>A Bottom session object.</returns>
        public IBottomExternalCommunicationSpecific getSpecificInstance(String handlePartner)
        {
            return new SessionExternalComm(comGeneric, comHookEngine, handlePartner, userIO);
        }
    }

    /// <summary>
    /// Class representing a participant specific Bottom session. 
    /// </summary>
    internal class SessionExternalComm : Protocol.Session, IBottomExternalCommunicationSpecific
    {
        private BottomCOM.Generic comGeneric;
        private BottomCOM.HookingEngine comHookEngine;
        private String handlePartner;
        private IUserIO userIO;

        private Surface.DSendMessageSpecific _sendMessage;

        private const int BIT_STRENGTH_KEY = 128;
        private const TCryptoAlgo SYMMETRIC_ALGORITHM = TCryptoAlgo.AES;
        private CryptoKey sessionKey;
        private bool gotSessionKey;

        private uint sessionId;

        /// <inheritdoc />
        public Surface.DSendMessageSpecific sendMessage { set { _sendMessage = value; } }

        /// <summary>
        /// The public constructor.
        /// </summary>
        /// <param name="comGeneric">An already connected COM object of type BottomCOM.Generic.</param>
        /// <param name="comHookEngine">An already connected COM object of type BottomCOM.HookingEngine.</param>
        /// <param name="handlePartner">The handle of the communication parter.</param>
        /// <param name="userIO">An interface for communication with the user.</param>
        public SessionExternalComm(BottomCOM.Generic comGeneric, BottomCOM.HookingEngine comHookEngine, String handlePartner, IUserIO userIO)
        {
            this.comGeneric = comGeneric;
            this.comHookEngine = comHookEngine;
            this.handlePartner = handlePartner;
            this.userIO = userIO;

            this.gotSessionKey = false;
        }

        /// <inheritdoc />
        public bool encrypt(int sessionId, TSession toEncrypt, CryptoKey key, bool primaryActor)
        {
            if (toEncrypt != TSession.call)
            {
                Logger.log(TLogLevel.logUser, "Error: Currently only the encryption of calls is explicitely supported.");
                return false;
            }

            //check strength of key
            if ((key.bitStrength != BIT_STRENGTH_KEY) || (key.algorithm != SYMMETRIC_ALGORITHM))
            {
                return false;
            }
            sessionKey = key;
            gotSessionKey = true;

            //start the Bottom protocol
            if (primaryActor == true)
            {
                start();
            }
            return true; //TODO: think about return value
        }

        /// <inheritdoc />
        public int getSupportedKeyBitStrength()
        { 
            return BIT_STRENGTH_KEY;
        }

        public TCryptoAlgo getSupportedKeyAlgorithm()
        {
            return SYMMETRIC_ALGORITHM;
        }

        //secure access protocol state data (only once setable)
        private String _peerConnectionsListsRaw;
        private bool _peerConnectionsListsRawSet = false;
        private String peerConnectionsListsRaw {
            set {
                if (_peerConnectionsListsRawSet == false) {
                    _peerConnectionsListsRawSet = true;
                    _peerConnectionsListsRaw = value;
                }
            }
            get {
                return _peerConnectionsListsRaw;
            }
        }

        private String _peerMatchesRaw;
        private bool _peerMatchesRawSet = false;
        private String peerMatchesRaw { 
            set {
                if (_peerMatchesRawSet == false) {
                    _peerMatchesRawSet = true;
                    _peerMatchesRaw = value;
                }
            }
            get {
                return _peerMatchesRaw;
            }
        }

        private ConnectionsList _ownConnectionsIncoming;
        private bool _ownConnectionsIncomingSet = false;
        private ConnectionsList ownConnectionsIncoming {
            set {
                if (_ownConnectionsIncomingSet == false)
                {
                    _ownConnectionsIncomingSet = true;
                    _ownConnectionsIncoming = value;
                }
            }
            get {
                return _ownConnectionsIncoming;
            }
        }

        private ConnectionsList _ownConnectionsOutgoing;
        private bool _ownConnectionsOutgoingSet = false;
        private ConnectionsList ownConnectionsOutgoing
        {
            set
            {
                if (_ownConnectionsOutgoingSet == false)
                {
                    _ownConnectionsOutgoingSet = true;
                    _ownConnectionsOutgoing = value;
                }
            }
            get
            {
                return _ownConnectionsOutgoing;
            }
        }

        private BoundChannelMatches _channelsToEncrypt;
        private bool _channelsToEncryptSet = false;
        private BoundChannelMatches channelsToEncrypt
        {
            set
            {
                if (_channelsToEncryptSet == false)
                {
                    _channelsToEncryptSet = true;
                    _channelsToEncrypt = value;
                }
            }
            get
            {
                return _channelsToEncrypt;
            }
        }

        /// <inheritdoc />
        public void messageReceived(String rawMessage)
        {
            BottomMessage bottomMessage = new BottomMessage(rawMessage);
            // We don't want any race-condition here, aight?
            Logger.log(TLogLevel.logEverything, "Info: Received Bottom message: " + bottomMessage.type);

            lock(this)
            {
                switch (bottomMessage.type)
                {
                    case TBottomMessage.RequestConnections:
                        peerConnectionsListsRaw = bottomMessage.payload;
                        requestConnectionsReceived();
                        break;

                    case TBottomMessage.MyConnections:
                        peerConnectionsListsRaw = bottomMessage.payload;
                        myConnectionsReceived();
                        break;

                    case TBottomMessage.Matches:
                        peerMatchesRaw = bottomMessage.payload;
                        matchesReceived();
                        break;

                    case TBottomMessage.Confirm:
                        confirmReceived();
                        break;

                    case TBottomMessage.Abort:
                        abortReceived();
                        break;

                    case TBottomMessage.Unknown:
                        unknownReceived();
                        break;

                    default:
                        //drop
                        break;
                }
            }
        }
        ///////////////////////////////////////////////////////////////////////////////
        private const string STRING_DELIMITER = ":";

        private String getConnectionsListsBase64()
        {
            //get connections lists
            int[,] incoming = (int[,])comHookEngine.getOpenConnectionsIncoming();
            int[,] outgoing = (int[,])comHookEngine.getOpenConnectionsOutgoing();

            ownConnectionsIncoming = new ConnectionsList(incoming);
            ownConnectionsOutgoing = new ConnectionsList(outgoing);

            //clean-up lists by removing entries that do only appear in one or another list
            ownConnectionsIncoming.syncIps(ownConnectionsOutgoing);
            ownConnectionsOutgoing.syncIps(ownConnectionsIncoming);

            Dictionary<uint, uint> ipReplacements = new Dictionary<uint, uint>();

            String lists = ownConnectionsIncoming.toAnonymBase64(ipReplacements) + STRING_DELIMITER + ownConnectionsOutgoing.toAnonymBase64(ipReplacements);

            Logger.log(TLogLevel.logDebug, "Sending own connections:");
            Logger.log(TLogLevel.logDebug, "Incoming:");
            Logger.log(TLogLevel.logDebug, ownConnectionsIncoming.toLogString());
            Logger.log(TLogLevel.logDebug, "Outgoing:");
            Logger.log(TLogLevel.logDebug, ownConnectionsOutgoing.toLogString());

            return lists;
        }

        internal override void sendRequestConnections()
        {
            //REQUEST_CONNECTIONS message contains own list of connections as well
            _sendMessage(new BottomMessage(TBottomMessage.RequestConnections, getConnectionsListsBase64()).text);
        }

        internal override void sendMyConnections()
        {
            _sendMessage(new BottomMessage(TBottomMessage.MyConnections, getConnectionsListsBase64()).text);
        }

        internal override void sendMatches()
        {
            String matches = channelsToEncrypt.incoming.toBase64() + STRING_DELIMITER + channelsToEncrypt.outgoing.toBase64();
            _sendMessage(new BottomMessage(TBottomMessage.Matches, matches).text);
        }

        internal override void sendConfirm()
        {
            _sendMessage(new BottomMessage(TBottomMessage.Confirm).text);
        }

        internal override void sendAbort()
        {
            _sendMessage(new BottomMessage(TBottomMessage.Abort).text);
        }


        /// <inheritdoc />
        internal override void startSecureChannel()
        {
            Logger.log(TLogLevel.logUser, "Info: Starting secure session with " + handlePartner + " at " + HelperFunctions.ipToString(channelsToEncrypt.peerIp));

            if (gotSessionKey != true)
            {
                Logger.log(TLogLevel.logUser, "Fatal Error: Could not start secure session, because not symmetric key is present. This should never happen.");
                return;
            }

            //create new session which can be extended with new connections at runtime
            sessionId = comHookEngine.createSession(1);
            

            //add incoming channels to encrypt to session
            Logger.log(TLogLevel.logUser, "Info: Securing the following incoming channels:");
            foreach (Channel channel in channelsToEncrypt.incoming.channels)
            {
                Logger.log(TLogLevel.logUser, channel.toLogString());
                if (comHookEngine.addConnectionIncoming(sessionId, channelsToEncrypt.peerIp, channel.srcPort, channel.dstPort, (uint)channel.protocol) != 1)
                {
                    Logger.log(TLogLevel.logUser, "Fatal Error: Failed to encrypt an incoming channel.");
                    throw new ExceptionEncryptionSetUpFailed("Could not add incoming channel");
                }
            }

            //add outgoing channels to encrypt to session
            Logger.log(TLogLevel.logUser, "Info: Securing the following outgoing channels:");
            foreach (Channel channel in channelsToEncrypt.outgoing.channels)
            {
                Logger.log(TLogLevel.logUser,channel.toLogString());
                if (comHookEngine.addConnectionOutgoing(sessionId, channelsToEncrypt.peerIp, channel.srcPort, channel.dstPort, (uint)channel.protocol) != 1)
                {
                    Logger.log(TLogLevel.logUser, "Fatal Error: Failed to encrypt an outgoing channel.");
                    throw new ExceptionEncryptionSetUpFailed("Could not add incoming channel");
                }
            }
            
            //start the session
            if (comHookEngine.startSession(sessionId, sessionKey.getRawKey()) != 1)
            {
                Logger.log(TLogLevel.logUser, "Fatal Error: Could not start secure session.");
                throw new ExceptionEncryptionSetUpFailed("Could not start secure sesison.");
            }
            userIO.goneSecure(handlePartner, TSession.call);
        }

        /// <inheritdoc />
        internal override bool evaluateReceivedMatches()
        {
            char[] delimiters = { (char)STRING_DELIMITER[0] };
            String[] b64Lists = peerMatchesRaw.Split(delimiters);

            if ((b64Lists.Length != 2) || (b64Lists.Rank != 1))
            {
                return false;
            }

            //try unmarshalling the received data
            ChannelMatchesList peerMatchesIncoming, peerMatchesOutgoing;
            try
            {
                peerMatchesIncoming = new ChannelMatchesList(b64Lists[0]);
                peerMatchesOutgoing = new ChannelMatchesList(b64Lists[1]);
            }
            catch
            {
                Logger.log(TLogLevel.logDebug, "Error: Failed to unmarshal the channel matches lists received.");
                return false;
            }


            //Cross-compare received matches with own found matches. In case any difference is found, abort.
            if (peerMatchesIncoming.equals(channelsToEncrypt.outgoing) == false)
            {
                Logger.log(TLogLevel.logDebug, "Error: Found difference between own outgoing matches and the peer's incoming matches.");
                return false;
            }

            if (peerMatchesOutgoing.equals(channelsToEncrypt.incoming) == false)
            {
                Logger.log(TLogLevel.logDebug, "Error: Found difference between own incoming matches and the peer's outgoing matches.");
                return false;
            }
            return true;
        }

        /// <inheritdoc />
        internal override bool evaluateReceivedConnections()

        {
            char[] delimiters = { (char)STRING_DELIMITER[0] };
            String[] b64Lists = peerConnectionsListsRaw.Split(delimiters);

            if ((b64Lists.Length != 2) || (b64Lists.Rank != 1))
            {
                return false;
            }

            //try unmarshalling the received data
            ConnectionsList peerConnectionsIncoming, peerConnectionsOutgoing;
            try
            {
                peerConnectionsIncoming = new ConnectionsList(b64Lists[0]);
                peerConnectionsOutgoing = new ConnectionsList(b64Lists[1]);
            }
            catch 
            {
                Logger.log(TLogLevel.logDebug, "Error: Failed to unmarshal the connections lists received.");
                return false;
            }

            Logger.log(TLogLevel.logDebug, "Received connections:");
            Logger.log(TLogLevel.logDebug, "Incoming:");
            Logger.log(TLogLevel.logDebug, peerConnectionsIncoming.toLogString());
            Logger.log(TLogLevel.logDebug, "Outgoing:");
            Logger.log(TLogLevel.logDebug, peerConnectionsOutgoing.toLogString());


            //NOTE: ips in peerConnectionsIncoming and peerConnectionsOutgoing are just placeholders
            try
            {
                //find matches with own connections
                //// own incoming <-> peer outgoing
                PortMatch[] matchesOwnIncomingPeerOutgoing = findPortMatches(ownConnectionsIncoming, peerConnectionsOutgoing);

                //// own outgoing <-> peer incoming
                PortMatch[] matchesOwnOutgoingPeerIncoming = findPortMatches(ownConnectionsOutgoing, peerConnectionsIncoming);

                ////check if any match was found
                /*
                if (matchesOwnIncomingPeerOutgoing.Length == 0 || matchesOwnOutgoingPeerIncoming.Length == 0)
                
                {
                    Logger.log(TLogLevel.logUser, "Info: Looks like your conversation is being relayed by another host.");
                    Logger.log(TLogLevel.logUser, "Trying to identify the relaying host...");
                    //// looks like we got a relayed connections, try to identify the relaying host

                    comHookEngine.startCountingConnections();
                    System.Threading.Thread.Sleep(3000);
                    comHookEngine.stopCountingConnections();

                    Logger.log(TLogLevel.logUser, "Info: " + HelperFunctions.ipToString(comHookEngine.getMostUsedPeerIP()) + " was identified as the realying host.");

                }
                */
                //// now identify the channels effectively used for communication by self and the peer, by looking for ip matches in the previously found port matches
                channelsToEncrypt = generateBoundChannelMatches(matchesOwnIncomingPeerOutgoing, matchesOwnOutgoingPeerIncoming);
            }
            catch (ExceptionConnectionSelectionFailed)
            {
                Logger.log(TLogLevel.logUser, "Error: Could not unambigously identify ip/port matches in the connections lists received.");
                return false;
            }

            Logger.log(TLogLevel.logDebug, "Identified the following channels for encryption:\n" + channelsToEncrypt.toLogString());
            return true;
        }

        /// <summary>
        /// Idnetifies connections routed over a third party. Only destination ports are taken into account.
        /// </summary>
        /// <param name="connectionsA"></param>
        /// <param name="connectionsB"></param>
        /// <returns>A list of idnetified matches.</returns>
        private PortMatch[] findPortMatchesRelay(ConnectionsList connectionsA, ConnectionsList connectionsB)
        {
            //create dictionary <dstPort,indexes in connectionsA>
            Dictionary<ushort, HashSet<int>> portsA = new Dictionary<ushort, HashSet<int>>();

            int i = 0;
            foreach (Connection connectionA in connectionsA.connections)
            {
                //check if destination port is already known
                if (portsA.ContainsKey(connectionA.srcPort) == false)
                {
                    portsA[connectionA.dstPort] = new HashSet<int>();
                }
                //protocol/port combination is not present a second time -> add index                  
                portsA[connectionA.dstPort].Add(i);
                i++;
            }

            List<PortMatch> portMatches = new List<PortMatch>();
            //now find matches in connectionsB
            foreach (Connection connectionB in connectionsB.connections)
            {
                if (portsA.ContainsKey(connectionB.dstPort))
                {
                    //matching port found -> check for matching protocol
                    foreach (int indexPort in portsA[connectionB.dstPort])
                    {
                        Connection connectionA = connectionsA.connections[indexPort];
                        if (connectionA.protocol == connectionB.protocol)
                        {
                            //match found -> add
                            portMatches.Add(new PortMatch(connectionA.srcPort, connectionA.dstPort, connectionA.peerIp, connectionB.peerIp, connectionA.protocol));
                        }
                    }
                }
            }
            return portMatches.ToArray();
        }


        private PortMatch[] findPortMatches(ConnectionsList connectionsA, ConnectionsList connectionsB)
        {
            //create dictionary <portIdentifier,indexes in connectionsA>
            Dictionary<uint, HashSet<int>> portsA = new Dictionary<uint, HashSet<int>>();

            int i = 0;
            foreach (Connection connectionA in connectionsA.connections)
            {
                //check if port-identifier is already known
                if (portsA.ContainsKey(connectionA.portIdentifier) == false)
                {
                    portsA[connectionA.portIdentifier] = new HashSet<int>();
                }
                //protocol/port combination is not present a second time -> add index                  
                portsA[connectionA.portIdentifier].Add(i);
                i++;
            }

            List<PortMatch> portMatches = new List<PortMatch>();
            //now find matches in connectionsB
            foreach (Connection connectionB in connectionsB.connections)
            {
                if (portsA.ContainsKey(connectionB.portIdentifier))
                {
                    //matching port found -> check for matching protocol
                    foreach (int indexPort in portsA[connectionB.portIdentifier])
                    {
                        Connection connectionA = connectionsA.connections[indexPort];
                        if (connectionA.protocol == connectionB.protocol)
                        {
                            //match found -> add
                            portMatches.Add(new PortMatch(connectionA.srcPort, connectionA.dstPort, connectionA.peerIp, connectionB.peerIp, connectionA.protocol));
                        }
                    }
                }
            }
            return portMatches.ToArray();
        }

        private BoundChannelMatches generateBoundChannelMatches(PortMatch[] portMatchesA, PortMatch[] portMatchesB)
        {
            //use ips of first port match as reference
            uint refPeerIp;
            uint refPlaceholderIp;
            if (portMatchesA.Length > 0)
            {
                refPeerIp = portMatchesA[0].peerIp;
                refPlaceholderIp = portMatchesA[0].placeholderIp;
            }
            else if (portMatchesB.Length > 0)
            {
                refPeerIp = portMatchesB[0].peerIp;
                refPlaceholderIp = portMatchesB[0].placeholderIp;
            }
            else
            {
                Logger.log(TLogLevel.logDebug, "Error: Could not identify any port-matches.");
                throw new ExceptionConnectionSelectionFailed("Got two empty port matches lists.");
            }

            List<Channel> channelsIncoming = new List<Channel>();
            List<Channel> channelsOutgoing = new List<Channel>();

            //fill list of incoming channels
            foreach (PortMatch portMatch in portMatchesA)
            {
                if ((portMatch.peerIp != refPeerIp) || (portMatch.placeholderIp != refPlaceholderIp))
                {
                    Logger.log(TLogLevel.logDebug, "Conflict in port matches (incoming):\nReference: " + portMatchesA[0].toLogString() + "\nConflict: " + portMatch.toLogString());
                    throw new ExceptionConnectionSelectionFailed("Found channel matches for more than one ip pair.");
                }
                channelsIncoming.Add(new Channel(portMatch.srcPort, portMatch.dstPort, portMatch.protocol));
            }

            //fill list of outgoing channels
            foreach (PortMatch portMatch in portMatchesB)
            {
                if ((portMatch.peerIp != refPeerIp) || (portMatch.placeholderIp != refPlaceholderIp))
                {
                    Logger.log(TLogLevel.logDebug, "Conflict in port matches (outgoing):\nReference: " + portMatchesA[0].toLogString() + "\nConflict: " + portMatch.toLogString());
                    throw new ExceptionConnectionSelectionFailed("Found channel matches for more than one ip pair.");
                }
                channelsOutgoing.Add(new Channel(portMatch.srcPort, portMatch.dstPort, portMatch.protocol));
            }

            //clone channels if one list should be empty
            if (channelsOutgoing.Count == 0 && channelsIncoming.Count != 0)
            {
                foreach (Channel channel in channelsIncoming)
                {
                    Channel reverseChannel = new Channel(channel.dstPort, channel.srcPort, channel.protocol);
                    channelsOutgoing.Add(reverseChannel);
                }
            }
            else if (channelsIncoming.Count == 0 && channelsOutgoing.Count != 0)
            {
                foreach (Channel channel in channelsOutgoing)
                {
                    Channel reverseChannel = new Channel(channel.dstPort, channel.srcPort, channel.protocol);
                    channelsIncoming.Add(reverseChannel);
                }
            }
            return new BoundChannelMatches(refPeerIp, new ChannelMatchesList(channelsIncoming.ToArray()), new ChannelMatchesList(channelsOutgoing.ToArray()));
        }

        /// <inheritdoc />
        public bool close()
        {
            if (comHookEngine.destroySession(sessionId) != 1)
            {
                Logger.log(TLogLevel.logUser, "Error: Failed to stop encrypted session.");
                return false;
            }

            userIO.goneInsecure(handlePartner, TSession.call);
            return true;
        }

        /// <summary>
        /// Represents an unbound ip channel (defined by source- and destination port).
        /// </summary>
        internal class Channel
        {
            private ushort _srcPort;
            private ushort _dstPort;
            private TIpProtocol _protocol;
            private uint _identifier;
            private uint _identifierInverse;

            public ushort srcPort { get { return _srcPort; } }
            public ushort dstPort { get { return _dstPort; } }
            public TIpProtocol protocol { get { return _protocol; } }

            /// <summary>
            /// An unambiguous idendtifier for the connection.
            /// </summary>
            public uint portIdentifier { get { return _identifier; } }

            /// <summary>
            /// The same as portIdentifier but in inverted order.
            /// </summary>
            public uint portIdentifierInverse { get { return _identifierInverse; } }

            public Channel(ushort srcPort, ushort dstPort, TIpProtocol protocol)
            {
                this._srcPort = srcPort;
                this._dstPort = dstPort;
                this._protocol = protocol;

                _identifier = ((uint)dstPort << 16) | (uint)srcPort;
                _identifierInverse = ((uint)srcPort << 16) | (uint)dstPort;
            }

            public virtual String toLogString()
            {
                return "src port: " + srcPort + " dst port: " + dstPort + " proto: " + protocol;
            }
        }

        /// <summary>
        /// A Channel bind to a specific peer ip.
        /// </summary>
        internal class Connection : Channel
        {
            private uint _peerIp;

            public uint peerIp { get { return _peerIp; } }

            public Connection(uint peerIp, ushort srcPort, ushort dstPort, TIpProtocol protocol, uint timestamp)
                : base(srcPort, dstPort, protocol)
            {
                this._peerIp = peerIp;
            }

            public override String toLogString()
            {
                return "peer ip:" + HelperFunctions.ipToString(peerIp) + "\n" + base.toLogString();
            }
        }

        /// <summary>
        /// Represents a found match between own connections and the received peer's connections.
        /// </summary>
        private class PortMatch : Channel
        {
            private uint _placeholderIp;
            private uint _peerIp;

            public uint placeholderIp { get { return _placeholderIp; } }
            public uint peerIp { get { return _peerIp; } }

            public PortMatch(ushort srcPort, ushort dstPort, uint peerIp, uint placeholderIp, TIpProtocol protocol)
                : base(srcPort, dstPort, protocol)
            {
                this._placeholderIp = placeholderIp;
                this._peerIp = peerIp;
            }

            public override String toLogString()
            {
                return "peer ip: " + HelperFunctions.ipToString(peerIp) + "\nplaceholder ip: " + HelperFunctions.ipToString(placeholderIp) + "\n" + base.toLogString();
            }
        }

        /// <summary>
        /// A list of matched channels.
        /// </summary>
        internal class ChannelMatchesList
        {
            public Channel[] channels { get { return _channels; } }
            public int length { get { return _channels.Length; } }

            private Channel[] _channels;

            private const int NUM_OF_CHANNEL_IDENTIFIERS = 3;

            public ChannelMatchesList(Channel[] channels)
            {
                _channels = channels;
            }
            
            /// <summary>
            /// Unmarshals an ChannelMatchesList object from a Base64 string.
            /// </summary>
            /// <param name="base64">The Base64 to marshal from</param>
            /// /// <remarks>Format of the Base64 encoded integer-array: 
            /// rawConnections[i,0] : srcPort
            /// rawConnections[i,1] : dstPort
            /// rawConnections[i,2] : protocol
            /// </remarks>
            public ChannelMatchesList(String base64)
            {
                int[,] rawChannels = HelperFunctions.base64ToIntegerArray2d(base64, NUM_OF_CHANNEL_IDENTIFIERS);
                

                _channels = new Channel[rawChannels.GetLength(0)];
                for (int i = 0; i < rawChannels.GetLength(0); i++)
                {
                    _channels[i] = new Channel((ushort)rawChannels[i, 0], (ushort)rawChannels[i, 1], (TIpProtocol)rawChannels[i, 2]);
                }
            }

            /// <summary>
            /// Gets the contents of the list as a base64 encoded string.
            /// </summary>
            /// <returns>The base64 encoded string.</returns>
            public String toBase64()
            {
                int[,] rawChannels = new int[channels.Length, NUM_OF_CHANNEL_IDENTIFIERS];
                
                int i = 0;
                foreach (Channel channel in _channels)
                {
                    rawChannels[i, 0] = channel.srcPort;
                    rawChannels[i, 1] = channel.dstPort;
                    rawChannels[i, 2] = (int)channel.protocol;
                    i++;
                }
                return HelperFunctions.integerArray2dToBase64(rawChannels, NUM_OF_CHANNEL_IDENTIFIERS);
            }

            /// <summary>
            /// Checks if two ChannelMatchesLists contain exactly the same items.
            /// </summary>
            /// <param name="otherList">ChannelMatchesList to compare with</param>
            /// <returns>A flag indicating the equalness of both lists.</returns>
            public bool equals(ChannelMatchesList otherList)
            {
                if (this.length != otherList.length)
                {
                    return false;
                }

                //create mapping portIdentifier->index in a
                Dictionary<uint, HashSet<TIpProtocol>> channelProtocols = new Dictionary<uint, HashSet<TIpProtocol>>();
                foreach (Channel channel in this.channels)
                {
                    if (channelProtocols.ContainsKey(channel.portIdentifier) == false)
                    {
                        channelProtocols[channel.portIdentifier] = new HashSet<TIpProtocol>();
                    }
                    channelProtocols[channel.portIdentifier].Add(channel.protocol);
                }

                foreach (Channel channel in otherList.channels)
                {
                    if (channelProtocols.ContainsKey(channel.portIdentifier) == false)
                    {
                        return false;
                    }

                    bool foundEqualMatch = false;
                    foreach (TIpProtocol protocol in channelProtocols[channel.portIdentifier])
                    {
                        if (channel.protocol == protocol)
                        {
                            foundEqualMatch = true;
                            break;
                        }
                    }
                    if (foundEqualMatch == false)
                    {
                        return false;
                    }
                }
                return true;
            }

            /// <summary>
            /// Creates a human readable string describing the contents of the list.
            /// </summary>
            /// <returns>The string.</returns>
            public String toLogString()
            {
                String logString = "";
                foreach (Channel channel in channels)
                {
                    logString += "src: " + channel.srcPort + " dst: " + channel.dstPort + " proto: " + channel.protocol + "\n";
                }
                return logString;
            }
        }

        /// <summary>
        /// A list of all matched channels with one specific peer.
        /// </summary>
        internal class BoundChannelMatches
        {
            public uint peerIp { get { return _peerIp; } }
            private uint _peerIp;

            private ChannelMatchesList _incoming;
            private ChannelMatchesList _outgoing;

            public ChannelMatchesList incoming { get { return _incoming; } }
            public ChannelMatchesList outgoing { get { return _outgoing; } }

            public BoundChannelMatches(uint peerIp, ChannelMatchesList incomingChannelMatches, ChannelMatchesList outgoingChannelMatches)
            {
                this._peerIp = peerIp;
                this._incoming = incomingChannelMatches;
                this._outgoing = outgoingChannelMatches;
            }

            /// <summary>
            /// Creates a human readable string describing the contents of the list.
            /// </summary>
            /// <returns>The string.</returns>
            public String toLogString()
            {
                String logString = HelperFunctions.ipToString(_peerIp);
                logString += "\nincoming:\n" + _incoming.toLogString();
                logString += "\noutgoing:\n" + _outgoing.toLogString();
                
                return logString;
            }
        }

        internal class ConnectionsList
        {
            public Connection[] connections { get { return _connections; } }
            public int length { get { return _connections.Length; } }

            private Connection[] _connections;
            private const int NUM_OF_CONNECTION_IDENTIFIERS = 4;

            /// <summary>
            /// Creates a ConnectionsList object from a two-dimensional integer-array describing various connections.
            /// </summary>
            /// <param name="rawConnections">The integer array.</param>
            /// <param name="hasTimestamp">A flag indicating whether the connections list is time-stamped.</param>
            /// <remarks>Format of the integer-array: 
            /// rawConnections[i,0] : ip
            /// rawConnections[i,1] : srcPort
            /// rawConnections[i,2] : dstPort
            /// rawConnections[i,3] : protocol
            /// rawConnections[i,4] : timestamp
            /// </remarks>
            public ConnectionsList(int[,] rawConnections, bool hasTimestamp = false)
            {
                parseIntegerArray(rawConnections, hasTimestamp);
            }

            /// <summary>
            /// Unmarshals an ConnectionsList object from a Base64 string.
            /// </summary>
            /// <param name="base64">The Base64 to marshal from</param>
            public ConnectionsList(String base64)
            {
                int[,] rawConnections = HelperFunctions.base64ToIntegerArray2d(base64, NUM_OF_CONNECTION_IDENTIFIERS);
                parseIntegerArray(rawConnections, false);
            }

            private void parseIntegerArray(int[,] rawConnections, bool hasTimestamp)
            {
                _connections = new Connection[rawConnections.GetLength(0)];
                for (int i = 0; i < rawConnections.GetLength(0); i++)
                {
                    uint timestamp;
                    if (hasTimestamp)
                    {
                        timestamp = (uint)rawConnections[i, 4];
                    }
                    else
                    {
                        timestamp = (uint)0xFFFFFFFF;
                    }
                    _connections[i] = new Connection((uint)rawConnections[i, 0], (ushort)rawConnections[i, 1], (ushort)rawConnections[i, 2], (TIpProtocol)rawConnections[i, 3], timestamp);
                }
            }

            /// <summary>
            /// Marshals the list to a anonymized Base64 string.
            /// </summary>
            /// <param name="ipReplacements">IP translation-table for used anonymization.</param>
            /// <returns>The Base64 string.</returns>
            public String toAnonymBase64(Dictionary<uint, uint> ipReplacements)
            {
                int[,] rawConnections = new int[_connections.Length, NUM_OF_CONNECTION_IDENTIFIERS];

                lock (ipReplacements)
                {
                    int i = 0;
                    foreach (Connection connection in _connections)
                    {
                        uint peerIp = 0;

                        if (ipReplacements.ContainsKey(connection.peerIp) == false)
                        {
                            uint replacement = (uint)ipReplacements.Count;
                            ipReplacements[connection.peerIp] = replacement;
                            peerIp = replacement;
                        }
                        else
                        {
                            peerIp = ipReplacements[connection.peerIp];
                        }

                        rawConnections[i, 0] = (int)peerIp;
                        rawConnections[i, 1] = connection.srcPort;
                        rawConnections[i, 2] = connection.dstPort;
                        rawConnections[i, 3] = (int)connection.protocol;

                        i++;
                    }
                }

                return HelperFunctions.integerArray2dToBase64(rawConnections, NUM_OF_CONNECTION_IDENTIFIERS);
            }

            /// <summary>
            /// Removes all entries from the list with peerIps that are not present in both lists.
            /// </summary>
            /// <param name="connectionsList">The connections list to sync with</param>
            public void syncIps(ConnectionsList connectionsList)
            {
                List<Connection> connectionsNew = new List<Connection>();
                HashSet<uint> ips = new HashSet<uint>();

                //first parse the list to sync with
                foreach (Connection connection in connectionsList.connections)
                {
                    ips.Add(connection.peerIp);
                }

                //now clean up own list
                foreach (Connection connection in _connections)
                {
                    if (ips.Contains(connection.peerIp) == true)
                    {
                        connectionsNew.Add(connection);
                    }
                }

                _connections = connectionsNew.ToArray();
            }

            /// <summary>
            /// Creates a human readable string describing the contents of the list.
            /// </summary>
            /// <returns>The string.</returns>
            public String toLogString()
            {
                String logString = "";
                foreach (Connection connection in connections)
                {
                    logString += HelperFunctions.ipToString(connection.peerIp);
                    logString += " src: " + connection.srcPort + " dst: " + connection.dstPort + " proto: " + connection.protocol + "\n";
                }
                return logString;
            }
        }
    }

    internal class HelperFunctions
    {
        private const int SIZE_OF_INTEGER = 4;
        private const int BITS_IN_BYTE = 8;

        /// <summary>
        /// Converts a 2 dimensional integer array to a base64 string.
        /// </summary>
        /// <param name="array">The array to convert.</param>
        ///  <param name="itemsPerRow">The number of items in each row of the 2-dimensional matrix.</param>
        /// <returns>The converted Base64 string.</returns>
        internal static String integerArray2dToBase64(int[,] array, int itemsPerRow)
        {
            //2d array to 1d array
            int[] flatArray = new int[array.Length];

            for (int i = 0, k = 0; i < array.GetLength(0); i++)
            {
                for (int j = 0; j < itemsPerRow; j++, k++)
                {
                    flatArray[k] = array[i, j];
                }
            }

            //1d array to byte array
            byte[] byteArray = new byte[flatArray.Length * SIZE_OF_INTEGER];
            for (int i = 0; i < flatArray.Length; i++)
            {
                int val = flatArray[i];
                //little endian
                for (int j = 0; j < SIZE_OF_INTEGER; j++)
                {
                    byteArray[i * SIZE_OF_INTEGER + j] = (byte)(val & 0xFF);
                    val = val >> BITS_IN_BYTE;
                }
            }
            return System.Convert.ToBase64String(byteArray);
        }

        /// <summary>
        /// Converts a Base64 string to a 2 dimensional integer array.
        /// </summary>
        /// <param name="b64">The Base64 string to convert.</param>
        /// <param name="itemsPerRow">The desired dimension.</param>
        /// <returns>The converted integer array.</returns>
        internal static int[,] base64ToIntegerArray2d(String b64, int itemsPerRow)
        {
            byte[] byteArray = System.Convert.FromBase64String(b64);
            int[] flatArray = new int[byteArray.Length / SIZE_OF_INTEGER];

            for (int i = 0; i < flatArray.Length; i++)
            {
                int val = 0;
                for (int j = 0; j < SIZE_OF_INTEGER; j++)
                {
                    val += (byteArray[i * SIZE_OF_INTEGER + j] << (BITS_IN_BYTE * j));
                }
                flatArray[i] = val;
            }

            int[,] array = new int[flatArray.Length / itemsPerRow, itemsPerRow];

            for (int i = 0, k = 0; i < flatArray.Length / itemsPerRow; i++)
            {
                for (int j = 0; j < itemsPerRow; j++, k++)
                {
                    array[i, j] = flatArray[k];
                }
            }
            return array;
        }

        internal static String ipToString(uint ip)
        {
            String str = "";
            str += ip & 0xFF;
            str += ".";
            str += (ip >> 8) & 0xFF;
            str += ".";
            str += (ip >> 16) & 0xFF;
            str += ".";
            str += ip >> 24;

            return str;
        }
    }
}

