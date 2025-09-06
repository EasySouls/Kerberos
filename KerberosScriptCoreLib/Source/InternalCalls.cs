using System.Runtime.CompilerServices;

namespace Kerberos
{
    public static class InternalCalls
    {

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void NativeLog(string message);


        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_GetTranslation(ulong entityID, out Vector3 translation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Entity_SetTranslation(ulong entityID, ref Vector3 translation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool Input_IsKeyDown(KeyCode key);
    }
}
