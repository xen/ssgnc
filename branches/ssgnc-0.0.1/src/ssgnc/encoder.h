#ifndef SSGNC_ENCODER_H
#define SSGNC_ENCODER_H

#include "varint-writer.h"

#include <stdio.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace ssgnc {

class Encoder
{
public:
	explicit Encoder(std::ostream *output)
		: byte_writer_(output), writer_(&byte_writer_), tmp_file_(NULL),
		last_key_id_(-1), last_address_(0), finished_(false) {}

	~Encoder()
	{
		if (tmp_file_ != NULL)
			::fclose(tmp_file_);
	}

	// Creates a temporary file.
	bool Init()
	{
		if (tmp_file_ != NULL)
			return false;

		tmp_file_ = ::tmpfile();
		return tmp_file_ != NULL;
	}

	// Encodes a pair of a key ID and an address.
	bool Encode(const char *line)
	{
		if (finished_)
			return false;

		char *delim;
		int key_id = static_cast<int>(std::strtol(line, &delim, 10));
		if (key_id < 0 || *delim != ' ')
			return false;

		std::size_t address = std::strtoll(delim + 1, &delim, 10);
		if (address < 0 || *delim != '\0')
			return false;

		if (!EncodePair(key_id, address))
			return false;

		last_key_id_ = key_id;
		last_address_ = address;
		return true;
	}

	// Outputs start positions of posting lists.
	bool Finish()
	{
		static const std::size_t ADDRESS_SIZE = sizeof(std::size_t);

		if (finished_)
			return false;

		// Writes the end position and the last key ID.
		if (!SavePosition(last_key_id_ + 1) ||
			::fwrite(&last_key_id_, sizeof(last_key_id_), 1, tmp_file_) != 1)
			return false;

		::rewind(tmp_file_);

		// Alignment.
		while ((byte_writer_.Total() % ADDRESS_SIZE) != 0)
			byte_writer_.Write(0);

		// Writes positions.
		char buf[ADDRESS_SIZE];
		for (int i = -1; i <= last_key_id_; ++i)
		{
			if (::fread(buf, ADDRESS_SIZE, 1, tmp_file_) != 1)
				return false;
			for (std::size_t j = 0; j < ADDRESS_SIZE; ++j)
				byte_writer_.Write(buf[j]);
		}

		// Writes the last key ID.
		if (::fread(buf, sizeof(last_key_id_), 1, tmp_file_) != 1)
			return false;
		for (std::size_t i = 0; i < sizeof(last_key_id_); ++i)
			byte_writer_.Write(buf[i]);

		finished_ = true;
		return true;
	}

private:
	ssgnc::ByteWriter byte_writer_;
	ssgnc::VarintWriter writer_;
	FILE *tmp_file_;

	int last_key_id_;
	std::size_t last_address_;
	bool finished_;

	// Disallows copies.
	Encoder(const Encoder &);
	Encoder &operator=(const Encoder &);

private:
	// Encodes a pair of a key ID and an address.
	bool EncodePair(int key_id, std::size_t address)
	{
		if (key_id < last_key_id_)
			return false;

		if (key_id != last_key_id_)
		{
			SavePosition(key_id);
			last_address_ = 0;
		}

		if (address < last_address_)
			return false;

		std::size_t diff = address - last_address_;
		writer_.Write(diff);
		return true;
	}

	// Saves a current position into a temporary file.
	bool SavePosition(int key_id)
	{
		std::size_t position = byte_writer_.Total();
		for (int i = last_key_id_; i < key_id; ++i)
		{
			if (::fwrite(&position, sizeof(position), 1, tmp_file_) != 1)
				return false;
		}
		return true;
	}
};

}  // ssgnc

#endif  // SSGNC_ENCODER_H
