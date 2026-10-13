/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "caption-dock.hpp"
#include "caption-dock.h"

#include <QLabel>
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

} // namespace

CaptionDock::CaptionDock(QWidget *parent) : QWidget(parent)
{
	auto *layout = new QVBoxLayout(this);

	auto *send = new QPushButton(obs_module_text("CaptionsDock.SendTest"), this);
	auto *clear = new QPushButton(obs_module_text("CaptionsDock.Clear"), this);
	status = new QLabel(obs_module_text("CaptionsDock.Status.Idle"), this);
	status->setWordWrap(true);

	layout->addWidget(send);
	layout->addWidget(clear);
	layout->addWidget(status);
	layout->addStretch(1);

	connect(send, &QPushButton::clicked, this, &CaptionDock::send_test_caption);
	connect(clear, &QPushButton::clicked, this, &CaptionDock::clear_captions);
}

void CaptionDock::send_test_caption()
{
	const char *text = obs_module_text(test_caption_keys[next_test]);
	bool sent = caption_out_send(text, test_display_s) == CAPTION_OUT_SENT;
	if (sent) {
		obs_log(LOG_INFO, "test caption %d sent", next_test + 1);
		next_test = (next_test + 1) % test_caption_count;
	}
	show_result(sent, "CaptionsDock.Status.Sent");
}

void CaptionDock::clear_captions()
{
	bool sent = caption_out_send("", 1.0) == CAPTION_OUT_SENT;
	if (sent)
		obs_log(LOG_INFO, "clear caption sent");
	show_result(sent, "CaptionsDock.Status.Cleared");
}

void CaptionDock::show_result(bool sent, const char *sent_key)
{
	status->setText(obs_module_text(sent ? sent_key : "CaptionsDock.Status.NotStreaming"));
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
