/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/*
 * Test captions, shared by the filter properties and the dock (UI thread
 * only). Texts come from data/locale and are sent in turn: plain text, then
 * CEA-608 accented characters and symbols.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Both return caption_out_send's CAPTION_SENT_* flags. */
unsigned caption_test_send_next(void);
unsigned caption_test_clear(void);

/* Locale key of a plain-language message saying where a caption went. */
const char *caption_test_result_key(unsigned sent_flags);

#ifdef __cplusplus
}
#endif
