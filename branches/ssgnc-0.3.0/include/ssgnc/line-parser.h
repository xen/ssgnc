#ifndef SSGNC_LINE_PARSER_H
#define SSGNC_LINE_PARSER_H

#include <ssgnc/parsed-line.h>
#include <ssgnc/vocab-dic.h>

namespace ssgnc {

class LineParser
{
public:
	LineParser() : dic_(NULL) {}
	explicit LineParser(const VocabDic &dic) : dic_(&dic) {}
	~LineParser() { Clear(); }

	const VocabDic &dic() { return *dic_; }
	void set_dic(const VocabDic &dic) { dic_ = &dic; }

	void Parse(const char *line, ParsedLine *parsed_line);

	void Clear() { dic_ = NULL; }
	void Swap(LineParser *target) { std::swap(dic_, target->dic_); }

private:
	const VocabDic *dic_;

	// Disallows copies.
	LineParser(const LineParser &);
	LineParser &operator=(const LineParser &);
};

}  // namespace ssgnc

#endif  // SSGNC_LINE_PARSER_H
