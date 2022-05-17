#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/time.hpp>

#include "tg.bind.db.hpp"
#include "wasm_db.hpp"

using namespace eosio;
using namespace amax;
using namespace wasm::db;

class [[eosio::contract("tg.bind")]] tgbind : public contract {
private:
    dbc                 _db;
    global_t            _gstate;
    global_singleton    _global;

public:
    using contract::contract;

    tgbind(name receiver, name code, datastream<const char*> ds): 
         _db(_self), contract(receiver, code, ds), _global(_self, _self.value) {
        if (_global.exists()) {
            _gstate = _global.get();

        } else {
            _gstate = global_t{};
        }
    }

    ~tgbind() {
        _global.set( _gstate, get_self() );
    }

    [[eosio::action]]
    void init(const name& account);

    [[eosio::action]]
    void bindtg(const name& account, const uint64_t& tgid);

    [[eosio::action]]
    void check(const uint64_t& tgid);

    [[eosio::action]]
    void delbind(const uint64_t& tgid);



};