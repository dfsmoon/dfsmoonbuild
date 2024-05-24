--- dfsmoonbuild Project ---

A membership and discount coupon contract suitable for web3 projects.

This contract code can be run on the DFS Chain

The code only contains the full code of the contract, for how to run it, please refer to the[https://github.com/DFSNetwork/jiucorevault](https://github.com/DFSNetwork/jiucorevault), I will not go into too much detail here.


--- Test  ---
`Please replace the public key and account in the test command below with your own.`


### Create account
```sh
cleos create account eosio dmoonincome EOS7ap72uddq1n6o39rEhvrDAYqLGj6NSVdv8wkNvB3FAZeWfpFAJ
cleos create account eosio dfsmoonbuild EOS7ap72uddq1n6o39rEhvrDAYqLGj6NSVdv8wkNvB3FAZeWfpFAJ
```

### Deploy contract
```sh
cleos set contract dfsmoonbuild ./dfsmoonbuild/build/dfsmoonbuild -p dfsmoonbuild
cleos set account permission dfsmoonbuild active --add-code dfsmoonbuild -p dfsmoonbuild 
```

### Set account permissions separately
If using active permissions, this step can be skipped.

Add a new permission to the dfsmoonopopp account, called ops.

```sh
cleos set account permission dfsmoonbuild ops '{"threshold":1,"keys":[{"key":"EOS7ap72uddq1n6o39rEhvrDAYqLGj6NSVdv8wkNvB3FAZeWfpFAJ"}], "accounts": [{"permission":{"actor":"dfsmoonbuild","permission":"eosio.code"},"weight":1}]}' "active" -p dfsmoonbuild@active
# View permissions
cleos get account dfsmoonbuild 
# Grant ops permission to execute the createtype operation of the dfsmoonbuild contract.
cleos set action permission dfsmoonbuild dfsmoonbuild createtype ops
```

### discount table
```sh
# create discount
cleos push action dfsmoonbuild addiscount '[1,0,     "2024-05-16T11:00:00"]' -p dfsmoonbuild@active
# update discount
cleos push action dfsmoonbuild updiscount '[1,1,     "2024-05-19T11:00:00"]' -p dfsmoonbuild@active
# delete discount
cleos push action dfsmoonbuild deldiscount '[2]' -p dfsmoonbuild@active
```

# membertype table
```sh
# create 
cleos push action dfsmoonbuild createtype '[1, 30, "1 YFC" "0.8000 YFC", 1]' -p dfsmoonbuild@active
cleos push action dfsmoonbuild createtype '[3, 30, "9.90000000 USDT" "6.90000000 USDT", 1]' -p dfsmoonbuild@active
cleos push action dfsmoonbuild createtype '[4, 30, "5.90000000 DFS" "3.90000000 DFS", 1]' -p dfsmoonbuild@active

# update
cleos push action dfsmoonbuild updatetype '[1, 60,  "1.00000000 DFS", "0.80000000 DFS", 0]' -p dfsmoonbuild@active
cleos push action dfsmoonbuild updatetype '[2, 30, "20000.0000 JIU" "10000.0000 JIU", 1]' -p dfsmoonbuild@active
cleos push action dfsmoonbuild updatetype '[4, 30, "5.90000000 DFS" "3.90000000 DFS", 2]' -p dfsmoonbuild@active

 # delete
 cleos push action dfsmoonbuild deletetype '[1]' -p dfsmoonbuild@active
```

### other
```sh
# Used to compensate users when there is a user loss
 # Administrators give users memberships directly or add hours.
 cleos push action dfsmoonbuild reparation '["aaaaaaaaaaa1", 2]' -p dfsmoonbuild@active
 # Modify Member Status
  cleos push action dfsmoonbuild updatemember '["aaaaaaaaaaa1", 2]' -p dfsmoonbuild@active
  # delete member
  cleos push action dfsmoonbuild deletemember '["aaaaaaaaaaa1"]' -p dfsmoonbuild@active

# Member list with benefits
cleos push action dfsmoonbuild adddismem '["aaaaaaaaaaa2", 2]' -p dfsmoonbuild@active
  # delete
cleos push action dfsmoonbuild deldismem '[1]' -p dfsmoonbuild@active
```

### buy member
```sh
cleos push action usdtusdtusdt transfer '["aaaaaaaaaaa2", "dfsmoonbuild", "0.49500000 USDT", "buy:27:1"]' -p aaaaaaaaaaa2@active
cleos push action eosio.token transfer '["aaaaaaaaaaa1", "dfsmoonbuild", "0.80000000 DFS", "buy:1"]' -p aaaaaaaaaaa1@active
cleos push action jiutokenmain transfer '["aaaaaaaaaaa1", "dfsmoonbuild", "10000.0000 JIU", "buy:2:0"]' -p aaaaaaaaaaa1@active

# Buy a membership with discounts.
cleos push action usdtusdtusdt transfer '["aaaaaaaaaaa2", "dfsmoonbuild", "6.90000000 USDT", "buy:3:0"]' -p aaaaaaaaaaa2@active
```

```sh
 ## User initiated refund
cleos push action dfsmoonbuild refundorder '["aaaaaaaaaaa1", 0]' -p aaaaaaaaaaa1@active

 ## User confirmation in advance
cleos push action dfsmoonbuild confirmorder '["aaaaaaaaaaa1", 2]' -p aaaaaaaaaaa1@active

 ## Contract confirmation
cleos push action dfsmoonbuild codeconfirm '[0, 2]' -p dfsmoonbuild@active
```
### Query table
```sh
cleos get table dfsmoonbuild dfsmoonbuild members 
cleos get table dfsmoonbuild dfsmoonbuild orders
cleos get table dfsmoonbuild dfsmoonbuild membertypes
cleos get table dfsmoonbuild dfsmoonbuild discounts
cleos get table dfsmoonbuild dfsmoonbuild dismems
cleos get table dfsmoonbuild dfsmoonbuild logs

# Search by index
cleos get table dfsmoonbuild dfsmoonbuild members --index 3 --key-type i64 --lower 1 --upper 1

{
  "json": true,
  "code": "dfsmoonbuild",
  "scope": "dfsmoonbuild",
  "table": "orders",
  "table_key": "id",
  "lower_bound": "0",
  "upper_bound": "100",
  "limit": 10
}

```

### Check account balance
```sh
cleos get currency balance yfctokenmain dfsmoonbuild YFC
```
