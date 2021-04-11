#pragma once

#include "InputManager.h"

struct KeyboardControl
{
    bool is_active = false;

    // 
    InputEvent::Type keyboard_event;
    std::uint8_t key;
};

struct InputControl
{
    KeyboardControl keyboard;
};

struct BilliardsControls
{
    InputControl walk = { { true, InputEvent::Type::kKeyDown, 'A' } };
};

struct BilliardsConfiguration
{
    bool dont_start_game = false;
    bool no_load_models = false;
    bool load_room = false;
    bool debug_cue_camera_enabled = true;
    bool use_cue_constrain = true;
    bool replay_enabled = false;
    bool always_autoreplay = false;
    bool limit_head_camera_look = false;

    enum class GameMode
    {
        kSingle,
        kSingleBot,
        kBot,
        kMultiplayer,
        kMultiplayerSingleBot,
        kMultiplayerBot
    } game_mode = GameMode::kSingle;

    enum class TableColliderType
    {
        // Table consists of simple procedural generated convex cushion shapes and surface.
        kSimple,
        // Table consists of single body.
        kSingleTriangleMesh,
        // Table consists of static triangle shapes with each physics material.
        kTriangleMeshes,
        // Consists of both.
        kCompound
    } table_collider_type = TableColliderType::kTriangleMeshes;

    enum class CueSimulationType
    {
        kDirect,
        kCollide
    } cue_type = CueSimulationType::kDirect;

    BilliardsControls controls;
};
