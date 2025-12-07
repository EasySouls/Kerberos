using System;
using Kerberos.Source.Kerberos.Core;
using Kerberos.Source.Kerberos.Scene;

namespace Kerberos.Source.Kerberos
{
    public class Player : Entity
    {
        public float Speed = 5.0f;

        private TransformComponent _transformComponent;
        private RigidBody3DComponent _rigidbody3DComponent;
        private AudioSource2DComponent _audioSource2DComponent;
        private Camera _mainCamera;

        // Implement OnXButtonClicked methods for 
        private bool _isPlayingAudio = false;

        /// <summary>
        /// Initializes a new Player instance with default field values.
        /// </summary>
        internal Player() : base()
        {
        }

        public Player(ulong id) : base(id)
        {
        }

        /// <summary>
        /// Initialize the player after it is created by caching the TransformComponent, any present optional components (RigidBody3DComponent and AudioSource2DComponent), and resolving the scene Camera entity.
        /// </summary>
        /// <remarks>
        /// Also writes a creation message containing the entity ID to the console.
        /// </remarks>
        protected override void OnCreate()
        {
            Console.WriteLine($"Player::OnCreate - {ID}");

            _transformComponent = GetComponent<TransformComponent>();

            if (HasComponent<RigidBody3DComponent>())
                _rigidbody3DComponent = GetComponent<RigidBody3DComponent>();

            if (HasComponent<AudioSource2DComponent>())
                _audioSource2DComponent = GetComponent<AudioSource2DComponent>();

            Entity cameraEntity = FindEntityByName("Camera");
            if (cameraEntity != null)
                _mainCamera = cameraEntity.As<Camera>();

        }

        /// <summary>
        /// Processes player input each frame to move the player, optionally apply a physics impulse, adjust camera zoom, and control 2D audio playback.
        /// </summary>
        /// <param name="deltaTime">Time elapsed since the last update used to scale movement and camera zoom adjustments.</param>
        /// <remarks>
        /// If a rigidbody component is present and the impulse key is pressed, an impulse is applied and the rest of the update is skipped for that frame. Camera zoom and audio controls only take effect when their respective components are available.
        /// </remarks>
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

            // Zoom camera in and out with Q and E
            if (_mainCamera == null) return;
            if (Input.IsKeyDown(KeyCode.Q))
                _mainCamera.DistanceFromPlayer += 1.0f * deltaTime;
            if (Input.IsKeyDown(KeyCode.E))
                _mainCamera.DistanceFromPlayer -= 1.0f * deltaTime;

            if (Input.IsKeyDown(KeyCode.P) && _audioSource2DComponent != null && !_isPlayingAudio)
            {
                _audioSource2DComponent.Play();
                _isPlayingAudio = true;
            }
            if (Input.IsKeyDown(KeyCode.O) && _audioSource2DComponent != null && _isPlayingAudio)
            {
                _audioSource2DComponent.Stop();
                _isPlayingAudio = false;
            }
        }
    }
}