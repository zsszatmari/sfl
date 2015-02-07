#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "SqliteBackend.h"

using namespace ClientDb;

TEST(Sqlite, GreaterThenClause)
{
	Predicate first;
	Predicate second;
	std::vector<Predicate> subs;
	subs.push_back(first);
	subs.push_back(second);
	Predicate predicate(Predicate::Operator::And, subs);
	SqliteBackend backend;
	auto where = backend.whereClause("Item", predicate);
	EXPECT_EQ("WHERE (1) AND (1)", where.first);
}