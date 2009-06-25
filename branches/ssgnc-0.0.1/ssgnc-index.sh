#! /bin/sh

## Shows the usage if the number of given arguments is invalid.
if [ $# -ne 2 -a $# -ne 3 ]
then
	echo "Usage: $0 InputDir OutputDir [MemorySize]"
	exit 1
fi

## Shows the start time.
date
echo

## Names command line arguments.
input_dir=`dirname "$1/input"`
output_dir=`dirname "$2/output"`
memory_size="512M"

if [ $# -eq 3 ]
then
	memory_size=$3
fi

## Shows the settings.
echo "Input directory: $input_dir"
echo "Output directory: $output_dir"
echo "Memory size: $memory_size"
echo

## Checks if the input directory exists or not.
if [ ! -d "$input_dir" ]
then
	echo "error: no such directory: $input_dir"
	exit 1
fi

## Creates the output directory if not exists.
if [ ! -d "$output_dir" ]
then
	echo "creating directory $output_dir ..."
	mkdir "$output_dir"
	if [ $? -ne 0 ]
	then
		exit 1
	fi
	echo "done!"
	echo
fi

## Generates a vocabulary path.
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

## Builds a dictionary.
dic_path="$output_dir/vocab.dic"
if [ -f "$dic_path" ]
then
	echo "$dic_path: already exists"
else
	echo "creating $dic_path ..."
	gzip -cd "$vocab_path" | ./ssgnc-build-dic > "$dic_path"
	if [ $? -ne 0 ]
	then
		exit 1
	fi
	echo "done!"
fi
echo

## Sorts 1-grams in descending frequency order.
gms_txt_path="$output_dir/1gms.txt"
if [ -f "$gms_txt_path" ]
then
	echo "$gms_txt_path: already exists"
else
	echo "creating $gms_txt_path ..."
	gzip -cd "$vocab_path" | sort -S "$memory_size" -srnk 2 -o "$gms_txt_path"
	if [ $? -ne 0 ]
	then
		exit 1
	fi
	echo "done!"
fi

## Sorts n-grams in descending frequency order.
n=2
while [ -d "$input_dir/${n}gms" ]
do
	gms_txt_path="$output_dir/${n}gms.txt"
	if [ -f "$gms_txt_path" ]
	then
		echo "$gms_txt_path: already exists"
	else
		echo "creating $gms_txt_path ..."
		gzip -cd "$input_dir/${n}gms/${n}gm-0"* | \
			sort -S "$memory_size" -srnk `expr $n + 1` -o "$gms_txt_path"
		if [ $? -ne 0 ]
		then
			exit 1
		fi
		echo "done!"
	fi
	n=`expr $n + 1`
done
echo

## Builds indices.
n=1
while [ -f "$output_dir/${n}gms.txt" ]
do
	gms_txt_path="$output_dir/${n}gms.txt"
	gms_idx_path="$output_dir/${n}gms.idx"
	if [ -f "$gms_idx_path" ]
	then
		echo "$gms_idx_path: already exists"
	else
		echo "creating $gms_idx_path ..."
		cat "$dic_path" "$gms_txt_path" | ./ssgnc-build-index | \
			sort -S "$memory_size" -snk 1 -uk 2 | \
			./ssgnc-encode-index > "$gms_idx_path"
		if [ $? -ne 0 ]
		then
			exit 1
		fi
		echo "done!"
	fi
	n=`expr $n + 1`
done
echo

## Shows the end time
date
