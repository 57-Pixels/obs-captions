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
#include "../caption-test.h"

namespace {

constexpr const char *dock_id = "simple-local-captions-dock";

/* Preview size in caption cells: 608 rows are at most 32 characters, and
 * the core allows up to 4 rows. */
constexpr int preview_cols = 32;
constexpr int preview_rows = 4;

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

	auto *send = new QPushButton(obs_module_text("Test.Send"), this);
	auto *clear = new QPushButton(obs_module_text("Test.Clear"), this);
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
	status->setText(obs_module_text(caption_test_result_key(sent_flags)));
}

void CaptionDock::send_test_caption()
{
	caption_test_send_next();
}

void CaptionDock::clear_captions()
{
	caption_test_clear();
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
