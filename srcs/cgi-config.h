#ifndef GOOGLE_NGRAM_DIRECTORY_CONFIGURE_H
#define GOOGLE_NGRAM_DIRECTORY_CONFIGURE_H

namespace ngram
{

class cgi_config
{
public:
	static const char *english_dir()
	{ return "/mnt/raid/google/ngram/index/english"; }
	static const char *japanese_dir()
	{ return "/mnt/raid/google/ngram/index/japanese"; }

private:
	cgi_config();
	~cgi_config();
};

}  // namespace ngram

#endif  // GOOGLE_NGRAM_DIRECTORY_CONFIGURE_H
