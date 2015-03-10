#include "test.h"
#include "SqliteBackend.h"

using namespace ClientDb;

TEST(FullTextSearch, Query)
{
	SqliteBackend backend;
	std::map<std::string, std::vector<Index>> indexes;
	indexes["Song"].push_back(Index("Song","fts",{"artist","title"},true));
	backend.open("", indexes, nullptr);


	auto clause = backend.whereClause("Song", 
		Predicate::compositeOr(Predicate(Predicate::Operator::Contains, "artist", "front"),
							   Predicate(Predicate::Operator::Contains, "title", "front")));
	// it must recognize that all the indexed fields are searched in and act accordingly, instead of ORing them together
	EXPECT_EQ("WHERE rowid IN (SELECT docid FROM SongFTS WHERE SongFTS MATCH ?500)", clause.first);
}