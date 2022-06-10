#include "redpackdb.hpp"
#include <wasm_db.hpp>

using namespace std;
using namespace wasm::db;

static constexpr eosio::name CNYD_TOKEN{"cnyd.token"_n};

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

    ~redpack() {
        _global.set( _gstate, get_self() );
    }

    [[eosio::on_notify("*::transfer")]] void ontransfer(name from, name to, asset quantity, string memo);

    [[eosio::action]] void claim( const name& claimer, const uint64_t& pack_id, const string& pwhash );

    [[eosio::action]] void cancel( const uint64_t& pack_id );

    [[eosio::action]] void addfee( const asset& fee, const name& contract);

    [[eosio::action]] void delfee( const symbol& coin );

    [[eosio::action]] void setconf(const name& admin, const uint16_t& hours);

    asset _calc_fee(const asset& fee, const uint64_t count);

    asset _calc_red_amt(const redpack_t& redpack);

    uint64_t rand(asset max_quantity);
private:

}; //contract redpack