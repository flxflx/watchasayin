using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using System.Threading;
using Microsoft.Win32;
using System.Runtime.InteropServices;
using SKYPE4COMLib;
using watchasayin.Surface;
using watchasayin.Surface.SkypeSpecific;
using watchasayin.Bottom;
using watchasayin.Kryptonite;

namespace watchasayin.Surface.GUI
{

    static class Program
    {
        //Generic consts
        const int NUMBER_OF_MESSAGES_TO_CACHE = 5;
        const String PATH_KRYPTONITE = "%AppData%\\watchasayin\\Kryptonite\\";
        const String PATH_BOTTOM = "%AppData%\\watchasayin\\Bottom\\";

        //Skype specific consts 
        const int SKYPE_PROTOCOL_VERSION = 8;
        const int CONNECTION_ATTEMPTS_BEFORE_ERROR = 5;
        private const String NAME_SKYPE = "Skype";
        private const String SUBKEY_REGISTRY_SKYPE = "Software\\Skype\\Phone";
        private const String VALUE_REGISTRY_SKYPE = "SkypePath";
        private const String CLASSNAME_SKYPE = "VideoDeviceMngr"; //characteristic class name for skype (found using IDA)

        private static FormMain form;
        private static Core core;
        private static bool coreInitialized;

       // [DllImport("Skype4COM.dll", SetLastError = true)]
       // static extern int DllRegisterServer();

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [MTAThread]
        static void Main()
        {
            System.Windows.Forms.Application.EnableVisualStyles();
            System.Windows.Forms.Application.SetCompatibleTextRenderingDefault(false);

            coreInitialized = false;

            //create gui
            form = new FormMain();
            Thread guiThread = new Thread(GuiThread);
            guiThread.Start();
            Thread.Sleep(25);

            //init logger
            Logger.logWriter.Add(form.log);
            Logger.log(TLogLevel.logUser, "Welcome to watchasayin!");

            //create working directories
            String pathKryptonite = Environment.ExpandEnvironmentVariables(PATH_KRYPTONITE);
            String pathBottom = Environment.ExpandEnvironmentVariables(PATH_BOTTOM);
            try
            {
                System.IO.Directory.CreateDirectory(pathKryptonite);
                System.IO.Directory.CreateDirectory(pathBottom);
            }
            catch
            {
                Logger.log(TLogLevel.logUser, "Fatal Error: Failed to access working directories.");
                return;
            }

            //init bottom module
            String pathSkype = "";
            try
            {
                RegistryKey keyPathSkype = Registry.CurrentUser.OpenSubKey(SUBKEY_REGISTRY_SKYPE);
                pathSkype = (String)keyPathSkype.GetValue(VALUE_REGISTRY_SKYPE);
                keyPathSkype.Close();
            }
            catch
            {
                Logger.log(TLogLevel.logUser, "Fatal Error: Skype does not seem to be installed on the system.");
            }          
            Bottom.InProcess bottomInProcess = new Bottom.InProcess(form, pathBottom, NAME_SKYPE, CLASSNAME_SKYPE, pathSkype, true);
            
            //connect to skype-api
            Logger.log(TLogLevel.logUser, "Connecting to Skype...");

            Skype skype;
            skype = new Skype();         

            int tries = 0;
            while (tries < CONNECTION_ATTEMPTS_BEFORE_ERROR)
            {
                try
                {
                    skype.Attach(SKYPE_PROTOCOL_VERSION);
                   break;
                }
                
                catch
                {
                    Logger.log(TLogLevel.logUser, "Could not connect - retrying.");
                    tries++; 
                }
            }
            
            if (tries == CONNECTION_ATTEMPTS_BEFORE_ERROR)
            {
                Logger.log(TLogLevel.logUser, "Fatal Error: Could not connect to Skype.");
                return;
            }

            //init other modules
            Logger.log(TLogLevel.logUser, "Connected.");

            APIEventHandler skypeApiEH = new APIEventHandler(skype);
            CommOverSkype surfaceCommSkype = new CommOverSkype(skype, NUMBER_OF_MESSAGES_TO_CACHE);

            Kryptonite.SharedLibrary kryptoniteShared = new Kryptonite.SharedLibrary(skype.CurrentUser.Handle, form, pathKryptonite);

            core = new Core(form, skypeApiEH, surfaceCommSkype, kryptoniteShared, bottomInProcess);
            coreInitialized = true;

        }
        [STAThread]
        static void GuiThread()
        {
            //start gui
            System.Windows.Forms.Application.Run(form);
            if (coreInitialized == true)
            {
                core.close();
            }
        }
    }
}
