#ifndef SSGNC_TOOLS_COMMON_H
#define SSGNC_TOOLS_COMMON_H

#include "ssgnc.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>

#define ERROR (std::cerr << __FILE__ << ':' << __LINE__ << ": error: ")

namespace ssgnc {
namespace tools {

void initIO();

bool parseNumTokens(const Int8 *str, Int32 *num_tokens);

bool readVocabDic(const char *path, ssgnc::VocabDic *vocab_dic);

bool mmapVocabDic(const char *path, ssgnc::FileMap *file_map,
	ssgnc::VocabDic *vocab_dic);

bool initFilePath(const char *temp_dir, const char *file_ext,
	Int32 num_tokens, FilePath *file_path);

bool openFiles(const char *temp_dir, const char *file_ext,
	Int32 num_tokens, std::vector<std::ifstream *> *files);

}  // namespace tools
}  // namespace ssgnc

#endif  // SSGNC_TOOLS_COMMON_H
