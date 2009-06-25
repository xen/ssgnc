AM_CXXFLAGS = -Wall -Weffc++

noinst_bindir = .
noinst_bin_PROGRAMS = \
	ssgnc-freq-to-id ssgnc-build-dic ssgnc-build-vocab-index \
	ssgnc-encode-text ssgnc-build-index ssgnc-merge-index ssgnc-find

ssgnc_freq_to_id_SOURCES = src/freq-to-id.cc
ssgnc_build_dic_SOURCES = src/build-dic.cc
ssgnc_build_vocab_index_SOURCES = src/build-vocab-index.cc
ssgnc_encode_text_SOURCES = src/encode-text.cc
ssgnc_build_index_SOURCES = src/build-index.cc
ssgnc_merge_index_SOURCES = src/merge-index.cc
ssgnc_find_SOURCES = src/ssgnc-find.cc

EXTRA_DIST = ssgnc-index.sh

noinst_HEADERS = \
	src/ssgnc/byte-reader.h \
	src/ssgnc/byte-writer.h \
	src/ssgnc/encoder.h \
	src/ssgnc/file-mapper.h \
	src/ssgnc/index-finder.h \
	src/ssgnc/index-reader.h \
	src/ssgnc/indexer.h \
	src/ssgnc/line-reader.h \
	src/ssgnc/mem-streambuf.h \
	src/ssgnc/merger.h \
	src/ssgnc/ngram.h \
	src/ssgnc/path-generator.h\
	src/ssgnc/text-finder.h \
	src/ssgnc/timer.h \
	src/ssgnc/varint-reader.h \
	src/ssgnc/varint-writer.h \
	src/ssgnc/vocab-dic.h \
	src/ssgnc/vocab-index.h
