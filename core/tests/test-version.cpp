/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <doctest/doctest.h>

#include <cstring>

#include "obscap/version.h"

TEST_CASE("core reports a version")
{
	CHECK(std::strlen(obscap::core_version()) > 0);
}
