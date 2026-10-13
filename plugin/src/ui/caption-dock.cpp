/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "caption-dock.hpp"
#include "caption-dock.h"

#include <QFontDatabase>
#include <QFontMetrics>
#include <QLabel>
#include <QMetaObject>
#include <QPushButton>
#include <QVBoxLayout>

/* obs_module_text: libobs/obs-module.h */
#include <obs-module.h>
/* obs_frontend_add_dock_by_id, obs_frontend_get_main_window:
 * UI/obs-frontend-api/obs-frontend-api.h */
#include <obs-frontend-api.h>

#include <plugin-support.h>

#include "../caption-out.h"

namespace {

constexpr const char *dock_id = "simple-local-captions-dock";

/* Locale keys of the test captions, sent in turn. */
constexpr const char *test_caption_keys[] = {
	"CaptionsDock.TestCaption.Plain",
	"CaptionsDock.TestCaption.Accents",
};
constexpr int test_caption_count = sizeof(test_caption_keys) / sizeof(test_caption_keys[0]);

/* Long enough to read; the next update may follow after this time. */
constexpr double test_display_s = 3.0;

/* Preview size in caption cells: 608 rows are at most 32 characters, and
 * the core allows up to 4 rows. */
constexpr int preview_cols = 32;
constexpr int preview_rows = 4;

const char *status_key(unsigned sent_flags)
{
	const bool stream = sent_flags & CAPTION_SENT_STREAM;
	const bool recording = sent_flags & CAPTION_SENT_RECORDING;
	if (stream && recording)
		return "CaptionsDock.Status.StreamAndRecording";
	if (stream)
		return "CaptionsDock.Status.Stream";
	if (recording)
		return "CaptionsDock.Status.Recording";
	return "CaptionsDock.Status.PreviewOnly";
}

} // namespace

CaptionDock::CaptionDock(QWidget *parent) : QWidget(parent)
{
	auto *layout = new QVBoxLayout(this);

	preview = new QLabel(this);
	QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	mono.setPointSize(mono.pointSize() + 2);
	preview->setFont(mono);
	preview->setStyleSheet("QLabel { background: black; color: white; padding: 6px; }");
	preview->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
	QFontMetrics metrics(mono);
	preview->setMinimumWidth(metrics.horizontalAdvance(QString(preview_cols, QChar('M'))) + 12);
	preview->setMinimumHeight(metrics.lineSpacing() * preview_rows + 12);
	/* Name = the caption text itself, so screen readers read the captions. */
	preview->setAccessibleDescription(obs_module_text("CaptionsDock.Preview"));

	auto *send = new QPushButton(obs_module_text("CaptionsDock.SendTest"), this);
	auto *clear = new QPushButton(obs_module_text("CaptionsDock.Clear"), this);
	status = new QLabel(obs_module_text("CaptionsDock.Status.Idle"), this);
	status->setWordWrap(true);

	layout->addWidget(preview);
	layout->addWidget(send);
	layout->addWidget(clear);
	layout->addWidget(status);
	layout->addStretch(1);

	connect(send, &QPushButton::clicked, this, &CaptionDock::send_test_caption);
	connect(clear, &QPushButton::clicked, this, &CaptionDock::clear_captions);

	caption_out_set_preview(&CaptionDock::on_caption_sent, this);
}

CaptionDock::~CaptionDock()
{
	/* Waits for an in-flight callback; events already posted to this
	 * widget are dropped by Qt once it is deleted. */
	caption_out_set_preview(nullptr, nullptr);
}

void CaptionDock::on_caption_sent(const char *text, unsigned sent_flags, void *data)
{
	/* Any thread, caption-out mutex held: copy and post to the UI thread. */
	auto *dock = static_cast<CaptionDock *>(data);
	QString copy = QString::fromUtf8(text);
	QMetaObject::invokeMethod(
		dock, [dock, copy, sent_flags]() { dock->show_caption(copy, sent_flags); }, Qt::QueuedConnection);
}

void CaptionDock::show_caption(const QString &text, unsigned sent_flags)
{
	preview->setText(text);
	status->setText(obs_module_text(status_key(sent_flags)));
}

void CaptionDock::send_test_caption()
{
	caption_out_send(obs_module_text(test_caption_keys[next_test]), test_display_s);
	obs_log(LOG_INFO, "test caption %d sent", next_test + 1);
	next_test = (next_test + 1) % test_caption_count;
}

void CaptionDock::clear_captions()
{
	caption_out_send("", 1.0);
	obs_log(LOG_INFO, "clear caption sent");
}

bool caption_dock_create(void)
{
	auto *main_window = static_cast<QWidget *>(obs_frontend_get_main_window());
	auto *dock = new CaptionDock(main_window);
	if (!obs_frontend_add_dock_by_id(dock_id, obs_module_text("CaptionsDock.Title"), dock)) {
		delete dock;
		return false;
	}
	return true;
}
