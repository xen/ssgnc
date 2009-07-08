AM_CXXFLAGS = -Wall -Weffc++

bin_PROGRAMS = \
	ssgnc-freq-to-id \
	ssgnc-build-vocab-dic \
	ssgnc-build-vocab-index \
	ssgnc-encode-text \
	ssgnc-sort-data \
	ssgnc-build-db \
	ssgnc-search

ssgnc_freq_to_id_SOURCES = freq-to-id.cc
ssgnc_build_vocab_dic_SOURCES = build-vocab-dic.cc
ssgnc_build_vocab_index_SOURCES = build-vocab-index.cc
ssgnc_encode_text_SOURCES = encode-text.cc
ssgnc_sort_data_SOURCES = sort-data.cc
ssgnc_build_db_SOURCES = build-db.cc
ssgnc_search_SOURCES = search.cc

ssgnc_includedir = $(includedir)/ssgnc
ssgnc_include_HEADERS = \
	ssgnc/byte-reader.h \
	ssgnc/byte-writer.h \
	ssgnc/data-divider.h \
	ssgnc/data-merger.h \
	ssgnc/db-divider.h \
	ssgnc/db-filter.h \
	ssgnc/db-merger.h \
	ssgnc/db-reader.h \
	ssgnc/fd-streambuf.h \
	ssgnc/file-mapper.h \
	ssgnc/inverted-db.h \
	ssgnc/line-reader.h \
	ssgnc/mem-streambuf.h \
	ssgnc/ngram.h \
	ssgnc/path-generator.h \
	ssgnc/query.h \
	ssgnc/tempfile-manager.h \
	ssgnc/text-encoder.h \
	ssgnc/timer.h \
	ssgnc/unified-db.h \
	ssgnc/unified-reader.h \
	ssgnc/varint-reader.h \
	ssgnc/varint-writer.h \
	ssgnc/vocab-dic.h \
	ssgnc/vocab-index.h \
	ssgnc/vocab-pair.h
