using Kerberos.Source.Kerberos.Core;

namespace Kerberos.Source.Kerberos.Scene
{
    public abstract class Entity
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

        protected abstract void OnCreate();

        protected abstract void OnUpdate(float deltaTime);

        protected Vector3 Translation 
        { 
            get 
            {
                InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 translation);
                return translation;
            }
            set => InternalCalls.TransformComponent_SetTranslation(ID, ref value);
        }

        //protected T GetComponent<T>() where T : Component, new()
        //{
            
        //}
    }
}
