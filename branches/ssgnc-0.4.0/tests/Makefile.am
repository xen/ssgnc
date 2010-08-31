AM_CXXFLAGS = -Wall -Weffc++ -lstdc++ -I../include

TESTS = \
	test-byte-reader \
	test-file-map \
	test-file-path \
	test-freq-handler \
	test-heap-queue \
	test-int-types \
	test-mem-pool \
	test-string \
	test-string-builder \
	test-vocab-dic

noinst_PROGRAMS = $(TESTS)

test_byte_reader_SOURCES = test-byte-reader.cc
test_byte_reader_LDADD = ../lib/libssgnc.a

test_file_map_SOURCES = test-file-map.cc
test_file_map_LDADD = ../lib/libssgnc.a

test_file_path_SOURCES = test-file-path.cc
test_file_path_LDADD = ../lib/libssgnc.a

test_freq_handler_SOURCES = test-freq-handler.cc
test_freq_handler_LDADD = ../lib/libssgnc.a

test_heap_queue_SOURCES = test-heap-queue.cc
test_heap_queue_LDADD = ../lib/libssgnc.a

test_int_types_SOURCES = test-int-types.cc
test_int_types_LDADD = ../lib/libssgnc.a

test_mem_pool_SOURCES = test-mem-pool.cc
test_mem_pool_LDADD = ../lib/libssgnc.a

test_string_SOURCES = test-string.cc
test_string_LDADD = ../lib/libssgnc.a

test_string_builder_SOURCES = test-string-builder.cc
test_string_builder_LDADD = ../lib/libssgnc.a

test_vocab_dic_SOURCES = test-vocab-dic.cc
test_vocab_dic_LDADD = ../lib/libssgnc.a
