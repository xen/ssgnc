#! /bin/sh

sh_path="../tool/ssgnc-build-index.sh"
bin_dir="../tool"
data_dir="./data"
index_dir="./index"

if [ ! -f "$sh_path" ]
then
	echo "error: no such file: $sh_path"
	exit 1
fi

if [ ! -d "$bin_dir" ]
then
	echo "error: no such directory: $bin_dir"
	exit 1
fi

if [ ! -d "$data_dir" ]
then
	echo "error: no such directory: $data_dir"
	exit 1
fi

if [ -d "$index_dir" ]
then
	rm -rf "$index_dir"
fi

"$sh_path" "$data_dir" "$index_dir" "$bin_dir"
if [ $? -ne 0 ]
then
	echo "error: failed to build index"
	exit 1
fi
