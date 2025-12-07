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

        /// <summary>
        /// Check whether the specified keyboard key is currently pressed.
        /// </summary>
        /// <param name="key">The key to check.</param>
        /// <returns>`true` if the specified key is currently pressed, `false` otherwise.</returns>

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Input_IsKeyDown(KeyCode key);

        /// <summary>
        /// Plays the 2D audio source attached to the specified entity.
        /// </summary>
        /// <param name="entityID">The unique identifier of the entity with the AudioSource2D component.</param>

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource2DComponent_Play(ulong entityID);

        /// <summary>
        /// Stops playback of the 2D audio source attached to the specified entity.
        /// </summary>
        /// <param name="entityID">The unique identifier of the entity whose 2D audio source will be stopped.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource2DComponent_Stop(ulong entityID);

        /// <summary>
        /// Sets the playback volume for the 2D audio source component on the specified entity.
        /// </summary>
        /// <param name="entityID">ID of the entity that owns the 2D audio source component.</param>
        /// <param name="volume">Volume multiplier where 1.0 represents the original volume.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource2DComponent_SetVolume(ulong entityID, float volume);

        /// <summary>
        /// Retrieves the current volume level of the 2D audio source attached to the specified entity.
        /// </summary>
        /// <param name="entityID">The entity identifier whose 2D audio source volume is requested.</param>
        /// <returns>The volume level as a float.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float AudioSource2DComponent_GetVolume(ulong entityID);

        /// <summary>
        /// Enables or disables looping for the 2D audio source attached to the specified entity.
        /// </summary>
        /// <param name="entityID">The entity's unique identifier.</param>
        /// <param name="looping">`true` to enable looping, `false` to disable it.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource2DComponent_SetLooping(ulong entityID, bool looping);

        /// <summary>
        /// Determines whether the 2D audio source on the specified entity is set to loop.
        /// </summary>
        /// <param name="entityID">The entity's unique identifier.</param>
        /// <returns>`true` if the audio source is set to loop, `false` otherwise.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool AudioSource2DComponent_IsLooping(ulong entityID);

        /// <summary>
        /// Plays the 3D audio source attached to the specified entity.
        /// </summary>
        /// <param name="entityID">The unique identifier of the entity.</param>

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource3DComponent_Play(ulong entityID);

        /// <summary>
        /// Stops playback of the specified entity's 3D audio source.
        /// </summary>
        /// <param name="entityID">The unique identifier of the entity whose 3D audio source will be stopped.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource3DComponent_Stop(ulong entityID);

        /// <summary>
        /// Sets the playback volume for the 3D audio source attached to the specified entity.
        /// </summary>
        /// <param name="entityID">The identifier of the entity that owns the 3D audio source.</param>
        /// <param name="volume">Desired volume level (engine-defined scale).</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource3DComponent_SetVolume(ulong entityID, float volume);

        /// <summary>
        /// Gets the playback volume of the 3D audio source attached to the specified entity.
        /// </summary>
        /// <param name="entityID">The entity's unique identifier.</param>
        /// <returns>The volume level where 0.0 is silent and 1.0 is the original/full volume.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float AudioSource3DComponent_GetVolume(ulong entityID);

        /// <summary>
        /// Sets whether the 3D audio source attached to the specified entity should loop playback.
        /// </summary>
        /// <param name="entityID">The unique identifier of the entity containing the audio source.</param>
        /// <param name="looping">`true` to enable looping, `false` to disable it.</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AudioSource3DComponent_SetLooping(ulong entityID, bool looping);

        /// <summary>
        /// Determines whether the 3D audio source on the specified entity is set to loop.
        /// </summary>
        /// <param name="entityID">The unique identifier of the entity containing the 3D audio source component.</param>
        /// <returns>`true` if looping is enabled on the entity's 3D audio source component, `false` otherwise.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool AudioSource3DComponent_IsLooping(ulong entityID);

    }
}