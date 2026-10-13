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
 * Contents:
 * - Preview: shows every caption caption-out.c sends, laid out as the CC
 *   rows viewers see, even when nothing is streaming or recording.
 * - "Send test caption": cycles through test captions (plain text, CEA-608
 *   accented characters and symbols).
 * - "Clear captions": sends an empty caption.
 * - Status: where the last caption went (preview only, stream, recording).
 */

#include <QString>
#include <QWidget>

class QLabel;

class CaptionDock : public QWidget {
public:
	explicit CaptionDock(QWidget *parent = nullptr);
	~CaptionDock() override;

private:
	static void on_caption_sent(const char *text, unsigned sent_flags, void *data);
	void show_caption(const QString &text, unsigned sent_flags);
	void send_test_caption();
	void clear_captions();

	QLabel *preview;
	QLabel *status;
	int next_test = 0;
};
