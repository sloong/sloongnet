using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.NetworkInformation;
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
    }
}
