AM_CXXFLAGS = -Wall -Weffc++

noinst_bindir = .
noinst_bin_PROGRAMS = \
	ssgnc-build-dic ssgnc-build-index ssgnc-encode-index ssgnc-find

ssgnc_build_dic_SOURCES = src/build-dic.cc
ssgnc_build_index_SOURCES = src/build-index.cc
ssgnc_encode_index_SOURCES = src/encode-index.cc
ssgnc_find_SOURCES = src/ssgnc-find.cc

noinstdir = .
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
	src/ssgnc/path-generator.h \
	src/ssgnc/text-finder.h \
	src/ssgnc/timer.h \
	src/ssgnc/varint-reader.h \
	src/ssgnc/varint-writer.h \
	src/ssgnc/vocab-dic.h

EXTRA_DIST = ssgnc-index.sh
