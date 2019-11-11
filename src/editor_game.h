#pragma once

#include "asset.h"
#include "render_group.h"

enum game_mode
{
	GameMode_None,

	GameMode_Game,
	GameMode_Editor,
};

struct assets
{

};

struct game_world_state
{

};

struct game_editor_state
{

};

struct game_state
{
	b32 IsInit;

	font_asset_info *FontAsset;

	game_mode GameMode;
	game_world_state *WorldState;

#ifdef DEVELOP_MODE
	game_editor_state *EditorState;
#endif
};

global_variable platform_api PlatformAPI;