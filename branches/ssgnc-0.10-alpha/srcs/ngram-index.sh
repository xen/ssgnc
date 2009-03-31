#! /bin/sh

## Shows usage if passed arguments are invalid.
if [ $# -lt 2 ]
then
	echo "Usage: $0 InputDir OutputDir [MemorySize]"
	exit 1
fi

## Names command line arguments.
input_dir=`dirname "$1/input"`
output_dir=`dirname "$2/output"`
memory_size=512
if [ $# -gt 2 ]
then
	memory_size=$3
fi

## Shows I/O directories.
echo "Input directory: $input_dir"
echo "Output directory: $output_dir"
echo "Memory size: ${memory_size}MB"
echo

## Builds programs.
make
if [ $? -ne 0 ]
then
	exit 1
fi
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
vocab_text_path="$output_dir/vocab.txt"

## Sorts vocabulary.
if [ -f "$vocab_text_path" ]
then
	echo "$vocab_text_path: already exists"
else
	echo "creating $vocab_text_path ..."
	gzip -cd "$vocab_path" | ./sort-vocab > "$vocab_text_path"
	if [ $? -ne 0 ]
	then
		exit 1
	fi
	echo "done!"
fi
echo

## Builds a double-array.
vocab_da_path="$output_dir/vocab.da"
if [ -f "$vocab_da_path" ]
then
	echo "$vocab_da_path: already exists"
else
	echo "creating $vocab_da_path ..."
	./build-vocab-da "$vocab_da_path" < "$vocab_text_path"
	if [ $? -ne 0 ]
	then
		exit 1
	fi
	echo "done!"
fi
echo

## Builds an index for vocabulary.
vocab_index="$output_dir/vocab.idx"
vocab_data="$output_dir/vocab.dat"
if [ -f "$vocab_index" -a -f "$vocab_data" ]
then
	echo "$vocab_index and $vocab_data: already exist"
else
	echo "creating $vocab_index and $vocab_data ..."
	./index-vocab "$vocab_index" < "$vocab_text_path" > "$vocab_data"
	if [ $? -ne 0 ]
	then
		exit 1
	fi
	echo "done!"
fi
echo

## Encodes 1-gram file.
gms_bin_path="$output_dir/1gms.bin"
if [ -f "$gms_bin_path" ]
then
	echo "$gms_bin_path: already exists"
else
	echo "creating $gms_bin_path ..."
	gzip -cd "$vocab_path" | ./encode-ngram "$vocab_da_path" > "$gms_bin_path"
	if [ $? -ne 0 ]
	then
		exit 1
	fi
	echo "done!"
fi
echo

## Encodes n-gram files.
n=2
while [ -d "$input_dir/${n}gms" ]
do
	gms_bin_path="$output_dir/${n}gms.bin"
	if [ -f "$gms_bin_path" ]
	then
		echo "$gms_bin_path: already exists"
	else
		echo "creating $gms_bin_path ..."
		gzip -cd "$input_dir/${n}gms/${n}gm-0"* \
			| ./encode-ngram "$vocab_da_path" > "$gms_bin_path"
		if [ $? -ne 0 ]
		then
			exit 1
		fi
		echo "done!"
	fi
	echo
	n=`expr $n + 1`
done

## Sorts n-gram files.
n=1
while [ -f "$output_dir/${n}gms.bin" ]
do
	gms_bin_path="$output_dir/${n}gms.bin"
	gms_bin_sorted_path="${gms_bin_path}.sorted"
	if [ -f "$gms_bin_sorted_path" ]
	then
		echo "$gms_bin_sorted_path: already exists"
	else
		echo "creating $gms_bin_sorted_path ..."
		./sort-ngram $n $memory_size "$gms_bin_path" \
			< "$gms_bin_path" > "$gms_bin_sorted_path"
		if [ $? -ne 0 ]
		then
			exit 1
		fi
		echo "done!"
	fi
	echo
	n=`expr $n + 1`
done

## Builds indices.
n=1
while [ -f "$output_dir/${n}gms.bin.sorted" ]
do
	gms_bin_path="$output_dir/${n}gms.bin.sorted"
	gms_index="$output_dir/${n}gms.idx"
	gms_data="$output_dir/${n}gms.dat"
	if [ -f "$gms_index" -a -f "$gms_data" ]
	then
		echo "$gms_index and $gms_data: already exist"
	else
		echo "creating $gms_index and $gms_data ..."
		./index-ngram $n $memory_size "$gms_index" "$gms_index" \
			< "$gms_bin_path" > "$gms_data"
		if [ $? -ne 0 ]
		then
			exit 1
		fi
		echo "done!"
	fi
	echo
	n=`expr $n + 1`
done

## Merges indices.
index_path="$output_dir/ngms.idx"
if [ -f "$index_path" ]
then
	echo "$index_path: already exists"
else
	echo "creating $index_path ..."
	./merge-index "$output_dir" > "$index_path"
	if [ $? -ne 0 ]
	then
		exit 1
	fi
	echo "done!"
fi
echo

## Removes temporary files.
rm -f "$vocab_text_path"
n=1
while [ -f "$output_dir/${n}gms.bin" ]
do
	rm -f "$output_dir/${n}gms.bin"
	rm -f "$output_dir/${n}gms.bin.sorted"
	rm -f "$output_dir/${n}gms.idx"
	n=`expr $n + 1`
done
