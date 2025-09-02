using System;
using System.Runtime.CompilerServices;

namespace Kerberos
{
    public class ScriptCoreLib
    {
        public float Value { get; set; }

        public ScriptCoreLib()
        {
            Console.WriteLine("Hello, World!");

            CppFunc();
        }

        public void PrintCurrentTime()
        {
            Console.WriteLine("Current Time: " + DateTime.Now.ToString("HH:mm:ss"));
        }

        public void PrintCustomMessage(string message)
        {
            Console.WriteLine(message);
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void CppFunc();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static void NativeLog(string message);
    }
}