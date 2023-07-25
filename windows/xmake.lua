add_rules("mode.debug", "mode.release")

add_includedirs("../src")
add_includedirs("../lib/lw_oopc")
add_includedirs("../src/interface")

-- add_cflags("-DMAKE_LUAC")

target("microlua_win")
    set_kind("binary")
    add_files("./*.c" , "../src/onelua.c", "../src/lua.c", "../src/interface/*.c")
target_end()