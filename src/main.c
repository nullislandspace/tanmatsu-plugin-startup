// SPDX-License-Identifier: MIT
// Startup Plugin - Plays startup sound
//
// Blocks launcher initialization until startup sound completes.
// Uses autostart: true to run at boot.

#include "tanmatsu_plugin.h"
#include "plugin_context.h"
#include "audio.h"
#include <stdio.h>

// Plugin metadata
static const plugin_info_t plugin_info = {
    .name = "Startup Sound",
    .slug = "at.cavac.startup",
    .version = "1.0.0",
    .author = "Rene Schickbauer",
    .description = "Plays sound at startup",
    .api_version = TANMATSU_PLUGIN_API_VERSION,
    .type = PLUGIN_TYPE_SERVICE,
    .flags = 0,
};

static const plugin_info_t* get_info(void) {
    return &plugin_info;
}

static int plugin_init(plugin_context_t* ctx) {
    asp_log_info("startup", "Initializing...");

    // Initialize audio subsystem
    if (audio_init() != 0) {
        asp_log_error("startup", "Audio init failed");
        return -1;
    }

    // Build audio path from plugin install directory
    char audio_path[512];
    snprintf(audio_path, sizeof(audio_path), "%s/tanmatsu_startup.mp3", ctx->plugin_path);

    // Play audio
    audio_play_file(audio_path);

    // Block until audio finishes - this prevents launcher from continuing
    while (!audio_is_finished()) {
        asp_plugin_delay_ms(50);
    }

    asp_log_info("startup", "Startup animation complete");

    // Cleanup audio before returning
    audio_stop();
    audio_cleanup();

    return 0;
}

static void plugin_cleanup(plugin_context_t* ctx) {
    asp_log_info("startup", "Cleaning up...");
    // Audio already cleaned up in init
}

static void plugin_service_run(plugin_context_t* ctx) {
    // Nothing to do - all work done in init()
    // Service returns immediately, triggering auto-unload
}

// Plugin entry point structure
static const plugin_entry_t entry = {
    .get_info = get_info,
    .init = plugin_init,
    .cleanup = plugin_cleanup,
    .menu_render = NULL,
    .menu_select = NULL,
    .service_run = plugin_service_run,
    .hook_event = NULL,
};

// Register this plugin with the host
TANMATSU_PLUGIN_REGISTER(entry);
