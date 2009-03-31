ALL: indexer key-sort cgi client

indexer:
	g++ -Wall -O3 sort-vocab.cpp -o sort-vocab
	g++ -Wall -O3 build-vocab-da.cpp -o build-vocab-da
	g++ -Wall -O3 index-vocab.cpp -o index-vocab
	g++ -Wall -O3 encode-ngram.cpp -o encode-ngram
	g++ -Wall -O3 sort-ngram.cpp -o sort-ngram
	g++ -Wall -O3 index-ngram.cpp -o index-ngram
	g++ -Wall -O3 merge-index.cpp -o merge-index

key-sort:
	g++ -Wall -O3 sort-keys.cpp -o sort-keys

cgi:
	g++ -Wall -O3 cgi-main.cpp -o google-ngram

client:
	g++ -Wall -O3 simple-client.cpp -o simple-client -lboost_program_options
