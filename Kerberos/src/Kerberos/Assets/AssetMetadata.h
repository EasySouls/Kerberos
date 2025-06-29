#pragma once

#include <filesystem>
#include "Asset.h"

namespace Kerberos
{
	struct AssetMetadata
	{
		AssetType Type;
		std::filesystem::path Filepath;
	};
}