AM_CXXFLAGS = -Wall -Weffc++ -I../include

TESTS = \
	$(noinst_PROGRAMS) \
	$(noinst_SCRIPTS)

noinst_PROGRAMS = \
	test-line-reader \
	test-memory-pool

noinst_SCRIPTS = \
	test-build.sh
#	test-search.sh

test_line_reader_SOURCES = test-line-reader.cc
test_line_reader_LDADD = ../lib/libssgnc.a

test_memory_pool_SOURCES = test-memory-pool.cc
test_memory_pool_LDADD = ../lib/libssgnc.a

dist_noinst_DATA = $(noinst_SCRIPTS)

EXTRA_DIST = \
	data/1gms/1gm-0000.gz \
	data/2gms/2gm-0000.gz \
	data/3gms/3gm-0000.gz \
	data/4gms/4gm-0000.gz \
	data/5gms/5gm-0000.gz \
	test-query \
	test-answer
