#pragma once

#include <glm/glm.hpp>

/**
 * @brief Compute shader capability limits for the current graphics context.
 *
 * Contains the maximum supported work group counts, work group sizes, and
 * total invocations per work group exposed by the underlying GPU/driver.
 */

/**
 * @brief Maximum number of work groups that can be dispatched in each dimension.
 */

/**
 * @brief Maximum size of a work group (threads) in each dimension.
 */

/**
 * @brief Maximum total invocations (threads) within a single work group per dimension.
 */

/**
 * @brief Abstract interface for a platform-specific graphics context.
 *
 * Implementations initialize GPU state and present rendered frames. Concrete
 * subclasses must provide initialization and buffer swap behaviour.
 */

/**
 * @brief Virtual destructor for proper cleanup in derived contexts.
 */

/**
 * @brief Initialize the graphics context and any associated GPU resources.
 */

/**
 * @brief Present the rendered frame by swapping the front and back buffers.
 */

/**
 * @brief Compute shader capability values for this context.
 *
 * Populated by concrete implementations to reflect the GPU/driver limits.
 */
namespace Kerberos
{
	struct ComputeInfo
	{
		glm::vec3 MaxWorkGroupCount{0};
		glm::vec3 MaxWorkGroupSize{0};
		glm::vec3 MaxWorkGroupInvocations{0};
	};

	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;

	protected:
		ComputeInfo m_ComputeInfo;
	};
}