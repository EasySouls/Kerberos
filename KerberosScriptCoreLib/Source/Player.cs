using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Kerberos
{
    public class Player : Entity
    {
        internal Player() : base()
        {
        }

        public Player(ulong id) : base(id)
        {
        }

        void OnCreate()
        {
            Console.WriteLine($"Player::OnCreate - {ID}");
        }

        void OnUpdate(float deltaTime)
        {
            float speed = 5.0f;

            Vector3 translation = Translation;
            translation.X += speed * deltaTime;
            Translation = translation;
        }
    }
}
