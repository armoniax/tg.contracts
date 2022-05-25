#include "redpackdb.hpp"
#include <wasm_db.hpp>

using namespace std;
using namespace wasm::db;

class [[eosio::contract("tg.redpack")]] redpack: public eosio::contract {
private:
    dbc                 _db;
    global_singleton    _global;
    global_t            _gstate;

public:
    using contract::contract;

    redpack(eosio::name receiver, eosio::name code, datastream<const char*> ds):
        _db(_self), 
        contract(receiver, code, ds),
        _global(_self, _self.value)
    {
        _gstate = _global.exists() ? _global.get() : global_t{};
    }

    [[eosio::on_notify("*::transfer")]] void ontransfer(name from, name to, asset quantity, string memo);
   
    [[eosio::action]] void claim( const name& admin, const name& claimer, const uint64_t& pack_id, const string& pwhash );

    [[eosio::action]] void cancel( const name& admin, const uint64_t& pack_id );

    [[eosio::action]] void addfee( const symbol& coin, const asset& fee );

    [[eosio::action]] void delfee( const symbol& coin );
private:

}; //contract redpack