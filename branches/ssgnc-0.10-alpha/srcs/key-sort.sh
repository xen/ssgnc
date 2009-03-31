#! /bin/sh

## Shows usage if passed arguments are invalid.
if [ $# -lt 1 ]
then
	echo "Usage: $0 InputDir [OutputDir] [MemorySize]"
	exit 1
fi

## Names command line arguments.
input_dir=`echo $1 | sed -e "s/\/$//"`
output_dir=$input_dir
if [ $# -gt 1 ]
then
	output_dir=`echo $2 | sed -e "s/\/$//"`
fi
memory_size=512
if [ $# -gt 2 ]
then
	memory_size=$3
fi

## Shows settings.
echo "Input directory: $input_dir"
echo "Output directory: $output_dir"
echo "Memory size: ${memory_size}MB"
echo

## Builds programs.
make key-sort
if [ $? -ne 0 ]
then
	exit 1
fi
echo

## Checks if the input directory exists or not.
if [ ! -d $input_dir ]
then
	echo "error: no such directory: $input_dir"
	exit 1
fi

## Checks if the output directory exists or not.
if [ ! -d $output_dir ]
then
	## Creates the output directory if not exists.
	echo "creating directory $output_dir ..."
	mkdir $output_dir
	if [ $? -ne 0 ]
	then
		exit 1
	fi
	echo "done!"
	echo
fi

## Sorts keys.
n=1
while [ -f $input_dir/${n}gms.bin ]
do
	gms_bin_path=$input_dir/${n}gms.bin
	gms_key_path=$output_dir/${n}gms.key
	if [ -f $gms_key_path ]
	then
		echo "$gms_key_path: already exists"
	else
		echo "creating $gms_key_path ..."
		./sort-keys $n $memory_size $gms_key_path \
			< $gms_bin_path > $gms_key_path
		if [ $? -ne 0 ]
		then
			exit 1
		fi
		echo "done!"
	fi
	echo
	n=`expr $n + 1`
done
