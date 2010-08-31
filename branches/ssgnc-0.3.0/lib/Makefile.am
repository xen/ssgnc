AM_CXXFLAGS = -Wall -Weffc++ -I../include

lib_LIBRARIES = libssgnc.a

libssgnc_a_SOURCES = \
	line-reader.cc \
	temp-file.cc \
	memory-pool.cc \
	vocab-indexer.cc \
	vocab-dic.cc \
	parsed-line.cc \
	parsed-line-pool.cc \
	parsed-line-reader.cc \
	line-parser.cc \
	front-encoder.cc \
	front-merger.cc \
	back-divider.cc \
	back-merger.cc

libssgnc_a_includedir = $(includedir)/ssgnc
libssgnc_a_include_HEADERS = \
	../include/ssgnc/darts.h \
	../include/ssgnc/exception.h \
	../include/ssgnc/line-reader.h \
	../include/ssgnc/temp-file.h \
	../include/ssgnc/memory-pool.h \
	../include/ssgnc/vocab-indexer.h \
	../include/ssgnc/vocab-dic.h \
	../include/ssgnc/parsed-line.h \
	../include/ssgnc/parsed-line-pool.h \
	../include/ssgnc/parsed-line-reader.h \
	../include/ssgnc/line-parser.h \
	../include/ssgnc/front-encoder.h \
	../include/ssgnc/front-merger.h \
	../include/ssgnc/back-divider.h \
	../include/ssgnc/back-merger.h
