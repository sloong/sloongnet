
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
                // send check key
                var key = "clinecheckkeyforsloongnet";
                var gbk = Encoding.GetEncoding("GB2312");
                byte[] sendByte = gbk.GetBytes(key);
                Utility.SendEx(info.m_Socket, sendByte);

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
                        string md5 = "";
                        List<byte> sendList = new List<byte>();
                        var gbk = Encoding.GetEncoding("GB2312");

                        // 计算长度
                        long lLen = gbk.GetByteCount(msg) + 32 + 8 + 1;

                        // 开始准备数据
                        /// 长度
                        sendList.AddRange(Utility.LongToBytes(lLen));

                        /// 优先级
                        sendList.Add(pack.level);

                        /// 流水号
                        if (AppStatus.bEnableSwift)
                        {
                            sendList.AddRange(Utility.LongToBytes(pack.SwiftNumber));
                        }

                        /// md5
                        if (AppStatus.bEnableMD5)
                        {
                            md5 = Utility.MD5_Encoding(msg, gbk);
                            sendList.AddRange(gbk.GetBytes(md5));
                        }


                        sendList.AddRange(gbk.GetBytes(msg));
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
                            Utility.SendEx(Sock, sendList);
                            pack.IsSent = true;
                            // only add to list when enable swift.
                            if (AppStatus.bEnableSwift)
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

                    long nLength = Utility.RecvDataLength(info.m_Socket, 10000);
                    

                    long nSwift = -1;
                    string md5 = "";
                    int index = 0;
                    if (AppStatus.bEnableSwift)
                    {
                        nSwift = Utility.BytesToLong(Utility.RecvEx(info.m_Socket, 8, 10000));
                        index = 8;
                    }
                    
                    if (AppStatus.bEnableMD5)
                    {
                        byte[] bMD5 = Utility.RecvEx(info.m_Socket, 32, 10000);
                        md5 = Encoding.GetEncoding("GB2312").GetString(bMD5);
                        index += 32;
                    }
                    byte[] data = Utility.RecvEx(info.m_Socket, nLength - index, 10000);
                    string strRecv = Encoding.GetEncoding("GB2312").GetString(data);
                    if (MsgList.ContainsKey(nSwift))
                    {
                        var pack = MsgList[nSwift];
                        pack.ReceivedMessages = strRecv;
                        if (pack.NeedExData)
                        {
                            byte[] exData = Utility.RecvEx(info.m_Socket, Utility.RecvDataLength(info.m_Socket, 0), 0);
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
