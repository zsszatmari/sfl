#ifndef PREDICATE_H
#define PREDICATE_H

#include "jsoncpp/json.h"
#include "DbSortDescriptor.h"

namespace ClientDb
{
	class Predicate final
	{
	public:
		// the numbers must not change, ever
		enum class Operator {
			Equals = 0,
			Contains = 1,
			NonEmpty = 8,
			LessThan = 16,
			LessEquals = 17,
			GreaterThan = 18,
			GreaterEquals = 19,

			And = 32,
			Or = 33
		};

		Predicate();
		// shorthand for equals
		Predicate(const std::string &key, const Json::Value &value);
		Predicate(Operator op, const std::string &key, const Json::Value &value, const std::vector<Predicate> &predicates = std::vector<Predicate>());
		Predicate(Operator op, const std::vector<Predicate> &predicates);

		const std::string &key() const;
		const Json::Value &value() const;	
		const std::vector<Predicate> &predicates() const;
		bool evaluate(const Json::Value &) const;
		Operator op() const;
		std::map<std::string,std::string> keyValues() const;
		bool subsetOf(const Predicate &other) const;

		operator std::string() const; 

		static Predicate greaterEquals(const SortDescriptor &sort, const Json::Value &base);
		static Predicate greaterThan(const SortDescriptor &sort, const Json::Value &base);
		static Predicate lessEquals(const SortDescriptor &sort, const Json::Value &base);
		static Predicate lessThan(const SortDescriptor &sort, const Json::Value &base);
		static Predicate compositeAnd(const Predicate &lhs, const Predicate &rhs);
		static Predicate compositeAnd(const Predicate &a, const Predicate &b, const Predicate &c);
		static Predicate compositeOr(const Predicate &lhs, const Predicate &rhs);

	private:
		static Predicate greater(const SortDescriptor &sort, const Json::Value &base, bool reverse, bool strictComparison);

		std::string _key;
		Json::Value _value;	
		std::vector<Predicate> _predicates;
		Operator _op;
	};
};

#endif