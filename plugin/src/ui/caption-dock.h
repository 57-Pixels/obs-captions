/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/* C entry point for the Qt caption dock (UI thread only). */

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Creates the dock and registers it with OBS, which owns it from then on
 * (it is deleted with the main window). */
bool caption_dock_create(void);

#ifdef __cplusplus
}
#endif
