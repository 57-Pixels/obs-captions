/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <doctest/doctest.h>

#include <cstring>

#include "slc/version.h"

TEST_CASE("core reports a version")
{
	CHECK(std::strlen(slc::core_version()) > 0);
}
