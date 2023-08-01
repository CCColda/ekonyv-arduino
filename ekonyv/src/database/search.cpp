#include "search.h"

#include "../middleware/parameter.mw.h"

#include "../string/string.h"
#include "../string/to_string.h"
#include "../string/url.h"

#include <Regexp.h>

namespace {
struct Between {
	uint64_t low;
	uint64_t high;
};

Between parseBetween(const String &str)
{
	const auto dash = str.indexOf('-');
	if (dash == -1)
		return Between{0, 0};

	return Between{
	    Str::fixedAtoi<uint64_t>(str.c_str(), dash),
	    Str::fixedAtoi<uint64_t>(str.c_str() + dash + 1, str.length() - dash - 1)};
}
} // namespace

/* static */ const char *Search::SEARCH_TYPES[st_size] = {
    "v",
    "btw",
    "like",
    "bin_and"};

/* static */ const char *Search::RELATION_TYPES[sr_size] = {
    "and",
    "or",
    "and_not",
    "or_not"};

/* static */ Search::SearchTerm Search::parseSearchTermFromURL(
    const String &path,
    uint32_t prep,
    term_t term_index,
    const char *headers[],
    uint8_t num_headers)
{
	auto result = SearchTerm{};

	const auto search_param = ParameterMiddleware(str('s', term_index), path, prep);

	const auto header_index = Str::compareToMap(
	    search_param.value.c_str(), search_param.value.length(),
	    headers, num_headers);

	if (header_index == Str::NOT_FOUND)
		return result;

	result.header_id = (uint8_t)header_index;
	result.search_type = SearchType::ANY;
	result.statement = "";
	result.relation = SearchRelation::AND;

	for (uint8_t i = 0; i < st_size; ++i) {
		const auto search_param = ParameterMiddleware(str(SEARCH_TYPES[i], term_index), path, prep);
		if (search_param) {
			result.search_type = (SearchType)i;
			result.statement = Str::urlDecode(search_param.value.c_str(), search_param.value.length());
			break;
		}
	}

	const auto relation_param = ParameterMiddleware(str('r', term_index), path, prep);

	if (relation_param) {
		const auto relation_index = Str::compareToMap(
		    relation_param.value.c_str(), relation_param.value.length(),
		    RELATION_TYPES, sr_size);

		if (relation_index != Str::NOT_FOUND)
			result.relation = (SearchRelation)relation_index;
	}

	return result;
}

/* static */ Search::term_t Search::parseSearchTermsFromURL(
    const String &path,
    uint32_t prep,
    const char *headers[],
    uint8_t num_headers,
    Vector<SearchTerm> &out)
{
	SearchTerm latest_search = {};

	term_t term_index = 0;
	do {
		latest_search = parseSearchTermFromURL(path, prep, term_index, headers, num_headers);

		if (latest_search.search_type == SearchType::ANY)
			break;

		++term_index;

		out.push_back(latest_search);
	} while (term_index != TERM_INDEX_MAX && !out.full());

	return term_index;
}

/* static */ bool Search::match(
    const Vector<SearchTerm> &terms,
    const void *data,
    const offset_t header_offsets[],
    const SearchField header_fields[],
    const uint8_t header_sizes[],
    uint8_t num_headers)
{
	bool match = true;

	for (uint8_t i = 0; i < terms.size(); ++i) {
		const auto &term = terms.at(i);
		bool term_match = false;

		if (term.header_id >= num_headers)
			continue;

		auto *const header_address = ((byte *)data) + header_offsets[term.header_id];
		const auto field_type = header_fields[term.header_id];

		switch (field_type) {
			case S128:
			case S64:
			case S32: {
				const auto length = header_sizes[term.header_id];

				if (term.search_type == VALUE) {
					if (term.statement.length() == length) {
						term_match = Str::compare((char *)header_address, term.statement.c_str(), length);
					}
				}
				else if (term.search_type == LIKE) {
					Serial.println("like");
					Serial.println(term.statement);
					auto ms = MatchState();
					ms.Target((char *)header_address, length);

					term_match = ms.Match(term.statement.c_str(), 0) == REGEXP_MATCHED;
				}

				break;
			}
			case U64:
			case U32:
			case U16:
			case U8: {
				const uint64_t field_value =
				    field_type == U8
				        ? *(uint8_t *)header_address
				        : (field_type == U16
				               ? *(uint16_t *)header_address
				               : (field_type == U32
				                      ? *(uint32_t *)header_address
				                      : *(uint64_t *)header_address));

				if (term.search_type == VALUE) {
					const uint64_t value = Str::fixedAtoi<uint64_t>(term.statement.c_str(), term.statement.length());
					term_match = value == field_value;
				}
				else if (term.search_type == BETWEEN) {
					const Between btw = parseBetween(term.statement);
					term_match = btw.low <= field_value && field_value <= btw.high;
				}
				else if (term.search_type == BINARY_AND) {
					term_match = (field_value & Str::fixedAtoi<uint8_t>(term.statement.c_str(), term.statement.length())) != 0;
				}

				break;
			}
		}

		switch (term.relation) {
			case AND: {
				match = match && term_match;
				break;
			}
			case AND_NOT: {
				match = match && !term_match;
				break;
			}
			case OR: {
				match = match || term_match;
				break;
			}
			case OR_NOT: {
				match = match || !term_match;
				break;
			}
		}
	}

	return match;
}