add_rules("mode.debug", "mode.release")

add_includedirs("./lua/")
add_includedirs("./interface/")

target("microlua")
    set_kind("binary")
    add_files("./*.c")
    add_files("./lua/*.c")
target_end()
