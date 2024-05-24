#include <dfsmoonbuild.hpp>

void dfsmoonbuild::dfs_token_in(name from, name to, asset quantity, string memo)
{
   handle_token_transfer(from, to, quantity, memo);
}

void dfsmoonbuild::jiu_token_in(name from, name to, asset quantity, string memo)
{
   handle_token_transfer(from, to, quantity, memo);
}

void dfsmoonbuild::usdt_token_in(name from, name to, asset quantity, string memo)
{
   handle_token_transfer(from, to, quantity, memo);
}

void dfsmoonbuild::reparation(name owner, uint32_t duration)
{
   require_auth(get_self());

   check(is_account(owner), "owner account does not exist");

   members _members(get_self(), get_self().value);
   auto member_itr = _members.find(owner.value);

   if (member_itr == _members.end())
   {
      _members.emplace(get_self(), [&](auto &row)
                       {
         row.owner = owner;
         row.status = ACTIVE;
         row.start_time = current_time_point();
         row.end_time = current_time_point() + seconds(duration);
         row.total_duration = duration; });
   }
   else
   {
      _members.modify(member_itr, get_self(), [&](auto &row)
                      {
         if (row.end_time < current_time_point())
         {
            row.start_time = current_time_point();
            row.end_time = current_time_point() + seconds(duration);
         }
         else{
            row.end_time = row.end_time + seconds(duration);
            
         } 
         row.total_duration += duration; });
   }
   write_log(owner, "reparation");
}

void dfsmoonbuild::updatemember(name owner, member_status status)
{
   require_auth(get_self());

   check(status == ACTIVE || status == INVALID, "invalid status");

   members _members(get_self(), get_self().value);
   auto itr = _members.require_find(owner.value, "member not found");

   _members.modify(itr, get_self(), [&](auto &row)
                     { row.status = status; });
   write_log(owner, "banmember");
}

void dfsmoonbuild::deletemember(name owner)
{
   require_auth(get_self());
   members _members(get_self(), get_self().value);
   auto itr = _members.require_find(owner.value, "member not found");
   _members.erase(itr);
   write_log(owner, "deletemember");
}

void dfsmoonbuild::codeconfirm(uint64_t orderid, uint8_t type)
{
   require_auth(get_self());
   check(type == 2 || type == 3, "invalid type");

   orders _orders(get_self(), get_self().value);
   auto itr = _orders.require_find(orderid, "order not found");

   name contract_name = get_contract_by_symbol(itr->quantity.symbol);
   if (type == 2)
   {
      _orders.modify(itr, get_self(), [&](auto &row)
                     { row.status = CONFIRMED; });

      // Add orders to the member table
      members _members(get_self(), get_self().value);
      // If the user already exists in the member table, update start_time, end_time
      auto member_itr = _members.find(itr->owner.value);
      if (member_itr == _members.end())
      {
         _members.emplace(get_self(), [&](auto &row)
                          {
            row.owner = itr->owner;
            row.status = ACTIVE;
            row.start_time = current_time_point();
            row.end_time = current_time_point() + seconds(itr->duration);
            row.total_duration = itr->duration; });
      }
      else
      {
         _members.modify(member_itr, get_self(), [&](auto &row)
                         {
         // Here row.end_time if it is the past time, you need to use the current time plus the member hours
         if (row.end_time < current_time_point())
         {
            row.start_time = current_time_point();
            row.end_time = current_time_point() + seconds(itr->duration);
         }
         else{
            row.end_time = row.end_time + seconds(itr->duration);
            
         } 
         row.total_duration += itr->duration; });
      }

      // Delete discount coupon records
      if (itr->discount_member_id != UINT64_MAX)
      {
         dismems _discountmembers(get_self(), get_self().value);
         auto dismems_itr = _discountmembers.find(itr->discount_member_id);
         if (dismems_itr != _discountmembers.end())
         {
            _discountmembers.erase(dismems_itr);
         }
      }

      // Transfer the money to the treasury account.
      utils::inline_transfer(contract_name,
                             get_self(),
                             INCOME_ACCOUNT,
                             itr->quantity,
                             string("fee"));
      write_log(itr->owner, "codeconfirm2");
   }
   else if (type == 3)
   {
      _orders.modify(itr, get_self(), [&](auto &row)
                     { row.status = REFUNDED; });

      // refund
      utils::inline_transfer(contract_name,
                             get_self(),
                             itr->owner,
                             itr->quantity,
                             string("dfsmoon.com refund"));
      // Restore the status of member coupons
      if (itr->discount_member_id != UINT64_MAX)
      {
         dismems _discountmembers(get_self(), get_self().value);
         auto dismems_itr = _discountmembers.find(itr->discount_member_id);
         if (dismems_itr != _discountmembers.end())
         {
            _discountmembers.modify(dismems_itr, get_self(), [&](auto &row)
                                    { row.status = DISCOUNT_UNUSED; });
         }
      }

      write_log(itr->owner, "codeconfirm3");
   }
   // Delete record
   _orders.erase(itr);
}

void dfsmoonbuild::confirmorder(name owner, uint64_t orderid)
{
   require_auth(owner);

   orders _orders(get_self(), get_self().value);
   auto itr = _orders.require_find(orderid, "order not found");
   check(itr->owner == owner, "invalid order");

   _orders.modify(itr, owner, [&](auto &row)
                  { row.status = CONFIRMED; });

   members _members(get_self(), get_self().value);
   auto member_itr = _members.find(owner.value);
   if (member_itr == _members.end())
   {
      _members.emplace(owner, [&](auto &row)
                       {
         row.owner = owner;
         row.status = ACTIVE;
         row.start_time = current_time_point();
         row.end_time = current_time_point() + seconds(itr->duration);
         row.total_duration = itr->duration; });
   }
   else
   {
      _members.modify(member_itr, owner, [&](auto &row)
                      {
         // Here row.end_time if it is the past time, you need to use the current time plus the member hours
         if (row.end_time < current_time_point())
         {
            row.start_time = current_time_point();
            row.end_time = current_time_point() + seconds(itr->duration);
         }
         else{
            row.end_time = row.end_time + seconds(itr->duration);
            
         } 
         row.total_duration += itr->duration; });
   }

   name contract_name = get_contract_by_symbol(itr->quantity.symbol);
   // Transferring money to the vault account
   utils::inline_transfer(contract_name,
                          get_self(),
                          INCOME_ACCOUNT,
                          itr->quantity,
                          string("fee"));

   // Delete Coupon Record, is completely deleted
   if (itr->discount_member_id != UINT64_MAX)
   {
      dismems _discountmembers(get_self(), get_self().value);
      auto dismems_itr = _discountmembers.find(itr->discount_member_id);
      if (dismems_itr != _discountmembers.end())
      {
         _discountmembers.erase(dismems_itr);
      }
   }

   // Delete order
   _orders.erase(itr);
   write_log(owner, "confirmorder");
}

// User refund
void dfsmoonbuild::refundorder(name owner, uint64_t orderid)
{
   require_auth(owner);

   orders _orders(get_self(), get_self().value);
   auto itr = _orders.require_find(orderid, "order not found");
   check(itr->owner == owner, "invalid order");

   _orders.modify(itr, owner, [&](auto &row)
                  { row.status = REFUNDED; });

   // refund
   name contract_name = get_contract_by_symbol(itr->quantity.symbol);
   utils::inline_transfer(contract_name,
                          get_self(),
                          owner,
                          itr->quantity,
                          string("dfsmoon.com refund"));
   // Restore the status of member coupons
   if (itr->discount_member_id != UINT64_MAX)
   {
      dismems _discountmembers(get_self(), get_self().value);
      auto dismems_itr = _discountmembers.find(itr->discount_member_id);
      if (dismems_itr != _discountmembers.end())
      {
         _discountmembers.modify(dismems_itr, get_self(), [&](auto &row)
                                 { row.status = DISCOUNT_UNUSED; });
      }
   }

   // Delete records
   _orders.erase(itr);
   write_log(owner, "refundorder");
}

// Discount type
void dfsmoonbuild::addiscount(uint8_t disid, uint8_t type, time_point_sec end_time)
{
   require_auth(get_self());
   discounts _discounts(get_self(), get_self().value);
   auto itr = _discounts.find(disid);
   check(itr == _discounts.end(), "disid already exists");

   // Check the time if it is in the past, report an error
   check(current_time_point() < eosio::time_point(end_time), "invalid end time");

   // type must be 0 or 1
   check(type == 0 || type == 1, "invalid type");

   _discounts.emplace(get_self(), [&](auto &row)
                      {
      row.disid = disid;
      row.type = type;
      row.end_time = end_time; });
}

void dfsmoonbuild::updiscount(uint8_t disid, uint8_t type, time_point_sec end_time)
{
   require_auth(get_self());

   // type must be 0 or 1
   check(type == 0 || type == 1, "invalid type");

   discounts _discounts(get_self(), get_self().value);
   auto itr = _discounts.require_find(disid, "disid not found");

   // Check the time if it is in the past, report an error
   check(current_time_point() < eosio::time_point(end_time), "invalid end time");

   _discounts.modify(itr, get_self(), [&](auto &row)
                     { 
                        row.end_time = end_time;
                        row.type = type; });
}

void dfsmoonbuild::deldiscount(uint8_t disid)
{
   require_auth(get_self());
   discounts _discounts(get_self(), get_self().value);
   auto itr = _discounts.require_find(disid, "disid not found");
   _discounts.erase(itr);
}

// Discounted member list
void dfsmoonbuild::adddismem(name owner, uint8_t disid)
{
   require_auth(get_self());
   discounts _discounts(get_self(), get_self().value);
   auto itr = _discounts.require_find(disid, "disid not found");

   dismems _discountmembers(get_self(), get_self().value);
   // Check if it already exists. Only one type of discount can exist, and disid and owner together form a unique combination. First search by owner, then loop through to find disid.
   auto owner_index = _discountmembers.get_index<"byowner"_n>();
   auto itr2 = owner_index.find(owner.value);
   while (itr2 != owner_index.end() && itr2->owner == owner)
   {
      check(itr2->disid != disid, "discount member already exists");
      itr2++;
   }

   _discountmembers.emplace(get_self(), [&](auto &row)
                            {
      row.id = _discountmembers.available_primary_key();
      row.owner = owner;
      row.status = DISCOUNT_UNUSED;
      row.disid = disid; });
   write_log(owner, "adddismem");
}

void dfsmoonbuild::deldismem(uint8_t id)
{
   require_auth(get_self());
   dismems _discountmembers(get_self(), get_self().value);
   auto itr = _discountmembers.require_find(id, "discount member not found");
   write_log(itr->owner, "deldismem");
   _discountmembers.erase(itr);
}

void dfsmoonbuild::createtype(uint8_t mtid, uint32_t duration, asset price, asset current_price, uint8_t disid)
{
   require_auth(get_self());
   discounts _discounts(get_self(), get_self().value);
   auto discount_itr = _discounts.find(disid);
   check(discount_itr != _discounts.end(), "disid not found");

   membertypes _membertypes(get_self(), get_self().value);
   auto itr = _membertypes.find(mtid);
   check(itr == _membertypes.end(), "membership mtid already exists");

   _membertypes.emplace(get_self(), [&](auto &row)
                        {
      row.mtid = mtid;
      row.duration = duration;
      row.price = price;
      row.current_price = current_price;
      row.disid = disid; });
}

void dfsmoonbuild::updatetype(uint8_t mtid, optional<uint32_t> duration, optional<asset> price, optional<asset> current_price, optional<uint8_t> disid)
{
   require_auth(get_self());
   if (disid)
   {
      discounts _discounts(get_self(), get_self().value);
      auto discount_itr = _discounts.find(*disid);
      check(discount_itr != _discounts.end(), "disid not found");
   }

   membertypes _membertypes(get_self(), get_self().value);
   auto itr = _membertypes.require_find(mtid, "membership mtid not found");

   _membertypes.modify(itr, get_self(), [&](auto &row)
                       {
      if (duration)
         row.duration = *duration;
      if (price)
         row.price = *price;
      if (current_price)
         row.current_price = *current_price;
      if (disid)
         row.disid = *disid; });
}

void dfsmoonbuild::deletetype(uint8_t mtid)
{
   require_auth(get_self());
   membertypes _membertypes(get_self(), get_self().value);
   auto itr = _membertypes.require_find(mtid, "membership mtid not found");
   _membertypes.erase(itr);
}

void dfsmoonbuild::handle_token_transfer(name from, name to, asset quantity, string memo)
{
   if (from == _self || to != _self)
      return;

   vector<string> parts = utils::split(memo, ":");
   check((parts.size() == 3 || parts.size() == 2) && parts[0] == "buy", "Invalid memo format");
   // The 2nd and 3rd bits of parts are mtid and discount_member_st respectively, both numbers

   // first param
   uint64_t temp_membership_Id = stoul(parts[1], nullptr, 10);
   // Check for overflow
   check(temp_membership_Id <= 255, "invalid mtid");
   uint8_t mtid = static_cast<uint8_t>(temp_membership_Id);
   membertypes _membertypes(get_self(), get_self().value);
   auto itr = _membertypes.require_find(mtid, "mtid not found");

   // parts.size() == 2, No discounts available
   if (parts.size() == 2)
   {
      uint8_t disid = itr->disid;
      //If disid is not equal to 0, it means that there is a discount, you need to find the status of the discounts table by disid, if type=0, it means that it is a coupon that applies to everyone.
      if (disid != 0)
      {
         discounts _discounts(get_self(), get_self().value);
         auto discount_itr = _discounts.find(disid);
         check(discount_itr != _discounts.end(), "disid not found");

         // Check if the discount has expired
         check(current_time_point() < eosio::time_point(discount_itr->end_time), "discount expired");

         // Check if the type is 0, =0 means it is a coupon applicable to everyone.
         check(discount_itr->type == 0, "invalid discount type");
      }

      // Check if the transfer amount is equal to the price of the membership type.
      check(quantity == itr->current_price, "invalid payment amount");

      // Next, put the data into the orders table
      orders _orders(get_self(), get_self().value);
      _orders.emplace(get_self(), [&](auto &row)
                      {
         row.id = _orders.available_primary_key();
         row.owner = from;
         row.quantity = quantity;
         row.mtid = mtid;
         row.discount_member_id = UINT64_MAX;
         row.status = PAID;
         row.duration = itr->duration;
         row.create_time = current_time_point(); });

      write_log(from, "buy");
      return;
   }

   /*********
   ********** Below are the discounted situations.
   **********/
   // Get the 3rd parameter: discount_member_id
   uint64_t discount_member_id = stoul(parts[2], nullptr, 10);

   dismems _discountmembers(get_self(), get_self().value);
   auto dismems_itr = _discountmembers.find(discount_member_id);
   check(dismems_itr != _discountmembers.end(), "discount member not found");

   // Check if the disid in membertypes is equal to the disid in discount_member.
   check(itr->disid == dismems_itr->disid, "invalid disid");

   // Check if the owner in dismems_itr is equal to from to prevent malicious transfers.
   check(dismems_itr->owner == from, "invalid discount member");

   // check dismems_itr->status == 1
   check(dismems_itr->status == DISCOUNT_UNUSED, "discount member used");

   // Get the discount_member_id corresponding to the discount_st
   discounts _discounts(get_self(), get_self().value);
   auto discount_st_itr = _discounts.find(dismems_itr->disid);
   check(discount_st_itr != _discounts.end(), "disid not found");

   // Check if the discount has expired
   check(current_time_point() < eosio::time_point(discount_st_itr->end_time), "discount expired");

   // Check if the transfer amount is equal to the price of the membership type.
   check(quantity == itr->current_price, "invalid payment amount");

   // Next, the data will be stored in the orders table.
   orders _orders(get_self(), get_self().value);
   _orders.emplace(get_self(), [&](auto &row)
                   {
      row.id = _orders.available_primary_key();
      row.owner = from;
      row.quantity = quantity;
      row.mtid = mtid;
      row.discount_member_id = discount_member_id;
      row.status = PAID;
      row.duration = itr->duration;
      row.create_time = current_time_point(); });

   // Change the status of member coupons to prevent duplicate usage.
   _discountmembers.modify(dismems_itr, get_self(), [&](auto &row)
                           { row.status = DISCOUNT_USED; });
   write_log(from, "buy");
}

name dfsmoonbuild::get_contract_by_symbol(const symbol &sym)
{
   name contract_name = name("usdtusdtusdt");
   if (sym == symbol("USDT", 8))
   {
      contract_name = name("usdtusdtusdt");
   }
   else if (sym == symbol("DFS", 8))
   {
      contract_name = name("eosio.token");
   }
   else if (sym == symbol("JIU", 4))
   {
      contract_name = name("jiutokenmain");
   }
   return contract_name;
}

void dfsmoonbuild::write_log(name user, string type)
{
   logs _logs(get_self(), get_self().value);

   _logs.emplace(get_self(), [&](auto &s)
                 {
      s.id = _logs.available_primary_key();
      s.user = user;
      s.trx_id = get_trx_id();
      s.type = type;
      s.time = current_time_point(); });
}

// trx_id helper
checksum256 dfsmoonbuild::get_trx_id()
{
   size_t size = transaction_size();
   char buf[size];
   size_t read = read_transaction(buf, size);
   return sha256(buf, read);
}