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
        private IDataCenter pageHost;
        private Dictionary<string, string> testCaseMap = new Dictionary<string, string>();
        const string TestCastMapPath = "UI\\TestCase.bin";

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

        public FormTest(IDataCenter _pageHost)
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
                    pageHost.SendMessage(MessageType.SendRequest, pam, ProcResult);
                    listBoxLog.Items.Add(textBoxMsg.Text);
                }

                //                     sockCurrent.ReceiveTimeout = 5000;
                //                                     // real message
                //                                     var msg = textBoxMsg.Text;
                //                                     var msgs = msg.Split('|');
                //                                     
                //                                     {
                //                                         nSwift++;
                //                                         
                //                                         string md5 = Utility.MD5_Encoding(msg, Encoding.UTF8);
                //                 
                //                                         var sendmsg = string.Format("{0}|{1}|{2}", md5, nSwift, msg);
                //                                         listBoxLog.Items.Add(sendmsg);
                //                                         var len = string.Format("{0:D8}", sendmsg.Length);
                //                                         sendmsg = len + sendmsg;
                //                 
                //                                         byte[] sendByte = Encoding.ASCII.GetBytes(sendmsg);
                //                                         lock (sockCurrent)
                //                                         {
                //                 
                //                                             Utility.SendEx(sockCurrent, sendByte);
                //                                         }
                //                                         if (nMul > 1)
                //                                         {
                //                                             int nInt;
                //                                             try
                //                                             {
                //                                                 nInt = Convert.ToInt32(textBoxNumInterval.Text);
                //                                             }
                //                                             catch(Exception)
                //                                             {
                //                                                 nInt = 0;
                //                                             }
                //                                             if( nInt > 0 )
                //                                                 System.Threading.Thread.Sleep(nInt);
                //                                         }
                //                                             
                //                                     }
                //                                     
                //                 
                //                                       for (int i = 0; i < nMul; i++)
                //                                       {
                //                                          // send end, rece the return.
                //                                          byte[] leng = Utility.RecvEx(sockCurrent, 8,0);
                //                                          long nlen = 0;
                //                                          byte[] data = null;
                //                                          try
                //                                          {
                //                                              nlen = BitConverter.ToInt32(leng, 0);
                //                                              data  = Utility.RecvEx(sockCurrent, nlen,0);
                //                                          }
                //                                          catch(Exception ex)
                //                                          {
                //                                              listBoxLog.SelectedIndex = listBoxLog.Items.Add(ex.ToString());
                //                                          }
                //                                         
                //                                          
                //                                          // check the return.
                //                                          string strres = Encoding.UTF8.GetString(data);
                //                                          var ress = strres.Split('|');
                //                                          string log = strres;
                //                                          try
                //                                          {
                //                                              if (ress[2] != "0")
                //                                              {
                //                                                  listBoxLog.SelectedIndex = listBoxLog.Items.Add("Fialed! The Error Message is:" + ress[3]);
                //                                                  return;
                //                                              }
                //                 
                //                                          }
                //                                           catch(Exception)
                //                                          {
                //                 
                //                                          }
                //                                          listBoxLog.SelectedIndex = listBoxLog.Items.Add(log);
                //                                          if (msgs[0] == "70001")
                //                                          {
                //                                              int nPicSize = Convert.ToInt32(ress[4]);
                //                 
                //                                              byte[] img = Utility.RecvEx(sockCurrent, nPicSize,0);
                //                 
                //                                              string filename = "receivePic";
                //                 
                //                                              if (!Directory.Exists("D:\\images\\"))
                //                                                  Directory.CreateDirectory("D:\\images\\");
                //                 
                //                                              FileStream fs = new FileStream("D:\\images\\" + filename + ".jpg", FileMode.OpenOrCreate, FileAccess.ReadWrite);
                //                 
                //                                              fs.Write(img, 0, nPicSize);
                //                                              fs.Dispose();
                //                                              fs.Close();
                //                 
                //                                          }
                //                                      }
                //                                    
            }
            catch (Exception ex)
            {
                listBoxLog.SelectedIndex = listBoxLog.Items.Add(ex.ToString());
            }
        }

        public bool ProcResult(object param)
        {
            MessagePackage pack = param as MessagePackage;
            var msgs = pack.ReceivedMessages;
            if (msgs[2] != "0")
            {
                listBoxLog.SelectedIndex = listBoxLog.Items.Add("Fialed! The Error Message is:" + msgs[3]);
                return false;
            }
            else
            {
                listBoxLog.SelectedIndex = listBoxLog.Items.Add(pack.SwiftNumber.ToString()+"|"+msgs[3]);
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
    }
}
