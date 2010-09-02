#! /bin/sh

ShowUsage()
{
	echo "Usage: DATA_DIR INDEX_DIR [TEMP_DIR] [CHECKER]"
	echo
	echo "DATA_DIR: DATA_DIR/Ngms/Ngm-KKKK [.gz, .xz]"
	echo "INDEX_DIR: INDEX_DIR/vocab.dic, ngms.idx, Ngm-KKKK.db"
	echo "TEMP_DIR: TEMP_DIR/Ngm-KKKK.bin, Ngm-KKKK.part, Ngms.idx"
	echo "CHECKER: time, valgrind"
	echo "  time: time -f 'real %E, user %U, sys %S'"
	echo "  valgrind: valgrind --leak-check=full"
}

CheckCommands()
{
	for cmd in $@
	do
		if [ ! `which "$cmd"` ]
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

	echo
	echo "ssgnc-vocab-dic-build"
	$filter "$DATA_DIR"/1gms/1gm-* | \
		$checker ssgnc-vocab-dic-build "$INDEX_DIR/vocab.dic"
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

	echo
	echo "INPUT_DIR: $input_dir"

	filter=`ChooseFilter "$input_dir/$num_tokens""gm-0000"`
	if [ $? -ne 0 ]
	then
		echo "Error: $input_dir/$num_tokens""gm-0000*: No such file"
		exit 401
	fi

	echo
	echo "ssgnc-ngms-encode"
	$filter "$input_dir/$num_tokens""gm-"* | \
		$checker ssgnc-ngms-encode \
		$num_tokens "$INDEX_DIR/vocab.dic" "$TEMP_DIR"
	if [ $? -ne 0 ]
	then
		exit 402
	fi

	echo
	echo "ssgnc-ngms-merge | ssgnc-ngms-split"
	$checker ssgnc-ngms-merge \
		$num_tokens "$INDEX_DIR/vocab.dic" "$TEMP_DIR" | \
		$checker ssgnc-ngms-split \
		$num_tokens "$INDEX_DIR/vocab.dic" "$TEMP_DIR"
	if [ $? -ne 0 ]
	then
		exit 403
	fi

	rm -f "$TEMP_DIR/$num_tokens""gm-"*".bin"
	if [ $? -ne 0 ]
	then
		exit 404
	fi

	echo
	echo "ssgnc-db-merge | ssgnc-db-split"
	$checker ssgnc-db-merge \
		$num_tokens "$INDEX_DIR/vocab.dic" "$TEMP_DIR" | \
		$checker ssgnc-db-split \
		$num_tokens "$INDEX_DIR/vocab.dic" "$INDEX_DIR" \
		> "$TEMP_DIR/$num_tokens""gms.idx"
	if [ $? -ne 0 ]
	then
		exit 405
	fi

	rm -f "$TEMP_DIR/$num_tokens""gm-"*".part"
	if [ $? -ne 0 ]
	then
		exit 406
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

	echo
	echo "ssgnc-idx-merge"
	$checker ssgnc-idx-merge \
		"$INDEX_DIR/vocab.dic" "$TEMP_DIR" > "$INDEX_DIR/ngms.idx"
	if [ $? -ne 0 ]
	then
		exit 500
	fi

	num_tokens=1
	while [ -f "$TEMP_DIR/$num_tokens""gms.idx" ]
	do
		rm -f "$TEMP_DIR/$num_tokens""gms.idx"
		if [ $? -ne 0 ]
		then
			exit 501
		fi

		num_tokens=`expr $num_tokens + 1`
	done
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

if [ $# -lt 2 -o $# -gt 4 ]
then
	ShowUsage
	exit 1
fi

DATA_DIR="$1"
INDEX_DIR="$2"
TEMP_DIR="$2"
CHECKER=""
if [ $# -gt 2 ]
then
	TEMP_DIR="$3"
fi

if [ $# -gt 3 ]
then
	if [ "$4" = "time" ]
	then
		checker="time -p"
	elif [ "$4" = "valgrind" ]
	then
		checker="valgrind --leak-check=full"
	else
		echo "Invalid CHECKER option: $4"
		echo
		ShowUsage
		exit 2
	fi
fi

echo "DATA_DIR: $DATA_DIR"
echo "INDEX_DIR: $INDEX_DIR"
echo "TEMP_DIR: $TEMP_DIR"
echo "CHECKER: $CHECKER"

if [ ! -d "$DATA_DIR" ]
then
	echo
	echo "Error: $DATA_DIR: No such directory"

	exit 3
fi

if [ ! -d "$INDEX_DIR" ]
then
	echo
	echo "Warning: $INDEX_DIR: No such directory"
	echo "Making INDEX_DIR: $INDEX_DIR"

	mkdir "$INDEX_DIR"
	if [ $? -ne 0 ]
	then
		exit 4
	fi
fi

EXISTS_TEMP_DIR="TRUE"
if [ ! -d "$TEMP_DIR" ]
then
	echo
	echo "Warning: $TEMP_DIR: No such directory"
	echo "Making TEMP_DIR: $TEMP_DIR"

	EXISTS_TEMP_DIR="FALSE"
	mkdir "$TEMP_DIR"
	if [ $? -ne 0 ]
	then
		exit 5
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
	echo
	echo "Removing TEMP_DIR: $TEMP_DIR"

	rm -rf "$TEMP_DIR"
fi

exit $EXIT_VALUE
