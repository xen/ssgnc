#include "ssgnc/ngram-index.h"
#include "ssgnc/reader.h"
#include "ssgnc/mapper.h"

namespace ssgnc {

NgramIndex::NgramIndex() : max_num_tokens_(0), max_token_id_(0),
	entries_(NULL), entries_buf_(), file_map_() {}

void NgramIndex::clear()
{
	max_num_tokens_ = 0;
	max_token_id_ = 0;
	entries_ = NULL;
	std::vector<FileEntry>().swap(entries_buf_);
	if (file_map_.is_open())
		file_map_.close();
}

bool NgramIndex::get(Int32 num_tokens, Int32 token_id, Entry *entry)
{
	if (num_tokens < 1 || num_tokens > max_num_tokens_)
	{
		SSGNC_ERROR << "Out of range #tokens: " << num_tokens << std::endl;
		return false;
	}
	else if (token_id < 0 || token_id > max_token_id_)
	{
		SSGNC_ERROR << "Out of range token ID: " << token_id << std::endl;
		return false;
	}
	else if (entry == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	UInt32 index = (max_num_tokens_ * token_id) + num_tokens - 1;
	if (!entry->set_file_id(entries_[index].file_id()))
	{
		SSGNC_ERROR << "ssgnc::NgramIndex::Entry::set_file_id() failed: "
			<< entries_[index].file_id() << std::endl;
		return false;
	}
	if (!entry->set_offset(entries_[index].offset()))
	{
		SSGNC_ERROR << "ssgnc::NgramIndex::Entry::set_offset() failed: "
			<< entries_[index].offset() << std::endl;
		return false;
	}

	Int64 diff = entries_[index + max_num_tokens_] - entries_[index];
	if (!entry->set_approx_size(diff))
	{
		SSGNC_ERROR << "ssgnc::NgramIndex::Entry::set_approx_size() failed: "
			<< diff << std::endl;
		return false;
	}

	return true;
}

bool NgramIndex::load(const Int8 *path)
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
		SSGNC_ERROR << "ssgnc::NgramIndex::read() failed: "
			<< path << std::endl;
		return false;
	}

	return true;
}

bool NgramIndex::read(std::istream *in)
{
	clear();

	if (in == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	if (!readData(in))
	{
		SSGNC_ERROR << "ssgnc::NgramIndex::readData() failed: " << std::endl;
		return false;
	}

	return true;
}

bool NgramIndex::mmap(const Int8 *path)
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
		file_map_.close();
		return false;
	}

	return true;
}

bool NgramIndex::map(const void *ptr, UInt32 size)
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

bool NgramIndex::readData(std::istream *in)
{
	Reader reader;
	if (!reader.open(in))
	{
		SSGNC_ERROR << "ssgnc::Reader::open() failed" << std::endl;
		return false;
	}

	Int32 max_num_tokens, max_token_id;
	if (!reader.read(&max_num_tokens) || !reader.read(&max_token_id))
	{
		SSGNC_ERROR << "ssgnc::Reader::read() failed: header" << std::endl;
		return false;
	}
	else if (max_num_tokens == 0 || max_token_id == 0)
	{
		SSGNC_ERROR << "Wrong header" << std::endl;
		return false;
	}

	UInt32 num_entries = max_num_tokens * (max_token_id + 2);
	std::vector<FileEntry> entries_buf;
	try
	{
		entries_buf.resize(num_entries);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<FileEntry>::resize() failed: "
			<< num_entries << std::endl;
		return false;
	}

	if (!reader.read(&entries_buf[0], num_entries))
	{
		SSGNC_ERROR << "ssgnc::Reader::read() failed: entries" << std::endl;
		return false;
	}

	max_num_tokens_ = max_num_tokens;
	max_token_id_ = max_token_id;

	entries_ = &entries_buf[0];
	entries_buf_.swap(entries_buf);

	return true;
}

bool NgramIndex::mapData(const void *ptr, UInt32 size)
{
	Mapper mapper;
	if (!mapper.open(ptr, size))
	{
		SSGNC_ERROR << "ssgnc::Mapper::open() failed" << std::endl;
		return false;
	}

	const Int32 *max_num_tokens, *max_token_id;
	if (!mapper.map(&max_num_tokens) || !mapper.map(&max_token_id))
	{
		SSGNC_ERROR << "ssgnc::Mapper::map() failed: header" << std::endl;
		return false;
	}
	else if (*max_num_tokens == 0 || *max_token_id == 0)
	{
		SSGNC_ERROR << "Wrong header" << std::endl;
		return false;
	}

	UInt32 num_entries = *max_num_tokens * (*max_token_id + 2);
	const FileEntry *entries;
	if (!mapper.map(&entries, num_entries))
	{
		SSGNC_ERROR << "ssgnc::Mapper::map() failed: entries" << std::endl;
		return false;
	}

	if (mapper.tell() != size)
	{
		SSGNC_ERROR << "Extra bytes: " << (size - mapper.tell()) << std::endl;
		return false;
	}

	max_num_tokens_ = *max_num_tokens;
	max_token_id_ = *max_token_id;
	entries_ = entries;

	return true;
}

}  // namespace ssgnc
