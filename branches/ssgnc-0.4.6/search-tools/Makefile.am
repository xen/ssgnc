AM_CXXFLAGS = -Wall -Weffc++ -I../include

bin_PROGRAMS = \
	ssgnc-predict \
	ssgnc-search \
	ssgnc-vocab-dic-lookup

ssgnc_predict_SOURCES = ssgnc-predict.cc
ssgnc_predict_LDADD = ../lib/libssgnc.a

ssgnc_search_SOURCES = ssgnc-search.cc
ssgnc_search_LDADD = ../lib/libssgnc.a

ssgnc_vocab_dic_lookup_SOURCES = ssgnc-vocab-dic-lookup.cc
ssgnc_vocab_dic_lookup_LDADD = ../lib/libssgnc.a
