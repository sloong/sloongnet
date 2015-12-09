using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;

namespace servctrl
{
    [Serializable]
    public class ConnectInfo
    {
        [NonSerialized]
        public Socket m_Socket;
        public IPEndPoint m_IPInfo;
        public string m_URL;
    }
}
