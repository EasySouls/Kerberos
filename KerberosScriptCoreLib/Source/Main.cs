using System;

namespace Kerberos
{
    public class ScriptCoreLib
    {
        public float Value { get; set; }

        public void Main(string[] args)
        {
            Console.WriteLine("Hello, World!");
        }

        public void PrintCurrentTime()
        {
            Console.WriteLine("Current Time: " + DateTime.Now.ToString("HH:mm:ss"));
        }

        public void PrintCustomMessage(string message)
        {
            Console.WriteLine(message);
        }
    }
}