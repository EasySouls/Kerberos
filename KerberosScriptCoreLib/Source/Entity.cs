using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Kerberos
{
    public abstract class Entity
    {
        void OnCreate()
        {
            Console.WriteLine("Entity::OnCreate");
        }

        void OnUpdate(float deltaTime)
        {
            Console.WriteLine($"Entity::OnUpdate - {deltaTime}");
        }
    }
}
