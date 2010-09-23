AM_CXXFLAGS = -Wall -Weffc++ -I../include

lib_LIBRARIES = libssgnc.a

libssgnc_a_SOURCES = \
	agent.cc \
	byte-reader.cc \
	common.cc \
	database.cc \
	file-map.cc \
	file-path.cc \
	mapper.cc \
	mem-pool.cc \
	ngram-index.cc \
	ngram-reader.cc \
	query.cc \
	reader.cc \
	string-builder.cc \
	vocab-dic.cc \
	writer.cc

include_HEADERS = \
	../include/ssgnc.h

libssgnc_a_includedir = $(includedir)/ssgnc
libssgnc_a_include_HEADERS = \
	../include/ssgnc/agent.h \
	../include/ssgnc/byte-reader.h \
	../include/ssgnc/common.h \
	../include/ssgnc/database.h \
	../include/ssgnc/file-map.h \
	../include/ssgnc/file-path.h \
	../include/ssgnc/freq-handler.h \
	../include/ssgnc/heap-queue.h \
	../include/ssgnc/mapper.h \
	../include/ssgnc/mem-pool.h \
	../include/ssgnc/ngram-index.h \
	../include/ssgnc/ngram-reader.h \
	../include/ssgnc/query.h \
	../include/ssgnc/reader.h \
	../include/ssgnc/string.h \
	../include/ssgnc/string-builder.h \
	../include/ssgnc/string-hash.h \
	../include/ssgnc/vocab-dic.h \
	../include/ssgnc/writer.h
