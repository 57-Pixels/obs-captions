# core/: caption library

Pure C++17, no OBS dependency, deterministic (time is passed in). The plugin
links it statically; tests build it standalone:

```
cmake -S core -B build_core
cmake --build build_core --config Debug
ctest --test-dir build_core -C Debug --output-on-failure
```

## Flow

```
Hypothesis (partial/final) --+
                             v
        word replacement -> hallucination filter (off by default)
                             v
     in-utterance repeat collapse -> repeat filter -> profanity mask
                             v
             transliteration to the CEA-608 set (stripped count)
                             v
                           scheduler: latest-wins slot, rolling window,
                           silence clear, ">> " speaker marks
                             v
                 tick(now) -> optional CaptionUpdate{text, display_duration}
```

| Where | What |
|-------|------|
| `include/slc/` | Public API: `CaptionPipeline`, events, `Config`, `Stats` |
| `formatter/` | UTF-8, 608 character set, transliteration, wrapping |
| `filters/` | Word list, hallucination, repeat, profanity |
| `scheduler/` | Latest-wins scheduler |
| `tests/golden/` | Event script -> expected caption updates (format in `tests/test-golden.cpp`) |

Every rule is documented in the header of the module that implements it.

## Defaults

| Setting | Default | Range / note |
|---------|---------|--------------|
| `cols` | 28 | 8-32 (32 = CEA-608 row) |
| `rows` | 2 | 1-4 (4 also keeps text within libobs' 128 bytes) |
| `min_interval_ms` | 1000 | >= 100; also the `display_duration` sent to OBS |
| `clear_after_ms` | 4000 | >= `min_interval_ms`, from the newest text's end time |
| `show_stable_partial` | off | committed text only |
| `repeat_threshold` | 3 | >= 2; the 3rd near-duplicate final in a row is dropped; exact re-sends are dropped at the 2nd copy |
| `repeat_window_ms` | 10000 | gap that ends a repeat run |
| `near_duplicate_ratio` | 0.2 | normalized edit distance |
| `hallucination_filter` | off | for Whisper-style backends; see `docs/OPEN_QUESTIONS.md` |
| `hallucination_max_words` | 8 | long enough for subtitle-credit phrases |
| `hallucination_isolation_ms` | 2000 | silence needed before and after |
| `word_replacements` | none | case-insensitive, whole word |
| `profanity_mask` | off | English default list |

## Regenerating the 608 tables

`formatter/charset-608-tables.h` is generated from OBS's libcaption so the
formatter never emits a character OBS cannot encode:

```
python core/formatter/gen-608-tables.py .deps/obs-studio-30.2.3/deps/libcaption/src/eia608_from_utf8.re2c
```
