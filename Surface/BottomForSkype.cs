using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Runtime.InteropServices;

namespace watchasayin.Bottom.SkypeSpecific
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
    /// Class extending the InProcess base class with the functionality to inject a Dll into the Skype process.
    /// </summary>
    public class InProcess : Bottom.InProcess
    {
        /// <summary>
        /// The file-name of the to be injected Dll.
        /// </summary>
        private const String FILENAME_BOTTOM_DLL = "Bottom.dll";

        private HANDLE hTarget;

        //Win32 api-function required for finding the Skype process and injecting a Dll.
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

        private String targetClassname;
        private String targetPath;
        private String targetName;


        /// <summary>
        /// Public constructor.
        /// <param name="userIO">An interface for communication with the user.</param>
        /// <param name="workingDir">The path of the dir to save/load files.</param>
        /// <param name="targetClassname">A characteristic classname registered by the target process. The classname is used to identify the target's process.</param>
        /// <param name="targetName">The name of the target - only used for logging.</param>
        /// <param name="targetPath">The path to the target executable.</param>
        /// <param name="targetStartIfNeeded">A flag indicating whether Skype should be launched if it is not running already.</param>
        /// </summary>
        public InProcess(IUserIO userIO, String workingDir, String targetName, String targetClassname, String targetPath = "", bool targetStartIfNeeded = false)
        {
            this.targetClassname = targetClassname;
            this.targetPath = targetPath;
            this.targetName = targetName;

            this.userIO = userIO;
            this.workingDir = workingDir;
            this.initSuccessful = false;

            hTarget = new HANDLE();

            hookTarget(targetStartIfNeeded);
        }

        /// <inheritdoc />
        override protected bool hookTarget(bool startIfNeeded)
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
            HWND hwndTarget = FindWindowA(targetClassname, 0);//FindWindowA(0, WINDOWNAME_SKYPE);
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

                //get Skype's process id
                DWORD pidSkype = new DWORD();
                GetWindowThreadProcessId(hwndTarget, ref pidSkype);

                Logger.log(TLogLevel.logDebug, "Info: Identified process with id " + pidSkype + " as " + targetName + " process.");

                //finally get the corresponding process handle
                hTarget = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD, 0, pidSkype);
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
                Logger.log(TLogLevel.logDebug, "Error: Could not allocate memory in Skype process.");
                File.Delete(FILENAME_BOTTOM_DLL);
                return false;
            }
            DWORD written = new DWORD();
            if (WriteProcessMemory(hTarget, addrPath, pathBottomDll, (SIZE_T)pathBottomDll.Length, ref written) == 0)
            {
                Logger.log(TLogLevel.logDebug, "Error: Could not write path of Bottom.dll to Skype process.");
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
                Logger.log(TLogLevel.logDebug, "Error: Could not create a new thread in the skype process. Last error: " + Marshal.GetLastWin32Error().ToString());
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
                Logger.log(TLogLevel.logUser, "Fatal Error: Failed to attach to " + targetName + " process.");
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

        /// <inheritdoc />
        override protected bool unhookTarget()
        {
            comGeneric.unload(FILENAME_BOTTOM_DLL);
            return true;
        }
    }
}


