#include "ssgnc/ngram-index.h"
#include "ssgnc/reader.h"
#include "ssgnc/mapper.h"

namespace ssgnc {

bool NgramIndex::FileEntry::set_file_id(Int32 file_id)
{
	if (file_id < 0 || file_id > MAX_FILE_ID)
	{
		SSGNC_ERROR << "Out of range file ID: " << file_id << std::endl;
		return false;
	}
	file_id_ = static_cast<Int16>(file_id);
	return true;
}

bool NgramIndex::FileEntry::set_offset(UInt32 offset)
{
	if (offset > MAX_OFFSET)
	{
		SSGNC_ERROR << "Too large offset: " << offset << std::endl;
		return false;
	}
	offset_lo_ = static_cast<UInt16>(offset & 0xFFFF);
	offset_hi_ = static_cast<UInt16>(offset >> 16);
	return true;
}

bool NgramIndex::Entry::set_file_id(Int32 file_id)
{
	if (file_id < 0 || file_id > MAX_FILE_ID)
	{
		SSGNC_ERROR << "Out of range file ID: " << file_id << std::endl;
		return false;
	}
	file_id_ = file_id;
	return true;
}

bool NgramIndex::Entry::set_offset(UInt32 offset)
{
	if (offset > MAX_OFFSET)
	{
		SSGNC_ERROR << "Too large offset: " << offset << std::endl;
		return false;
	}
	offset_ = offset;
	return true;
}

bool NgramIndex::Entry::set_approx_size(Int64 approx_size)
{
	if (approx_size < 0 || approx_size > MAX_APPROX_SIZE)
	{
		SSGNC_ERROR << "Out of range approximate size: "
			<< approx_size << std::endl;
		return false;
	}
	approx_size_ = approx_size;
	return true;
}

NgramIndex::NgramIndex() : max_num_tokens_(0), max_token_id_(0),
	entries_(NULL), file_map_() {}

NgramIndex::~NgramIndex()
{
	if (is_open())
		close();
}

bool NgramIndex::open(const Int8 *path, FileMap::Mode mode)
{
	if (is_open())
	{
		SSGNC_ERROR << "Already opened" << std::endl;
		return false;
	}
	else if (path == NULL)
	{
		SSGNC_ERROR << "Null pointer" << std::endl;
		return false;
	}

	if (!file_map_.open(path, mode))
	{
		SSGNC_ERROR << "ssgnc::FileMap::open() failed: " << path << std::endl;
		return false;
	}

	if (!mapData(file_map_.ptr(), file_map_.size()))
	{
		SSGNC_ERROR << "ssgnc::NgramIndex::mapData() failed: "
			<< path << ", " << file_map_.size() << std::endl;
		file_map_.close();
		return false;
	}

	return true;
}

bool NgramIndex::close()
{
	if (!is_open())
	{
		SSGNC_ERROR << "Not opened" << std::endl;
		return false;
	}

	max_num_tokens_ = 0;
	max_token_id_ = 0;
	entries_ = NULL;
	file_map_.close();
	return true;
}

bool NgramIndex::get(Int32 num_tokens, Int32 token_id, Entry *entry) const
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
