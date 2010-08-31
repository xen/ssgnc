AM_CXXFLAGS = -Wall -Weffc++ -I../include

lib_LIBRARIES = libssgnc.a

libssgnc_a_SOURCES = \
	file-map.cc \
	vocab-dic.cc

include_HEADERS = \
	../include/ssgnc.h

libssgnc_a_includedir = $(includedir)/wc
libssgnc_a_include_HEADERS = \
	../include/ssgnc/byte-reader.h \
	../include/ssgnc/file-map.h \
	../include/ssgnc/file-path.h \
	../include/ssgnc/freq-handler.h \
	../include/ssgnc/heap-queue.h \
	../include/ssgnc/int-types.h \
	../include/ssgnc/mem-pool.h \
	../include/ssgnc/string.h \
	../include/ssgnc/string-builder.h \
	../include/ssgnc/string-hash.h \
	../include/ssgnc/vocab-dic.h
