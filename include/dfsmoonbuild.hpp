#include <utils.hpp>

CONTRACT dfsmoonbuild : public contract
{
   public:
      using contract::contract;

      // Define the income account as a constant.
      const name INCOME_ACCOUNT = "dfsmonincome"_n;

      // Membership status
      typedef uint8_t member_status;
      const member_status ACTIVE = 1; 
      const member_status INVALID = 2;

      // Order Status
      typedef uint8_t order_status;
      const order_status PAID = 1; 
      const order_status CONFIRMED = 2; // Confirmed, it will take effect once the user confirms.
      const order_status REFUNDED = 3; 

      // Define the status of the discount coupon, used, unused, expired
      const uint8_t DISCOUNT_UNUSED = 1;
      const uint8_t DISCOUNT_USED = 2;
      const uint8_t DISCOUNT_EXPIRED = 3;
      

      // Action listeners
      [[eosio::on_notify("eosio.token::transfer")]]
      void dfs_token_in(name from, name to, asset quantity, string memo);

      [[eosio::on_notify("jiutokenmain::transfer")]]
      void jiu_token_in(name from, name to, asset quantity, string memo);

      [[eosio::on_notify("usdtusdtusdt::transfer")]]
      void usdt_token_in(name from, name to, asset quantity, string memo);

      // The administrator adds members for users, for compensation or activities.
      // Used to compensate users when there is a user loss
      // type is membership_type
      ACTION reparation(name owner, uint32_t duration);
  
      ACTION updatemember(name owner, member_status status);
      // Delete member, not normally used
      ACTION deletemember(name owner);

      // Add discounts to users, such as Vote coupons, inviting coupons, etc.
      ACTION adddismem(name owner, uint8_t disid);
      ACTION deldismem(uint8_t id);

      // Membership Type Management
      ACTION createtype(uint8_t mtid, uint32_t duration, asset price, asset current_price, uint8_t disid);
      ACTION updatetype(uint8_t mtid, optional<uint32_t> duration, optional<asset> price, optional<asset> current_price, optional<uint8_t> disid);
      ACTION deletetype(uint8_t mtid);

      // Discount Type Management
      ACTION addiscount(uint8_t disid, uint8_t type, time_point_sec end_time);
      ACTION updiscount(uint8_t disid, uint8_t type, time_point_sec end_time);
      ACTION deldiscount(uint8_t disid);

     // Confirm order, Mainly contract account call, type = 2 CONFIRMED, type=3 REFUNDED
      ACTION codeconfirm(uint64_t orderid, uint8_t type);

      // User confirms order in advance
      ACTION confirmorder(name owner, uint64_t orderid);

      // User initiated refund
      ACTION refundorder(name owner, uint64_t orderid);

   private:
     // Order table
     TABLE order_st
      {
        uint64_t      id;
        name         owner;
        asset quantity;
        uint8_t      mtid;
        // The id used to store the discount, defaulting to 0, indicates that no coupon has been used.
        uint64_t discount_member_id;
        order_status status;
        uint32_t duration; // Membership duration, unit: seconds
        time_point_sec create_time;

        uint64_t primary_key() const { return id; }
        uint64_t get_owner() const { return owner.value; }
      };
      typedef eosio::multi_index<"orders"_n, order_st, indexed_by<"byowner"_n, const_mem_fun<order_st, uint64_t, &order_st::get_owner>>> orders;

      // Member table
      TABLE member_st
      {
        name         owner;
        member_status status;
        time_point_sec start_time;
        time_point_sec end_time;
        // This is used to store the total time a user has been a member, and then rewards can be issued based on the length of membership.
         uint32_t total_duration;

        uint64_t primary_key() const { return owner.value; }
        uint64_t get_status() const { return status; }
      };
      // typedef multi_index<name("members"), member_st> members;
      typedef eosio::multi_index<"members"_n, member_st, indexed_by<"bystatus"_n, const_mem_fun<member_st, uint64_t, &member_st::get_status>>> members;

      // Membership type structure
      TABLE membership_type {
         uint8_t mtid;
         uint32_t duration;
         asset price;
         asset current_price;
         uint8_t disid; // discount id

         uint64_t primary_key() const { return mtid; }
         uint64_t get_discount_id() const { return disid; }
      };
      typedef eosio::multi_index<"membertypes"_n, membership_type, indexed_by<"bydiscount"_n, const_mem_fun<membership_type, uint64_t, &membership_type::get_discount_id>>> membertypes;

      // discount table
      TABLE discount_st
      {
        uint8_t disid; 
        uint8_t type; // Default is 0, 0: means it applies to everyone, 1: means it is only available to those who have received a discount.
        time_point_sec end_time; // Discount end time

        uint64_t primary_key() const { return disid; }
      };
      typedef multi_index<name("discounts"), discount_st> discounts;

      // Membership table for getting discounts, one user can have multiple discounts
      TABLE discount_member_st
      {
        uint64_t      id;
        name owner;
        //  default DISCOUNT_UNUSED
        uint8_t status;
        uint8_t disid;

        uint64_t primary_key() const { return id; }
        uint64_t get_owner() const { return owner.value; }
      };
      typedef multi_index<name("dismems"), discount_member_st,
                      indexed_by<name("byowner"), const_mem_fun<discount_member_st, uint64_t, &discount_member_st::get_owner>>
                      > dismems;

      TABLE logs_st
      {
         uint64_t id;
         name user;
         string type;
         checksum256 trx_id;
         time_point_sec time;
         uint64_t byuser() const { return user.value; }
         uint64_t primary_key() const { return id; }
      };

      typedef multi_index<"logs"_n, logs_st, indexed_by<"byuser"_n, const_mem_fun<logs_st, uint64_t, &logs_st::byuser>>> logs;
      
      void write_log(name user, string type);

      void handle_token_transfer(name from, name to, asset quantity, string memo);
      // Get the contract account through symbol.
      name get_contract_by_symbol(const symbol &sym);
      checksum256 get_trx_id();
};