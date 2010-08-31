AM_CXXFLAGS = -Wall -Weffc++ -I../include

bin_SCRIPTS = \
	ssgnc-build.sh

bin_PROGRAMS = \
	ssgnc-db-lookup \
	ssgnc-db-merge \
	ssgnc-db-split \
	ssgnc-idx-merge \
	ssgnc-ngms-encode \
	ssgnc-ngms-merge \
	ssgnc-ngms-split \
	ssgnc-search \
	ssgnc-vocab-dic-build \
	ssgnc-vocab-dic-lookup

ssgnc_db_lookup_SOURCES = ssgnc-db-lookup.cc tools-common.cc
ssgnc_db_lookup_LDADD = ../lib/libssgnc.a

ssgnc_db_merge_SOURCES = ssgnc-db-merge.cc tools-common.cc
ssgnc_db_merge_LDADD = ../lib/libssgnc.a

ssgnc_db_split_SOURCES = ssgnc-db-split.cc tools-common.cc
ssgnc_db_split_LDADD = ../lib/libssgnc.a

ssgnc_idx_merge_SOURCES = ssgnc-idx-merge.cc tools-common.cc
ssgnc_idx_merge_LDADD = ../lib/libssgnc.a

ssgnc_ngms_encode_SOURCES = ssgnc-ngms-encode.cc tools-common.cc
ssgnc_ngms_encode_LDADD = ../lib/libssgnc.a

ssgnc_ngms_merge_SOURCES = ssgnc-ngms-merge.cc tools-common.cc
ssgnc_ngms_merge_LDADD = ../lib/libssgnc.a

ssgnc_ngms_split_SOURCES = ssgnc-ngms-split.cc tools-common.cc
ssgnc_ngms_split_LDADD = ../lib/libssgnc.a

ssgnc_search_SOURCES = ssgnc-search.cc tools-common.cc
ssgnc_search_LDADD = ../lib/libssgnc.a

ssgnc_vocab_dic_build_SOURCES = ssgnc-vocab-dic-build.cc tools-common.cc
ssgnc_vocab_dic_build_LDADD = ../lib/libssgnc.a

ssgnc_vocab_dic_lookup_SOURCES = ssgnc-vocab-dic-lookup.cc tools-common.cc
ssgnc_vocab_dic_lookup_LDADD = ../lib/libssgnc.a

EXTRA_DIST = \
	ssgnc-build.sh \
	tools-common.h

tools-common.cc: tools-common.h
