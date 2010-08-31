#include <ssgnc/front-merger.h>

#include <ssgnc/exception.h>

namespace ssgnc {

void FrontMerger::Open(TempFile *file, const std::vector<std::size_t> &points)
{
	FrontMerger temp;

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

void FrontMerger::Close()
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
}

bool FrontMerger::Next()
{
	if (top_ != NULL)
	{
		if (top_->Next())
			queue_->push(top_);
	}

	if (queue_->empty())
		return false;

	top_ = queue_->top();
	queue_->pop();

	return true;
}

void FrontMerger::Swap(FrontMerger *target)
{
	PriorityQueue *temp = queue_.release();
	queue_ = target->queue_;
	target->queue_.reset(temp);

	readers_.swap(target->readers_);
	std::swap(top_, target->top_);
}

}  // namespace ssgnc
