namespace servctrl
{
    partial class FormMain
    {
        /// <summary>
        /// 必需的设计器变量。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 清理所有正在使用的资源。
        /// </summary>
        /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows 窗体设计器生成的代码

        /// <summary>
        /// 设计器支持所需的方法 - 不要修改
        /// 使用代码编辑器修改此方法的内容。
        /// </summary>
        private void InitializeComponent()
        {
            this.tabControl = new System.Windows.Forms.TabControl();
            this.tabPageStatus = new System.Windows.Forms.TabPage();
            this.tabPageTest = new System.Windows.Forms.TabPage();
            this.tabPageLogin = new System.Windows.Forms.TabPage();
            this.tabPageManage = new System.Windows.Forms.TabPage();
            this.tabPageLog = new System.Windows.Forms.TabPage();
            this.buttonConnect = new System.Windows.Forms.Button();
            this.comboBoxServList = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.textBoxPort = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.textBoxAddr = new System.Windows.Forms.TextBox();
            this.checkBoxSSL = new System.Windows.Forms.CheckBox();
            this.tabControl.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl
            // 
            this.tabControl.Controls.Add(this.tabPageStatus);
            this.tabControl.Controls.Add(this.tabPageTest);
            this.tabControl.Controls.Add(this.tabPageLogin);
            this.tabControl.Controls.Add(this.tabPageManage);
            this.tabControl.Controls.Add(this.tabPageLog);
            this.tabControl.Location = new System.Drawing.Point(9, 6);
            this.tabControl.Name = "tabControl";
            this.tabControl.SelectedIndex = 0;
            this.tabControl.Size = new System.Drawing.Size(910, 525);
            this.tabControl.TabIndex = 9;
            // 
            // tabPageStatus
            // 
            this.tabPageStatus.Location = new System.Drawing.Point(4, 22);
            this.tabPageStatus.Name = "tabPageStatus";
            this.tabPageStatus.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageStatus.Size = new System.Drawing.Size(902, 499);
            this.tabPageStatus.TabIndex = 0;
            this.tabPageStatus.Text = "Status";
            this.tabPageStatus.UseVisualStyleBackColor = true;
            // 
            // tabPageTest
            // 
            this.tabPageTest.Location = new System.Drawing.Point(4, 22);
            this.tabPageTest.Name = "tabPageTest";
            this.tabPageTest.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageTest.Size = new System.Drawing.Size(902, 499);
            this.tabPageTest.TabIndex = 1;
            this.tabPageTest.Text = "Test";
            this.tabPageTest.UseVisualStyleBackColor = true;
            // 
            // tabPageLogin
            // 
            this.tabPageLogin.Location = new System.Drawing.Point(4, 22);
            this.tabPageLogin.Name = "tabPageLogin";
            this.tabPageLogin.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageLogin.Size = new System.Drawing.Size(902, 499);
            this.tabPageLogin.TabIndex = 2;
            this.tabPageLogin.Text = "Login";
            this.tabPageLogin.UseVisualStyleBackColor = true;
            // 
            // tabPageManage
            // 
            this.tabPageManage.Location = new System.Drawing.Point(4, 22);
            this.tabPageManage.Name = "tabPageManage";
            this.tabPageManage.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageManage.Size = new System.Drawing.Size(902, 499);
            this.tabPageManage.TabIndex = 3;
            this.tabPageManage.Text = "Manage";
            this.tabPageManage.UseVisualStyleBackColor = true;
            // 
            // tabPageLog
            // 
            this.tabPageLog.Location = new System.Drawing.Point(4, 22);
            this.tabPageLog.Name = "tabPageLog";
            this.tabPageLog.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageLog.Size = new System.Drawing.Size(902, 499);
            this.tabPageLog.TabIndex = 4;
            this.tabPageLog.Text = "Log";
            this.tabPageLog.UseVisualStyleBackColor = true;
            // 
            // buttonConnect
            // 
            this.buttonConnect.Location = new System.Drawing.Point(839, 3);
            this.buttonConnect.Name = "buttonConnect";
            this.buttonConnect.Size = new System.Drawing.Size(75, 23);
            this.buttonConnect.TabIndex = 21;
            this.buttonConnect.Text = "Connect";
            this.buttonConnect.UseVisualStyleBackColor = true;
            this.buttonConnect.Click += new System.EventHandler(this.buttonConnect_Click);
            // 
            // comboBoxServList
            // 
            this.comboBoxServList.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxServList.FormattingEnabled = true;
            this.comboBoxServList.Location = new System.Drawing.Point(344, 4);
            this.comboBoxServList.Name = "comboBoxServList";
            this.comboBoxServList.Size = new System.Drawing.Size(201, 20);
            this.comboBoxServList.TabIndex = 22;
            this.comboBoxServList.SelectedIndexChanged += new System.EventHandler(this.comboBoxServList_SelectedIndexChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(722, 8);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(11, 12);
            this.label3.TabIndex = 20;
            this.label3.Text = ":";
            // 
            // textBoxPort
            // 
            this.textBoxPort.Location = new System.Drawing.Point(733, 4);
            this.textBoxPort.Name = "textBoxPort";
            this.textBoxPort.Size = new System.Drawing.Size(56, 21);
            this.textBoxPort.TabIndex = 19;
            this.textBoxPort.Text = "8055";
            this.textBoxPort.TextChanged += new System.EventHandler(this.textBoxAddr_TextChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(551, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(29, 12);
            this.label1.TabIndex = 18;
            this.label1.Text = "Addr";
            // 
            // textBoxAddr
            // 
            this.textBoxAddr.Location = new System.Drawing.Point(582, 4);
            this.textBoxAddr.Name = "textBoxAddr";
            this.textBoxAddr.Size = new System.Drawing.Size(137, 21);
            this.textBoxAddr.TabIndex = 16;
            this.textBoxAddr.Text = "127.0.0.1";
            this.textBoxAddr.TextChanged += new System.EventHandler(this.textBoxAddr_TextChanged);
            // 
            // checkBoxSSL
            // 
            this.checkBoxSSL.AutoSize = true;
            this.checkBoxSSL.Location = new System.Drawing.Point(795, 6);
            this.checkBoxSSL.Name = "checkBoxSSL";
            this.checkBoxSSL.Size = new System.Drawing.Size(42, 16);
            this.checkBoxSSL.TabIndex = 23;
            this.checkBoxSSL.Text = "SSL";
            this.checkBoxSSL.UseVisualStyleBackColor = true;
            // 
            // FormMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(926, 536);
            this.Controls.Add(this.checkBoxSSL);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.buttonConnect);
            this.Controls.Add(this.textBoxPort);
            this.Controls.Add(this.comboBoxServList);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.textBoxAddr);
            this.Controls.Add(this.tabControl);
            this.Name = "FormMain";
            this.Text = "Main";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.FormMain_FormClosed);
            this.tabControl.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl;
        private System.Windows.Forms.TabPage tabPageStatus;
        private System.Windows.Forms.TabPage tabPageTest;
        private System.Windows.Forms.Button buttonConnect;
        private System.Windows.Forms.ComboBox comboBoxServList;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textBoxPort;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox textBoxAddr;
        private System.Windows.Forms.TabPage tabPageLogin;
        private System.Windows.Forms.TabPage tabPageManage;
        private System.Windows.Forms.TabPage tabPageLog;
        private System.Windows.Forms.CheckBox checkBoxSSL;
    }
}

