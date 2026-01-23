// SPDX-License-Identifier: MIT
// Startup Plugin - Displays logo and plays startup sound
//
// Blocks launcher initialization until startup animation completes.
// Uses autostart: true to run at boot.

#include "tanmatsu_plugin.h"
#include "audio.h"
#include "pax_gfx.h"
#include <pax_codecs.h>
#include <asp/display.h>
#include <stdio.h>

#define LOGO_PATH "/int/plugins/tanmatsu_startup.png"
#define AUDIO_PATH "/int/plugins/tanmatsu_startup.mp3"

// Plugin metadata
static const plugin_info_t plugin_info = {
    .name = "Startup Animation",
    .slug = "startup",
    .version = "1.0.0",
    .author = "Tanmatsu",
    .description = "Displays logo and plays sound at startup",
    .api_version = TANMATSU_PLUGIN_API_VERSION,
    .type = PLUGIN_TYPE_SERVICE,
    .flags = 0,
};

static const plugin_info_t* get_info(void) {
    return &plugin_info;
}

static bool display_logo(void) {
    pax_buf_t* fb = NULL;
    asp_disp_get_pax_buf(&fb);
    if (!fb) {
        asp_log_error("startup", "Failed to get framebuffer");
        return false;
    }

    asp_log_info("startup", "Screen size: %dx%d", fb->width, fb->height);

    // Clear to black
    pax_background(fb, 0xFF000000);

    // Load PNG
    FILE* f = fopen(LOGO_PATH, "rb");
    if (!f) {
        asp_log_error("startup", "Cannot open %s", LOGO_PATH);
        asp_disp_write_pax(fb);
        return false;
    }

    asp_log_info("startup", "Decoding PNG...");
    bool result = pax_insert_png_fd(fb, f, 0, 0, 0);
    fclose(f);

    if (!result) {
        asp_log_error("startup", "Failed to decode PNG");
        asp_disp_write_pax(fb);
        return false;
    }

    asp_log_info("startup", "Writing to display...");
    asp_disp_write_pax(fb);
    asp_log_info("startup", "Logo displayed successfully");
    return true;
}

static int plugin_init(plugin_context_t* ctx) {
    asp_log_info("startup", "Initializing...");

    // Initialize audio subsystem
    if (audio_init() != 0) {
        asp_log_error("startup", "Audio init failed");
        return -1;
    }

    // Display logo
    if (!display_logo()) {
        asp_log_warn("startup", "Logo display failed, continuing with audio only");
    }

    // Play audio
    audio_play_file(AUDIO_PATH);

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
