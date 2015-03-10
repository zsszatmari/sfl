#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include "SqliteBackend.h"
#include "sqlite/sqlite3.h"
#include "SqliteStatement.h"
#include "sfl/Split.h"
#include "sfl/ImmutableVector.h"

namespace ClientDb
{
#define method SqliteBackend::

	using std::function;

	method SqliteBackend()
	{
		for (int i = 0 ; i < connectionCount ; i++) {
			_db[i] = nullptr;
		}
	}

	method ~SqliteBackend()
	{
		for (int i = 0 ; i < connectionCount ; i++) {
			if (_db[i]) {
				sqlite3_close(_db[i]);
			}
#ifdef SERIALIZED_DATABASE
			break;
#endif
		}
	}

	static int sqlite_multiopen(const char *filename, sqlite3 **ppDb, int flags, int count, Comparator comparator)
	{
		for (int i = 0 ; i < count ; i++) {
#ifdef SERIALIZED_DATABASE
			if (i > 0) {
				ppDb[i] = ppDb[0];
				continue;
			}
#endif
			auto result = sqlite3_open_v2(filename, ppDb + i, flags, nullptr);
			if (SQLITE_OK != result) {
				for (int j = 0 ; j < i ; ++j) {
					sqlite3_close(ppDb[j]);
					ppDb[j] = 0;
				}
				return result;
			}
			sqlite3_create_collation(ppDb[i], "Custom", SQLITE_UTF8, nullptr, comparator);
		}
		return SQLITE_OK;
	}  

	bool method open(const std::string &path, const std::map<std::string,std::vector<Index>> &indexes, Comparator comparator)
	{
		_indexes = indexes;
		// TODO: Note to Windows Runtime users: The temporary directory must be set prior to calling sqlite3_open() or sqlite3_open_v2(). Otherwise, various features that require the use of temporary files may fail.
		
		// fullmutex: serialized mode

#ifdef SERIALIZED_DATABASE
// SQLITE_OPEN_FULLMUTEX: makes database access safer, but makes it pointless to have two separate connections...
// SQLITE_OPEN_NOMUTEX: the normal mulithreading mode (still thread safe if one connection not used in several threads)

		if (SQLITE_OK != sqlite_multiopen(path.c_str(), _db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_SHAREDCACHE, connectionCount, comparator)) {
			
			std::cout << "failed to open " << path << " keeping things in memory" << std::endl;
			if (SQLITE_OK != sqlite_multiopen("file::memory:?cache=shared", _db, SQLITE_OPEN_URI | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_SHAREDCACHE, connectionCount, comparator)) {
				return false;
			}
		}
#else
		if (SQLITE_OK != sqlite_multiopen(path.c_str(), _db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, connectionCount, comparator)) {
			
			std::cout << "failed to open " << path << " keeping things in memory" << std::endl;
			if (SQLITE_OK != sqlite_multiopen("file::memory:?cache=shared", _db, SQLITE_OPEN_URI | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, connectionCount, comparator)) {
				return false;
			}
		}
#endif
		// TODO: check unused pages and VACUUM accordingly
		// TODO: add custom collation (text ordering) 

		#pragma message("TODO: reverse ordering does not work")

		// this makes an approximately 1.4x speedup for initial fetching
		execute("PRAGMA journal_mode=WAL");
		execute("PRAGMA cache_size=-25000");
		execute("PRAGMA temp_store=MEMORY");
		execute("PRAGMA synchronous=OFF");
		//execute("PRAGMA synchronous=NORMAL");

		for (auto &entity : indexes) {
			auto &entityName = entity.first;
			
			auto &entityIndexes = entity.second;
			if (entityIndexes.empty()) {
				throw std::runtime_error("there must be at least one index");
			} else {
				/*auto &fields = entityIndexes[0].fields();
				if (fields.empty()) {
					throw std::runtime_error("there must be at least one field in the first index");
				}*/
				// an 'integer' primary key becomes an alias for the rowid with the added
				// effect that it won't change on vacuuming. it's necessary to stay in sync with an fts table
				if (!execute("CREATE TABLE IF NOT EXISTS " + entityName + " (r INTEGER PRIMARY KEY)")) {
					std::cout << "not okk\n";

					return false;
				}
			}			

			
			std::vector<std::string> columns;
			for (auto &index : entityIndexes) {
				for (auto &column : index.fields()) {
					if (find(columns.begin(), columns.end(), column) == columns.end()) {
						columns.push_back(column);
						execute(std::string("ALTER TABLE ") + entityName + " ADD COLUMN " + column + (comparator ? " COLLATE Custom": ""), false);
					}
				}
			}
			_columns[entityName] = columns;
			for (auto &index : entityIndexes) {
				if (index.fullTextIndex()) {
					_fullTextIndex[entityName] = index.fields();
					{
						std::stringstream ss;
						ss << "CREATE VIRTUAL TABLE " + entityName + "FTS USING fts4(content=\"" << entityName 
						   << "\",matchinfo=fts3,tokenize=unicode61";
						for (auto &field : index.fields()) {
							ss << "," << field;
						} 
						ss << ")";
						if (!execute(ss.str(), false)) {
							//std::cout << "not ok virtual\n";
						}
					}
					{
						std::stringstream ss;
						ss << "CREATE TRIGGER " << entityName << "_ai AFTER INSERT ON " << entityName << " BEGIN " 
								<< "INSERT INTO " << entityName << "FTS (docid";
						for (auto &field : index.fields()) {
							ss << "," << field;
						}
						ss << ") VALUES(new.rowid";
						for (auto &field : index.fields()) {
							ss << ",new." << field;
						}
						ss << "); END";
						if (!execute(ss.str(),false)) {
							//std::cout << "not ok triggerinsert\n";
						}
					}
					{
						std::stringstream ss;
						ss << "CREATE TRIGGER " << entityName << "_au AFTER UPDATE ON " << entityName << " BEGIN "
								<< "UPDATE " << entityName << "FTS SET ";
						bool first = true;
						for (auto &field : index.fields()) {
							if (!first) {
								ss << ",";
							}
							ss << field << "=" << "new." << field;
							first = false;
						}
						ss << " WHERE docid=new.rowid; END";
						if (!execute(ss.str(),false)) {
							//std::cout << "not ok triggerupdate\n";
						}
					}
					{
						std::stringstream ss;
						ss << "CREATE TRIGGER " << entityName << "_bd BEFORE DELETE ON " << entityName << " BEGIN " 
								<< "DELETE FROM " << entityName << "FTS WHERE docid=old.rowid; END";
						if (!execute(ss.str(),false)) {
							//std::cout << "not ok triggerdelete\n";
						}	
					}

					continue;
				}
				std::stringstream indexName;
				std::stringstream columnList;
				indexName << entityName;
				bool first = true;
				for (auto &column : index.fields()) {
					indexName << column;
					if (first == true) {
						first = false;
					} else {
						columnList << ",";
					}
					columnList << column;
				}

				// todo: unique?
				if (!execute("CREATE INDEX IF NOT EXISTS " + indexName.str() + " ON " + entityName + " (" + columnList.str() + ")")) {

					std::cout << "not ok\n";
					return false;
				}
			}
		}
		return true;
	}

	void method freeUpMemory(int connectionId)
	{
		execute("PRAGMA shrink_memory", true, connectionId);
	}

	bool method execute(const std::string &statement, bool throwOnError, int connectionId)
	{
		SqliteStatement stmt(_db[connectionId], statement, throwOnError);
		return stmt.step();
	}

	void method destroy(const std::string &path)
	{
		::remove(path.c_str());
	}

	Json::Value method upsert(const std::string &entity, Json::Value &value, const std::function<void(const Json::Value &, Json::Value &)> &oldObjectHandler)
	{
		const auto &primaryIndex = _indexes[entity][0];

		Json::Value oldValue;

		std::string whereClause;
		{
			std::stringstream sql;
			sql << " WHERE ";
			bool first = true;
			for (auto &field : primaryIndex.fields()) {
				if (first) {
					first = false;
				} else {
					sql << "AND ";
				}
				if (value[field].isNull()) {
					sql << field << " IS NULL ";
				} else {
					sql << field << " == ? ";
				}
			}
			whereClause = sql.str();
		}

		{
			// based on primary key...
			std::stringstream sql;
			sql << "SELECT ";
			bool first = true;
			const auto &fields = _columns[entity];
			for (auto &field : fields) {
				if (first) {
					first = false;
				} else {
					sql << ",";
				}
				sql << field;
			}
			sql << " FROM " << entity << whereClause;
			
			SqliteStatement stmt(_db[0], sql.str());
			for (auto &field : primaryIndex.fields()) {
				// let it throw exception if these are missing...
				if (!value[field].isNull()) {
					stmt.bindNext(value[field]);
				}
			}

			oldValue = stmt.fetchRow(fields);
		}

		if (oldObjectHandler) {
			oldObjectHandler(oldValue, value);
		}
		if (!oldValue.isNull()) {

			bool equals = true;
			for (const auto &member : value.getMemberNames()) {
				auto oldField = oldValue.get(member, Json::nullValue);
				// if the old field is zero, we might still want to put in the new value (otherwise how would we )
				if (/*!oldField.isNull() &&*/ value.get(member, Json::nullValue) != oldField) {
		   			Json::Value leftDebug = value.get(member, Json::nullValue);
		   			Json::Value rightDebug = oldValue.get(member, Json::nullValue);
           			Json::FastWriter writer;
           			//std::cout << "key: " << member << " oldvalue: " << writer.write(value) << " newvalue: " << writer.write(oldValue) << std::endl;

           			equals = false;
           			break;
           		}
			}

			// deliberately assymmetric: surely there are fields the client does not want to change
            //bool equals = (oldValue == value);
            
			if (!equals) {
				Json::FastWriter writer;
				//std::cout << "old: " << writer.write(oldValue) << " vs. new: " << writer.write(value) << std::endl;

				std::stringstream sql;
				sql << "UPDATE " << entity << " SET ";
				auto &indexFields = primaryIndex.fields();
				auto enumerateNonIndexFields = [&](const function<void(const std::string &)> &f){
					for (std::string &field : value.getMemberNames()) {
						if (find(indexFields.begin(), indexFields.end(), field) == indexFields.end()) {
							f(field);
						}
					}
				};
				
				bool first = true;
				enumerateNonIndexFields([&](const std::string &field){
					if (value.get(field,Json::nullValue).isNull()) {
						return;
					}
					if (first) {
						first = false;
					} else {
						sql << ",";
					}
					sql << field << " = ?";
				});
				sql << whereClause;

				SqliteStatement stmt(_db[0], sql.str());

				enumerateNonIndexFields([&](const std::string &field){
					auto v = value.get(field,Json::nullValue);
					if (v.isNull()) {
						return;
					}
					stmt.bindNext(v);
				});
				for (auto &field : primaryIndex.fields()) {
                    if (!value[field].isNull()) {
                        stmt.bindNext(value[field]);
                    }
				}

				stmt.step();
			}
		} else {
			std::stringstream sql;
			sql << "INSERT INTO " << entity << " (";
	
			Json::FastWriter writer;
			//std::cout << "inserting sqlite: " << writer.write(value) << std::endl;

			bool first = true;
			for (std::string &field : value.getMemberNames()) {
				if (value.get(field,Json::nullValue).isNull()) {
					continue;
				}
				if (first) {
					first = false;
				} else {
					sql << ",";
				}
				sql << field;
			}
			sql << ") VALUES (";
			first = true;
			for (std::string &field : value.getMemberNames()) {
				if (value.get(field,Json::nullValue).isNull()) {
					continue;
				}

				if (first) {
					first = false;
				} else {
					sql << ",";
				}

				sql << "?";
			}
			sql << ")";

			SqliteStatement stmt(_db[0], sql.str());
			for (std::string &field : value.getMemberNames()) {
				auto v = value.get(field,Json::nullValue);
				if (v.isNull()) {
					continue;
				}
				stmt.bindNext(v);
			}

			stmt.step();
		}

		return oldValue;
	}
	
	static std::pair<std::string,std::string> fullTextSearchClauseAndBind(const std::string &entity, int index, const std::vector<std::string> &fields, const std::string &query)
	{
		std::stringstream clause;
		clause << "rowid" << " IN (SELECT docid FROM " << entity << "FTS WHERE " << entity << "FTS MATCH ?" << index << ")";

		// like: (title:a* OR artist:a*) AND (title:b* OR artist:b*)
		// because we are using the 'standard query syntax', OR has highest precedence, and AND is explicit, so it becomes:
		// title:a* OR artist:a* title:b* OR artist:b*
#ifdef SQLITE_ENABLE_FTS3_PARENTHESIS
#error it is assumed here that the enhanced query syntax is turned off
#endif

		std::stringstream strstr(query);
		std::istream_iterator<std::string> it(strstr);
		std::istream_iterator<std::string> end;
		std::vector<std::string> texts(it, end);

		std::stringstream b;
		bool firstText = true;
		for (auto &text : texts) {
			if (firstText) {
				firstText = false;
			} else {
				b << " ";
			}
			if (fields.empty()) {
				b << text << "*";
			} else {
				bool first = true;
				for (auto &field : fields) {
					if (first) {
						first = false;
					} else {
						b << " OR ";
					}
					b << field << ":" << text << "*";
				}
			}
		}
		//std::cout << "full text match: " << b.str() << std::endl;
		return std::make_pair(clause.str(), b.str());
	}

	// returns a query string, and a function that will bind the parameters
	std::pair<std::string,function<void(SqliteStatement&)>> method whereClause(const std::string &entity, const Predicate &predicate, const std::string &prefix)
	{
		int index = 500;
		std::map<int,Json::Value> toBind;

		function<std::string(const Predicate &p)> process = [&toBind,&index,&process,this,&entity](const Predicate &p)->std::string{
			
			// bear in mind that null must come after anything else
			// we must handle nulls in predicates since very often that's what we get from the db and we must navigate based on them
			switch(p.op()) {
				case Predicate::Operator::Equals:
					if (p.value().isNull()) {
						return p.key() + " IS NULL";
					}
					toBind[index] = p.value();
					// +: if we have an index implied by the ordering, we can in many cases forget about 
					// the where cluase for query optimization
					return std::string("+") + p.key() + " == ?" + std::to_string(index++); 
				case Predicate::Operator::Contains: {
					std::string text = p.value().asString();
					if (text.empty()) {
						return std::string("1");
					}
					
					auto it = _fullTextIndex.find(entity);
					if (it != _fullTextIndex.end() && find(it->second.begin(),it->second.end(), p.key()) != it->second.end()) {
						std::vector<std::string> fields = {p.key()};

						std::pair<std::string, std::string> clauseAndBind = fullTextSearchClauseAndBind(entity, index, fields, p.value().asString());
						toBind[index] = clauseAndBind.second;
						++index;
						return clauseAndBind.first;
					} else {
						toBind[index] = std::string("%") + text + "%";
						return p.key() + " LIKE ?" + std::to_string(index++);
					}
				}
				case Predicate::Operator::NonEmpty: {
					return "length(" + p.key() + ") > 0";
				}
				case Predicate::Operator::LessThan:
					if (p.value().isNull()) {
						return p.key() + " IS NOT NULL";
					}
					toBind[index] = p.value();
					return p.key() + " < ?" + std::to_string(index++); 
				case Predicate::Operator::LessEquals:
					if (p.value().isNull()) {
						return "1";
					}
					toBind[index] = p.value();
					return p.key() + " <= ?" + std::to_string(index++); 
				case Predicate::Operator::GreaterThan:
					if (p.value().isNull()) {
						return "0";
					}
					toBind[index] = p.value();
					return "(" + p.key() + " > ?" + std::to_string(index++) + " OR " + p.key() + " IS NULL)";
 				case Predicate::Operator::GreaterEquals:
	 				if (p.value().isNull()) {
						return p.key() + " IS NULL";
					}
					toBind[index] = p.value();
					return "(" + p.key() + " >= ?" + std::to_string(index++) + " OR " + p.key() + " IS NULL";
				case Predicate::Operator::Or: {
					// special case: ORing 'Contain's which are all full text indexed
					auto it = _fullTextIndex.find(entity);
					if (it != _fullTextIndex.end()) {
						auto &fullTextFields = it->second;
						std::vector<std::string> fullTextFieldCandidates;
						bool fullTextSearchPossible = true;
						std::string text;
						for (auto &subp : p.predicates()) {
							//std::cout << "fulltext iterating over field " << subp.key() << std::endl;
							if (subp.op() == Predicate::Operator::Contains 
									&& find(fullTextFields.begin(),fullTextFields.end(),subp.key()) != fullTextFields.end()
									&& (text.empty() || text == subp.value().asString())) {
								//std::cout << "fulltext iterating ok " << subp.key() << std::endl;
								fullTextFieldCandidates.push_back(subp.key());
								text = subp.value().asString();
                                if (text.empty()) {
                                    // don't search for empty string, that would be pointless (and also very slow)
                                    fullTextSearchPossible = false;
                                    break;
                                }
							} else {
								fullTextSearchPossible = false;
								break;
							}
						}
						if (fullTextSearchPossible && !fullTextFieldCandidates.empty()) {
							std::pair<std::string, std::string> clauseAndBind;
							if (fullTextFieldCandidates.size() == fullTextFields.size()) {
								clauseAndBind = fullTextSearchClauseAndBind(entity, index, std::vector<std::string>(), text);
							} else {
								clauseAndBind = fullTextSearchClauseAndBind(entity, index, fullTextFieldCandidates, text);
							}
							toBind[index] = clauseAndBind.second;
							++index;
							return clauseAndBind.first;
						}
					}
					// go on with the 'normal' or/and
				}
				case Predicate::Operator::And: {
					bool first = true;
					std::stringstream ss;
					if (p.predicates().empty()) {
						if (p.op() == Predicate::Operator::And) {
							return std::string("1");
						} else {
							return std::string("0");
						}
					}
					for (auto &subp : p.predicates()) {
						if (first) {
							first = false;
						} else {
							if (p.op() == Predicate::Operator::And) {
								ss << " AND ";
							} else {
								ss << " OR ";
                                //std::cout << "subp: '" << process(subp) << "'\n";
							}
						}
                        ss << "(" << process(subp) << ")";
					} 
					return ss.str();
				}
				case Predicate::Operator::Not:
					return std::string("NOT (") + process(sfl::head(p.predicates())) + ")";
			}
		};

		std::pair<std::string,function<void(SqliteStatement&)>> ret;
		ret.first = prefix + " " + process(predicate);
		ret.second = [=](SqliteStatement &stmt){
			for (const auto &p : toBind) {
				//std::cout << "binding " << p.first << " to " << p.second << std::endl;
				if (p.second.isNull()) {
					continue;
				}

				stmt.bind(p.first,p.second);
			}
		};
		return ret;
	}

	void method remove(const std::string &entity, const Predicate &predicate, int connectionId)
	{
		//std::cout << "DELETE FROM: " << (std::string)predicate << std::endl;
		auto where = whereClause(entity, predicate);
		std::string sql = "DELETE FROM " + entity + " " + where.first; 
		SqliteStatement stmt(_db[connectionId], sql);
		where.second(stmt);
		stmt.step();
	}

	std::vector<Json::Value> method remove(const std::string &entity, const Predicate &predicatePrefix, const std::string &field, const std::vector<std::string> &values, int connectionId)
	{
		static const int bindLimit = 400;

		using namespace sfl;

		// we cut the objects to be deleted into smaller pieces (because sqlite has a limit on binds),
		// fetch then delete them
		auto predicates = map([&](const ImmutableVector<std::string> &chunk){
			return Predicate::compositeAnd(predicatePrefix, 
					   Predicate(Predicate::Operator::Or, 
					   		map([&](const std::string &value){
					   			return Predicate(Predicate::Operator::Equals,field,value);
					   		}, chunk)));	
		}, chunkR(bindLimit, toImmutableVector(values)));

		return concat(map([&](const Predicate &predicate){
			auto fetched = fetch(entity, predicate, SortDescriptor(), Json::Value(), 0, 0, connectionId);
			if (!fetched.empty()) {
				remove(entity, predicate, connectionId);
			}
			return std::move(fetched);
		}, predicates));
	}

	size_t method count(const std::string &entity, const Predicate &predicate, int connectionId)
	{
		auto where = whereClause(entity, predicate);
		std::string sql = "SELECT COUNT(*) FROM " + entity + " " + where.first; 
		SqliteStatement stmt(_db[connectionId], sql);
		where.second(stmt);

		return stmt.fetchValue().asInt();
	}

	int32_t method max(const std::string &entity, const Predicate &predicate, const std::string &field)
	{
		auto where = whereClause(entity, predicate);
		std::string sql = "SELECT MAX(" + field + ") FROM " + entity + " " + where.first; 
		SqliteStatement stmt(_db[0], sql);
		where.second(stmt);

		return stmt.fetchValue().asInt();
	}

	static std::string orderClause(const SortDescriptor &sort)
	{
		const auto &fields = sort.orderBy();
		if (fields.empty()) {
			return "";
		}

		std::stringstream sql;
		sql << "ORDER BY ";
		bool first = true;
		for (auto &field : fields) {
			if (first) {
				first = false;
			} else {
				sql << ",";
			}
			sql << field;
			if (!sort.ascending()) {
				sql << " DESC";
			}
		}
		return sql.str();
	}

	std::vector<Json::Value> method fetch(const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, const Json::Value &justBefore, size_t limit, size_t offset, int connectionId)
	{
		//std::cout << "fetching after " << justBefore << std::endl;

		std::stringstream sql;
		sql << "SELECT ";
		bool first = true;
		const auto &fields = _columns[entity];
		for (auto &field : fields) {
			if (first) {
				first = false;
			} else {
				sql << ",";
			}
			sql << field;
		}
		auto where = whereClause(entity, Predicate::compositeAnd(Predicate::greaterThan(orderBy, justBefore), predicate));
		sql << " FROM " << entity << " " << where.first << " " << orderClause(orderBy);
		if (limit > 0) {
			sql << " LIMIT " << limit;
		}
		if (offset > 0) {
			sql << " OFFSET " << offset;
		}

		SqliteStatement stmt(_db[connectionId], sql.str());
		where.second(stmt);

		std::vector<Json::Value> ret;
		if (limit > 0) {
			ret.reserve(limit);
		}

		for (;;) {
			auto value = stmt.fetchRow(fields);
			if (value.isNull()) {
				//std::cout << "got nil\n";
				break;
			}
			//std::cout << "got one row\n";  // lol this is a resource hog
			ret.push_back(std::move(value));
		}
		return ret;
	}

	std::vector<std::string> method fetch(const std::string &entity, const Predicate &predicate, const SortDescriptor &orderBy, const std::string &field)
	{
		std::stringstream sql;
		sql << "SELECT " << field;
		auto where = whereClause(entity, predicate);
		sql << " FROM " << entity << " " << where.first << " " << orderClause(orderBy);
		SqliteStatement stmt(_db[0], sql.str());
		where.second(stmt);

		std::vector<std::string> ret;
		while (stmt.stepForRow()) {
			ret.push_back(stmt.fetchString(0));
		}
 		return ret;
	}

	std::vector<Json::Value> method fetchGroupings(const std::string &entity, const ClientDb::Predicate &predicate, const ClientDb::SortDescriptor &orderBy,
											const std::vector<std::string> &fetchFields, const std::vector<std::string> &groupBy)
	{
		std::stringstream sql;
		sql << "SELECT ";
		bool first = true;
		for (const auto &field : fetchFields) {
			if (field.empty()) {
				continue;
			}
			if (first) {
				first = false;
			} else {
				sql << ",";
			}
			if (field[0] == ':') {
				std::string kDistinctCountPrefix = ":distinctcount:";
				if (field.compare(0, kDistinctCountPrefix.length(),kDistinctCountPrefix) == 0) {
					sql << "COUNT(DISTINCT " << field.substr(kDistinctCountPrefix.length()) << ")";
				}
			} else {
				sql << field;
			}
		}
		sql << " FROM " << entity << " GROUP BY ";
		first = true;
		for (const auto &group : groupBy) {
			if (first) {
				first = false;
			} else {
				sql << ",";
			}
			sql << group;
		}

		auto where = whereClause(entity, predicate, "HAVING");
		sql << " " << where.first << " " << orderClause(orderBy);

		//std::cout << "grouping fetch: " << sql.str() << std::endl;

		SqliteStatement stmt(_db[0], sql.str());
		where.second(stmt);
		
		std::vector<Json::Value> ret;
		for (;;) {
			auto value = stmt.fetchRow(fetchFields);
			if (value.isNull()) {
				//std::cout << "got nil\n";
				break;
			}
			//std::cout << "got one row\n";  // lol this is a resource hog
			ret.push_back(std::move(value));
		}
		return ret;
	}
}