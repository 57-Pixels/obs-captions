/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
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

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "loaded (version %s)", PLUGIN_VERSION);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "unloaded");
}
