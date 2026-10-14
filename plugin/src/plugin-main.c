/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * Module entry points. This file only loads, registers and unloads; all
 * behaviour lives in the other plugin/src modules (see docs/ARCHITECTURE.md).
 */

/* OBS_DECLARE_MODULE, OBS_MODULE_USE_DEFAULT_LOCALE, obs_module_load,
 * obs_module_unload: libobs/obs-module.h */
#include <obs-module.h>
/* LOG_INFO: libobs/util/base.h (via obs-module.h) */
#include <plugin-support.h>

#include "audio-tap.h"
#include "caption-out.h"
#include "engine-client.h"
#include "frontend.h"
#include "ui/caption-dock.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
	if (!caption_out_init())
		return false;
	if (!engine_client_init())
		return false;
	frontend_init();
	if (!audio_tap_init())
		return false;
	if (!caption_dock_create())
		obs_log(LOG_WARNING, "could not add the Captions dock");

	obs_log(LOG_INFO, "loaded (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	engine_client_shutdown();
	audio_tap_shutdown();
	frontend_shutdown();
	caption_out_shutdown();
	obs_log(LOG_INFO, "unloaded");
}
