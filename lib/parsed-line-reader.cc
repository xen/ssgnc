#include <ssgnc/parsed-line-reader.h>

#include <ssgnc/exception.h>

#include <algorithm>
#include <cstring>

namespace ssgnc {

ParsedLineReader::ParsedLineReader(std::size_t buf_size) :
	file_(NULL), file_cur_(0), file_end_(0), buf_size_(0), buf_(), pos_(0),
	length_(0), parsed_line_(), num_keys_(0), num_lines_(0), count_(0)
{
	if (buf_size == 0)
		buf_size = DEFAULT_BUF_SIZE;

	buf_size_ = buf_size;
	try
	{
		buf_.reserve(buf_size_);
	}
	catch (...)
	{
		SSGNC_THROW("failed to initialize ParsedLineReader: "
			"std::vector::reserve() failed");
	}
}

void ParsedLineReader::Open(TempFile *file, std::size_t file_begin,
	std::size_t file_end)
{
	ParsedLineReader temp;

	temp.file_ = file;
	temp.file_cur_ = file_begin;
	temp.file_end_ = file_end;

	temp.Fill();

	if (temp.buf_.size() < sizeof(std::size_t) * 2)
		SSGNC_THROW("failed to open parsed line reader: no headers");
	const std::size_t *header =
		reinterpret_cast<const std::size_t *>(&temp.buf_[0]);
	temp.num_keys_ = header[0];
	temp.num_lines_ = header[1];
	temp.pos_ = sizeof(std::size_t) * 2;

	Swap(&temp);
}

void ParsedLineReader::Close()
{
	file_ = NULL;
	file_cur_ = 0;
	file_end_ = 0;
	buf_size_ = buf_size_;
	buf_.clear();
	pos_ = 0;
	length_ = 0;
	parsed_line_.Clear();
	num_keys_ = 0;
	num_lines_ = 0;
	count_ = 0;
}

bool ParsedLineReader::Next()
{
	if (count_ >= num_lines_)
		return false;

	if (!Parse())
	{
		Fill();
		if (!Parse())
			SSGNC_THROW("failed to read next parsed line: incomplete line");
	}

	++count_;
	return true;
}

void ParsedLineReader::Swap(ParsedLineReader *target)
{
	std::swap(file_, target->file_);
	std::swap(file_cur_, target->file_cur_);
	std::swap(file_end_, target->file_end_);
	std::swap(buf_size_, target->buf_size_);
	buf_.swap(target->buf_);
	std::swap(pos_, target->pos_);
	std::swap(length_, target->length_);
	parsed_line_.Swap(&target->parsed_line_);
	std::swap(num_keys_, target->num_keys_);
	std::swap(num_lines_, target->num_lines_);
	std::swap(count_, target->count_);
}

void ParsedLineReader::Fill()
{
	std::size_t avail = buf_.size() - pos_;
	if (!buf_.empty() && avail > 0)
		std::memmove(&buf_[0], &buf_[pos_], avail);
	pos_ = 0;

	std::size_t next = std::min(file_end_ - file_cur_, buf_size_ - avail);
	buf_.resize(avail + next);
	file_->Seek(file_cur_);
	file_->Read(&buf_[avail], next);
	file_cur_ += next;
}

bool ParsedLineReader::Parse()
{
	parsed_line_.Clear();
	std::size_t pos = pos_;

	if (pos >= buf_.size())
		return false;

	int count = 0;
	unsigned char byte = buf_[pos++];

	long long value = byte & 0x7F;
	while (byte >= 0x80)
	{
		if (pos >= buf_.size())
			return false;
		byte = buf_[pos++];
		value |= static_cast<long long>(byte & 0x7F) << (7 * ++count);
	}
	parsed_line_.set_value(value);

	for (std::size_t i = 0; i < num_keys_; ++i)
	{
		if (pos >= buf_.size())
			return false;

		count = 0;
		byte = buf_[pos++];

		int key = byte & 0x7F;
		while (byte >= 0x80)
		{
			if (pos >= buf_.size())
				return false;
			byte = buf_[pos++];
			key |= (byte & 0x7F) << (7 * ++count);
		}
		parsed_line_.append_key(key);
	}

	length_ = pos - pos_;
	pos_ = pos;
	return true;
}

}  // namespace ssgnc
