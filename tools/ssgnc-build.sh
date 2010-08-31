#! /bin/sh

CheckCommands()
{
	for cmd in $@
	do
		which "$cmd"
		if [ $? -ne 0 ]
		then
			echo "Error: $cmd: No such command"
			echo
			echo "Please install ssgnc commands"
			exit 100
		fi
	done
}

ChooseFilter()
{
	for path in "$1"*
	do
		case "$path" in
			*.gz)
				echo "gzip -cd"
				;;
			*.bz2)
				echo "bzip2 -cd"
				;;
			*.xz)
				echo "xz -cd"
				;;
			*\*)
				exit 200
				;;
			*)
				echo "cat"
				;;
		esac
	done
}

BuildVocabDic()
{
	echo "Building VOCAB_DIC: $INDEX_DIR/vocab.dic"

	if [ ! -d "$DATA_DIR/1gms" ]
	then
		echo "Error: $DATA_DIR/1gms: No such directory"
		exit 300
	fi

	filter=`ChooseFilter "$DATA_DIR/1gms/1gm-0000"`
	if [ $? -ne 0 ]
	then
		echo "Error: $DATA_DIR/1gms/1gm-0000*: No such file"
		exit 301
	fi

	echo "ssgnc-vocab-dic-build"
	$filter "$DATA_DIR"/1gms/1gm-* | \
		ssgnc-vocab-dic-build > "$INDEX_DIR/vocab.dic"
	if [ $? -ne 0 ]
	then
		exit 302
	fi
}

BuildIndex()
{
	if [ $# -ne 1 ]
	then
		exit 400
	fi

	num_tokens=$1

	input_dir="$DATA_DIR/$num_tokens""gms"

	filter=`ChooseFilter "$input_dir/$num_tokens""gm-0000"`
	if [ $? -ne 0 ]
	then
		echo "Error: $input_dir/$num_tokens""gm-0000*: No such file"
		exit 401
	fi

	echo "ssgnc-ngms-encode"
	$filter "$input_dir/$num_tokens""gm-"* | \
		ssgnc-ngms-encode $num_tokens "$INDEX_DIR/vocab.dic" "$TEMP_DIR"
	if [ $? -ne 0 ]
	then
		exit 402
	fi

	echo "ssgnc-ngms-merge | ssgnc-ngms-split"
	ssgnc-ngms-merge $num_tokens "$INDEX_DIR/vocab.dic" "$TEMP_DIR" | \
		ssgnc-ngms-split $num_tokens "$INDEX_DIR/vocab.dic" "$TEMP_DIR"
	if [ $? -ne 0 ]
	then
		exit 403
	fi

	echo "ssgnc-db-merge | ssgnc-db-split"
	ssgnc-db-merge $num_tokens "$INDEX_DIR/vocab.dic" "$TEMP_DIR" | \
		ssgnc-db-split $num_tokens "$INDEX_DIR/vocab.dic" "$INDEX_DIR" \
			> "$TEMP_DIR/$num_tokens""gms.idx"
	if [ $? -ne 0 ]
	then
		exit 404
	fi
}

BuildIndices()
{
	num_tokens=1
	while [ -d "$DATA_DIR/$num_tokens""gms" ]
	do
		BuildIndex $num_tokens
		if [ $? -ne 0 ]
		then
			exit $?
		fi

		num_tokens=`expr $num_tokens + 1`
	done

	echo "ssgnc-idx-merge"
	ssgnc-idx-merge "$INDEX_DIR/vocab.dic" "$TEMP_DIR" \
		> "$INDEX_DIR/ngms.idx"
	if [ $? -ne 0 ]
	then
		exit 500
	fi
}

CheckCommands \
	ssgnc-db-merge ssgnc-db-split \
	ssgnc-idx-merge \
	ssgnc-ngms-encode ssgnc-ngms-merge ssgnc-ngms-split \
	ssgnc-vocab-dic-build
if [ $? -ne 0 ]
then
	exit $?
fi

if [ $# -lt 2 -o $# -gt 3 ]
then
	echo "Usage: DATA_DIR INDEX_DIR [TEMP_DIR]"
	exit 1
fi

DATA_DIR="$1"
INDEX_DIR="$2"
TEMP_DIR="$2"
if [ $# -gt 2 ]
then
	TEMP_DIR="$3"
fi

echo "DATA_DIR: $DATA_DIR"
echo "INDEX_DIR: $INDEX_DIR"
echo "TEMP_DIR: $TEMP_DIR"

if [ ! -d "$DATA_DIR" ]
then
	echo "Error: $DATA_DIR: No such directory"
	exit 2
fi

if [ ! -d "$INDEX_DIR" ]
then
	echo "Warning: $INDEX_DIR: No such directory"
	echo "Making INDEX_DIR: $INDEX_DIR"
	mkdir "$INDEX_DIR"
	if [ $? -ne 0 ]
	then
		exit 3
	fi
fi

EXISTS_TEMP_DIR="TRUE"
if [ ! -d "$TEMP_DIR" ]
then
	echo "Warning: $TEMP_DIR: No such directory"
	echo "Making TEMP_DIR: $TEMP_DIR"
	EXISTS_TEMP_DIR="FALSE"
	mkdir "$TEMP_DIR"
	if [ $? -ne 0 ]
	then
		exit 4
	fi
fi

BuildVocabDic
EXIT_VALUE=$?
if [ $EXIT_VALUE -eq 0 ]
then
	BuildIndices
	EXIT_VALUE=$?
fi

if [ "$EXISTS_TEMP_DIR" = "FALSE" ]
then
	echo "Removing TEMP_DIR: $TEMP_DIR"
	rm -rf "$TEMP_DIR"
fi

exit $EXIT_VALUE
