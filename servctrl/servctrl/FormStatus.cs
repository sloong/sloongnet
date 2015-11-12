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
    public partial class FormStatus : Form
    {
        Socket m_Socket;
        public FormStatus( ref Socket socket)
        {
            InitializeComponent();
            m_Socket = socket;
        }

       

        private void listBoxLog_SelectedIndexChanged(object sender, EventArgs e)
        {

        }
    }
}
