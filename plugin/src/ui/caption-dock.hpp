/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Caption dock (Qt). Created and touched only on the Qt UI thread. All
 * user-facing strings come from data/locale via obs_module_text.
 *
 * Phase 2 contents: "Send test caption" (cycles through test captions that
 * check plain text and CEA-608 accented characters on Twitch) and "Clear
 * captions" (checks whether an empty caption clears the screen).
 */

#include <QWidget>

class QLabel;

class CaptionDock : public QWidget {
public:
	explicit CaptionDock(QWidget *parent = nullptr);

private:
	void send_test_caption();
	void clear_captions();
	void show_result(bool sent, const char *sent_key);

	QLabel *status;
	int next_test = 0;
};
