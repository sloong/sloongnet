namespace servctrl
{
    partial class FormTest
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
            this.label2 = new System.Windows.Forms.Label();
            this.buttonSend = new System.Windows.Forms.Button();
            this.textBoxMsg = new System.Windows.Forms.TextBox();
            this.listBoxLog = new System.Windows.Forms.ListBox();
            this.comboBoxTestCase = new System.Windows.Forms.ComboBox();
            this.label4 = new System.Windows.Forms.Label();
            this.checkBoxSingle = new System.Windows.Forms.CheckBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label3 = new System.Windows.Forms.Label();
            this.textBoxNumInterval = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.textBoxMulNum = new System.Windows.Forms.TextBox();
            this.buttonSave = new System.Windows.Forms.Button();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.checkBoxEnableSwift = new System.Windows.Forms.CheckBox();
            this.checkBoxEnableMD5 = new System.Windows.Forms.CheckBox();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.buttonUpload = new System.Windows.Forms.Button();
            this.buttonUploadTCP = new System.Windows.Forms.Button();
            this.label7 = new System.Windows.Forms.Label();
            this.textBoxLevel = new System.Windows.Forms.TextBox();
            this.checkBoxEnableZip = new System.Windows.Forms.CheckBox();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(52, 68);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(53, 12);
            this.label2.TabIndex = 13;
            this.label2.Text = "消息内容";
            // 
            // buttonSend
            // 
            this.buttonSend.Location = new System.Drawing.Point(592, 63);
            this.buttonSend.Name = "buttonSend";
            this.buttonSend.Size = new System.Drawing.Size(75, 23);
            this.buttonSend.TabIndex = 12;
            this.buttonSend.Text = "Send";
            this.buttonSend.UseVisualStyleBackColor = true;
            this.buttonSend.Click += new System.EventHandler(this.buttonSend_Click);
            // 
            // textBoxMsg
            // 
            this.textBoxMsg.Location = new System.Drawing.Point(127, 65);
            this.textBoxMsg.Name = "textBoxMsg";
            this.textBoxMsg.Size = new System.Drawing.Size(437, 21);
            this.textBoxMsg.TabIndex = 11;
            // 
            // listBoxLog
            // 
            this.listBoxLog.FormattingEnabled = true;
            this.listBoxLog.HorizontalScrollbar = true;
            this.listBoxLog.ItemHeight = 12;
            this.listBoxLog.Location = new System.Drawing.Point(54, 167);
            this.listBoxLog.Name = "listBoxLog";
            this.listBoxLog.Size = new System.Drawing.Size(715, 304);
            this.listBoxLog.TabIndex = 16;
            this.listBoxLog.SelectedIndexChanged += new System.EventHandler(this.listBoxLog_SelectedIndexChanged);
            // 
            // comboBoxTestCase
            // 
            this.comboBoxTestCase.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBoxTestCase.FormattingEnabled = true;
            this.comboBoxTestCase.Location = new System.Drawing.Point(127, 21);
            this.comboBoxTestCase.Name = "comboBoxTestCase";
            this.comboBoxTestCase.Size = new System.Drawing.Size(437, 20);
            this.comboBoxTestCase.TabIndex = 17;
            this.comboBoxTestCase.SelectedIndexChanged += new System.EventHandler(this.comboBoxTestCase_SelectedIndexChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(52, 24);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(53, 12);
            this.label4.TabIndex = 18;
            this.label4.Text = "测试用例";
            // 
            // checkBoxSingle
            // 
            this.checkBoxSingle.AutoSize = true;
            this.checkBoxSingle.Checked = true;
            this.checkBoxSingle.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxSingle.Location = new System.Drawing.Point(822, 11);
            this.checkBoxSingle.Name = "checkBoxSingle";
            this.checkBoxSingle.Size = new System.Drawing.Size(48, 16);
            this.checkBoxSingle.TabIndex = 21;
            this.checkBoxSingle.Text = "单步";
            this.checkBoxSingle.UseVisualStyleBackColor = true;
            this.checkBoxSingle.CheckedChanged += new System.EventHandler(this.checkBox1_CheckedChanged);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.textBoxNumInterval);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.textBoxMulNum);
            this.groupBox1.Location = new System.Drawing.Point(688, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(200, 93);
            this.groupBox1.TabIndex = 22;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "并发测试";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(21, 56);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(53, 12);
            this.label3.TabIndex = 26;
            this.label3.Text = "并发间隔";
            // 
            // textBoxNumInterval
            // 
            this.textBoxNumInterval.Location = new System.Drawing.Point(96, 53);
            this.textBoxNumInterval.Name = "textBoxNumInterval";
            this.textBoxNumInterval.Size = new System.Drawing.Size(77, 21);
            this.textBoxNumInterval.TabIndex = 25;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(21, 29);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(53, 12);
            this.label1.TabIndex = 24;
            this.label1.Text = "并发次数";
            // 
            // textBoxMulNum
            // 
            this.textBoxMulNum.Location = new System.Drawing.Point(96, 26);
            this.textBoxMulNum.Name = "textBoxMulNum";
            this.textBoxMulNum.Size = new System.Drawing.Size(77, 21);
            this.textBoxMulNum.TabIndex = 23;
            // 
            // buttonSave
            // 
            this.buttonSave.Location = new System.Drawing.Point(592, 21);
            this.buttonSave.Name = "buttonSave";
            this.buttonSave.Size = new System.Drawing.Size(75, 23);
            this.buttonSave.TabIndex = 23;
            this.buttonSave.Text = "Save";
            this.buttonSave.UseVisualStyleBackColor = true;
            this.buttonSave.Click += new System.EventHandler(this.button1_Click);
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(127, 118);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(77, 21);
            this.textBox1.TabIndex = 27;
            this.textBox1.Text = "1";
            // 
            // checkBoxEnableSwift
            // 
            this.checkBoxEnableSwift.AutoSize = true;
            this.checkBoxEnableSwift.Checked = true;
            this.checkBoxEnableSwift.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxEnableSwift.Location = new System.Drawing.Point(592, 123);
            this.checkBoxEnableSwift.Name = "checkBoxEnableSwift";
            this.checkBoxEnableSwift.Size = new System.Drawing.Size(84, 16);
            this.checkBoxEnableSwift.TabIndex = 29;
            this.checkBoxEnableSwift.Text = "启用流水号";
            this.checkBoxEnableSwift.UseVisualStyleBackColor = true;
            this.checkBoxEnableSwift.CheckedChanged += new System.EventHandler(this.checkBoxEnableSwift_CheckedChanged);
            // 
            // checkBoxEnableMD5
            // 
            this.checkBoxEnableMD5.AutoSize = true;
            this.checkBoxEnableMD5.Checked = true;
            this.checkBoxEnableMD5.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxEnableMD5.Location = new System.Drawing.Point(592, 101);
            this.checkBoxEnableMD5.Name = "checkBoxEnableMD5";
            this.checkBoxEnableMD5.Size = new System.Drawing.Size(66, 16);
            this.checkBoxEnableMD5.TabIndex = 30;
            this.checkBoxEnableMD5.Text = "启用MD5";
            this.checkBoxEnableMD5.UseVisualStyleBackColor = true;
            this.checkBoxEnableMD5.CheckedChanged += new System.EventHandler(this.checkBoxEnableMD5_CheckedChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(64, 121);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(41, 12);
            this.label5.TabIndex = 31;
            this.label5.Text = "流水号";
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(589, 142);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(281, 12);
            this.label6.TabIndex = 32;
            this.label6.Text = "更改MD5和流水号选项前,请确认所有消息以收发完毕";
            // 
            // buttonUpload
            // 
            this.buttonUpload.Location = new System.Drawing.Point(786, 178);
            this.buttonUpload.Name = "buttonUpload";
            this.buttonUpload.Size = new System.Drawing.Size(91, 23);
            this.buttonUpload.TabIndex = 33;
            this.buttonUpload.Text = "Upload";
            this.buttonUpload.UseVisualStyleBackColor = true;
            this.buttonUpload.Click += new System.EventHandler(this.buttonUpload_Click);
            // 
            // buttonUploadTCP
            // 
            this.buttonUploadTCP.Location = new System.Drawing.Point(784, 222);
            this.buttonUploadTCP.Name = "buttonUploadTCP";
            this.buttonUploadTCP.Size = new System.Drawing.Size(93, 23);
            this.buttonUploadTCP.TabIndex = 34;
            this.buttonUploadTCP.Text = "UploadWithTCP";
            this.buttonUploadTCP.UseVisualStyleBackColor = true;
            this.buttonUploadTCP.Click += new System.EventHandler(this.buttonUploadTCP_Click);
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(328, 121);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(41, 12);
            this.label7.TabIndex = 36;
            this.label7.Text = "优先级";
            // 
            // textBoxLevel
            // 
            this.textBoxLevel.Location = new System.Drawing.Point(391, 118);
            this.textBoxLevel.Name = "textBoxLevel";
            this.textBoxLevel.Size = new System.Drawing.Size(77, 21);
            this.textBoxLevel.TabIndex = 35;
            this.textBoxLevel.Text = "1";
            // 
            // checkBoxEnableZip
            // 
            this.checkBoxEnableZip.AutoSize = true;
            this.checkBoxEnableZip.Checked = true;
            this.checkBoxEnableZip.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBoxEnableZip.Location = new System.Drawing.Point(688, 120);
            this.checkBoxEnableZip.Name = "checkBoxEnableZip";
            this.checkBoxEnableZip.Size = new System.Drawing.Size(66, 16);
            this.checkBoxEnableZip.TabIndex = 37;
            this.checkBoxEnableZip.Text = "启用Zip";
            this.checkBoxEnableZip.UseVisualStyleBackColor = true;
            // 
            // FormTest
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(900, 500);
            this.Controls.Add(this.checkBoxEnableZip);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.textBoxLevel);
            this.Controls.Add(this.buttonUploadTCP);
            this.Controls.Add(this.buttonUpload);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.checkBoxEnableMD5);
            this.Controls.Add(this.checkBoxEnableSwift);
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.buttonSave);
            this.Controls.Add(this.checkBoxSingle);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.comboBoxTestCase);
            this.Controls.Add(this.listBoxLog);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.buttonSend);
            this.Controls.Add(this.textBoxMsg);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "FormTest";
            this.ShowInTaskbar = false;
            this.Text = "FormTest";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.FormTest_FormClosed);
            this.Shown += new System.EventHandler(this.FormTest_Shown);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button buttonSend;
        private System.Windows.Forms.TextBox textBoxMsg;
        private System.Windows.Forms.ListBox listBoxLog;
        private System.Windows.Forms.ComboBox comboBoxTestCase;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.CheckBox checkBoxSingle;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox textBoxNumInterval;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox textBoxMulNum;
        private System.Windows.Forms.Button buttonSave;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.CheckBox checkBoxEnableSwift;
        private System.Windows.Forms.CheckBox checkBoxEnableMD5;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Button buttonUpload;
        private System.Windows.Forms.Button buttonUploadTCP;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.TextBox textBoxLevel;
        private System.Windows.Forms.CheckBox checkBoxEnableZip;
    }
}