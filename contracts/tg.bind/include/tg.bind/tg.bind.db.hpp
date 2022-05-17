
#pragma once

#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <eosio/privileged.hpp>
#include <eosio/name.hpp>

#include "wasm_db.hpp"


using namespace eosio;


namespace amax {

using namespace std;
using namespace eosio;

namespace bind_status {
    static constexpr eosio::name INIT       = "init"_n;
    static constexpr eosio::name BIND       = "bind"_n;
    static constexpr eosio::name CHECKED    = "checked"_n;
};

#define CUSTODY_TBL [[eosio::table, eosio::contract("tg.bind")]]

struct [[eosio::table("global"), eosio::contract("tg.bind")]] global_t {
    bool                enable = false;
    name                tg_admin;

    EOSLIB_SERIALIZE( global_t, (enable)(tg_admin) )
};

typedef eosio::singleton< "global"_n, global_t > global_singleton;

struct CUSTODY_TBL bind_t {
    uint64_t            tgid;
    name                account;
    name                status = bind_status::BIND;
    time_point          updated_at;

    uint64_t    primary_key()const { return tgid; }
    uint64_t    scope() const { return 0; }

    bind_t() {}
    bind_t(const uint64_t& i): tgid(i) {}
    uint64_t by_account() const { return account.value; }

    EOSLIB_SERIALIZE( bind_t, (tgid)(tgid)(status)(updated_at) )

    typedef eosio::multi_index
    <"binds"_n, bind_t,
        indexed_by<"account"_n, const_mem_fun<bind_t, uint64_t, &bind_t::by_account> >
    > idx_t;
};

} //amax