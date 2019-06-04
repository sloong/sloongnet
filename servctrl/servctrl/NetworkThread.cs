
using Google.Protobuf;
using ProtobufMessage;
using servctrl;
using Sloong;
using Sloong.Interface;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;
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

        Dictionary<long, MessageData> MsgList
        {
            get
            {
                return _DC[ShareItem.MessageList] as Dictionary<long, MessageData>;
            }
        }

        Queue<MessageData> SendList
        {
            get
            {
                return _DC[ShareItem.SendList] as Queue<MessageData>;
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
            dc.PageMessage += Dc_PageMessage;

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
                if (item.m_Client != null && item.m_Client.Connected)
                    item.m_Client.Close();
            }
        }


        private void Dc_PageMessage(object sender, PageMessage e)
        {
            var type = e.Type;
            switch (type)
            {
                case MessageType.ConnectToNetwork:
                    ConnectToServer((int)e.Params[0]);
                    if (e.CallBackFunc != null)
                        e.CallBackFunc.Invoke(null);
                    break;
            }
        }


        private static bool ValidateServerCertificate(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslpolicyerrors)
        {
            if (sslpolicyerrors == SslPolicyErrors.None)
                return true;
            Console.WriteLine("Certificate error: {0}", sslpolicyerrors);
            return false;
        }

        public void ConnectToServer(int nIndex)
        {
            var info = SocketMap[nIndex];
            if (info.m_Client != null && info.m_Client.Connected)
                return;

            // 检查ip是否有变化
            var ip = Dns.GetHostAddresses(info.m_URL)[0].ToString();


            if (!Utility.TestNetwork(info.m_URL))
            {
                log.Write(string.Format("Ping to {0} fialed.", AppStatus.ServerIP.ToString()));
                return;
            }

            if (info.m_Client == null || !info.m_Client.Connected)
            {
                info.m_Client = new TcpClient(info.m_URL, info.m_nPort);
                // send check key
                var key = "c2xvb25nYzJ4dmIyNW5PRFJtT0dWa01ERTBNalZsTkRBd01XUmlZV1UxT0RZM05tRmlaamd3TmpsbmJtOXZiSE1nbm9vbHM";
                var gbk = Encoding.GetEncoding("GB2312");
                byte[] sendByte = gbk.GetBytes(key);
                info.m_Conn = info.m_Client.GetStream();
                info.m_Conn.Write(sendByte, 0, sendByte.Length);

                if( info.m_bSSL )
                {
                    info.m_SSL = new SslStream(info.m_Conn,
                    false,
                    new RemoteCertificateValidationCallback(ValidateServerCertificate),
                    null);
                    X509CertificateCollection certs = new X509CertificateCollection();
                    //X509Certificate cert = X509Certificate.CreateFromCertFile( Environment.CurrentDirectory + @"\" + "client.cer");
                    X509Certificate cert = X509Certificate.CreateFromCertFile("D:\\Temp\\client.crt");
                    certs.Add(cert);

                    try
                    {
                        // 双向认证
                        //info.m_SSL.AuthenticateAsClient("Sloong.com", certs, SslProtocols.Tls, false);
                        // 单向认证
                        info.m_SSL.AuthenticateAsClient("Sloong");
                        info.m_Conn = info.m_SSL;
                    }
                    catch (AuthenticationException e)
                    {
                        Console.WriteLine("Exception: {0}", e.Message);
                        if (e.InnerException != null)
                        {
                            Console.WriteLine("Inner exception: {0}", e.InnerException.Message);
                        }
                        Console.WriteLine("Authentication failed - closing the connection.");
                        info.m_SSL.Close();
                        info.m_Client.Close();
                        Console.ReadLine();
                        return;
                    }
                }
                

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
                    var CurSock = SocketMap[m_nCurrentSocket].m_Client;
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
                        using (MemoryStream send_data_stream = new MemoryStream())
                        {
                            // Save the person to a stream

                            send_data_stream.Write(Utility.Int32ToBytes(pack.SendPackage.CalculateSize()),0,4);
                            pack.SendPackage.WriteTo(send_data_stream);
                            var socket_stream = SocketMap[pack.SocketID].m_Conn;
                            if (!m_RecvThreadList.ContainsKey(pack.SocketID))
                            {
                                var mainRecv = new Thread(() => RecvWorkLoop(pack.SocketID));
                                mainRecv.Name = "Network Recv Thread :" + pack.SocketID.ToString();
                                m_RecvThreadList[pack.SocketID] = mainRecv;
                                mainRecv.Start();
                            }

                            lock (socket_stream)
                            {
                                socket_stream.Write(send_data_stream.ToArray(), 0, (int)send_data_stream.Length);

                                //Utility.SendEx(Sock, sendList);
                                pack.IsSent = true;
                                pack.send = DateTime.Now;
                                // only add to list when enable swift.
                                MsgList.Add(pack.SendPackage.SerialNumber, pack);
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

        public void RecvWorkLoop( int id )
        {
            while (AppStatus != null && AppStatus.RunStatus != RunStatus.Exit)
            {
                var info = SocketMap[id];
                try
                {
                    if (info.m_Client == null || !info.m_Client.Connected)
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

                    // TODO : 需要替换为支持SSL的接收                    
                    long nLength = Utility.RecvDataLength(info.m_Conn, 10000);
                    
                    byte[] data = Utility.RecvEx(info.m_Conn, nLength, 10000);
                    var receivePackage = MessagePackage.Parser.ParseFrom(data);


                    if (MsgList.ContainsKey(receivePackage.SerialNumber))
                    {
                        var pack = MsgList[receivePackage.SerialNumber];
                        pack.recv = DateTime.Now;
                        pack.ReceivePackage = receivePackage;
                        pack.IsReceived = true;
                        if (pack.ReceivedHandler != null)
                        {
                            pack.ReceivedHandler.BeginInvoke(pack, WhenRecvDone, pack.ReceivePackage.SerialNumber);
                        }
                    }
                    else
                    {
                        // no the swift number, error.
                        throw new Exception("Received message but no checked the swift number.");
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
            long nIndex = Convert.ToInt64(res.AsyncState);
            MsgList.Remove(nIndex);
        }
    }
}
