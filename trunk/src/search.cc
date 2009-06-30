#include "ssgnc/file-mapper.h"
#include "ssgnc/inverted-db.h"
#include "ssgnc/path-generator.h"
#include "ssgnc/vocab-dic.h"
#include "ssgnc/vocab-index.h"

#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

namespace {

bool AnalyzeOptions(int argc, char *argv[],
	boost::program_options::variables_map *vmap)
{
	namespace opts = boost::program_options;

	opts::options_description options("Options");
	options.add_options()
		("help,h", "display this help and exit")
		("dir,d", opts::value<std::string>(), "index directory (required)")
		("order,o", opts::value<std::string>()->default_value("unordered"),
			"query order (unordered, ordered, phrase or fixed)")
		("results,r", opts::value<long long>()->default_value(20),
			"maximum number of results")
		("freq,f", opts::value<long long>()->default_value(0),
			"minimum frequency");

	opts::store(opts::parse_command_line(argc, argv, options), *vmap);
	opts::notify(*vmap);

	if (vmap->count("help") || !vmap->count("dir"))
	{
		std::cerr << options << std::endl;
		return false;
	}

	return true;
}

}  // namespace

int main(int argc, char *argv[])
{
	boost::program_options::variables_map vmap;
	if (!AnalyzeOptions(argc, argv, &vmap))
		return 1;

	std::string input_dir = vmap["dir"].as<std::string>();
	ssgnc::PathGenerator path_gen(input_dir);

	// Opens a vocabulary dictionary.
	ssgnc::FileMapper dic_file;
	std::cerr << "VocabDic: " << path_gen.VocabDicPath() << std::endl;
	if (!dic_file.Map(path_gen.VocabDicPath().c_str()))
	{
		std::cerr << "error: failed to map dictionary" << std::endl;
		return 1;
	}
	ssgnc::VocabDic vocab_dic;
	vocab_dic.MapDic(dic_file);

	// Opens a vocabulary index.
	ssgnc::FileMapper index_file;
	std::cerr << "VocabIndex: " << path_gen.VocabIndexPath() << std::endl;
	if (!index_file.Map(path_gen.VocabIndexPath().c_str()))
	{
		std::cerr << "error: failed to map index" << std::endl;
		return 1;
	}
	ssgnc::VocabIndex vocab_index;
	vocab_index.MapIndex(index_file);

	// Opens databases.
	std::vector<boost::shared_ptr<ssgnc::FileMapper> > db_files(1);
	std::vector<boost::shared_ptr<ssgnc::InvertedDb> > dbs(1);
	for (int i = 1; ; ++i)
	{
		boost::shared_ptr<ssgnc::FileMapper> db_file(new ssgnc::FileMapper);
		if (!db_file->Map(path_gen.DbPath(i).c_str()))
			break;
		std::cerr << "Database: " << path_gen.DbPath(i) << std::endl;
		boost::shared_ptr<ssgnc::InvertedDb> db(new ssgnc::InvertedDb);
		db->MapDb(i, *db_file);

		db_files.push_back(db_file);
		dbs.push_back(db);
	}

	std::string line;
	while (std::getline(std::cin, line))
	{
		std::vector<std::string> key_strings;
		boost::split(key_strings, line, boost::is_space());

		// Gets key IDs.
		std::vector<int> key_ids;
		for (std::size_t i = 0; i < key_strings.size(); ++i)
		{
			int key_id;
			if (!vocab_dic.Find(key_strings[i].c_str(), &key_id))
			{
				std::cerr << "warning: failed to find key: "
					<< key_strings[i] << std::endl;
				break;
			}
			key_ids.push_back(key_id);
			std::cerr << "Key: " << key_strings[i]
				<< ": " << key_id << std::endl;
		}
		if (key_ids.size() != key_strings.size())
			continue;

		// Searches n-grams from databases.
		for (std::size_t i = key_ids.size(); i < dbs.size(); ++i)
		{
			// Finds the uniquest key.
			std::size_t min_id = 0;
			std::size_t min_size = static_cast<std::size_t>(1) << 63;
			for (std::size_t j = 0; j < key_ids.size(); ++j)
			{
				ssgnc::DbReader reader;
				dbs[i]->Find(key_ids[j], &reader);
				if (reader.size() < min_size)
				{
					min_id = j;
					min_size = reader.size();
				}
			}
			std::cerr << "Size: " << key_strings[min_id]
				<< ": " << min_size << '\n';
			ssgnc::DbReader reader;
			if (!dbs[i]->Find(key_ids[min_id], &reader))
				continue;

			// Reads n-grams.
			ssgnc::Ngram ngram;
			while (reader.Read(&ngram))
			{
				std::cout << ngram.freq() << ':';
				for (int j = 0; j < ngram.key_size(); ++j)
				{
					const char *key_string;
					if (vocab_index.Find(ngram.key(j), &key_string))
						std::cout << ' ' << key_string;
					else
					{
						std::cerr << "error: failed to find key: "
							<< ngram.key(j) << std::endl;
					}
				}
				std::cout << '\n';
			}
		}
	}

	return 0;
}
