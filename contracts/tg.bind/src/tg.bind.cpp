#include "tg.bind.hpp"
#include "utils.hpp"
#include <tg.bind/tg.bind.hpp>
#include <tg.bind/wasm_db.hpp>

void tgbind::init(const name& account) 
{
    require_auth( _self );
    _gstate.tg_admin = account;
}

void tgbind::bindtg(const name& account, const uint64_t& tgid)
{
    require_auth( account );
    bind_t::idx_t binds( _self, _self.value );
    auto bind_itr = binds.find( tgid );
    CHECK( bind_itr == binds.end(),  "tgid bind record already exists");

    auto account_index 			    = binds.get_index<"account"_n>();
    const auto& itr 			    = account_index.find( account.value );
    CHECK( itr == account_index.end(), "account bind record already exists" );

    auto bind_record                = bind_t( tgid );
    bind_record.account             = account;
    bind_record.updated_at          = time_point_sec( current_time_point() );
   _db.set( bind_record );
}

void tgbind::check(const uint64_t& tgid) 
{
    require_auth( _gstate.tg_admin );
    bind_t::idx_t binds( _self, _self.value );
    auto bind_itr = binds.find( tgid );
    CHECK( bind_itr != binds.end(),  "tgid bind record does not found");
    CHECK( bind_itr->status == bind_status::BIND,  "bind status is not BIND");

    binds.modify(*bind_itr, _self, [&]( auto& row) {
        row.status = bind_status::CHECKED;
    });
}

void tgbind::delbind(const uint64_t& tgid) 
{
    require_auth( _gstate.tg_admin );
    bind_t::idx_t binds( _self, _self.value );
    auto itr = binds.find( tgid );
    CHECK( itr != binds.end(),  "tgid bind record does not found");
    binds.erase(itr);
}