using Sloong;
using Sloong.Interfaqce;
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
        private IPageHost pageHost;
        private Dictionary<string, string> testCaseMap = null;
        const string TestCastMapPath = "UI\\TestCase.bin";
        static int nSwift = 0;


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

            // read the cache
            try
            {
                testCaseMap = (Dictionary<string, string>)Utility.Deserialize(TestCastMapPath);
            }
            catch (Exception)
            {
                testCaseMap = new Dictionary<string, string>();
            }
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
                nSwift++;
                sockCurrent.ReceiveTimeout = 5000;
                // real message
                var msg = textBoxMsg.Text;
                var msgs = msg.Split('|');
                string md5 = Utility.MD5_Encoding(msg,Encoding.UTF8);

                msg = string.Format("{0}|{1}|{2}", md5, nSwift, msg);
                listBoxLog.Items.Add(msg);
                var len = string.Format("{0:D8}", msg.Length);
                msg = len + msg;
                
                byte[] sendByte = Encoding.ASCII.GetBytes(msg);
                lock (sockCurrent)
                {
                    Utility.SendEx(sockCurrent,sendByte );
                }

                // send end, rece the return.
                byte[] leng = Utility.RecvEx(sockCurrent, 8);
                var nlen = BitConverter.ToInt64(leng, 0);
                byte[] data = Utility.RecvEx(sockCurrent,nlen);
                // check the return.
                string strres = Encoding.ASCII.GetString(data);
                var ress=strres.Split('|');
                string log = strres;
                if (ress[2] != "0")
                {
                    log = ress[3];
                }
                listBoxLog.SelectedIndex = listBoxLog.Items.Add(log);
                if(msgs[0] == "70001")
                {
                    int nPicSize = Convert.ToInt32(ress[4]);

                    byte[] img = Utility.RecvEx(sockCurrent, nPicSize);

                    string filename = "receivePic";

                    if (!Directory.Exists("D:\\images\\"))
                        Directory.CreateDirectory("D:\\images\\");

                    FileStream fs = new FileStream("D:\\images\\" + filename  + ".jpg", FileMode.OpenOrCreate, FileAccess.ReadWrite);

                    fs.Write(img, 0, nPicSize);
                    fs.Dispose();
                    fs.Close();
                    
                }
            }
            catch (Exception ex)
            {
                listBoxLog.SelectedIndex = listBoxLog.Items.Add(ex.ToString());
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

        private void FormTest_FormClosed(object sender, FormClosedEventArgs e)
        {
            Utility.Serialize(testCaseMap, TestCastMapPath);
        }
    }
}
