#! /bin/sh

cd ..

index_dir="./test/index"
bin_path="./src/ssgnc-search"
query_path="./test/test-query"
result_path="./test/test-result"
answer_path="./test/test-answer"

if [ ! -f "$bin_path" ]
then
	echo "error: no such file: $bin_path"
	exit 1
fi

"$bin_path" -d "$index_dir" < "$query_path" > "$result_path"
if [ $? -ne 0 ]
then
	echo "error: failed to build index"
	exit 1
fi

cmp "$result_path" "$answer_path"
if [ $? -ne 0 ]
then
	echo "error: wrong result"
	exit 1
fi

rm -f "$result_path"
