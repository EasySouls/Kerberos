using System;
using System.Runtime.CompilerServices;
using Kerberos.Source.Kerberos.Core;
using Kerberos.Source.Kerberos.Scene;

namespace Kerberos.Source
{
    public static class InternalCalls
    {

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void NativeLog(string message);

        // ----------------------------- Entity -----------------------------

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Entity_HasComponent(ulong id, Type componentType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern ulong Entity_FindEntityByName(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern object Entity_GetScriptInstance(ulong entityID);

        // ----------------------------- TransformComponent -----------------------------

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetRotation(ulong entityID, out Vector3 rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetRotation(ulong entityID, ref Vector3 rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_GetScale(ulong entityID, out Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TransformComponent_SetScale(ulong entityID, ref Vector3 scale);

        // ----------------------------- Rigidbody3DComponent -----------------------------

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Rigidbody3DComponent_ApplyImpulse(ulong entityID, ref Vector3 impulse);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Rigidbody3DComponent_ApplyImpulseAtPoint(ulong entityID, ref Vector3 impulse, ref Vector3 point);

        // ----------------------------- TextComponent --------------------------------------

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_SetText(ulong entityID, string text);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern string TextComponent_GetText(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_SetColor(ulong entityID, ref Vector4 color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_GetColor(ulong entityID, out Vector4 color);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_SetFontSize(ulong entityID, float size);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float TextComponent_GetFontSize(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void TextComponent_SetFontPath(ulong entityID, string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern string TextComponent_GetFontPath(ulong entityID);

        // ----------------------------- Input -----------------------------

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsKeyDown(KeyCode key);

        // ------------------------- AudioSource2DComponent --------------------------

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource2DComponent_Play(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource2DComponent_Stop(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource2DComponent_SetVolume(ulong entityID, float volume);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float AudioSource2DComponent_GetVolume(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource2DComponent_SetLooping(ulong entityID, bool looping);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool AudioSource2DComponent_IsLooping(ulong entityID);

        // ------------------------- AudioSource3DComponent --------------------------

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource3DComponent_Play(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource3DComponent_Stop(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource3DComponent_SetVolume(ulong entityID, float volume);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float AudioSource3DComponent_GetVolume(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource3DComponent_SetLooping(ulong entityID, bool looping);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool AudioSource3DComponent_IsLooping(ulong entityID);

    }
}
