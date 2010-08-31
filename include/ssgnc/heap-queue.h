#ifndef SSGNC_HEAP_QUEUE_H
#define SSGNC_HEAP_QUEUE_H

#include "int-types.h"

#include <functional>
#include <vector>

namespace ssgnc {

template <typename T, typename LessThan = std::less<T> >
class HeapQueue
{
public:
	HeapQueue() : buf_(), less_than_() {}
	~HeapQueue() { clear(); }

	void clear() { buf_.clear(); }

	void push(const T &value);
	void pop();

	void popPush(const T &value);

	const T &top() const { return buf_[getRootIndex()]; }

	bool empty() const { return buf_.empty(); }
	UInt32 size() const { return buf_.size(); }

private:
	std::vector<T> buf_;
	LessThan less_than_;

	static UInt32 getRootIndex() { return 0; }
	static UInt32 getParentIndex(UInt32 index) { return (index - 1) / 2; }
	static UInt32 getChildIndex(UInt32 index) { return (index * 2) + 1; }

	// Disallows copies.
	HeapQueue(const HeapQueue &);
	HeapQueue &operator=(const HeapQueue &);
};

template <typename T, typename LessThan>
void HeapQueue<T, LessThan>::push(const T &value)
{
	UInt32 index = static_cast<UInt32>(buf_.size());
	buf_.resize(buf_.size() + 1);
	while (index > getRootIndex())
	{
		UInt32 parent_index = getParentIndex(index);
		if (!less_than_(value, buf_[parent_index]))
			break;

		buf_[index] = buf_[parent_index];
		index = parent_index;
	}
	buf_[index] = value;
}

template <typename T, typename LessThan>
void HeapQueue<T, LessThan>::pop()
{
	const T &value = buf_.back();
	popPush(value);
	buf_.pop_back();
}

template <typename T, typename LessThan>
void HeapQueue<T, LessThan>::popPush(const T &value)
{
	UInt32 index = getRootIndex();
	for ( ; ; )
	{
		UInt32 child_index = getChildIndex(index);
		if (child_index >= buf_.size())
			break;

		if (child_index + 1 < buf_.size() &&
			less_than_(buf_[child_index + 1], buf_[child_index]))
			++child_index;

		if (!less_than_(buf_[child_index], value))
			break;

		buf_[index] = buf_[child_index];
		index = child_index;
	}
	buf_[index] = value;
}

}  // namespace ssgnc

#endif  // SSGNC_HEAP_QUEUE_H
