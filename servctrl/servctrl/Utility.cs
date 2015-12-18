using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Runtime.Serialization.Formatters.Binary;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace servctrl
{
    public class Utility
    {
        /// <summary>
        /// 通过ping测试目标地址是否联通
        /// </summary>
        public static bool TestNetwork(string targetIP)
        {
            try
            {
                Ping pingSender = new Ping();
                PingOptions options = new PingOptions();
                options.DontFragment = true;
                string data = "Sloong soft Network Test";
                byte[] buffer = Encoding.ASCII.GetBytes(data);
                int timeout = 120;
                PingReply reply = pingSender.Send(targetIP, timeout, buffer, options);
                if (reply.Status == IPStatus.Success)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
                return false;
            }
        }

        public static void Serialize( object obj, string path )
        {
            string temp = path + ".tmp";
            BinaryFormatter formatter = new BinaryFormatter();
            Stream stream = new FileStream(temp, FileMode.Create, FileAccess.Write);
            formatter.Serialize(stream, obj);
            stream.Close();
            if (File.Exists(path))
                File.Delete(path);
            File.Move(temp, path);
        }

        public static object Deserialize(string path)
        {
            if( !File.Exists(path) )
            {
                throw new Exception("Deserialize failed. File no exist");
            }
            BinaryFormatter formatter = new BinaryFormatter();
            FileStream stream = new FileStream(path, FileMode.Open, FileAccess.Read);
            var obj = formatter.Deserialize(stream);
            stream.Close();
            return obj;
        }

        public static string MD5_Encoding( string str, Encoding enc = null )
        {
            MD5 md = MD5.Create();
            if (enc == null)
                enc = Encoding.ASCII;
            return BitConverter.ToString(md.ComputeHash(enc.GetBytes(str))).Replace("-", ""); 
        }

        public static byte[] RecvEx( Socket sock, long len )
        {
           
            byte[] recvRes = new byte[len];
            int nRecvSize = 0;
            int nBufferSize = sock.ReceiveBufferSize;
            try
            {
            while (nRecvSize < len)
            {
                byte[] recv;
                int nNorecvSize = (int)len - nRecvSize;
                if (nNorecvSize > nBufferSize)
                {
                    recv = new byte[nBufferSize];
                }
                else
                {
                    recv = new byte[nNorecvSize];
                }

                int readsize = sock.Receive(recv, recv.Length,0);
                recv.CopyTo(recvRes, nRecvSize);
                nRecvSize += readsize;
            }
            }
           catch(Exception e)
            {
                Console.WriteLine(e.ToString());
            }

            return recvRes;
        }

        /// <summary>  
        /// 截取字节数组  
        /// </summary>  
        /// <param name="srcBytes">要截取的字节数组</param>  
        /// <param name="startIndex">开始截取位置的索引</param>  
        /// <param name="length">要截取的字节长度</param>  
        /// <returns>截取后的字节数组</returns>  
        public static byte[] SubByte(byte[] srcBytes, int startIndex, int length)
        {
            System.IO.MemoryStream bufferStream = new System.IO.MemoryStream();
            byte[] returnByte = new byte[] { };
            if (srcBytes == null) { return returnByte; }
            if (startIndex < 0) { startIndex = 0; }
            if (startIndex < srcBytes.Length)
            {
                if (length < 1 || length > srcBytes.Length - startIndex) { length = srcBytes.Length - startIndex; }
                bufferStream.Write(srcBytes, startIndex, length);
                returnByte = bufferStream.ToArray();
                bufferStream.SetLength(0);
                bufferStream.Position = 0;
            }
            bufferStream.Close();
            bufferStream.Dispose();
            return returnByte;
        }  

        public static void SendEx(Socket sock, byte[] data)
        {
            int nSentSize = 0;
            int nBufferSize = sock.SendBufferSize;
            while (nSentSize < data.Length)
            {
                byte[] send;
                int nNosendSize = data.Length - nSentSize;
                if (nNosendSize > nBufferSize)
                {
                    send = SubByte(data, nSentSize, nBufferSize);
                }
                else
                {
                    send = SubByte(data, nSentSize, nNosendSize);
                }

                int nSent = sock.Send(send);
                nSentSize += nSent;
            }
        }
    }
}
