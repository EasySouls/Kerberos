using System;
using Kerberos.Source.Kerberos.Core;
using Kerberos.Source.Kerberos.Scene;

namespace Kerberos.Source.Kerberos
{
    public class Player : Entity
    {
        private float Speed { get; set; } = 5.0f;

        internal Player() : base()
        {
        }

        public Player(ulong id) : base(id)
        {
        }

        protected override void OnCreate()
        {
            Console.WriteLine($"Player::OnCreate - {ID}");
        }

        protected override void OnUpdate(float deltaTime)
        {
            Vector3 velocity = Vector3.Zero;

            if (Input.IsKeyDown(KeyCode.A))
                velocity.X -= 1.0f;
            if (Input.IsKeyDown(KeyCode.D))
                velocity.X += 1.0f;
            if (Input.IsKeyDown(KeyCode.W))
                velocity.Z += 1.0f;
            if (Input.IsKeyDown(KeyCode.S))
                velocity.Z -= 1.0f;
            if (Input.IsKeyDown(KeyCode.Space))
                velocity.Y += 1.0f;

            velocity *= Speed;

            Vector3 translation = Translation;
            translation += velocity * deltaTime;
            Translation = translation;
        }
    }
}
