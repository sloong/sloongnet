using Sloong.Interfaqce;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace servctrl
{
    class NetworkThread
    {
        IPageHost m_PageHost = null;
        Socket m_Socket = null;
        Thread m_WorkThread = null;
        ApplicationStatus AppStatus
        {
            get
            {
                return m_PageHost[ShareItem.AppStatus] as ApplicationStatus;
            }
        }
        public NetworkThread(IPageHost pageHost)
        {
            m_PageHost = pageHost;

            m_WorkThread = new Thread(() => WorkLoop());
            m_WorkThread.Name = "Network Thread";
        }

        public Log log
        {
            get
            {
                return m_PageHost[ShareItem.Log] as Log;
            }
        }

        public void Run()
        {
            m_WorkThread.Start();
        }

        public void Exit()
        {
            if (m_WorkThread.ThreadState == ThreadState.Running)
            {
                m_WorkThread.Abort();
            }
            if (m_Socket != null)
            {
                m_Socket.Close();
            }
        }

        public void ConnectToServer(ref Socket s, string ip)
        {
            if (!Utility.TestNetwork(AppStatus.ServerIP))
            {
                throw new Exception();
            }
            if (s == null || !s.Connected)
            {
                s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                IPEndPoint ipEndPoint = new IPEndPoint(IPAddress.Parse(AppStatus.ServerIP), AppStatus.SendProt);
                s.Connect(ipEndPoint);
                AppStatus.RecvBufferSize = s.ReceiveBufferSize;

                string connectMsg = "SloongWalls Client is connect.";
                m_PageHost.SendMessage(MessageType.SendPake, connectMsg);
                //m_hSocket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveTimeout, 5000); //设置接收数据超时                    
                //sRecvPicTemp.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendTimeout, 5000);//设置发送数据超时                    
                //sRecvPicTemp.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer, 1024);//设置发送缓冲区大小--1K大小                   
                //m_hSocket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer, bufferSize); //设置接收缓冲区大小
                //continue;
            }
        }

        public void WorkLoop()
        {
            while (AppStatus != null && AppStatus.RunStatus != RunStatus.Exit)
            {
                try
                {
                    ConnectToServer(ref m_Socket, AppStatus.ServerIP);
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.ToString());
                    System.Threading.Thread.Sleep(500);
                    continue;
                }
                try
                {
                    // Process send list
                    var sendMsg = m_PageHost[ShareItem.SendPack] as Queue<string>;

                    if (sendMsg != null && sendMsg.Count > 0)
                    {
                        var msg = sendMsg.Dequeue();

                        byte[] bmsg = Encoding.ASCII.GetBytes(msg);
                        var len = string.Format("{0:D8}", bmsg.Length);
                        msg = len + msg;
                        log.Write("Send Message:" + msg);
                        byte[] sendByte = Encoding.ASCII.GetBytes(msg);
                        lock (m_Socket)
                        {
                            m_Socket.Send(sendByte, sendByte.Length, 0);
                        }
                    }

                    // Receive message
                    byte[] header = new byte[24];
                    byte[] msgSize = new byte[4];

                    m_Socket.Receive(header, 24, 0);
                    string temp = (Encoding.Unicode.GetString(header));
                    if ("SloongWalls\0" != temp)
                    {
                        continue;
                    }
                    m_Socket.Receive(msgSize, 4, 0);
                    long packSize = BitConverter.ToInt64(msgSize, 0);
                    if (packSize <= 0)
                    {
                        continue;
                    }

                    byte[] recvPackBytes = new byte[packSize];
                    int nSize = 0;
                    Dictionary<byte[], int> list = new Dictionary<byte[], int>();
                    while (nSize < recvPackBytes.Length)
                    {
                        try
                        {
                            byte[] recvPic;
                            if (recvPackBytes.Length - nSize > AppStatus.RecvBufferSize)
                            {
                                recvPic = new byte[recvPackBytes.Length - nSize];
                            }
                            else
                            {
                                recvPic = new byte[AppStatus.RecvBufferSize];
                            }
                            int readsize = m_Socket.Receive(recvPic);
                            recvPic.CopyTo(recvPackBytes, nSize);
                            nSize += readsize;
                        }
                        catch (Exception e)
                        {
                            Console.WriteLine(e.ToString());
                            break;
                        }


                    }
                }
                catch (Exception ex)
                {
                    Console.Write(ex.Message);
                }
                finally
                {
                }
            }
        }
    }
}
