#ifndef SQLITESTATEMENT_H
#define SQLITESTATEMENT_H

#include <stdexcept>
#include <cstdint>
#include "stdplus.h"
#include "sqlite/sqlite3.h"

namespace ClientDb
{
//#define LOG_SQL
#ifdef LOG_SQL
//#define LOG_BIND
#endif

	class SqliteStatement final
	{
	public:
		SqliteStatement(sqlite3 *db, const std::string &statement, bool throwOnError = true) :
			_statement(nullptr),
#ifdef LOG_BIND
			_statementString(statement),
#endif			
			_valid(false),
			_bind(1),
			_throwOnError(throwOnError)
		{
			auto result = sqlite3_prepare_v2(db, statement.c_str(), statement.size(), &_statement, nullptr);
			if (!throwOnError && result != SQLITE_OK) {
#ifdef LOG_SQL
                std::cout << "warning " << std::to_string(result) << " in statement: " << _statement << " Error: " << sqlite3_errmsg(db) << std::endl;
#endif
				return;
			}
			if (SQLITE_OK != result) {
				throw std::runtime_error("error " + std::to_string(result) + " in statement: " + statement + " Error: " + sqlite3_errmsg(db));
			} else {
				_valid = true;
				
#ifdef LOG_SQL
				//if (statement.find("ORDER BY") != std::string::npos || statement.find("INSERT INTO") != std::string::npos) {
				//if (statement.find("SELECT") != std::string::npos && statement.find("ORDER") != std::string::npos) {
				//if (statement.find("SELECT") != std::string::npos && statement.find("COUNT") != std::string::npos) {
				if (statement.find("DELETE FROM") != std::string::npos) {
	                std::cout << "executing " + statement << std::endl;
	            }
#endif
			}
		}

		void bind(int index, const Json::Value &value)
		{
			if (value.isIntegral() || value.isBool()) {
				bind(index,value.asInt64());
			} else if (value.isDouble()) {
				bind(index,value.asDouble());
			} else if (value.isString()) {
				bind(index,value.asString());
			}
		}

		void bindNext(const Json::Value &value)
		{
			bind(_bind, value);
			_bind++;
		}

		bool step()
		{
			if (!_valid) {
				return false;
			}
//#define SQLITE_ROW         100  /* sqlite3_step() has another row ready */
//#define SQLITE_DONE        101  /* sqlite3_step() has finished executing */

			auto result = sqlite3_step(_statement);
			//std::cout << "sqlite step result: " << result << std::endl;
			if (result == SQLITE_OK || result == SQLITE_DONE || result == SQLITE_ROW) {
				return true;
			} else {
				if (_throwOnError) {
					throw std::runtime_error("error " + std::to_string(result) + " in statement: " + sqlite3_errmsg(sqlite3_db_handle(_statement)));
				}
#ifdef LOG_SQL
                std::cout << "warning " << std::to_string(result) << " in statement. " << " Error: " << sqlite3_errmsg(sqlite3_db_handle(_statement)) << std::endl;
#endif
				return false;
			}
		}

		bool stepForRow()
		{
			if (!_valid) {
				return false;
			}
			auto result = sqlite3_step(_statement);
			return result == SQLITE_ROW;
		}

		Json::Value fetchRow(const std::vector<std::string> &columns)
		{
			if (!_valid) {
				return Json::Value();
			}

			auto result = sqlite3_step(_statement);
			if (result != SQLITE_ROW) {
				return Json::Value();
			}

			Json::Value ret(Json::objectValue);
			int num = columns.size();
			for (int i = 0 ; i < num ; ++i) {
				auto type = sqlite3_column_type(_statement, i);
				const std::string &key = columns[i];
				auto value = fetchValue(i);
				// don't return null values, it would makes upsert more complicated (compare null vs non-existant)
				if (!value.isNull()) {
					ret[key] = fetchValue(i);
				}
			}
			return std::move(ret);
		}

		Json::Value fetchValue()
		{
			if (!_valid) {
				return Json::Value();
			}

			auto result = sqlite3_step(_statement);
			if (result != SQLITE_ROW) {
				return Json::Value();
			}

			return fetchValue(0);
		}

		std::string fetchString(int index)
		{
			if (!_valid) {
				return "";
			}
			return (const char *)sqlite3_column_text(_statement,index);
		}

		~SqliteStatement()
		{
			if (_statement) {
				sqlite3_finalize(_statement);
			}
		}

	private:
		Json::Value fetchValue(int i)
		{
			auto type = sqlite3_column_type(_statement, i);
			switch(type) {
				case SQLITE_INTEGER:
					return sqlite3_column_int64(_statement, i);
				case SQLITE_FLOAT:
					return sqlite3_column_double(_statement,i);
				case SQLITE_TEXT:
				case SQLITE_BLOB:
					return (const char *)sqlite3_column_text(_statement,i);
				//case SQLITE_NULL:
			}
			return Json::nullValue;		
		}

		void bind(int index, const std::string &value)
		{
#ifdef LOG_BIND
			std::cout << "bind '" << _statementString << "'" << index << ": " << value << std::endl;
#endif
            int ret = sqlite3_bind_text(_statement, index, value.c_str(), value.length(), SQLITE_TRANSIENT);
			if (SQLITE_OK != ret) {
                std::stringstream ss;
                ss << "cannot bind " << value << " code " << ret;
				throw std::runtime_error(ss.str());
			}
		}
		
		void bind(int index, int64_t value)
		{
#ifdef LOG_BIND
			std::cout << "bind " << index << ": " << value << std::endl;
#endif
			if (SQLITE_OK != sqlite3_bind_int64(_statement, index, value)) {
				throw std::runtime_error(std::string("cannot bind") + std::to_string(value));
			}
		}

		void bind(int index, double value)
		{
#ifdef LOG_BIND
			//std::cout << "bind " << index << ": " << value << std::endl;
#endif
			if (SQLITE_OK != sqlite3_bind_double(_statement, index, value)) {
				throw std::runtime_error(std::string("cannot bind") + std::to_string(value));
			}
		}

		sqlite3_stmt *_statement;
		bool _valid;
		int _bind;
		bool _throwOnError;
#ifdef LOG_BIND
		std::string _statementString;
#endif
	};
}

#endif