using Sloong;
using Sloong.Interfaqce;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace servctrl
{
    public partial class FormTest : Form
    {
        private IPageHost pageHost;
        private Dictionary<string, string> testCaseMap = new Dictionary<string,string>();



        public Dictionary<string, ConnectInfo> SocketMap
        {
            get
            {
                return pageHost[ShareItem.SocketMap] as Dictionary<string, ConnectInfo>;
            }
            set { }
        }


        Socket sockCurrent
        {
            get
            {
                var cbox = pageHost[ShareItem.ConfigSelecter] as ComboBox;
                if (cbox != null)
                {
                    return SocketMap[cbox.Text].m_Socket;
                }
                return null;
            }

            set { }
        }

        public FormTest( IPageHost _pageHost )
        {
            InitializeComponent();
            this.TopLevel = false;
            pageHost = _pageHost;

            pageHost.PageMessage += PageMessageHandler;
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
            try
            {
                Socket sock = sockCurrent;
                var msg = textBoxMsg.Text;
                byte[] bmsg = Encoding.ASCII.GetBytes(msg);
                var len = string.Format("{0:D8}", bmsg.Length);
                msg = len + msg;
                listBoxLog.Items.Add(msg);
                byte[] sendByte = Encoding.ASCII.GetBytes(msg);
                lock (sockCurrent)
                {
                    sockCurrent.Send(sendByte, sendByte.Length, 0);
                }

                // send end, rece the return.
                byte[] leng = new byte[8];
                sockCurrent.Receive(leng, leng.Length, 0);
                var nlen = BitConverter.ToInt64(leng, 0);
                byte[] data = new byte[nlen];
                sockCurrent.Receive(data, data.Length, 0);
                listBoxLog.Items.Add(Encoding.ASCII.GetString(data));
            }
            catch (Exception ex)
            {
                listBoxLog.Items.Add(ex.ToString());
            }
        }

        private void listBoxLog_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            groupBox1.Enabled = !checkBox1.Checked;
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

        }

        private void comboBoxTestCase_SelectedIndexChanged(object sender, EventArgs e)
        {
            textBoxMsg.Text = testCaseMap[comboBoxTestCase.Text];
        }
    }
}
