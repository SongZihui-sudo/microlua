add_rules("mode.debug", "mode.release")

target("microlua")
    add_includedirs("./lua/")
    set_kind("binary")
    add_files("./*.c")
    add_files("./lua/*.c")
target_end()
