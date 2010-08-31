#include <ssgnc/back-merger.h>

#include <ssgnc/exception.h>

#include <cstring>

namespace ssgnc {

void BackMerger::Open(TempFile *file, const std::vector<std::size_t> &points)
{
	BackMerger temp;

	temp.readers_.resize(points.size() - 1, NULL);
	temp.queue_.reset(new PriorityQueue);
	for (std::size_t i = 0; i < temp.readers_.size(); ++i)
	{
		temp.readers_[i] = new ParsedLineReader;
		temp.readers_[i]->Open(file, points[i], points[i + 1]);
		if (temp.readers_[i]->Next())
		{
			try
			{
				temp.queue_->push(temp.readers_[i]);
			}
			catch (...)
			{
				SSGNC_THROW("failed to open parsed line reader: "
					"std::priority_queue::push() failed");
			}
		}
	}

	Swap(&temp);
}

void BackMerger::Close()
{
	for (std::size_t i = 0; i < readers_.size(); ++i)
	{
		if (readers_[i] != NULL)
			delete readers_[i];
		readers_[i] = NULL;
	}
	std::vector<ParsedLineReader *>().swap(readers_);

	queue_.reset();
	top_ = NULL;
	length_ = 0;
}

bool BackMerger::Next()
{
	int last_key = -1;
	long long last_value = 0;
	if (top_ != NULL)
	{
		last_key = key();
		last_value = value();
		if (top_->Next())
			queue_->push(top_);
	}

	if (queue_->empty())
		return false;

	top_ = queue_->top();
	queue_->pop();

	Encode(last_key, last_value);
	return true;
}

void BackMerger::Swap(BackMerger *target)
{
	PriorityQueue *temp = queue_.release();
	queue_ = target->queue_;
	target->queue_.reset(temp);

	readers_.swap(target->readers_);
	std::swap(top_, target->top_);
	std::swap(length_, target->length_);
}

void BackMerger::Encode(int last_key, long long last_value)
{
	length_ = 0;

	long long value_diff = (last_key == key()) ?
		(last_value - value()) : value();
	unsigned char byte = static_cast<unsigned char>(value_diff & 0x7F);
	while ((value_diff >>= 7) > 0)
	{
		bytes_[length_++] = byte | 0x80;
		byte = static_cast<unsigned char>(value_diff & 0x7F);
	}
	bytes_[length_++] = byte;

	const char *bytes = top_->bytes();
	std::size_t begin = 0;
	while (static_cast<unsigned char>(bytes[begin]) >= 0x80)
		++begin;
	++begin;
	std::size_t end = top_->length() - 1;
	while (static_cast<unsigned char>(bytes[end - 1]) >= 0x80)
		--end;

	std::memcpy(&bytes_[length_], &bytes[begin], end - begin);
	length_ += end - begin;
}

}  // namespace ssgnc
