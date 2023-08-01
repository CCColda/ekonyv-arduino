#ifndef EKONYV_SEARCH_H
#define EKONYV_SEARCH_H

#include <Arduino.h>
#include <Vector.h>

class Search {
public:
	enum SearchType : uint8_t {
		VALUE,
		BETWEEN,
		LIKE,
		BINARY_AND,
		st_size,

		ANY
	};

	enum SearchRelation : uint8_t {
		AND,
		OR,
		AND_NOT,
		OR_NOT,

		sr_size
	};

	enum SearchField : uint8_t {
		S128,
		S64,
		S32,
		U8,
		U16,
		U32,
		U64,

		ft_size
	};

	struct SearchTerm {
		uint8_t header_id;
		SearchType search_type;
		SearchRelation relation;

		String statement;
	};

	using term_t = uint8_t;
	using offset_t = uint16_t;

public:
	static const char *SEARCH_TYPES[st_size];
	static const char *RELATION_TYPES[sr_size];

private:
	constexpr static const term_t TERM_INDEX_MAX = ~term_t(0);

public:
	static SearchTerm parseSearchTermFromURL(
	    const String &path,
	    uint32_t prep,
	    term_t term_index,
	    const char *headers[],
	    uint8_t num_headers);

	static term_t parseSearchTermsFromURL(
	    const String &path,
	    uint32_t prep,
	    const char *headers[],
	    uint8_t num_headers,
	    Vector<SearchTerm> &out);

	static bool match(
	    const Vector<SearchTerm> &terms,
	    const void *data,
	    const offset_t header_offsets[],
	    const SearchField header_fields[],
	    const uint8_t header_sizes[],
	    uint8_t num_headers);
};

#endif // !defined(EKONYV_SEARCH_H)