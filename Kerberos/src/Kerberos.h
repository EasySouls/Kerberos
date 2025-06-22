#pragma once

/// For use by Kerberos applications

#include "Kerberos/Application.h"
#include "Kerberos/Core.h"
#include "Kerberos/Log.h"
#include "Kerberos/Layer.h"
#include "Kerberos/Core/UUID.h"

#include "Kerberos/Core/Timestep.h"
#include "Kerberos/Core/Timer.h"

#include "Kerberos/ImGui/ImGuiLayer.h"

/// ---- Events ---------------------
#include "Kerberos/Events/Event.h"
#include "Kerberos/Events/ApplicationEvent.h"
#include "Kerberos/Events/KeyEvent.h"
#include "Kerberos/Events/MouseEvent.h"
/// ---------------------------------

/// ---- Renderer -------------------
#include "Kerberos/Renderer/Renderer.h"
#include "Kerberos/Renderer/Renderer2D.h"
#include "Kerberos/Renderer/RenderCommand.h"
#include "Kerberos/Renderer/OrthographicCamera.h"
#include "Kerberos/Renderer/Buffer.h"
#include "Kerberos/Renderer/Shader.h"
#include "Kerberos/Renderer/Texture.h"
#include "Kerberos/Renderer/SubTexture2D.h"
#include "Kerberos/Renderer/TextureCube.h"
#include "Kerberos/Renderer/VertexArray.h"
#include "Kerberos/Renderer/Framebuffer.h"
#include "Kerberos/Renderer/Light.h"
#include "Kerberos/Renderer/Material.h"
#include "Kerberos/Renderer/Vertex.h"
#include "Kerberos/Renderer/Renderer3D.h"
/// ---------------------------------

/// ---- Scene ----------------------
#include "Kerberos/Scene/Scene.h"
#include "Kerberos/Scene/Components.h"
#include "Kerberos/Scene/Entity.h"
#include "Kerberos/Scene/ScriptableEntity.h"
#include "Kerberos/Scene/SceneCamera.h"
#include "Kerberos/Scene/EditorCamera.h"
#include "Kerberos/Scene/SceneSerializer.h"
/// ---------------------------------

/// ---- Input ----------------------
#include "Kerberos/Core/Input.h"
#include "Kerberos/Core/KeyCodes.h"
#include "Kerberos/Core/MouseButtonCodes.h"
#include "Kerberos/OrthographicCameraController.h"
/// ---------------------------------

/// ---- Assets ---------------------
#include "Kerberos/Assets/Model.h"
//#include "Kerberos/Assets/AssetManager.h"
/// ---------------------------------
