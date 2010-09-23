#ifndef SSGNC_TOOLS_COMMON_H
#define SSGNC_TOOLS_COMMON_H

#include <ssgnc.h>

#include <cstdlib>
#include <string>

namespace ssgnc {
namespace tools {

enum { MIN_NUM_TOKENS = 1, MAX_NUM_TOKENS = 30 };
enum { MIN_MEM_LIMIT = 512, MAX_MEM_LIMIT = 4096 };

void initIO();

bool parseInt64(const Int8 *str, Int64 *value);
bool parseNumTokens(const Int8 *str, Int32 *num_tokens);
bool parseMemLimit(const Int8 *str, UInt64 *mem_limit);

bool readLine(std::istream *stream, std::string *line);

bool readFreq(ByteReader *byte_reader, StringBuilder *buf, Int16 *freq);
bool readTokens(Int32 num_tokens, const VocabDic &vocab_dic,
	ByteReader *byte_reader, StringBuilder *buf, std::vector<Int32> *tokens);

bool initFilePath(const String &dirname, const String &file_ext,
	Int32 num_tokens, FilePath *file_path);
bool openFiles(const String &dirname, const String &file_ext,
	Int32 num_tokens, std::vector<std::ifstream *> *files);
bool closeFiles(std::vector<std::ifstream *> *files);

}  // namespace tools
}  // namespace ssgnc

#endif  // SSGNC_TOOLS_COMMON_H
