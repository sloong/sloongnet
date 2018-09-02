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
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace servctrl.UI
{
    public partial class FormLog : Form
    {
        IDataCenter _DC = null;
        Socket logSocket = null;
        Encoding gbk = Encoding.GetEncoding("GB2312");

        ApplicationStatus AppStatus
        {
            get
            {
                return _DC[ShareItem.AppStatus] as ApplicationStatus;
            }
        }

        public FormLog(IDataCenter dc)
        {
            InitializeComponent();
            this.TopLevel = false;
            _DC = dc;
        }

        private void buttonConnect_Click(object sender, EventArgs e)
        {
            try
            {
                logSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                IPEndPoint ipEndPoint = new IPEndPoint(IPAddress.Parse(textBoxIp.Text), Convert.ToInt32(textBoxPort.Text));
                logSocket.Connect(ipEndPoint);
                var thread = new Thread(() => ReceiveLogWorkLoop());
                thread.Start();
            }
            catch(Exception)
            {

            }
        }

        void ReceiveLogWorkLoop()
        {
            while (AppStatus != null && AppStatus.RunStatus != RunStatus.Exit)
            {
                long nLength = Utility.RecvDataLength(logSocket, 10000);
                byte[] data = Utility.RecvEx(logSocket, nLength, 10000);
                listBoxLog.SelectedIndex = listBoxLog.Items.Add(gbk.GetString(data));
            }
        }
    }
}
