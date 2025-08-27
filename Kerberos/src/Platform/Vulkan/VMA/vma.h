#pragma once

#include "Kerberos/Core/Scoped.h"

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Kerberos::vma
{
    struct Deleter
    {
        void operator()(VmaAllocator allocator) const noexcept;
    };

    using Allocator = Scoped<VmaAllocator, Deleter>;

    [[nodiscard]] 
    Allocator CreateAllocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
}