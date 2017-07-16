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
#ifndef _KUL_DBS_POSTGRES_HPP_
#define _KUL_DBS_POSTGRES_HPP_

#include <pqxx/pqxx>

#include "kul/db.hpp"

#include "kul/threads.hpp"

namespace kul { namespace db {

class Postgres : public kul::DB{
    private:
        kul::Mutex m;
        pqxx::connection c;
        std::string exec_return(const std::string& sql) KTHROW(db::Exception){
            kul::ScopeLock l(m);
            pqxx::work w(c);
            pqxx::result r = w.exec(sql);
            w.commit();
            return r[0][0].as<std::string>();
        }
        void exec(const std::string& sql) KTHROW(db::Exception){
            kul::ScopeLock l(m);
            pqxx::work w(c);
            w.exec(sql);
            w.commit();
        }
    public:
        ~Postgres(){ c.disconnect(); }
        Postgres(const std::string& h, const std::string& d, const std::string& s, const std::string& u, const std::string& p)
            : kul::DB(s), c("host="+h+ " dbname="+d+" user="+u+" password="+p){}
        pqxx::result query(const std::string& sql){
            kul::ScopeLock l(m);
            return pqxx::nontransaction(c).exec(sql);
        }
};

class PostgresORM : public kul::ORM{
    private:
        Postgres& pdb;
    protected:
        virtual void populate(const std::string& s, std::vector<kul::hash::map::S2S>& vals){
            pqxx::result r(pdb.query(s));
            for(pqxx::result::const_iterator c = r.begin(); c != r.end(); ++c){
                kul::hash::map::S2S map;
                for(uint16_t i = 0; i < c.size(); i++) 
                    map.insert(c[i].name(), c[i].is_null() ? "" : c[i].as<std::string>());
                vals.push_back(map);
            }
        }
    public:
        PostgresORM(Postgres& pdb) : kul::ORM(pdb), pdb(pdb){}
};

}}

#endif /* _KUL_DBS_POSTGRES_HPP_ */
