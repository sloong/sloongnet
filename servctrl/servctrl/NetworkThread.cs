using servctrl;
using Sloong;
using Sloong.Interface;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Sloong
{
    class NetworkThread
    {
        IDataCenter _DC = null;
        Thread m_SendWorkThread = null;
        int m_nCurrentSocket = 0;
        Dictionary<int, Thread> m_RecvThreadList = null;

        ApplicationStatus AppStatus
        {
            get
            {
                return _DC[ShareItem.AppStatus] as ApplicationStatus;
            }
        }

        Dictionary<long, MessagePackage> MsgList
        {
            get
            {
                return _DC[ShareItem.MessageList] as Dictionary<long, MessagePackage>;
            }
        }

        Queue<MessagePackage> SendList
        {
            get
            {
                return _DC[ShareItem.SendList] as Queue<MessagePackage>;
            }
        }

        List<ConnectInfo> SocketMap
        {
            get
            {
                return _DC[ShareItem.SocketMap] as List<ConnectInfo>;
            }
        }


        public NetworkThread(IDataCenter dc)
        {
            _DC = dc;

            m_SendWorkThread = new Thread(() => SendWorkLoop());
            m_SendWorkThread.Name = "Network Send Thread";

            m_RecvThreadList = new Dictionary<int, Thread>();
        }

        public Log log
        {
            get
            {
                return _DC[ShareItem.Log] as Log;
            }
        }

        public void Run()
        {
            m_SendWorkThread.Start();
            foreach( var item in m_RecvThreadList)
            {
                item.Value.Start(item.Key);
            }
        }

        public void Exit()
        {
            if (m_SendWorkThread.ThreadState == ThreadState.Running)
            {
                m_SendWorkThread.Abort();
            }
            foreach( var item in m_RecvThreadList )
            {
                if (item.Value.ThreadState == ThreadState.Running)
                {
                    item.Value.Abort();
                }
            }
            
            foreach (var item in SocketMap)
            {
                if (item.m_Socket != null && item.m_Socket.Connected)
                    item.m_Socket.Close();
            }
        }

        public void ConnectToServer(int nIndex)
        {
            var info = SocketMap[nIndex];
            if (!Utility.TestNetwork(info.m_URL))
            {
                log.Write(string.Format("Ping to {0} fialed.", AppStatus.ServerIP.ToString()));
                return;
            }
            if (info.m_Socket == null || !info.m_Socket.Connected)
            {
                info.m_Socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, info.m_Type);
                info.m_Socket.Connect(info.m_IPInfo);
                //AppStatus.RecvBufferSize = s.ReceiveBufferSize;
                /*_DC.Add(ShareItem.ConnectStatus, m_Socket.Connected);*/
                //string connectMsg = "SloongWalls Client is connect.";
                //m_PageHost.SendMessage(MessageType.send, connectMsg);
                //m_hSocket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveTimeout, 5000); //设置接收数据超时                    
                //sRecvPicTemp.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendTimeout, 5000);//设置发送数据超时                    
                //sRecvPicTemp.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer, 1024);//设置发送缓冲区大小--1K大小                   
                //m_hSocket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer, bufferSize); //设置接收缓冲区大小
                //continue;
            }
        }

        public void SendWorkLoop()
        {
            while (AppStatus != null && AppStatus.RunStatus != RunStatus.Exit)
            {
                try
                {
                    var CurSock = SocketMap[m_nCurrentSocket].m_Socket;
                    if (CurSock == null || !CurSock.Connected)
                        ConnectToServer(m_nCurrentSocket);
                    //_DC[ShareItem.ConnectStatus] = m_Socket.Connected;
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
                    if (SendList != null && SendList.Count > 0)
                    {
                        var pack = SendList.Dequeue();
                        var msg = pack.SendMessage;
                        string md5 = Utility.MD5_Encoding(msg, Encoding.UTF8);
                        var sendmsg = string.Format("{0}|{1}|{2}", md5, pack.SwiftNumber, msg);
                        var len = string.Format("{0:D8}", sendmsg.Length);
                        sendmsg = len + sendmsg;

                        byte[] sendByte = Encoding.ASCII.GetBytes(sendmsg);
                        var Sock = SocketMap[pack.SocketID].m_Socket;
                        if( !m_RecvThreadList.ContainsKey(pack.SocketID))
                        {
                            var mainRecv = new Thread(() => RecvWorkLoop(pack.SocketID));
                            mainRecv.Name = "Network Recv Thread :" + pack.SocketID.ToString();
                            m_RecvThreadList[pack.SocketID] = mainRecv;
                            mainRecv.Start();
                        }
                        
                        lock (Sock)
                        {
                            Utility.SendEx(Sock, sendByte);
                            pack.IsSent = true;
                            MsgList.Add(pack.SwiftNumber, pack);
                        }
                    }
                    else
                    {
                        Thread.Sleep(500);
                        continue;
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

        public static long RecvDataLength(Socket sock, int overTime)
        {
            byte[] leng = Utility.RecvEx(sock, 8, overTime);

            long packSize = BitConverter.ToInt64(leng, 0);
            if (packSize <= 0)
            {
                throw new Exception(string.Format("Recv length error. the size is less than zero. the length is:{0}. the data is:{1}", packSize, leng.ToString()));
            }

            return packSize;
        }

        public void RecvWorkLoop( int id )
        {
            while (AppStatus != null && AppStatus.RunStatus != RunStatus.Exit)
            {
                var info = SocketMap[id];
                try
                {
                    if (info.m_Socket == null || !info.m_Socket.Connected)
                    {
                        System.Threading.Thread.Sleep(100);
                        continue;
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.ToString());
                    System.Threading.Thread.Sleep(500);
                    continue;
                }
                try
                {
                    // Receive message
                    // TODO:: add the static hread 
                    //                     byte[] header = new byte[24];
                    //                     m_Socket.Receive(header, 24, 0);
                    //                     string temp = (Encoding.Unicode.GetString(header));
                    //                     if ("SloongWalls\0" != temp)
                    //                     {
                    //                         continue;
                    //                     }


                    byte[] data = Utility.RecvEx(info.m_Socket, RecvDataLength(info.m_Socket, 10000), 10000);

                    string strRecv = Encoding.UTF8.GetString(data);
                    var RecvDatas = strRecv.Split('|');
                    long nIndex = Convert.ToInt64(RecvDatas[1]);
                    if (MsgList.ContainsKey(nIndex))
                    {
                        var pack = MsgList[nIndex];
                        pack.ReceivedMessages = RecvDatas;
                        if (pack.NeedExData)
                        {
                            byte[] exData = Utility.RecvEx(info.m_Socket, RecvDataLength(info.m_Socket, 0), 0);
                            pack.ReceivedExData = exData;
                        }
                        pack.IsReceived = true;
                        if (pack.ReceivedHandler != null)
                        {
                            pack.ReceivedHandler.BeginInvoke(pack, WhenRecvDone, pack.SwiftNumber);
                        }
                    }
                    else
                    {
                        // no the swift number, error.
                        throw new Exception("Received message but no checked the swift number. please check the message:" + strRecv);
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

        public void WhenRecvDone(IAsyncResult res)
        {
            long nIndex = (long)res.AsyncState;
            MsgList.Remove(nIndex);
        }
    }
}
