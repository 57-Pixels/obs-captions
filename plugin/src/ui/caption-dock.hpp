/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

/*
 * Caption dock (Qt). Created and touched only on the Qt UI thread. All
 * user-facing strings come from data/locale via obs_module_text.
 *
 * Stub: not registered with OBS until roadmap phase 2.
 */

#include <QWidget>

class CaptionDock : public QWidget {
public:
	explicit CaptionDock(QWidget *parent = nullptr);
};
