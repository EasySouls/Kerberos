#pragma once

/**
 * @brief Identifier for a generic asset browser item type.
 */

/**
 * @brief Identifier for a 2D texture asset type in the asset browser.
 */

/**
 * @brief Identifier for a cubemap (texture cube) asset type in the asset browser.
 */

/**
 * @brief Identifier for a mesh asset type in the asset browser.
 */

/**
 * @brief Identifier for a scene asset type in the asset browser.
 */

/**
 * @brief Identifier for an audio asset type in the asset browser.
 */

/**
 * @brief Identifier for a font asset type in the asset browser.
 */
namespace Kerberos
{
	constexpr const char* ASSET_BROWSER_ITEM = "ASSET_BROWSER_ITEM";
	constexpr const char* ASSET_BROWSER_TEXTURE = "ASSET_BROWSER_TEXTURE";
	constexpr const char* ASSET_BROWSER_TEXTURE_CUBE = "ASSET_BROWSER_TEXTURE_CUBE";
	constexpr const char* ASSET_BROWSER_MESH = "ASSET_BROWSER_MESH";
	constexpr const char* ASSET_BROWSER_SCENE = "ASSET_BROWSER_SCENE";
	constexpr const char* ASSET_BROWSER_AUDIO = "ASSET_BROWSER_AUDIO";
	constexpr const char* ASSET_BROWSER_FONT = "ASSET_BROWSER_FONT";
}