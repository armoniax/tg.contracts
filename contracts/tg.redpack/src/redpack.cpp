
#include <amax.token.hpp>
#include "redpack.hpp"
#include "utils.hpp"
#include <algorithm>
#include <chrono>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

using std::chrono::system_clock;
using namespace wasm;

static constexpr eosio::name active_permission{"active"_n};

// transfer out from contract self
#define TRANSFER(bank, to, quantity, memo) \
    { action(permission_level{get_self(), "active"_n }, bank, "transfer"_n, std::make_tuple( _self, to, quantity, memo )).send(); }


inline int64_t get_precision(const symbol &s) {
    int64_t digit = s.precision();
    CHECK(digit >= 0 && digit <= 18, "precision digit " + std::to_string(digit) + " should be in range[0,18]");
    return calc_precision(digit);
}

inline int64_t get_precision(const asset &a) {
    return get_precision(a.symbol);
}

//issue-in op: transfer tokens to the contract and lock them according to the given plan
void redpack::ontransfer( name from, name to, asset quantity, string memo )
{
    if (from == _self || to != _self) return;

	CHECK( quantity.amount > 0, "quantity must be positive" )

    auto fee_info = fee_t(quantity.symbol);
    CHECK( _db.get(fee_info), "coin not found" );

    //memo params format:
    //${pwhash} | count
    vector<string_view> memo_params = split( memo, ":" );
    auto parts = split( memo, ":" );
    CHECK( parts.size() >= 2, "Expected format 'pwhash : count'" );

    auto count = stoi(string(parts[1]));

    auto fee = _calc_fee( fee_info.fee, count );
    CHECK(fee < quantity, "not enough ")
    redpack_t::idx_t redpacks( _self, _self.value );
    auto id = redpacks.available_primary_key();
    redpacks.emplace( _self, [&]( auto& row ) {
        row.id 					        = id;
        row.sender 			            = from;
        row.pw_hash                     = string( parts[0] );
        row.total_quantity              = quantity;
        row.fee                         = fee;
        row.receiver_count		        = count;
        //asset( 0, quantity.symbol )
        row.remain_quantity		        = quantity;
        row.remain_count	            = count;
        row.status			            = redpack_status::CREATED;
        row.created_at                  = time_point_sec( current_time_point() );
        row.updated_at                  = time_point_sec( current_time_point() );
   });

}
void redpack::claim( const name& admin, const name& claimer, const uint64_t& pack_id, const string& pwhash )
{
    require_auth( admin );
    CHECK( _gstate.tg_admin == admin, " permission denied " );

    redpack_t redpack(pack_id);
    CHECK( _db.get(redpack), "redpack not found" );
    CHECK( redpack.pw_hash == pwhash, "incorrect password" );
    CHECK( redpack.status == redpack_status::CREATED, "redpack has expired" );

    claim_t::idx_t claims(_self, _self.value);
    auto claims_index = claims.get_index<"unionid"_n>();
    uint128_t sec_index = get_unionid(claimer,pack_id);
    check( claims_index.find(sec_index) == claims_index.end() , "Can't repeat to receive" );

    fee_t fee(redpack.total_quantity.symbol);
    CHECK( _db.get(fee), "fee not found" );

    asset redpack_quantity =_calc_red_amt(redpack);
    // eosio::print(redpack_quantity.amount);
    TRANSFER(fee.contract_name, claimer, redpack_quantity, string("red pack transfer"));

    redpack.remain_count--;
    redpack.remain_quantity-=redpack_quantity;
    redpack.updated_at = time_point_sec( current_time_point() );
    if(redpack.remain_count == 0){
        redpack.status = redpack_status::FINISHED;
    }
    _db.set(redpack);

    auto id = claims.available_primary_key();
    claims.emplace( _self, [&]( auto& row ) {
        row.id                  = id;
        row.pack_id 			= pack_id;
        row.sender              = redpack.sender;
        row.receiver            = claimer;
        row.quantity            = redpack_quantity;
        row.claimed_at		    = time_point_sec( current_time_point() );
   });

}

void redpack::cancel( const name& admin, const uint64_t& pack_id )
{
    require_auth( admin );
    redpack_t redpack(pack_id);
    CHECK( _db.get(redpack), "redpack not found" );
    // CHECK( redpack.status == redpack_status::CREATED, "redpack has expired" );
    // CHECK( current_time_point() > redpack.created_at + eosio::days(_gstate.expire_hours), "expiration date is not reached" );

    fee_t fee(redpack.total_quantity.symbol);
    CHECK( _db.get(fee), "fee not found" );
    TRANSFER(fee.contract_name, redpack.sender, redpack.remain_quantity, string("red pack cancel transfer"));
    redpack.status = redpack_status::CANCELLED;
    _db.set(redpack);

}

void redpack::addfee( const asset& fee, const name& contract)
{
    require_auth( _self );

    auto fee_info = fee_t(fee.symbol);
    CHECK( !_db.get(fee_info), "coin already exists" );
    fee_info.fee = fee;
    fee_info.contract_name = contract;
    _db.set( fee_info );
}

void redpack::delfee( const symbol& coin )
{
    require_auth( _self );
    auto fee_info = fee_t(coin);
    CHECK( _db.get(fee_info), "coin not found" );

    _db.del( fee_info );
}

void redpack::setconf(const name& admin, const uint16_t& hours)
{
    require_auth( _self );

    _gstate.tg_admin = admin;
    _gstate.expire_hours = hours;

}

asset redpack::_calc_fee(const asset& fee, const uint64_t count) {
    // calc order quantity value by price
    auto value = multiply_decimal64(fee.amount, count, 0);

    return asset(value, fee.symbol);
}

asset redpack::_calc_red_amt(const redpack_t& redpack) {
    // calc order quantity value by price
    if( redpack.remain_count == 1 ){
        return redpack.remain_quantity;

    }else{
        uint64_t quantity = redpack.remain_quantity.amount / redpack.remain_count * 2;
        return asset(rand(asset(quantity,redpack.remain_quantity.symbol)), redpack.remain_quantity.symbol);

    }
}

uint64_t redpack::rand(asset max_quantity) {
    uint64_t value = 0;
    eosio::print(get_precision(max_quantity) * 100);
    if(abs(tapos_block_prefix()) % max_quantity.amount < get_precision(max_quantity) / 100){
        value = get_precision(max_quantity) * 100;
        return value;

    }else{
        value = abs(tapos_block_prefix()) % max_quantity.amount;
        return value;

    }

}
