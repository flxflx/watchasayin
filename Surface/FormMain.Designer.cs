namespace watchasayin.Surface.GUI
{
    partial class FormMain
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.listLog = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.columnCallee = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnType = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.listCalls = new System.Windows.Forms.ListView();
            this.columnStatus = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnFingerprint = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.close = new System.Windows.Forms.Button();
            this.labelFingerprint = new System.Windows.Forms.TextBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // listLog
            // 
            this.listLog.BackColor = System.Drawing.Color.Silver;
            this.listLog.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.listLog.FormattingEnabled = true;
            this.listLog.Location = new System.Drawing.Point(12, 174);
            this.listLog.Name = "listLog";
            this.listLog.Size = new System.Drawing.Size(494, 104);
            this.listLog.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(228)))), ((int)(((byte)(35)))), ((int)(((byte)(240)))));
            this.label1.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(229)))), ((int)(((byte)(13)))));
            this.label1.Location = new System.Drawing.Point(13, 43);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(117, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "YOUR FINGERPRINT:";
            // 
            // columnCallee
            // 
            this.columnCallee.Text = "Name";
            // 
            // columnType
            // 
            this.columnType.Text = "Type";
            this.columnType.Width = 51;
            // 
            // listCalls
            // 
            this.listCalls.BackColor = System.Drawing.Color.Silver;
            this.listCalls.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.listCalls.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnCallee,
            this.columnType,
            this.columnStatus,
            this.columnFingerprint});
            this.listCalls.Location = new System.Drawing.Point(12, 65);
            this.listCalls.Name = "listCalls";
            this.listCalls.Size = new System.Drawing.Size(494, 96);
            this.listCalls.TabIndex = 1;
            this.listCalls.UseCompatibleStateImageBehavior = false;
            this.listCalls.View = System.Windows.Forms.View.Details;
            // 
            // columnStatus
            // 
            this.columnStatus.Text = "Status";
            // 
            // columnFingerprint
            // 
            this.columnFingerprint.Text = "Fingerprint";
            this.columnFingerprint.Width = 316;
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = global::watchasayin.Properties.Resources.logo;
            this.pictureBox1.Location = new System.Drawing.Point(6, 3);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(160, 50);
            this.pictureBox1.TabIndex = 4;
            this.pictureBox1.TabStop = false;
            // 
            // close
            // 
            this.close.Enabled = false;
            this.close.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.close.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(228)))), ((int)(((byte)(35)))), ((int)(((byte)(240)))));
            this.close.Location = new System.Drawing.Point(496, 3);
            this.close.Name = "close";
            this.close.Size = new System.Drawing.Size(20, 20);
            this.close.TabIndex = 6;
            this.close.Text = "X";
            this.close.UseVisualStyleBackColor = true;
            this.close.Visible = false;
            this.close.Click += new System.EventHandler(this.close_Click);
            // 
            // labelFingerprint
            // 
            this.labelFingerprint.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(228)))), ((int)(((byte)(35)))), ((int)(((byte)(240)))));
            this.labelFingerprint.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.labelFingerprint.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(229)))), ((int)(((byte)(13)))));
            this.labelFingerprint.Location = new System.Drawing.Point(128, 43);
            this.labelFingerprint.Name = "labelFingerprint";
            this.labelFingerprint.ReadOnly = true;
            this.labelFingerprint.Size = new System.Drawing.Size(378, 13);
            this.labelFingerprint.TabIndex = 7;
            // 
            // FormMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(73)))), ((int)(((byte)(73)))), ((int)(((byte)(73)))));
            this.ClientSize = new System.Drawing.Size(518, 291);
            this.Controls.Add(this.labelFingerprint);
            this.Controls.Add(this.close);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.listCalls);
            this.Controls.Add(this.listLog);
            this.Controls.Add(this.pictureBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.Name = "FormMain";
            this.Text = "watchasayin";
            this.Click += new System.EventHandler(this.FormMain_Click);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox listLog;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ColumnHeader columnCallee;
        private System.Windows.Forms.ColumnHeader columnType;
        private System.Windows.Forms.ListView listCalls;
        private System.Windows.Forms.ColumnHeader columnStatus;
        private System.Windows.Forms.ColumnHeader columnFingerprint;
        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Button close;
        private System.Windows.Forms.TextBox labelFingerprint;
    }
}