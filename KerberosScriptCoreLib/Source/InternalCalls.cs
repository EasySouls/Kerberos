using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace Kerberos
{
    public static class InternalCalls
    {

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void NativeLog(string message);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Entity_GetTranslation(ulong entityID, out Vector3 translation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public static void Entity_SetTranslation(ulong entityID, ref Vector3 translation);
    }
}
