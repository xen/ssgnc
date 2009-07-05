#ifndef SSGNC_UNIFIED_DB_H
#define SSGNC_UNIFIED_DB_H

#include "inverted-db.h"
#include "path-generator.h"

#include <vector>

#include <boost/shared_ptr.hpp>

namespace ssgnc {

class UnifiedDb
{
public:
	UnifiedDb() : db_files_(), dbs_() {}

	// Returns the number of databases.
	int db_size() const { return dbs_.size(); }
	const InvertedDb &db(int id) const { return *dbs_[id]; }

	// Closes all databases.
	void Clear()
	{
		db_files_.clear();
		dbs_.clear();
	}

	// Opens databases in a directory.
	bool Open(const PathGenerator &path_gen)
	{
		Clear();
		for (int i = 1; ; ++i)
		{
			if (!AddDb(i, path_gen.DbPath(i).c_str()))
				break;
		}
		return dbs_.size() > 0;
	}

private:
	std::vector<boost::shared_ptr<FileMapper> > db_files_;
	std::vector<boost::shared_ptr<InvertedDb> > dbs_;

	// Disallows copies.
	UnifiedDb(const UnifiedDb &);
	UnifiedDb &operator=(const UnifiedDb &);

	// Opens a database.
	bool AddDb(int n, const char *file_name)
	{
		boost::shared_ptr<ssgnc::FileMapper> db_file(new ssgnc::FileMapper);
		if (!db_file->Map(file_name))
			return false;

		boost::shared_ptr<ssgnc::InvertedDb> db(new ssgnc::InvertedDb);
		db->MapDb(n, *db_file);

		db_files_.push_back(db_file);
		dbs_.push_back(db);
		return true;
	}
};

}  // namespace ssgnc

#endif  // SSGNC_UNIFIED_DB_H
