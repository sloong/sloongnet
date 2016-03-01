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
        public Socket m_Socket = null;
        public IPEndPoint ip;
        Thread m_SendWorkThread = null;
        Thread m_RecvWorkThread = null;

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


        public NetworkThread(IDataCenter dc)
        {
            _DC = dc;

            m_SendWorkThread = new Thread(() => SendWorkLoop());
            m_SendWorkThread.Name = "Network Send Thread";

            m_RecvWorkThread = new Thread(() => RecvWorkLoop());
            m_RecvWorkThread.Name = "Network Recv Thread";
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
            m_RecvWorkThread.Start();
        }

        public void Exit()
        {
            if (m_SendWorkThread.ThreadState == ThreadState.Running)
            {
                m_SendWorkThread.Abort();
            }
            if (m_RecvWorkThread.ThreadState == ThreadState.Running)
            {
                m_RecvWorkThread.Abort();
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
                log.Write(string.Format("Ping to {0} fialed.", AppStatus.ServerIP.ToString()));
                return;
            }
            if (s == null || !s.Connected)
            {
                s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                IPEndPoint ipEndPoint = new IPEndPoint(IPAddress.Parse(AppStatus.ServerIP), AppStatus.SendProt);
                s.Connect(ipEndPoint);
                AppStatus.RecvBufferSize = s.ReceiveBufferSize;
                _DC.Add(ShareItem.ConnectStatus, m_Socket.Connected);
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
                    if (m_Socket == null || !m_Socket.Connected)
                        ConnectToServer(ref m_Socket, AppStatus.ServerIP);
                    _DC[ShareItem.ConnectStatus] = m_Socket.Connected;
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
                        //   if (MsgList.ContainsKey(nProcIndex))
                        {
                            var pack = SendList.Dequeue();
                            var msg = pack.SendMessage;
                            string md5 = Utility.MD5_Encoding(msg, Encoding.UTF8);
                            var sendmsg = string.Format("{0}|{1}|{2}", md5, pack.SwiftNumber, msg);
                            var len = string.Format("{0:D8}", sendmsg.Length);
                            sendmsg = len + sendmsg;
                            
                            byte[] sendByte = Encoding.ASCII.GetBytes(sendmsg);
                            lock (m_Socket)
                            {
                                Utility.SendEx(m_Socket, sendByte);
                                pack.IsSent = true;
                                MsgList.Add(pack.SwiftNumber, pack);
                            }
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

        public void RecvWorkLoop()
        {
            while (AppStatus != null && AppStatus.RunStatus != RunStatus.Exit)
            {
                try
                {
                    if (m_Socket == null || !m_Socket.Connected)
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


                    byte[] data = Utility.RecvEx(m_Socket, RecvDataLength(m_Socket, 10000), 10000);

                    string strRecv = Encoding.UTF8.GetString(data);
                    var RecvDatas = strRecv.Split('|');
                    long nIndex = Convert.ToInt64(RecvDatas[1]);
                    if (MsgList.ContainsKey(nIndex))
                    {
                        var pack = MsgList[nIndex];
                        pack.ReceivedMessages = RecvDatas;
                        if (pack.NeedExData)
                        {
                            byte[] exData = Utility.RecvEx(m_Socket, RecvDataLength(m_Socket, 0), 0);
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
