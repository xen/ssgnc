#! /bin/sh

## Shows the usage of this script if the number of given arguments is wrong.
if [ $# -lt 2 -o $# -gt 4 ]
then
	echo "Usage: $0 DataDir IndexDir [BinDir] [TempDir]"
	exit 1
fi

## Names command line arguments.
input_dir=`dirname "$1/input"`
output_dir=`dirname "$2/output"`
bin_dir=""
temp_dir=""
front_file=""
back_file=""

## The 3th argument specifies a directory where binary files exist.
if [ $# -ge 3 ]
then
	bin_dir="`dirname "$3/bin"`/"
	if [ ! -d "$bin_dir" ]
	then
		echo "error: no such directory: $bin_dir"
		exit 1
	fi
else
	if [ -x "./ssgnc-db-builder" ]
	then
		bin_dir="./"
	elif [ -x "tool/ssgnc-db-builder" ]
	then
		bin_dir="tool/"
	fi
fi

## The 3rd argument specifies a directory where temporary files will be made.
if [ $# -ge 4 ]
then
	temp_dir="$4"
	if [ ! -d "$temp_dir" ]
	then
		echo "error: no such directory: $temp_dir"
		exit 1
	fi
	front_file="`dirname "$temp_dir/front"`/ssgnc-front-file"
	back_file="`dirname "$temp_dir/back"`/ssgnc-back-file"
fi

date

## Shows I/O directories.
echo "Data directory (Input): $input_dir"
echo "Index directory (Output): $output_dir"
echo "Binary directory: $bin_dir"
echo "Temporary directory: $temp_dir"
echo

## Checks if the input directory exists or not.
if [ ! -d "$input_dir" ]
then
	echo "error: no such directory: $input_dir"
	exit 1
fi

## Checks if the output directory exists or not.
if [ ! -d "$output_dir" ]
then
	## Creates the output directory if it does not exist.
	echo "warning: no such directory: $output_dir"
	echo "creating directory $output_dir ..."
	mkdir "$output_dir"
	if [ $? -ne 0 ]
	then
		exit 1
	fi
	echo "done!"
	echo
fi

## Generates vocabulary paths.
if [ -f "$input_dir/1gms/1gm-0000.gz" ]
then
	vocab_path="$input_dir/1gms/1gm-0000.gz"
elif [ -f "$input_dir/1gms/vocab.gz" ]
then
	## vocab.gz is used if there is not 1gm-0000.gz
	vocab_path="$input_dir/1gms/vocab.gz"
else
	echo "error: 1gm-0000.gz and vocab.gz are not found"
	exit 1
fi

## Builds a dictionary and an index from the vocabulary file.
vocab_dic_path="$output_dir/vocab.dic"
vocab_index_path="$output_dir/vocab.idx"
if [ -f "$vocab_dic_path" ]
then
	echo "$vocab_dic_path: already exists"
else
	echo "creating $vocab_dic_path ..."
	gzip -cd "$vocab_path" | \
		"$bin_dir"ssgnc-vocab-indexer "$vocab_dic_path" "$vocab_index_path"
	if [ $? -ne 0 ]
	then
		exit 1
	fi
	echo "done!"
fi
echo

## Builds an inverted 1-gram database.
db_path="$output_dir/1gms.db"
if [ -f "$db_path" ]
then
	echo "$db_path: already exists"
else
	echo "creating $db_path ..."
	gzip -cd "$vocab_path" | \
		"$bin_dir"ssgnc-db-builder "$vocab_dic_path" > "$db_path"
	if [ $? -ne 0 ]
	then
		exit 1
	fi
	echo "done!"
fi
echo

## Builds inverted n-gram databases.
n=2
while [ -d "$input_dir/${n}gms" ]
do
	db_path="$output_dir/${n}gms.db"
	if [ -f "$db_path" ]
	then
		echo "$db_path: already exists"
	else
		echo "creating $db_path ..."
		gzip -cd "$input_dir/${n}gms/${n}gm-0"* | \
			"$bin_dir"ssgnc-db-builder "$vocab_dic_path" > "$db_path"
		if [ $? -ne 0 ]
		then
			exit 1
		fi
		echo "done!"
	fi
	echo
	n=`expr $n + 1`
done

date
