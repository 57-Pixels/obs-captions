/*
 * obs-captions
 * SPDX-FileCopyrightText: 2026 obs-captions contributors
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "formatter/wrap.h"

#include "formatter/utf8.h"

namespace obscap {

namespace {

struct Word {
	std::string text;
	size_t len; /* code points */
};

std::vector<Word> split_words(std::string_view text)
{
	std::vector<Word> words;
	size_t i = 0;
	while (i < text.size()) {
		size_t end = text.find(' ', i);
		if (end == std::string_view::npos)
			end = text.size();
		if (end > i) {
			std::string w(text.substr(i, end - i));
			size_t len = utf8::length(w);
			words.push_back({std::move(w), len});
		}
		i = end + 1;
	}
	return words;
}

std::string join(const std::vector<Word> &words, size_t from, size_t to)
{
	std::string out;
	for (size_t i = from; i < to; i++) {
		if (i > from)
			out += ' ';
		out += words[i].text;
	}
	return out;
}

size_t span_len(const std::vector<Word> &words, size_t from, size_t to)
{
	size_t len = 0;
	for (size_t i = from; i < to; i++)
		len += words[i].len + (i > from ? 1 : 0);
	return len;
}

bool ends_phrase(const std::string &word)
{
	char last = word.back();
	return last == ',' || last == '.' || last == ';' || last == ':' || last == '?' || last == '!';
}

/*
 * Index of the word after which to break the current line instead of at its
 * end, or 0 if there is no usable phrase boundary (index 0 would leave the
 * tail on the next line, which is never wanted for a single word anyway).
 */
size_t phrase_break(const std::vector<Word> &line, size_t next_len, size_t cols)
{
	for (size_t i = line.size() - 1; i-- > 0;) {
		if (!ends_phrase(line[i].text))
			continue;
		size_t head = span_len(line, 0, i + 1);
		size_t tail = span_len(line, i + 1, line.size());
		if (head * 3 >= cols * 2 && tail + 1 + next_len <= cols)
			return i + 1;
		return 0; /* only the last boundary is considered */
	}
	return 0;
}

/* Splits a word longer than cols into cols-wide pieces; returns the rest. */
Word split_long(Word word, size_t cols, std::vector<std::string> &lines)
{
	std::u32string cps = utf8::decode(word.text);
	size_t pos = 0;
	while (cps.size() - pos > cols) {
		lines.push_back(utf8::encode(cps.substr(pos, cols)));
		pos += cols;
	}
	std::string rest = utf8::encode(cps.substr(pos));
	return {rest, cps.size() - pos};
}

} // namespace

std::vector<std::string> wrap_paragraph(std::string_view text, int cols_in)
{
	const size_t cols = cols_in > 0 ? static_cast<size_t>(cols_in) : 1;
	std::vector<std::string> lines;
	std::vector<Word> line;
	size_t line_len = 0;

	for (Word word : split_words(text)) {
		if (word.len > cols) {
			if (!line.empty())
				lines.push_back(join(line, 0, line.size()));
			line.clear();
			word = split_long(word, cols, lines);
		}
		if (line.empty()) {
			line_len = word.len;
			line.push_back(word);
			continue;
		}
		if (line_len + 1 + word.len <= cols) {
			line_len += 1 + word.len;
			line.push_back(word);
			continue;
		}

		size_t brk = phrase_break(line, word.len, cols);
		if (brk == 0)
			brk = line.size();
		lines.push_back(join(line, 0, brk));
		line.erase(line.begin(), line.begin() + static_cast<std::ptrdiff_t>(brk));
		line.push_back(word);
		line_len = span_len(line, 0, line.size());
	}
	if (!line.empty())
		lines.push_back(join(line, 0, line.size()));
	return lines;
}

std::string layout(const std::vector<std::string> &paragraphs, int cols, int rows, size_t max_bytes)
{
	std::vector<std::string> lines;
	for (const auto &p : paragraphs)
		for (auto &l : wrap_paragraph(p, cols))
			lines.push_back(std::move(l));

	size_t keep = rows > 0 ? static_cast<size_t>(rows) : 1;
	size_t first = lines.size() > keep ? lines.size() - keep : 0;

	auto joined = [&](size_t from) {
		std::string out;
		for (size_t i = from; i < lines.size(); i++) {
			if (i > from)
				out += '\n';
			out += lines[i];
		}
		return out;
	};

	std::string out = joined(first);
	while (out.size() > max_bytes && first + 1 < lines.size())
		out = joined(++first);
	return out;
}

} // namespace obscap
