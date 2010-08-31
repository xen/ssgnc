#include <ssgnc/vocab-dic.h>

#include <ssgnc/exception.h>

#include <fstream>

namespace ssgnc {

void VocabDic::Open(const char *dic_file_name)
{
	VocabDic temp;

	temp.dic_.reset(new Darts::DoubleArray);

	std::ifstream dic_file(dic_file_name, std::ios::binary);
	if (!dic_file)
	{
		SSGNC_THROW("failed to open vocab dictionary: "
			"std::ifstream::open() failed");
	}

	int num_keys;
	if (!dic_file.read(reinterpret_cast<char *>(&num_keys),
		sizeof(num_keys)))
	{
		SSGNC_THROW("failed to open vocab dictionary: "
			"std::ifstream::read() failed");
	}

	int num_units;
	if (!dic_file.read(reinterpret_cast<char *>(&num_units),
		sizeof(num_units)))
	{
		SSGNC_THROW("failed to open vocab dictionary: "
			"std::ifstream::read() failed");
	}

	dic_file.close();

	temp.dic_->open(dic_file_name, "rb", sizeof(int) * 2,
		temp.dic_->unit_size() * num_units);
	temp.num_keys_ = num_keys;

	Swap(&temp);
}

void VocabDic::Close()
{
	dic_.reset();
	num_keys_ = 0;
}

void VocabDic::Map(const void *address)
{
	VocabDic temp;

	temp.dic_.reset(new Darts::DoubleArray);

	int num_keys = *static_cast<const int *>(address);
	int num_units = *(static_cast<const int *>(address) + 1);
	temp.dic_->set_array(static_cast<const int *>(address) + 2, num_units);
	temp.num_keys_ = num_keys;

	Swap(&temp);
}

void VocabDic::Swap(VocabDic *target)
{
	Darts::DoubleArray *temp = dic_.release();
	dic_ = target->dic_;
	target->dic_.reset(temp);

	std::swap(num_keys_, target->num_keys_);
}

}  // namespace ssgnc
