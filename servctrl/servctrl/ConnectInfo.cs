using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Security;
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
        public TcpClient m_Client;
        public int m_nPort;
        public string m_URL;
        public ProtocolType m_Type = ProtocolType.Tcp;
        public SslStream m_SSL;
    }
}
