using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using watchasayin.Surface;

namespace watchasayin.Surface.GUI
{
    /// <summary>
    /// The GUI for both the Surface and the Kryptonite module.
    /// </summary>
    public partial class FormMain : Form, IUserOutput, Kryptonite.IUserIO, Bottom.IUserIO
    {
        private const String NOTIFICATION_CAPTION = "Note";
        private const String INFORMATION_CAPTION = "Info";
        private const String CONFIRMATION_CAPTION = "Confirm";

        private const String SECURE = "secure";
        private const String INSECURE = "-";
        private const String NO_FINGERPRINT = "-";

        /// <summary>
        /// The public constructor.
        /// </summary>
        public FormMain()
        {
            InitializeComponent();
        }

        /// <inheritdoc/>        
        public void log(string msg)
        {
            Invoke((MethodInvoker)delegate
            {
                this.listLog.Items.Add(msg);
            });
        }

        /// <inheritdoc />
        public void addSession(String handlePartner, TSession sessionType)
        {
            Invoke((MethodInvoker)delegate
            {
                this.listCalls.Items.Add(new ListViewItem(new String[] { handlePartner, "" + sessionType, INSECURE, NO_FINGERPRINT }));
            });
        }

        /// <inheritdoc />
        public void removeSession(String handlePartner, TSession sessionType)
        {
            Invoke((MethodInvoker)delegate
            {
            //todo: does this work out?
            string str2nd = "" + sessionType;
            foreach (ListViewItem item in listCalls.Items)
            {
                if ((item.SubItems[0].Text == handlePartner) && (item.SubItems[1].Text == str2nd))
                {
                    listCalls.Items.Remove(item);
                    break;
                }
            }
            });
        }

        //Bottom
        private void playSoundSecure()
        {
            System.Media.SystemSounds.Asterisk.Play();
            System.Threading.Thread.Sleep(250);
            System.Media.SystemSounds.Asterisk.Play();
            System.Threading.Thread.Sleep(250);
            System.Media.SystemSounds.Asterisk.Play();
            System.Threading.Thread.Sleep(250);
        }

        private void playSoundInsecure()
        {
            System.Media.SystemSounds.Exclamation.Play();
            System.Threading.Thread.Sleep(250);
            System.Media.SystemSounds.Exclamation.Play();
            System.Threading.Thread.Sleep(250);
            System.Media.SystemSounds.Exclamation.Play();

        }

        /// <inheritdoc />
        public void goneSecure(String handlePartner, TSession sessionType)
        {
            Invoke((MethodInvoker)delegate
            {
            if (sessionType != TSession.call)
            {
                //we currently only explicitly support calls
                return;
            }
           
            string str2nd = "" + TSession.call;
            foreach (ListViewItem item in listCalls.Items)
            {
                if ((item.SubItems[0].Text == handlePartner) && (item.SubItems[1].Text == str2nd))
                {
                    item.SubItems[2].Text = SECURE;
                    System.Threading.Thread t = new System.Threading.Thread(playSoundSecure);
                    t.Start();
                    break;
                }
            }
            });
        }

        /// <inheritdoc />
        public void goneInsecure(String handlePartner, TSession sessionType)
        {
            Invoke((MethodInvoker)delegate
            {
            string str2nd = "" + TSession.call;
            foreach (ListViewItem item in listCalls.Items)
            {
                if ((item.SubItems[0].Text == handlePartner) && (item.SubItems[1].Text == str2nd))
                {
                    item.SubItems[2].Text = INSECURE;
                    System.Threading.Thread t = new System.Threading.Thread(playSoundInsecure);
                    t.Start();
                    
                    break;
                }
            }
            });
        }
        //Kryptonite

        /// <inheritdoc />
        public bool displayNotification(String notification)
        {
            DialogResult result = MessageBox.Show(notification, NOTIFICATION_CAPTION, MessageBoxButtons.OKCancel, MessageBoxIcon.Information);

            return (result == DialogResult.OK);
        }

        /// <inheritdoc />
        public bool noPrivateKeyFound()
        {
            DialogResult result = MessageBox.Show("No private key was found. Shall one be created?", CONFIRMATION_CAPTION, MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            return (result == DialogResult.Yes);
        }

        /// <inheritdoc />
        public bool privateKeyInvalid()
        {
            DialogResult result = MessageBox.Show("Your private key seems to be invalid. Shall it be replaced by a new one?", CONFIRMATION_CAPTION, MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            return (result == DialogResult.Yes);
        }

        /// <inheritdoc />
        public bool noFingerprintsFound()
        {
            DialogResult result = MessageBox.Show("No fingerprints file was found. Shall a new one be created?", CONFIRMATION_CAPTION, MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            return (result == DialogResult.Yes);
        }

        /// <inheritdoc />
        public bool fingerprintsInvalid()
        {
            DialogResult result = MessageBox.Show("Your fingerprints file seems to be invalid. Shall it be replaced by a new one?", CONFIRMATION_CAPTION, MessageBoxButtons.YesNo, MessageBoxIcon.Question);
            return (result == DialogResult.Yes);
        }

        /// <inheritdoc />
        public bool confirmNewFingerprint(String handlePartner, String fingerprint)
        {
            DialogResult result = MessageBox.Show(String.Format("Please confirm the following fingerprint for the user \"{0}\":\n\n{1}", handlePartner, fingerprint), CONFIRMATION_CAPTION, MessageBoxButtons.YesNo, MessageBoxIcon.Warning);
            return (result == DialogResult.Yes);
        }

        /// <inheritdoc />
        public void displayOwnFingerprint(String fingerprint)
        {
            Invoke((MethodInvoker)delegate
            {
            this.labelFingerprint.Text = fingerprint;
            });
        }

        /// <inheritdoc />
        public void akeSuccessful(String handlePartner, String fingerprint)
        {
            Invoke((MethodInvoker)delegate
            {
            foreach (ListViewItem item in listCalls.Items)
            {
                if (item.SubItems[0].Text == handlePartner)
                {
                    item.SubItems[3].Text = fingerprint;
                    break;
                }
            }
            });
        }

        private void close_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void FormMain_Click(object sender, EventArgs e)
        {
        }
    }
}
