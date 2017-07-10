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
#ifndef _KUL_DBS_MYSQL_HPP_
#define _KUL_DBS_MYSQL_HPP_

#include "kul/db.hpp"

#include "kul/threads.hpp"

namespace kul { namespace db {

class MySQL : public kul::DB{
    private:
        kul::Mutex m;
        void exec(const std::string& sql) throw(db::Exception){
            kul::ScopeLock l(m);
        }
    public:
        ~MySQL(){}
        MySQL(const std::string& h, const std::string& d, const std::string& u, const std::string& p)
            : kul::DB(d){}

};

class MySQLORM : public kul::ORM{
    private:
        MySQL& mdb;
    protected:
        virtual void populate(const std::string& s, std::vector<kul::hash::map::S2S>& vals){

        }
    public:
        MySQLORM(MySQL& mdb) : kul::ORM(mdb), mdb(mdb){}
};

}}

#endif /* _KUL_DBS_MYSQL_HPP_ */
