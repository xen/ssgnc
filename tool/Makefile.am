AM_CXXFLAGS = -Wall -Weffc++ -DSSGNC_DEBUG -I../include

bin_PROGRAMS = \
	ssgnc-vocab-indexer \
	ssgnc-db-builder

bin_SCRIPTS = ssgnc-build-index.sh

ssgnc_vocab_indexer_SOURCES = ssgnc-vocab-indexer.cc
ssgnc_vocab_indexer_LDADD = ../lib/libssgnc.a

ssgnc_db_builder_SOURCES = ssgnc-db-builder.cc
ssgnc_db_builder_LDADD = ../lib/libssgnc.a

dist_noinst_HEADERS =

EXTRA_DIST = ${bin_SCRIPTS}
