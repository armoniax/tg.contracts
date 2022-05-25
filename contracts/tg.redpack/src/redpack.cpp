
#include <amax.token.hpp>
#include "redpack.hpp"
#include "utils.hpp"
#include <algorithm>
#include <chrono>

using std::chrono::system_clock;
using namespace wasm;

static constexpr eosio::name active_permission{"active"_n};

// transfer out from contract self
#define TRANSFER_OUT(token_contract, to, quantity, memo) token::transfer_action(                                \
                                                             token_contract, {{get_self(), active_permission}}) \
                                                             .send(                                             \
                                                                 get_self(), to, quantity, memo);

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
        row.delivered_quantity		    = asset( 0, quantity.symbol );
        row.status			            = redpack_status::CREATED;
        row.created_at                  = time_point_sec( current_time_point() );
        row.updated_at                  = time_point_sec( current_time_point() );
   });

}

void redpack::claim( const name& claimer, const uint64_t& pack_id, const string& pwhash )
{
    require_auth( _global.tg_admin );
    
    
}

void redpack::cancel( const name& admin, const uint64_t& pack_id ) 
{


}

void redpack::addfee( const symbol& coin, const asset& fee )
{
    require_auth( _self );
    
   auto fee_info = fee_t(coin);
   CHECK( !_db.get(fee_info), "coin already exists" );
   fee_info.fee = fee;
   _db.set( fee_info );
}

void redpack::delfee( const symbol& coin )
{
    require_auth( _self );
    auto fee_info = fee_t(coin);
    CHECK( _db.get(fee_info), "coin not found" );

    _db.del( fee_info );
}

asset redpack::_calc_fee(const asset& fee, const uint64_t count) {
    // calc order quantity value by price
    auto value = multiply_decimal64(fee.amount, price.amount, 0);

    return asset(value, fee.symbol);
}
