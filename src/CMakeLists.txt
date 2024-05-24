project(dfsmoonbuild)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( dfsmoonbuild dfsmoonbuild dfsmoonbuild.cpp )
target_include_directories( dfsmoonbuild PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( dfsmoonbuild ${CMAKE_SOURCE_DIR}/../ricardian )