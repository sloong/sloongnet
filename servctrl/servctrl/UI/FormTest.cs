using Newtonsoft.Json.Linq;
using Sloong;
using Sloong.Interface;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace servctrl
{
    public partial class FormTest : Form
    {
        private IDataCenter _DC;
        private Dictionary<string, string> testCaseMap = new Dictionary<string, string>();
        const string TestCastMapPath = "UI\\TestCase.bin";

        public Dictionary<string, ConnectInfo> SocketMap
        {
            get
            {
                return _DC[ShareItem.SocketMap] as Dictionary<string, ConnectInfo>;
            }
            set { }
        }

        string[] DefTestCastMapKay =
        {
            "Reload",
            "GetText",
            "RunSql",
            "GetUInfo",
            "SetUInfo",
        };

        string[] DefTestCastMapValue =
        {
            "funcid=Reload",
            "funcid=GetText",
            "funcid=RunSql|cmd=Test",
            "funcid=GetUInfo|key=Test",
            "funcid=SetUInfo|key=Test|value=TestV",
        };


        Socket sockCurrent
        {
            get
            {
                var cbox = _DC[ShareItem.ConfigSelecter] as ComboBox;
                if (cbox != null)
                {
                    return SocketMap[cbox.Text].m_Socket;
                }
                return null;
            }

            set { }
        }

        public FormTest(IDataCenter _pageHost)
        {
            InitializeComponent();
            Control.CheckForIllegalCrossThreadCalls = false;
            this.TopLevel = false;
            _DC = _pageHost;

            _DC.PageMessage += PageMessageHandler;

            try
            {
                testCaseMap = (Dictionary<string, string>)Utility.Deserialize(TestCastMapPath);
            }
            catch(Exception)
            {
                for( int i = 0; i< DefTestCastMapKay.Length; i++ )
                {
                    testCaseMap.Add(DefTestCastMapKay[i], DefTestCastMapValue[i]);
                }
            }
            foreach (var item in testCaseMap)
            {
                comboBoxTestCase.Items.Add(item.Key);
            }
            comboBoxTestCase.SelectedIndex = 0;
        }

        void PageMessageHandler(object sender, PageMessage e)
        {
            var type = e.Type;
            switch (type)
            {
                case MessageType.ExitApp:
                    break;
            }
        }

        private void buttonSend_Click(object sender, EventArgs e)
        {
            JObject pam = JObject.Parse("{}");
            var AllMsg = textBoxMsg.Text.Split('|');
            foreach( var item in AllMsg )
            {
                var sitem = item.Split('=');
                pam[sitem[0]] = sitem[1];
            }

            int nMul = 0;
            if (checkBoxSingle.Checked)
                nMul = 1;
            else
                nMul = Convert.ToInt32(textBoxMulNum.Text);

            try
            {
                for (int i = 0; i < nMul; i++)
                {
                    _DC.SendMessage(MessageType.SendRequest, pam, ProcResult);
                    listBoxLog.Items.Add(textBoxMsg.Text);
                }                    
            }
            catch (Exception ex)
            {
                listBoxLog.SelectedIndex = listBoxLog.Items.Add(ex.ToString());
            }
        }

        public bool ProcResult(object param)
        {
            MessagePackage pack = param as MessagePackage;
            JObject jres = JObject.Parse(pack.ReceivedMessages);
            if (jres["errno"].ToString() != "0")
            {
                listBoxLog.SelectedIndex = listBoxLog.Items.Add("Fialed! The Error Message is:" + jres["errmsg"].ToString());
                return false;
            }
            else
            {
                listBoxLog.SelectedIndex = listBoxLog.Items.Add(pack.SwiftNumber.ToString()+"|"+jres.ToString());
            }
            return true;
        }

        private void listBoxLog_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            groupBox1.Enabled = !checkBoxSingle.Checked;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            AskForm askform = new AskForm();
            askform.ShowDialog();
            testCaseMap[askform.InputText] = textBoxMsg.Text;
            comboBoxTestCase.SelectedIndex = comboBoxTestCase.Items.Add(askform.InputText);
        }

        private void FormTest_Shown(object sender, EventArgs e)
        {
            checkBoxEnableMD5.Checked = (_DC[ShareItem.AppStatus] as ApplicationStatus).bEnableMD5;
            checkBoxEnableSwift.Checked = (_DC[ShareItem.AppStatus] as ApplicationStatus).bEnableSwift;
        }

        private void comboBoxTestCase_SelectedIndexChanged(object sender, EventArgs e)
        {
            try
            {
                textBoxMsg.Text = testCaseMap[comboBoxTestCase.Text];
            }
            catch (Exception)
            {

            }
        }

        private void FormTest_FormClosed(object sender, FormClosedEventArgs e)
        {
            Utility.Serialize(testCaseMap, TestCastMapPath);
        }

        private void checkBoxEnableMD5_CheckedChanged(object sender, EventArgs e)
        {
            (_DC[ShareItem.AppStatus] as ApplicationStatus).bEnableMD5 = checkBoxEnableMD5.Checked;
        }

        private void checkBoxEnableSwift_CheckedChanged(object sender, EventArgs e)
        {
            (_DC[ShareItem.AppStatus] as ApplicationStatus).bEnableSwift = checkBoxEnableSwift.Checked;
        }
    }
}
