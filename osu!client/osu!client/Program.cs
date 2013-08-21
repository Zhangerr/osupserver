using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.IO;
using System.Threading;

namespace osu_client
{
    class Program
    {
        static BinaryReader sr;
        static StreamWriter sw;
        static void Main(string[] args)
        {
            TcpClient c = new TcpClient("cho.ppy.sh", 13382);
            sr = new BinaryReader(c.GetStream());
            sw = new StreamWriter (c.GetStream());
            string payload = "b20120905|1|0|aa6c1db670ffa222afa30e66e60fd695:02004C4F4F50.F46D0412F034.080027009CE9..00000000000000E0.00000000000000E0.00000000000000E0.00000000000000E0.00000000000000E0.:b3c78c8b5d6747a04838aee0a9dce9c6:369f547397cd5936d37b7401b3be68f1:4244259fb07be69e52cc016b2b528ff9\r\n";            
            string user = "test65\r\n";
            string pass = "39dd7a8aa04b8421b1269553f4f3be0fx\r\n";
            sw.Write(user + pass + payload);
            sw.Flush();
            while (true)
            {
                try
                {
                    /*byte[] s = new byte[254];
                    int y = sr.Read(s, 0, 254);
                        StringBuilder sb = new StringBuilder();
                    for (int x = 0; x < 254; x++)
                    {
                        sb.Append(Convert.ToChar(s[x]));
                    }
                    Console.Write(sb.ToString());*/
                    uint data = (uint)sr.ReadInt16();
                    Console.WriteLine(data);
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                    break;
                }
                Console.ReadKey();
            }
            Console.ReadKey();
        }
    }
}
