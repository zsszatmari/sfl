#include <stdexcept>
#include <algorithm>
#include "Predicate.h"
#include "sfl/Prelude.h"
#include "sfl/Vector.h"

namespace ClientDb
{
#define method Predicate::

	using std::vector;
	using std::string;

	method Predicate() :
		_op(Operator::And)
	{	
	}

	method Predicate(const std::string &key, const Json::Value &value) :
		_op(Predicate::Operator::Equals),
		_key(key),
		_value(value)
	{
	}

	method Predicate(Operator op, const string &key, const Json::Value &value, const std::vector<Predicate> &predicates) :
		_op(op),
		_key(key),
		_value(value),
		_predicates(predicates)
	{
	}

	method Predicate(Operator op, const vector<Predicate> &predicates) :
		_op(op),
		_predicates(predicates)
	{
	}

	const std::string & method key() const
	{
		return _key;
	}

	const Json::Value & method value() const
	{
		return _value;
	}	

	const std::vector<Predicate> & method predicates() const
	{
		return _predicates;
	}

	Predicate::Operator method op() const
	{
		return _op;
	}

	Predicate method greater(const SortDescriptor &sort, const Json::Value &base, bool reverse, bool strictComparison)
	{
		if (base.isNull()) {
			return Predicate();
		}

		auto ascending = sort.ascending();
		if (reverse) {
			ascending = !ascending;
		}

		Predicate ret;
		auto &orderBy = sort.orderBy();

		bool first = true;
		for (auto it = orderBy.rbegin() ; it != orderBy.rend() ; ++it) {

			auto &key = *it;
			auto value = base.get(key, Json::nullValue);
			// e.g. a > 2 || (a == 2 && (b > 2 || (b == 2 && c >= 2)
			if (first) {
				if (!strictComparison) {
					if (ascending) {
						ret = Predicate(Operator::GreaterEquals, key, value);
					} else {
						ret = Predicate(Operator::LessEquals, key, value);		
					}
				} else {
					if (ascending) {
						ret = Predicate(Operator::GreaterThan, key, value);
					} else {
						ret = Predicate(Operator::LessThan, key, value);		
					}
				}
				first = false;
			} else {
				if (ascending) {
					ret = compositeOr(Predicate(Operator::GreaterThan,key,value),compositeAnd(Predicate(Operator::Equals, key, value),ret));
				} else {
					ret = compositeOr(Predicate(Operator::LessThan,key,value),compositeAnd(Predicate(Operator::Equals, key, value),ret));
				}
			}
		}
		return ret;
	}

	Predicate method greaterEquals(const SortDescriptor &sort, const Json::Value &base)
	{
		return greater(sort, base, false, false);
	}

	Predicate method greaterThan(const SortDescriptor &sort, const Json::Value &base)
	{
		return greater(sort, base, false, true);
	}

	Predicate method lessEquals(const SortDescriptor &sort, const Json::Value &base)
	{
		return greater(sort, base, true, false);
	}

	Predicate method lessThan(const SortDescriptor &sort, const Json::Value &base)
	{
		return greater(sort, base, true, true);
	}

	static vector<Predicate> v(const Predicate &lhs, const Predicate &rhs)
	{
		vector<Predicate> ret;
		ret.reserve(2);
		ret.push_back(lhs);
		ret.push_back(rhs);
		return ret;
	}

	static vector<Predicate> v(const Predicate &a, const Predicate &b, const Predicate &c)
	{
		vector<Predicate> ret;
		ret.reserve(3);
		ret.push_back(a);
		ret.push_back(b);
		ret.push_back(c);
		return ret;
	}
		
	Predicate method compositeAnd(const Predicate &lhs, const Predicate &rhs)
	{
		return Predicate(Operator::And, v(lhs, rhs));
	}

	Predicate method compositeAnd(const Predicate &a, const Predicate &b, const Predicate &c)
	{
		return Predicate(Operator::And, v(a,b,c));
	}

	Predicate method compositeOr(const Predicate &lhs, const Predicate &rhs)
	{
		return Predicate(Operator::Or, v(lhs, rhs));
	}

	Predicate method compositeNot(const Predicate &lhs)
	{
		return Predicate(Operator::Not, sfl::Vector::singleton(lhs));
	}

	std::map<string,string> method keyValues() const
	{
		std::map<string,string> ret;
		if (_op == Operator::Equals) {
			ret[_key] = _value.asString();
		} else if (_op == Operator::And) {
			for (auto &sub : _predicates) {
				if (sub._op == Operator::Equals) {
					ret[sub._key] = sub._value.asString();
				}
			}
		}
		return ret;
	}

	bool method subsetOf(const Predicate &other) const
	{
		// it's the other way around: if there are more strict requirements, it means that the resulting set after filter is a subset
		auto bigger = keyValues();
		auto smaller = other.keyValues();
		return std::includes(bigger.begin(), bigger.end(), smaller.begin(), smaller.end());
	}

	bool method evaluate(const Json::Value &obj) const
	{
		switch(_op) {
			case Operator::And:
				for (auto &sub : _predicates) {
					if (!sub.evaluate(obj)) {
						return false;
					}
				}
				return true;
			case Operator::Or:
				for (auto &sub : _predicates) {
					if (sub.evaluate(obj)) {
						return true;
					}
 				}
 				return false;
 			case Operator::Not:
 				return !sfl::head(_predicates).evaluate(obj);
 			case Operator::Equals:
 				return obj.get(_key,"") == _value;
            case Operator::Contains: {
#pragma message("TODO: must work the same way as sqlite")
            	string value = _value.asString();
                if (value.empty() || obj.get(_key,"").asString().find(value) != std::string::npos) {
                    return true;
                } else {
                	return false;
                }
            }
		}
		throw std::runtime_error("unimplemented evaluation");
	}

	method operator std::string() const
	{
		using namespace sfl;

		auto multi = [&](const string &separator){
			return intercalate(separator,
							map([&](const Predicate &pred){return string("(") + (string)pred + ")";}, _predicates));
		};

		switch(_op) {
			case Operator::And:
				return multi(" AND ");
			case Operator::Or:
				return multi(" OR ");
 			case Operator::Equals:
 				return string(_key) + " == " + _value.asString();
            case Operator::Contains: {
            	return string(_key) + " CONTAINS " + _value.asString();
            }
            case Operator::Not: {
            	return "NOT (" + (string)head(_predicates) + ")";
            }
		}
	}

}