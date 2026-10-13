/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <doctest/doctest.h>

#include <string>

#include "formatter/charset-608.h"
#include "formatter/transliterate.h"
#include "formatter/utf8.h"

using namespace slc;

namespace {

std::string tr(const std::string &in)
{
	return transliterate_608(in).text;
}

} // namespace

TEST_CASE("utf8 round trip and invalid bytes")
{
	CHECK(utf8::length("caf\xC3\xA9") == 4);
	CHECK(utf8::encode(utf8::decode("a\xC3\xA9\xE2\x99\xAA")) == "a\xC3\xA9\xE2\x99\xAA");
	CHECK(utf8::decode("\xFF") == std::u32string(1, U'�'));
}

TEST_CASE("608 character set mirrors libcaption")
{
	for (char32_t c = 0x20; c < 0x7F; c++)
		CHECK(is_608_char(c));
	CHECK_FALSE(is_608_char(0x7F));
	CHECK_FALSE(is_608_char(U'\n'));
	CHECK(is_608_char(U'é'));       /* e acute */
	CHECK(is_608_char(U'Ñ'));       /* N tilde */
	CHECK(is_608_char(U'♪'));       /* eighth note */
	CHECK(is_608_char(U'ß'));       /* sharp s */
	CHECK_FALSE(is_608_char(U'Ł')); /* L stroke */
	CHECK_FALSE(is_608_char(U'ý')); /* y acute */
	CHECK_FALSE(is_608_char(U'\U0001F389'));
}

TEST_CASE("transliteration table")
{
	/* Smart punctuation always becomes ASCII (brief: smart quotes, dashes,
	 * ellipsis), even where 608 has its own glyph. */
	CHECK(tr("\xE2\x80\x98hi\xE2\x80\x99") == "'hi'");
	CHECK(tr("\xE2\x80\x9Chi\xE2\x80\x9D") == "\"hi\"");
	CHECK(tr("a\xE2\x80\x94"
		 "b") == "a--b"); /* em dash */
	CHECK(tr("a\xE2\x80\x93"
		 "b") == "a--b"); /* en dash */
	CHECK(tr("wait\xE2\x80\xA6") == "wait...");

	/* 608 extended characters are kept. */
	CHECK(tr("caf\xC3\xA9") == "caf\xC3\xA9");
	CHECK(tr("Espa\xC3\xB1"
		 "a") == "Espa\xC3\xB1"
			 "a");
	CHECK(tr("\xC3\x9C"
		 "ber") == "\xC3\x9C"
			   "ber");
	CHECK(tr("\xE2\x99\xAA la la") == "\xE2\x99\xAA la la");

	/* Latin letters without a 608 glyph fall back to the base letter. */
	CHECK(tr("\xC5\x81\xC3\xB3\x64\xC5\xBA") == "L\xC3\xB3" /* o acute kept */
						    "dz");
	CHECK(tr("\xC3\xBD") == "y");  /* y acute */
	CHECK(tr("\xC4\x8D") == "c");  /* c caron */
	CHECK(tr("\xC3\xA6") == "ae"); /* ae */
	CHECK(tr("\xC5\x93uvre") == "oeuvre");

	/* Whitespace is normalized; invisible characters vanish uncounted. */
	CHECK(tr("  a\t\nb  ") == "a b");
	CHECK(tr("a\xC2\xA0"
		 "b") == "a b"); /* no-break space */
	CHECK(tr("a\xE2\x80\x8B"
		 "b") == "ab");        /* zero width space */
	CHECK(tr("e\xCC\x81") == "e"); /* combining acute dropped */
}

TEST_CASE("unshowable characters are stripped and counted")
{
	Transliterated t = transliterate_608("gg \xF0\x9F\x8E\x89\xF0\x9F\x8E\x89 \xE4\xBD\xA0");
	CHECK(t.text == "gg");
	CHECK(t.stripped == 3);
	CHECK(transliterate_608("plain").stripped == 0);
}
