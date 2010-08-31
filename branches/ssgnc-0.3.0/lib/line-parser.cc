#include <ssgnc/line-parser.h>

#include <ssgnc/exception.h>

#include <cstdlib>
#include <limits>

namespace ssgnc {

void LineParser::Parse(const char *line, ParsedLine *parsed_line)
{
	parsed_line->Clear();
	do
	{
		std::size_t length = 0;
		while (line[length] != '\0')
		{
			if (line[length] == ' ' || line[length] == '\t')
				break;
			++length;
		}
		if (line[length] == '\0')
			SSGNC_THROW("failed to parse line: unexpected end of line");

		int key;
		if (!dic_->Find(line, length, &key))
		{
			SSGNC_THROW("failed to parse line: "
				"ssgnc::VocabDic::Find() failed");
		}
		parsed_line->append_key(key);

		line += length;
	} while (*line++ != '\t');

	char *value_end;
	long long value = std::strtoll(line, &value_end, 10);
	if (*value_end != '\0')
		SSGNC_THROW("failed to parse line: invalid byte in value");
	else if (value <= 0)
		SSGNC_THROW("failed to parse line: zero or less value");
	else if (value == std::numeric_limits<long long>::max())
		SSGNC_THROW("failed to parse line: too large value");
	parsed_line->set_value(value);
}

}  // namespace ssgnc
