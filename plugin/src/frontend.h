/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Frontend glue: reacts to stream and recording start/stop (obs-frontend-api
 * events, UI thread). On every start it hands the current output to
 * caption-out.c (libobs frees the caption queue when an output stops, so
 * state is re-armed each time); on stop it detaches it.
 *
 * Later phases add health checks that feed plain-language status to the
 * dock, and reset the core caption pipeline on stream start.
 */

#include <stdbool.h>

bool frontend_init(void);
void frontend_shutdown(void);
