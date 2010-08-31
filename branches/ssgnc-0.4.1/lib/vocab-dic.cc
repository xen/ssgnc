#include "ssgnc.h"

namespace ssgnc {

VocabDic::VocabDic() : num_keys_(0), table_size_(0), total_size_(0),
	table_(NULL), offsets_(NULL), keys_(NULL),
	table_buf_(), offsets_buf_(), keys_buf_(), file_map_() {}

void VocabDic::clear()
{
	num_keys_ = 0;
	table_size_ = 0;
	total_size_ = 0;

	table_ = NULL;
	offsets_ = NULL;
	keys_ = NULL;

	std::vector<Int32>().swap(table_buf_);
	std::vector<UInt32>().swap(offsets_buf_);
	std::vector<Int8>().swap(keys_buf_);

	if (file_map_.is_open())
		file_map_.close();
}

bool VocabDic::find(const String &key, Int32 *key_id) const
{
	if (key_id == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	UInt32 hash_id = StringHash()(key) % table_size_;
	for ( ; ; )
	{
		*key_id = table_[hash_id];
		if (table_[hash_id] == INVALID_KEY_ID)
			return false;
		else if (key == restoreKey(*key_id))
			return true;

		hash_id = (hash_id + 1) % table_size_;
	}
}

bool VocabDic::build(const String *keys, UInt32 num_keys)
{
	clear();

	if (keys == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}
	else if (num_keys == 0)
	{
		SSGNC_ERROR << "No keys" << std::endl;
		return false;
	}

	UInt32 table_size = num_keys + (num_keys / 4) + 1;

	UInt32 total_length = 0;
	for (UInt32 i = 0; i < num_keys; ++i)
		total_length += keys[i].length();

	std::vector<Int32> table_buf;
	try
	{
		table_buf.resize(table_size, INVALID_KEY_ID);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<Int32>::resize() failed: "
			<< sizeof(Int32) << " * " << table_size << std::endl;
		return false;
	}

	std::vector<UInt32> offsets_buf;
	try
	{
		offsets_buf.reserve(num_keys + 1);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<Int32>::reserve() failed: "
			<< sizeof(Int32) << " * " << (num_keys + 1) << std::endl;
		return false;
	}

	std::vector<Int8> keys_buf;
	try
	{
		keys_buf.reserve(total_length);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<Int8>::reserve() failed: "
			<< total_length << std::endl;
		return false;
	}

	UInt32 total_size = static_cast<UInt32>(
		sizeof(table_buf[0]) * table_buf.size()
		+ sizeof(offsets_buf[0]) * (num_keys + 1)
		+ total_length);

	offsets_buf.push_back(0);
	for (UInt32 i = 0; i < num_keys; ++i)
	{
		if (keys[i].empty())
		{
			SSGNC_ERROR << "Zero length: " << i << "th key" << std::endl;
			return false;
		}

		UInt32 hash_id = StringHash()(keys[i]) % table_size;
		for ( ; ; )
		{
			Int32 key_id = table_buf[hash_id];
			if (key_id == INVALID_KEY_ID)
				break;
			else if (keys[i] == String(&keys_buf[0] + offsets_buf[key_id],
				offsets_buf[key_id + 1] - offsets_buf[key_id]))
			{
				SSGNC_ERROR << "Duplication: "
					<< i << "th key, " << keys[i] << std::endl;
				return false;
			}
			hash_id = (hash_id + 1) % table_size;
		}
		table_buf[hash_id] = static_cast<Int32>(i);

		try
		{
			for (UInt32 j = 0; j < keys[i].length(); ++j)
				keys_buf.push_back(keys[i][j]);
			offsets_buf.push_back(static_cast<UInt32>(keys_buf.size()));
		}
		catch (...)
		{
			SSGNC_ERROR << "Unexpected error" << std::endl;
			return false;
		}
	}

	num_keys_ = num_keys;
	table_size_ = table_size;
	total_size_ = total_size;

	table_ = &table_buf[0];
	offsets_ = &offsets_buf[0];
	keys_ = &keys_buf[0];

	table_buf_.swap(table_buf);
	offsets_buf_.swap(offsets_buf);
	keys_buf_.swap(keys_buf);

	return true;
}

bool VocabDic::load(const Int8 *path)
{
	clear();

	if (path == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	std::ifstream file(path, std::ios::binary);
	if (!file)
	{
		SSGNC_ERROR << "std::ifstream::open() failed: " << path << std::endl;
		return false;
	}

	if (!readData(&file))
	{
		SSGNC_ERROR << "ssgnc::VocabDic::readData() failed: "
			<< path << std::endl;
		return false;
	}

	return true;
}

bool VocabDic::read(std::istream *in)
{
	clear();

	if (in == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	if (!readData(in))
	{
		SSGNC_ERROR << "ssgnc::VocabDic::readData() failed: " << std::endl;
		return false;
	}

	return true;
}

bool VocabDic::mmap(const Int8 *path)
{
	clear();

	if (path == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	if (!file_map_.open(path))
	{
		SSGNC_ERROR << "ssgnc::FileMap::open() failed: " << path << std::endl;
		return false;
	}

	if (!mapData(file_map_.ptr(), file_map_.size()))
	{
		SSGNC_ERROR << "ssgnc::VocabDic::mapData() failed: "
			<< path << ", " << file_map_.size() << std::endl;
		return false;
	}

	return true;
}

bool VocabDic::map(const void *ptr, UInt32 size)
{
	clear();

	if (ptr == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	if (!mapData(ptr, size))
	{
		SSGNC_ERROR << "ssgnc::VocabDic::mapData() failed: "
			<< size << std::endl;
		return false;
	}

	return true;
}

bool VocabDic::readData(std::istream *in)
{
	Reader reader;
	if (!reader.open(in))
	{
		SSGNC_ERROR << "ssgnc::Reader::open() failed" << std::endl;
		return false;
	}

	UInt32 num_keys, table_size, total_size;
	if (!reader.read(&num_keys) || !reader.read(&table_size) ||
		!reader.read(&total_size))
	{
		SSGNC_ERROR << "ssgnc::Reader::read() failed: header" << std::endl;
		return false;
	}
	else if (num_keys == 0 || table_size == 0 || total_size == 0)
	{
		SSGNC_ERROR << "Wrong header" << std::endl;
		return false;
	}

	std::vector<Int32> table_buf;
	try
	{
		table_buf.resize(table_size);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<Int32>::resize() failed: "
			<< sizeof(Int32) << " * " << table_size << std::endl;
		return false;
	}
	if (!reader.read(&table_buf[0], table_size))
	{
		SSGNC_ERROR << "ssgnc::Reader::read() failed: table" << std::endl;
		return false;
	}

	std::vector<UInt32> offsets_buf;
	try
	{
		offsets_buf.resize(num_keys + 1);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<Int32>::reserve() failed: "
			<< sizeof(Int32) << " * " << (num_keys + 1) << std::endl;
		return false;
	}
	if (!reader.read(&offsets_buf[0], num_keys + 1))
	{
		SSGNC_ERROR << "ssgnc::Reader::read() failed: offsets" << std::endl;
		return false;
	}

	std::vector<Int8> keys_buf;
	try
	{
		keys_buf.resize(offsets_buf.back());
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<Int8>::reserve() failed: "
			<< offsets_buf.back() << std::endl;
		return false;
	}
	if (!reader.read(&keys_buf[0], offsets_buf.back()))
	{
		SSGNC_ERROR << "ssgnc::Reader::read() failed: keys" << std::endl;
		return false;
	}

	num_keys_ = num_keys;
	table_size_ = table_size;
	total_size_ = total_size;

	table_ = &table_buf[0];
	offsets_ = &offsets_buf[0];
	keys_ = &keys_buf[0];

	table_buf_.swap(table_buf);
	offsets_buf_.swap(offsets_buf);
	keys_buf_.swap(keys_buf);

	return true;
}

bool VocabDic::mapData(const void *ptr, UInt32 size)
{
	Mapper mapper;
	if (!mapper.open(ptr, size))
	{
		SSGNC_ERROR << "ssgnc::Mapper::open() failed" << std::endl;
		return false;
	}

	const UInt32 *num_keys, *table_size, *total_size;
	if (!mapper.map(&num_keys) || !mapper.map(&table_size) ||
		!mapper.map(&total_size))
	{
		SSGNC_ERROR << "ssgnc::Mapper::map() failed: header" << std::endl;
		return false;
	}
	else if (*num_keys == 0 || *table_size == 0 || *total_size == 0)
	{
		SSGNC_ERROR << "Wrong header" << std::endl;
		return false;
	}

	const Int32 *table;
	if (!mapper.map(&table, *table_size))
	{
		SSGNC_ERROR << "ssgnc::Mapper::map() failed: table" << std::endl;
		return false;
	}

	const UInt32 *offsets;
	if (!mapper.map(&offsets, *num_keys + 1))
	{
		SSGNC_ERROR << "ssgnc::Mapper::map() failed: offsets" << std::endl;
		return false;
	}

	const Int8 *keys;
	if (!mapper.map(&keys, offsets[*num_keys]))
	{
		SSGNC_ERROR << "ssgnc::Mapper::map() failed: keys" << std::endl;
		return false;
	}

	if (sizeof(table[0]) * *table_size
		+ sizeof(offsets[0]) * (*num_keys + 1)
		+ offsets[*num_keys] != *total_size)
	{
		SSGNC_ERROR << "Conflicted sizes: " << *table_size
			<< ", " << *num_keys << ", " << offsets_[*num_keys]
			<< ", " << *total_size << std::endl;
		return false;
	}

	if (mapper.tell() != size)
	{
		SSGNC_ERROR << "Extra bytes: " << (size - mapper.tell()) << std::endl;
		return false;
	}

	num_keys_ = *num_keys;
	table_size_ = *table_size;
	total_size_ = *total_size;

	table_ = table;
	offsets_ = offsets;
	keys_ = keys;

	return true;
}

bool VocabDic::save(const Int8 *path) const
{
	if (path == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	std::ofstream file(path, std::ios::binary);
	if (!file)
	{
		SSGNC_ERROR << "std::ofstream::open() failed: " << path << std::endl;
		return false;
	}

	if (!write(&file))
	{
		SSGNC_ERROR << "ssgnc::VocabDic::write() failed: "
			<< path << std::endl;
		return false;
	}

	return true;
}

bool VocabDic::write(std::ostream *out) const
{
	if (num_keys_ == 0)
	{
		SSGNC_ERROR << "Empty dictionary" << std::endl;
		return false;
	}
	else if (out == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	Writer writer;
	if (!writer.open(out))
	{
		SSGNC_ERROR << "ssgnc::Writer::write() failed" << std::endl;
		return false;
	}

	if (!writer.write(num_keys_) || !writer.write(table_size_) ||
		!writer.write(total_size_))
	{
		SSGNC_ERROR << "ssgnc::Writer::write() failed: header" << std::endl;
		return false;
	}

	if (!writer.write(table_, table_size_))
	{
		SSGNC_ERROR << "ssgnc::Writer::write() failed: table" << std::endl;
		return false;
	}

	if (!writer.write(offsets_, num_keys_ + 1))
	{
		SSGNC_ERROR << "ssgnc::Writer::write() failed: offsets" << std::endl;
		return false;
	}

	if (!writer.write(keys_, offsets_[num_keys_]))
	{
		SSGNC_ERROR << "ssgnc::Writer::write() failed: keys" << std::endl;
		return false;
	}

	return true;
}

}  // namespace ssgnc
