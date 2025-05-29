#pragma once
#include <d3d11sdklayers.h>
#include <map>
#include <string_view>

namespace Kerberos::D3D11Utils
{
    template<UINT TDebugNameLength>
    inline void SetDebugName(
        _In_ ID3D11DeviceChild* deviceResource,
        _In_z_ const char(&debugName)[TDebugNameLength])
    {
        deviceResource->SetPrivateData(WKPDID_D3DDebugObjectName, TDebugNameLength - 1, debugName);
    }

	inline const std::map<D3D11_MESSAGE_SEVERITY, std::string_view>& GetD3D11SeverityMap()
    {
        static const std::map<D3D11_MESSAGE_SEVERITY, std::string_view> severityMap = {
            {D3D11_MESSAGE_SEVERITY_CORRUPTION, "CORRUPTION"},
            {D3D11_MESSAGE_SEVERITY_ERROR,      "ERROR"},
            {D3D11_MESSAGE_SEVERITY_WARNING,    "WARNING"},
            {D3D11_MESSAGE_SEVERITY_INFO,       "INFO"},
            {D3D11_MESSAGE_SEVERITY_MESSAGE,    "MESSAGE"}
        };
        return severityMap;
    }

    inline const std::map<D3D11_MESSAGE_ID, std::string_view>& GetD3D11MessageIdMap()
    {
        static const std::map<D3D11_MESSAGE_ID, std::string_view> messageIdMap = {
            {D3D11_MESSAGE_ID_DEVICE_DRAW_CONSTANT_BUFFER_TOO_SMALL, "DEVICE_DRAW_CONSTANT_BUFFER_TOO_SMALL"},
            {D3D11_MESSAGE_ID_DEVICE_DRAW_INDEX_BUFFER_TOO_SMALL, "DEVICE_DRAW_INDEX_BUFFER_TOO_SMALL"},
            {D3D11_MESSAGE_ID_DEVICE_DRAW_VERTEX_BUFFER_TOO_SMALL, "DEVICE_DRAW_VERTEX_BUFFER_TOO_SMALL"},
            // Add more message IDs as needed
        };
        return messageIdMap;
	}

    inline const std::map<D3D11_MESSAGE_CATEGORY, std::string_view>& GetD3D11MessageCategoryMap()
    {
        static const std::map<D3D11_MESSAGE_CATEGORY, std::string_view> messageCategoryMap = {
            { D3D11_MESSAGE_CATEGORY_APPLICATION_DEFINED, "APPLICATION_DEFINED" },
            { D3D11_MESSAGE_CATEGORY_MISCELLANEOUS, "MISCELLANEOUS" } ,
            { D3D11_MESSAGE_CATEGORY_INITIALIZATION, "INITIALIZATION" },
            { D3D11_MESSAGE_CATEGORY_CLEANUP, "CLEANUP" },
            { D3D11_MESSAGE_CATEGORY_COMPILATION, "COMPILATION" },
            { D3D11_MESSAGE_CATEGORY_STATE_CREATION, "STATE_CREATION" },
            { D3D11_MESSAGE_CATEGORY_STATE_SETTING, "STATE_SETTING" },
            { D3D11_MESSAGE_CATEGORY_STATE_GETTING, "STATE_GETTING" },
            { D3D11_MESSAGE_CATEGORY_RESOURCE_MANIPULATION, "RESOURCE_MANIPULATION" },
            { D3D11_MESSAGE_CATEGORY_EXECUTION, "EXECUTION" },
            { D3D11_MESSAGE_CATEGORY_SHADER, "SHADER" }
        };
        return messageCategoryMap;
    }

    inline std::string_view GetMessageIdName(const D3D11_MESSAGE_ID messageId)
    {
        const auto& map = GetD3D11MessageIdMap();
        const auto it = map.find(messageId);
        if (it != map.end())
        {
            return it->second;
        }
        return "UNKNOWN_MESSAGE_ID";
	}

    inline std::string_view GetMessageSeverityName(const D3D11_MESSAGE_SEVERITY severity)
    {
        const auto& map = GetD3D11SeverityMap();
        const auto it = map.find(severity);
        if (it != map.end())
        {
            return it->second;
        }
        return "UNKNOWN_SEVERITY";
    }

    inline std::string_view GetMessageCategoryName(const D3D11_MESSAGE_CATEGORY category)
    {
        const auto& map = GetD3D11MessageCategoryMap();
        const auto it = map.find(category);
        if (it != map.end())
        {
            return it->second;
        }
        return "UNKNOWN_CATEGORY";
    }
}