//
//  Chain.h
//  G-Ear core
//
//  Created by Zsolt Szatmari on 09/02/14.
//  Copyright (c) 2014 Treasure Box. All rights reserved.
//

#ifndef __G_Ear_core__Chain__
#define __G_Ear_core__Chain__

#include <vector>
#include <algorithm>

namespace Gear
{
    using std::vector;
    using std::pair;
    
    template<class T>
    class Chain
    {
    public:
        Chain();
        Chain(const std::function<bool(const T&,const T&)> &comp);
        // Chain &operator=(const Chain &); // default
        Chain(const shared_ptr<const vector<T>> &);
        Chain(const shared_ptr<const vector<T>> &, const std::function<bool(const T&,const T&)> &comp);
        Chain<T> operator+(const shared_ptr<const vector<T>> &) const;
        Chain<T> add(const T *rangeBegin, const T *rangeEnd) const;
        Chain<T> trim(size_t pieces) const;
        size_t size() const;
        const T & at(size_t index) const;
        size_t indexOf(const T &) const;
        pair<shared_ptr<const vector<T>>,size_t> pieceAt(size_t pos, size_t &piecesBefore) const;
        operator vector<T>() const;
        
        void sortedAdd(const T &);
        void sortedRemove(const T &);
        void remove(const T &);
        void clear();
        
        static const size_t npos = -1;
        
    private:
        Chain(const vector<shared_ptr<const vector<T>>> &pieces, const vector<size_t> &indices, const std::function<bool(const T&,const T&)> &comp);
        
        vector<size_t> _indices;  // starting indexes
        // it is very important that these are const vectors
		vector<shared_ptr<const vector<T>>> _pieces;
        std::function<bool(const T&,const T&)> _comparator;
        
        void reindex(int firstPiece);
       
		// decltype(_pieces) does not work here with visual studio
		typedef pair<typename vector<shared_ptr<const vector<T>>>::iterator, typename vector<T>::const_iterator> iteratorPair;
        iteratorPair findPlace(const T &value);
    };
    
    

#define def template<class T>
#define method Chain<T>::
    
    def
    method Chain()
        // no comparator!
    {
    }
    
    def
    method Chain(const std::function<bool(const T&,const T&)> &comp) :
        _comparator(comp)
    {
    }

    def
    method Chain(const shared_ptr<const vector<T>> &v) :
        Chain(v, std::function<bool(const T&,const T&)>())
    {
    }    

    def
    method Chain(const shared_ptr<const vector<T>> &rhs, const std::function<bool(const T&,const T&)> &comp) :
        _indices(1, 0),
        _pieces(1, rhs),
        _comparator(comp)
    {
    }
    
    def
    method Chain(const vector<shared_ptr<const vector<T>>> &pieces, const vector<size_t> &indices, const std::function<bool(const T&,const T&)> &comp) :
        _pieces(pieces),
        _indices(indices),
        _comparator(comp)
    {
    }
    
    def
    Chain<T> method operator+(const shared_ptr<const vector<T>> &rhs) const
    {
        auto pieces = _pieces;
        pieces.push_back(rhs);
        
        auto indices = _indices;
        indices.push_back(size());
        
        return Chain(pieces, indices, _comparator);
    }

    def
    Chain<T> method trim(size_t num) const
    {
        size_t diff = _indices.at(num);
        vector<size_t> indices(_indices.begin() + num, _indices.end());
        std::transform(indices.begin(), indices.end(), indices.begin(), [&](size_t original){
            return original - diff;
        });
        return Chain(vector<shared_ptr<const vector<T>>>(_pieces.begin() + num, _pieces.end()), indices, _comparator);
    }
    
    def
    method operator vector<T>() const
    {
        vector<T> ret;
        ret.reserve(size());
        for (auto &pPiece : _pieces) {
            auto &piece = *pPiece;
            ret.insert(ret.end(), piece.begin(), piece.end());
        }
        return ret;
    }
    
    def
    Chain<T> method add(const T *rangeBegin, const T *rangeEnd) const
    {
        return operator+(shared_ptr<const vector<T>>(new vector<T>(rangeBegin,rangeEnd)));
    }
    
    def
    size_t method size() const
    {
        if (_indices.empty()) {
            return 0;
        }
        return _indices.back() + _pieces.back()->size();
    }
    
    def
    const T & method at(size_t index) const
    {
        auto it = upper_bound(_indices.begin(), _indices.end(), index) -1;
        // if bound gives _indices.begin(), all is lost
        return _pieces.at(it - _indices.begin())->at(index - *it);
    }
    
    def
    pair<shared_ptr<const vector<T>>,size_t> method pieceAt(size_t pos, size_t &piecesBefore) const
    {
        auto it = upper_bound(_indices.begin(), _indices.end(), pos) -1;
        piecesBefore = it - _indices.begin();
        return make_pair(_pieces.at(it- _indices.begin()), *it);
    }
    
    def
    size_t method indexOf(const T &rhs) const
    {
        int index = 0;
        for (auto &sv : _pieces) {
            for (const T &entry : *sv) {
                if (entry == rhs) {
                    return index;
                }
                ++index;
            }
        }
        
        return npos;
    }
    
    //
    def
    typename Chain<T>::iteratorPair method findPlace(const T &value)
    {
        iteratorPair it;
        it.first = _pieces.begin();
        if (_comparator) {
            for (; it.first != _pieces.end() ; ++it.first) {
                auto &piece = *it.first;
                it.second = lower_bound(piece->begin(),piece->end(),value,_comparator);
                if (it.second != piece->end()) {
                    
                    // found approximate position
                    break;
                }
            }
        } else {
            if (it.first != _pieces.end()) {
                it.second = (*it.first)->begin();
            }
        }
        
        if (it.first == _pieces.end()) {
            return it;
        }
        
        // what if we start with an empty piece?
        while (it.second == (*it.first)->end()) {
            ++it.first;
            if (it.first == _pieces.end()) {
                return it;
            }
            it.second = (*it.first)->begin();
        }
        
        // while (*it > value), thus we are equals by compare key
        // must also work without an actual comparator
        while (!_comparator || !_comparator(value, *it.second)) {
            
            if (*it.second == value) {
                return it;
            }
            
            ++it.second;
         
            // it.second is never invalid if it.first is valid
            while (it.second == (*it.first)->end()) {
                ++it.first;
                if (it.first == _pieces.end()) {
                    return it;
                }
                it.second = (*it.first)->begin();
            }
        }
        return it;
    }
    
    def
    void method sortedAdd(const T &toAdd)
    {
		auto it = findPlace(toAdd);
        if (it.first == _pieces.end()) {
            // we haven't found a right place to add
            _pieces.push_back(shared_ptr<const vector<T>>(new vector<T>(1, toAdd)));
            reindex(_pieces.size());
        } else {
            if (*it.second == toAdd) {
                // already present, don't add
                return;
            }
            
            auto &v = **it.first;
            shared_ptr<vector<T>> modified(new vector<T>(v));
            (*modified)[it.second - v.begin()] = toAdd;
            
            int index = it.first - _pieces.begin();
            _pieces[index] = modified;
            reindex(index);
        }
    }

    def
    void method sortedRemove(const T &toRemove)
    {
        auto it = findPlace(toRemove);
        if (it.first == _pieces.end()) {
            // not found
            return;
        }
        
        if (*it.second == toRemove) {
            auto &v = **it.first;
            shared_ptr<vector<T>> modified(new vector<T>(v));
            auto pos = (it.second - v.begin());
            modified->erase(modified->begin() + pos);
            
            int index = it.first - _pieces.begin();
            _pieces[index] = modified;
            reindex(index);
        }
    }
    
    def
    void method remove(const T &rhs)
    {
        int index = 0;
        for (auto itPiece = _pieces.begin() ; itPiece != _pieces.end() ; ++itPiece) {
            shared_ptr<const vector<T>> &sv = *itPiece;
            for (auto it = sv->begin() ; it != sv->end() ; ++it) {
                const T &entry = *it;
                if (entry == rhs) {
                    // found it
                    shared_ptr<vector<T>> modified(new vector<T>(*sv));
                    modified->erase(modified->begin() + (it - sv->begin()));
                    int index = itPiece - _pieces.begin();
                    _pieces[index] = modified;
                    reindex(index);
                    return;
                }
                ++index;
            }
        }
    }
    
    def
    void method reindex(int piece)
    {
        int value;
        if (piece == 0) {
            value = 0;
        } else {
            value = _indices.at(piece-1) + _pieces.at(piece-1)->size();
        }
        
        _indices.resize(piece);
        while (piece < _pieces.size()) {
            _indices.push_back(value);
            value += _pieces.at(piece)->size();
            ++piece;
        }
    }
    
#undef method
#undef def
}

#endif /* defined(__G_Ear_core__Chain__) */
