using Newtonsoft.Json.Linq;
using servctrl.UI;
using Sloong;
using Sloong.Interface;
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
    public partial class FormMain : Form
    {
        private DataCenter share { get; set; }

        IDataCenter pageHost
        {
            get
            {
                return share as IDataCenter;
            }

            set { }
        }
        private List<ConnectInfo> sockMap { get; set; }

        private ApplicationStatus appStatus { get; set; }
        // UI defines
        private FormStatus formStatus = null;
        private FormTest formTest = null;
        FormLogin formLogin = null;
        FormManage formManage = null;
        private Log log = null;
        private NetworkThread _Nt = null;
        private Queue<MessagePackage> _SendList = null;
        Dictionary<long, MessagePackage> _MessageList = null;
        int _SwiftNum = 0;
        int nCurrentSocketIndex = 0;

        public List<ConnectInfo> SocketMap
        {
            get
            {
                return pageHost[ShareItem.SocketMap] as List<ConnectInfo>;
            }
            set { }
        }

        Queue<MessagePackage> SendList
        {
            get
            {
                return pageHost[ShareItem.SendList] as Queue<MessagePackage>;
            }
        }
        ComboBox ServList
        {
            get
            {
                var cbox = pageHost[ShareItem.ConfigSelecter] as ComboBox;
                return cbox;
            }

            set { }
        }

        const string sockMapPath = "SockMap.bin";

        public FormMain()
        {
            InitializeComponent();

            // Init log
            log = new Log();

            InitMemberVariable();
            InitPageHost();
            InitFormStatus();
            _Nt.Run();
            Control.CheckForIllegalCrossThreadCalls = false;
        }

        private void InitMemberVariable()
        {
            this.share = new DataCenter();
            appStatus = new ApplicationStatus();
            _Nt = new NetworkThread(this.share);
            _SendList = new Queue<MessagePackage>();
            _MessageList = new Dictionary<long, MessagePackage>();
            appStatus.ExitApp = false;
            appStatus.RunStatus = RunStatus.Run;

            try
            {
                sockMap = (List<ConnectInfo>)Utility.Deserialize(sockMapPath);
                foreach (var item in sockMap)
                {
                    try
                    {
                        var info = ConnectToServer(item.m_URL, item.m_IPInfo.Port);
                        item.m_IPInfo = info.m_IPInfo;
                        item.m_Socket = info.m_Socket;
                        if (!comboBoxServList.Items.Contains(item.m_URL))
                            comboBoxServList.Items.Add(item.m_URL);
                    }
                    catch (Exception e)
                    {
                        log.Write(e.ToString());
                    }
                }
            }
            catch (Exception e)
            {
                log.Write(e.ToString());
                sockMap = new List<ConnectInfo>();
            }

            formStatus = new FormStatus(share);
            formTest = new FormTest(share);
            formLogin = new FormLogin(share);
            formManage = new FormManage(share);

            tabPageStatus.Controls.Add(formStatus);
            formStatus.Show();
            tabPageTest.Controls.Add(formTest);
            formTest.Show();
            tabPageLogin.Controls.Add(formLogin);
            formLogin.Show();
            tabPageManage.Controls.Add(formManage);
            formManage.Show();
        }

        private void InitFormStatus()
        {
            if (ServList.Items.Count > 0)
                ServList.SelectedIndex = 0;
        }

        private void InitPageHost()
        {
            this.share.PageMessage += PageMessageHandler;

            pageHost.Add(ShareItem.AppStatus, appStatus);
            this.share.Add(ShareItem.SocketMap, sockMap);
            this.share.Add(ShareItem.ConfigSelecter, comboBoxServList);
            share.Add(ShareItem.SendList, _SendList);
            share.Add(ShareItem.MessageList, _MessageList);
        }

        private ConnectInfo ConnectToServer(string url, int port)
        {
            if (string.IsNullOrEmpty(url))
                throw new Exception("url is empty.");

            // before connect, ping test.
            var ip = Dns.GetHostAddresses(url)[0].ToString();
            if (false == Utility.TestNetwork(ip))
                throw new Exception(string.Format("ping to {0} fialed.", ip));

            Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            IPEndPoint ipEndPoint = new IPEndPoint(IPAddress.Parse(ip), port);

            try
            {
                sock.Connect(ipEndPoint);

                ConnectInfo info = new ConnectInfo();
                info.m_Socket = sock;
                info.m_IPInfo = ipEndPoint;
                info.m_URL = url;

                return info;
            }
            catch (Exception ex)
            {
                throw new Exception(string.Format("connect to {1} fialed. error message:", textBoxAddr.Text, ex.Message));
            }
        }

        private void buttonConnect_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(textBoxAddr.Text))
                return;

            try
            {
                foreach (var item in SocketMap)
                {
                    if (item.m_URL == textBoxAddr.Text)
                    {
                        if (item.m_Socket.Connected)
                        {
                            item.m_Socket.Disconnect(true);
                        }
                        else
                        {
                            item.m_Socket.Connect(item.m_IPInfo);
                        }
                        UpdateButtonText();
                        return;
                    }
                }

                // no found
                var info = ConnectToServer(textBoxAddr.Text, Convert.ToInt32(textBoxPort.Text));
                if (info.m_Socket.Connected)
                {
                    SocketMap.Add(info);
                    comboBoxServList.SelectedIndex = comboBoxServList.Items.Add(textBoxAddr.Text);
                }
                UpdateButtonText();
            }
            catch (Exception ex)
            {
                MessageBox.Show(string.Format("connect to {1} fialed. error message:", textBoxAddr.Text, ex.Message));
            }

        }

        void PageMessageHandler(object sender, PageMessage e)
        {
            var type = e.Type;
            switch (type)
            {
                case MessageType.ExitApp:
                    break;
                case MessageType.SendRequest:
                    AddMessageRequest(e.Params, false, e.CallBackFunc);
                    break;
                case MessageType.SendRequestWithData:
                    AddMessageRequest(e.Params, true, e.CallBackFunc);
                    break;
            }
        }

        private void comboBoxServList_SelectedIndexChanged(object sender, EventArgs e)
        {
            foreach (var item in SocketMap)
            {
                if (item.m_URL == comboBoxServList.Text)
                {
                    textBoxAddr.Text = item.m_URL;
                    textBoxPort.Text = item.m_IPInfo.Port.ToString();
                    UpdateButtonText();

                    nCurrentSocketIndex = SocketMap.IndexOf(item);
                }
            }
        }

        private void FormMain_FormClosed(object sender, FormClosedEventArgs e)
        {
            foreach (var item in SocketMap)
            {
                if (item.m_Socket != null && item.m_Socket.Connected)
                    item.m_Socket.Close();
                item.m_Socket = null;
            }
            appStatus.RunStatus = RunStatus.Exit;
            appStatus.ExitApp = true;
            _Nt.Exit();
            Utility.Serialize(SocketMap, sockMapPath);
            log.Dispose();
        }

        void AddMessageRequest(object[] Params, bool bExData, Func<MessagePackage, bool> pCallBack)
        {
            try
            {
                JObject msg = Params[0] as JObject;
                if (msg == null)
                    return;
                MessagePackage pack = new MessagePackage();
                pack.SendMessage = msg.ToString();
                pack.SwiftNumber = _SwiftNum;
                pack.NeedExData = bExData;
                pack.MessageExInfo = Params;
                if (pCallBack != null)
                    pack.ReceivedHandler = new CallBackFunc(pCallBack);
                SendList.Enqueue(pack);
                _SwiftNum++;

            }
            catch (Exception e)
            {
                log.Write(e.ToString());
            }
        }

        void UpdateButtonText()
        {
            foreach (var item in SocketMap)
            {
                if (item.m_URL == textBoxAddr.Text && item.m_IPInfo.Port.ToString() == textBoxPort.Text)
                {
                    if( item.m_Socket.Connected )
                    {
                        buttonConnect.Text = "Disconnect";
                        return;
                    }
                }
            }

            buttonConnect.Text = "Connect";
        }

        private void textBoxAddr_TextChanged(object sender, EventArgs e)
        {
            UpdateButtonText();
        }
    }

}
