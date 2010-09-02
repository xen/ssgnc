#ifndef SSGNC_HEAP_QUEUE_H
#define SSGNC_HEAP_QUEUE_H

#include "common.h"

namespace ssgnc {

template <typename T, typename LessThan = std::less<T> >
class HeapQueue
{
public:
	HeapQueue() : buf_(), less_than_() {}
	~HeapQueue() { clear(); }

	void clear() { buf_.clear(); }

	bool push(const T &value) SSGNC_WARN_UNUSED_RESULT;
	bool pop();

	bool popPush(const T &value);

	bool top(T *value) const;

	bool empty() const { return buf_.empty(); }
	UInt32 size() const { return buf_.size(); }

private:
	std::vector<T> buf_;
	LessThan less_than_;

	void popPushInside(const T &value);

	static UInt32 getRootIndex() { return 0; }
	static UInt32 getParentIndex(UInt32 index) { return (index - 1) / 2; }
	static UInt32 getChildIndex(UInt32 index) { return (index * 2) + 1; }

	// Disallows copies.
	HeapQueue(const HeapQueue &);
	HeapQueue &operator=(const HeapQueue &);
};

template <typename T, typename LessThan>
bool HeapQueue<T, LessThan>::push(const T &value)
{
	UInt32 index = static_cast<UInt32>(buf_.size());

	try
	{
		buf_.resize(buf_.size() + 1);
	}
	catch (...)
	{
		SSGNC_ERROR << "std::vector<T>::resize() failed: "
			<< sizeof(T) << " * " << (buf_.size() + 1) << std::endl;
		return false;
	}

	while (index > getRootIndex())
	{
		UInt32 parent_index = getParentIndex(index);
		if (!less_than_(value, buf_[parent_index]))
			break;

		buf_[index] = buf_[parent_index];
		index = parent_index;
	}
	buf_[index] = value;
	return true;
}

template <typename T, typename LessThan>
bool HeapQueue<T, LessThan>::pop()
{
	if (buf_.empty())
	{
		SSGNC_ERROR << "Empty heap queue" << std::endl;
		return false;
	}

	const T &value = buf_.back();
	popPushInside(value);
	buf_.pop_back();
	return true;
}

template <typename T, typename LessThan>
bool HeapQueue<T, LessThan>::popPush(const T &value)
{
	if (buf_.empty())
	{
		SSGNC_ERROR << "Empty heap queue" << std::endl;
		return false;
	}

	popPushInside(value);
	return true;
}

template <typename T, typename LessThan>
void HeapQueue<T, LessThan>::popPushInside(const T &value)
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

template <typename T, typename LessThan>
bool HeapQueue<T, LessThan>::top(T *value) const
{
	if (buf_.empty())
	{
		SSGNC_ERROR << "Empty heap queue" << std::endl;
		return false;
	}

	*value = buf_[getRootIndex()];
	return true;
}

}  // namespace ssgnc

#endif  // SSGNC_HEAP_QUEUE_H
