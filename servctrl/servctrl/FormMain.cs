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
        Socket m_Socket;
        public FormMain()
        {
            InitializeComponent();
            var fstatus = new FormStatus(ref m_Socket);
            var ftest = new FormTest(ref m_Socket);
            fstatus.TopLevel = ftest.TopLevel = false;
            tabPageStatus.Controls.Add(fstatus);
            tabPageTest.Controls.Add(ftest);
            fstatus.Show();
            ftest.Show();
        }

        private void buttonConnect_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(textBoxAddr.Text))
                MessageBox.Show("No input address");

            m_Socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            IPEndPoint ipEndPoint = new IPEndPoint(IPAddress.Parse(textBoxAddr.Text), Convert.ToInt32(textBoxPort.Text));
            try
            {
                m_Socket.Connect(ipEndPoint);
            }
            catch (Exception ex)
            {

            }
        }
    }
}
