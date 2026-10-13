/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <doctest/doctest.h>

#include <string>
#include <vector>

#include "formatter/wrap.h"

using namespace obscap;
using Lines = std::vector<std::string>;

TEST_CASE("wrap fills lines up to the column limit")
{
	CHECK(wrap_paragraph("one two three", 7) == Lines{"one two", "three"});
	CHECK(wrap_paragraph("exactly seven", 7) == Lines{"exactly", "seven"});
	CHECK(wrap_paragraph("", 10).empty());
	CHECK(wrap_paragraph("a  b", 10) == Lines{"a b"});
}

TEST_CASE("wrap counts code points, not bytes")
{
	/* 10 characters, 13 bytes: must fit a 10-column row. */
	CHECK(wrap_paragraph("\xC3\xA9\xC3\xA9\xC3\xA9 abc de", 10) == Lines{"\xC3\xA9\xC3\xA9\xC3\xA9 abc de"});
}

TEST_CASE("wrap splits words longer than a row")
{
	CHECK(wrap_paragraph("abcdefghij xy", 4) == Lines{"abcd", "efgh", "ij", "xy"});
}

TEST_CASE("wrap prefers phrase boundaries")
{
	/* Break after "now," (line stays >= 2/3 full) instead of after "we". */
	CHECK(wrap_paragraph("okay right now, we push", 20) == Lines{"okay right now,", "we push"});
	/* Boundary too early in the line: normal break. */
	CHECK(wrap_paragraph("ok, so we go to the left side", 20) == Lines{"ok, so we go to the", "left side"});
}

TEST_CASE("layout keeps the newest rows and starts paragraphs on new lines")
{
	/* "one two" / "three" / "four" -> newest two rows */
	CHECK(layout({"one two three four"}, 9, 2) == "three\nfour");
	CHECK(layout({"hello", ">> hi"}, 20, 2) == "hello\n>> hi");
	CHECK(layout({"a", "b", "c"}, 20, 2) == "b\nc");
	CHECK(layout({}, 20, 2).empty());
}

TEST_CASE("layout respects the 128-byte libobs limit")
{
	/* 4 rows of 32 two-byte characters would be 259 bytes. */
	std::string row;
	for (int i = 0; i < 32; i++)
		row += "\xC3\xA9";
	std::string text = row + " " + row + " " + row + " " + row;
	std::string out = layout({text}, 32, 4);
	CHECK(out.size() <= caption_max_bytes);
	CHECK(out == row); /* only the newest row fits */
}
