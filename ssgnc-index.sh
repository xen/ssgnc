#! /bin/sh

date

## Shows usage if passed arguments are invalid.
if [ $# -lt 2 -o $# -gt 4 ]
then
	echo "Usage: $0 InputDir OutputDir [MemSize] [TempDir]"
	exit 1
fi

## Names command line arguments.
input_dir=`dirname "$1/input"`
output_dir=`dirname "$2/output"`
mem_size=512
temp_dir="/tmp"

## The 3rd argument: The available memory size.
if [ $# -ge 3 ]
then
	mem_size="$3"
	if [ $mem_size -lt 512 ]
	then
		echo "error: memory size must be no less than 512: $mem_size"
		exit 1
	fi
fi

## The 4th argument: The temporary directory
if [ $# -ge 4 ]
then
	temp_dir="$4"
	if [ ! -d $temp_dir ]
	then
		echo "error: no such directory: $temp_dir"
		exit 1
	fi
fi

## Shows I/O directories.
echo "Input directory: $input_dir"
echo "Output directory: $output_dir"
echo "Memory size: ${mem_size}MB"
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
	## Creates the output directory if not exists.
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
	vocab_path="$input_dir/1gms/vocab.gz"
else
	echo "error: 1gm-0000.gz and vocab.gz are not found"
	exit 1
fi

## Builds a vocabulary dictionary.
vocab_dic_path="$output_dir/vocab.dic"
if [ -f "$vocab_dic_path" ]
then
	echo "$vocab_dic_path: already exists"
else
	echo "creating $vocab_dic_path ..."
	gzip -cd "$vocab_path" | \
		LANG=C sort -S "$mem_size" -T "$temp_dir" -srnk 2 | \
		src/ssgnc-freq-to-id | \
		LANG=C sort -S "$mem_size" -T "$temp_dir" | \
		src/ssgnc-build-vocab-dic > "$vocab_dic_path"
	if [ $? -ne 0 ]
	then
		exit 1
	fi
	echo "done!"
fi
echo

## Builds a vocabulary index.
vocab_index_path="$output_dir/vocab.idx"
if [ -f "$vocab_index_path" ]
then
	echo "$vocab_index_path: already exists"
else
	echo "creating $vocab_index_path ..."
	gzip -cd "$vocab_path" | \
		LANG=C sort -S "$mem_size" -T "$temp_dir" -srnk 2 | \
		src/ssgnc-build-vocab-index > "$vocab_index_path"
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
		src/ssgnc-encode-text "$vocab_dic_path" | \
		src/ssgnc-sort-data 1 "$mem_size" "$temp_dir" | \
		src/ssgnc-build-db 1 "$mem_size" "$temp_dir" > "$db_path"
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
			src/ssgnc-encode-text "$vocab_dic_path" | \
			src/ssgnc-sort-data "$n" "$mem_size" "$temp_dir" | \
			src/ssgnc-build-db "$n" "$mem_size" "$temp_dir" > "$db_path"
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
