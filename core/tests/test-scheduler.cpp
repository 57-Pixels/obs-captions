/*
 * simple-local-captions
 * SPDX-FileCopyrightText: 2026 simple-local-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <doctest/doctest.h>

#include "slc/config.h"
#include "slc/pipeline.h"
#include "scheduler/scheduler.h"

using namespace slc;

TEST_CASE("stable_words keeps whole words only")
{
	CHECK(stable_words("we need to", 7) == "we need");
	CHECK(stable_words("we need to", 5) == "we");
	CHECK(stable_words("we need to", 0) == "");
	CHECK(stable_words("we need to", 99) == "we need to");
	CHECK(stable_words("we need", 3) == "we");
}

TEST_CASE("updates carry display_duration equal to the interval")
{
	Config cfg;
	cfg.scheduler.min_interval_ms = 1500;
	Scheduler s(cfg.formatter, cfg.scheduler);
	Stats stats;
	s.commit("hi", "", 100);
	auto u = s.tick(100, stats);
	REQUIRE(u.has_value());
	CHECK(u->text == "hi");
	CHECK(u->display_duration_ms == 1500);
	CHECK(stats.emitted == 1);
}

TEST_CASE("lag estimate follows the newest text on screen")
{
	Config cfg;
	Scheduler s(cfg.formatter, cfg.scheduler);
	Stats stats;
	s.commit("hello", "", 1000);
	s.tick(1300, stats);
	CHECK(stats.lag_ms == 300);
	s.tick(1800, stats);
	CHECK(stats.lag_ms == 800);
	s.tick(5000, stats); /* cleared */
	CHECK(stats.lag_ms == 0);
}

TEST_CASE("nothing is emitted when the text is unchanged")
{
	Config cfg;
	Scheduler s(cfg.formatter, cfg.scheduler);
	Stats stats;
	s.commit("hello", "", 1000);
	CHECK(s.tick(1000, stats).has_value());
	CHECK_FALSE(s.tick(2500, stats).has_value());
	CHECK_FALSE(s.tick(3000, stats).has_value());
}

TEST_CASE("sanitize clamps configuration")
{
	Config c;
	c.formatter.cols = 99;
	c.formatter.rows = 0;
	c.scheduler.min_interval_ms = 1;
	c.scheduler.clear_after_ms = 10;
	c.filters.repeat_threshold = 1;
	Config s = sanitize(c);
	CHECK(s.formatter.cols == max_cols);
	CHECK(s.formatter.rows == min_rows);
	CHECK(s.scheduler.min_interval_ms == min_interval_floor_ms);
	CHECK(s.scheduler.clear_after_ms >= s.scheduler.min_interval_ms);
	CHECK(s.filters.repeat_threshold == 2);
}

TEST_CASE("reset forgets captions but keeps stats")
{
	CaptionPipeline p;
	Hypothesis h;
	h.kind = HypothesisKind::final;
	h.text = "hello";
	h.t_start_ms = 0;
	h.t_end_ms = 500;
	p.push(h);
	REQUIRE(p.tick(500).has_value());
	p.reset();
	CHECK(p.stats().emitted == 1);
	/* After a reset (new stream) the first text is sent immediately. */
	h.text = "again";
	h.t_start_ms = 600;
	h.t_end_ms = 700;
	p.push(h);
	auto u = p.tick(700);
	REQUIRE(u.has_value());
	CHECK(u->text == "again");
}
