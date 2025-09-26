using Kerberos.Source.Kerberos.Core;

namespace Kerberos.Source.Kerberos.Scene
{
    public class Entity
    {
        protected Entity()
        {
            ID = 0;
        }

        internal Entity(ulong id)
        {
            ID = id;
        }

        public readonly ulong ID;

        protected virtual void OnCreate() {}

        protected virtual void OnUpdate(float deltaTime) {}

        public Vector3 Translation
        {
            get
            {
                InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 translation);
                return translation;
            }
            protected set => InternalCalls.TransformComponent_SetTranslation(ID, ref value);
        }

        protected bool HasComponent<T>() where T : Component, new()
        {
            return InternalCalls.Entity_HasComponent(ID, typeof(T));
        }

        protected T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T() { Entity = this };
            return component;
        }

        protected static Entity FindEntityByName(string name)
        {
            ulong entityID = InternalCalls.Entity_FindEntityByName(name);
            return entityID == 0 ? null : new Entity(entityID);
        }

        protected internal T As<T>() where T : Entity, new()
        {
            object instance = InternalCalls.Entity_GetScriptInstance(ID);
            return instance as T;
        }
    }
}
