/**
Copyright (c) 2016, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _KUL_DB_HPP_
#define _KUL_DB_HPP_

#include <vector>
#include <sstream>

#include "kul/map.hpp"
#include "kul/except.hpp"

#include "kul/db/def.hpp"

namespace kul{
namespace db{
class Exception : public kul::Exception{
    public:
        Exception(const char*f, const uint16_t& l, std::string s) : kul::Exception(f, l, s){}
};
}

class ORM;

class DB{
    protected:
        const std::string n;
        DB(const std::string& n) : n(n){}
        DB(){}
        friend class kul::ORM;
    public:
        virtual void exec(const std::string& s) throw(db::Exception) = 0;
};

namespace orm{

template <class T>
class AObject{
    private:
        bool n = 0;
        kul::hash::set::String di;
        kul::hash::map::S2S fs;
        template <class R> friend std::ostream& operator<<(std::ostream&, const AObject<R>&);
    protected:
        const std::string& field(const std::string& s) const{
            if(!fs.count(s))                KEXCEPTION("ORM error, no field: " + s);
            return (*fs.find(s)).second;
        }
        template<class R> 
        const R field(const std::string& s) const{
            std::stringstream ss(field(s));
            R r;
            ss >> r;
            return r;
        }
    public:
        const std::string& created() const { return (*fs.find(_KUL_DB_CREATED_COL_)).second; }
        const std::string& updated() const { return (*fs.find(_KUL_DB_UPDATED_COL_)).second; }
        template<class V = std::string> 
        AObject<T>& set(const std::string& s, const V& v){
            std::stringstream ss;
            ss << v;
            fs[s] = ss.str();
            di.insert(s);
            return *this;
        }
        const std::string& operator[](const std::string& s) const{
            return field(s);
        }
        template<class R> 
        R get(const std::string& s) const{
            return this->template field<R>(s);
        }
        friend class kul::ORM;
};
template <class T> std::ostream& operator<<(std::ostream &s, const kul::orm::AObject<T>& o);
}

class ORM{
    protected:
        DB& db;
        virtual void populate(const std::string& s, std::vector<kul::hash::map::S2S>& vals) = 0;
        template <class T> 
        std::string table(){
            return db.n.size() ? (db.n+"."+T::TABLE()) : T::TABLE();
        }
        template <class T> 
        void update(orm::AObject<T>& o){
            if(o.di.size() == 0) return;
            std::stringstream ss;
            ss << "UPDATE " << table<T>() << " SET ";
            for(const auto& d : o.di)
                ss << d << "='" << o.fs[d] << "',";
            ss << " updated = NOW()";
            ss << " WHERE id = '" << o.fs[_KUL_DB_ID_COL_] << "'";
            db.exec(ss.str());
            o.di.clear();
        }
        template <class T> 
        void insert(orm::AObject<T>& o){
            std::stringstream ss;
            ss << "INSERT INTO " << table<T>() << "(";
            for(const auto& p : o.fs) ss << p.first << ", ";
            ss << " created, updated) VALUES(";
            for(const auto& p : o.fs) ss << "'" << p.second << "', ";
            ss << " NOW(), NOW()) RETURNING id";
            db.exec(ss.str());
            o.n = 0;
            o.di.clear();
        }
    public:
        ORM(DB& db) : db(db){}
        template <class T> 
        void commit(orm::AObject<T>& o){
            if(o.n) insert(o);
            else    update(o);
        }
        template <class T> 
        void remove(const orm::AObject<T>& o){
            std::stringstream ss;
            ss << "DELETE FROM " << table<T>() << " t WHERE t.id = '" << o[_KUL_DB_ID_COL_] << "'";
            db.exec(ss.str());
        }
        template <class T> 
        void get(std::vector<T>& ts, const uint16_t& l = 100, const uint16_t& o = 0, const std::string& w = "", const std::string& g = ""){
            std::stringstream ss;
            ss << "SELECT * FROM " << table<T>() << " t ";
            if(!w.empty()) ss << " WHERE " << w;
            ss << " LIMIT " << l << " OFFSET " << o;
            if(!g.empty()) ss << " GROUP BY " << g;
            for(auto& t : ts) t.n = 0;
            std::vector<kul::hash::map::S2S> vals;
            populate(ss.str(), vals);
            for(const auto& v : vals){
                T t;
                for(const auto& m : v) t.fs.insert(m.first, m.second);
                ts.push_back(t);
            }
        }
        template <class T, class V = std::string> 
        T by(const std::string& c, const V& v) throw(db::Exception) {
            std::vector<T> ts;
            std::stringstream ss, id;
            id << v;
            ss << "t." << c << " = '" << v << "'";
            get(ts, 2, 0, ss.str());
            if(ts.size() == 0) KEXCEPT(db::Exception, "Table("+table<T>()+") : "+ _KUL_DB_ID_COL_ +":"+ id.str() +" does not exist");
            if(ts.size() >  1) KEXCEPT(db::Exception, "Table("+table<T>()+") : "+ _KUL_DB_ID_COL_ +":"+ id.str() +" is a duplicate");
            return ts[0];
        }
        template <class T> 
        T id(const _KUL_DB_ID_TYPE_& id) throw(db::Exception) {
            return by<T, _KUL_DB_ID_TYPE_>(_KUL_DB_ID_COL_, id);
        }
};

}
template <class T> std::ostream& kul::orm::operator<<(std::ostream &s, const kul::orm::AObject<T>& o){
    std::stringstream ss;
    for(const auto& p : o.fs)
        ss << p.first << " : " << p.second << std::endl;
    return s << ss.str();
}
#endif /* _KUL_DB_HPP_ */
