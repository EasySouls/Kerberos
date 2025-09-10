using System;
using Kerberos.Source.Kerberos.Core;
using Kerberos.Source.Kerberos.Scene;

namespace Kerberos.Source.Kerberos
{
    public class Player : Entity
    {
        private float Speed { get; set; } = 5.0f;

        private TransformComponent _transformComponent;
        private RigidBody3DComponent _rigidbody3DComponent;

        internal Player() : base()
        {
        }

        public Player(ulong id) : base(id)
        {
        }

        protected override void OnCreate()
        {
            Console.WriteLine($"Player::OnCreate - {ID}");

            _transformComponent = GetComponent<TransformComponent>();

            if (HasComponent<RigidBody3DComponent>())
                _rigidbody3DComponent = GetComponent<RigidBody3DComponent>();
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

            if (_rigidbody3DComponent != null && Input.IsKeyDown(KeyCode.F))
            {
                _rigidbody3DComponent.ApplyImpulse(new Vector3(0.5f, 0.0f, 0.0f));
                return;
            }

            velocity *= Speed;

            Vector3 translation = Translation;
            translation += velocity * deltaTime;
            Translation = translation;
        }
    }
}
