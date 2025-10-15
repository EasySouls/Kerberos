using Kerberos.Source.Kerberos.Core;
using System;
using System.IO;

namespace Kerberos.Source.Kerberos.Scene
{
    public abstract class Component
    {
        public Entity Entity { get; internal set; }
    }

    public class TagComponent : Component
    {
        public string Tag { get; set; }
    }

    public class TransformComponent : Component
    {
        public Vector3 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
                return translation;
            }
            set => InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
        }

        public Vector3 Rotation
        {
            get
            {
                InternalCalls.TransformComponent_GetRotation(Entity.ID, out Vector3 rotation);
                return rotation;
            }
            set => InternalCalls.TransformComponent_SetRotation(Entity.ID, ref value);
        }

        public Vector3 Scale
        {
            get
            {
                InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 scale);
                return scale;
            }
            set => InternalCalls.TransformComponent_SetScale(Entity.ID, ref value);
        }

        public void LookAt(Vector3 targetPosition)
        {
            Vector3 direction = targetPosition - Translation;

            const float epsilon = 0.000000f;
            if (direction.Magnitude < epsilon) 
            {
                Rotation = Vector3.Zero;
            }

            double yaw = Math.Atan2(direction.X, direction.Y) * Constants.Rad2Deg;
            float horizontalDistance = new Vector2(direction.X, direction.Z).Magnitude;
            double pitch = -Math.Atan2(direction.Y, horizontalDistance) * Constants.Rad2Deg;
            const double roll = 0;

            Rotation = new Vector3((float)pitch, (float)yaw, (float)roll);
        }
    }

    public class RigidBody3DComponent : Component
    {
        public void ApplyImpulse(Vector3 impulse) => InternalCalls.Rigidbody3DComponent_ApplyImpulse(Entity.ID, ref impulse);

        public void ApplyImpulse(Vector3 impulse, Vector3 point) => InternalCalls.Rigidbody3DComponent_ApplyImpulseAtPoint(Entity.ID, ref impulse, ref point);
    }

    public class TextComponent : Component
    {
        public string Text
        {
            get
            {
                InternalCalls.TextComponent_GetText(Entity.ID, out string text);
                return text;
            }
            set => InternalCalls.TextComponent_SetText(Entity.ID, ref value);
        }

        public Vector4 Color
           {
            get
            {
                InternalCalls.TextComponent_GetColor(Entity.ID, out Vector4 color);
                return color;
            }
            set => InternalCalls.TextComponent_SetColor(Entity.ID, ref value);
        }

        public float FontSize
        {
            get
            {
                InternalCalls.TextComponent_GetFontSize(Entity.ID, out float fontSize);
                return fontSize;
            }
            set
            {
                if (value <= 0)
                    throw new ArgumentOutOfRangeException("Font size must be greater than zero.");
                InternalCalls.TextComponent_SetFontSize(Entity.ID, ref value);
            }
        }

        public string FontPath
        {
            get
            {
                InternalCalls.TextComponent_GetFontPath(Entity.ID, out string fontPath);
                return fontPath;
            }
            set
            {
                if (!File.Exists(value))
                    throw new FileNotFoundException($"Font file not found at path: {value}");
                InternalCalls.TextComponent_SetFontPath(Entity.ID, ref value);
            }
        }
    }
}
