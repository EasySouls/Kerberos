using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Kerberos
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

        void OnCreate()
        {
        }

        void OnUpdate(float deltaTime)
        {
        }

        public Vector3 Translation 
        { 
            get 
            {
                InternalCalls.Entity_GetTranslation(ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.Entity_SetTranslation(ID, ref value);
            }
        }
    }
}
