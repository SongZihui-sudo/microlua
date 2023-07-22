add_rules("mode.debug", "mode.release")

includes("./ports/stm32/")

target("microlua_rp2")
    on_build(function (target)
        print("---------- MicroLua RP2 ------------")
        os.cd("./ports/rp2")
        if not os.isdir("build") then
            print(os.curdir())
            os.mkdir("build")
        end
        local  CMAKE_C_COMPILER = "B:/gccarm10 2021.10/bin/arm-none-eabi-gcc.exe"
        local  CMAKE_CXX_COMPILER = "B:/gccarm10 2021.10/bin/arm-none-eabi-g++.exe"
        -- local  CMAKE_C_COMPILER = "F:/gccarm10 2021.10/bin/arm-none-eabi-gcc.exe"
        -- local  CMAKE_CXX_COMPILER = "F:/gccarm10 2021.10/bin/arm-none-eabi-g++.exe"
        os.execv("cmake", {"--no-warn-unused-cli", "-DCMAKE_C_COMPILER:FILEPATH="..CMAKE_C_COMPILER, "-DCMAKE_CXX_COMPILER:FILEPATH="..CMAKE_CXX_COMPILER, "-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE", "-DCMAKE_BUILD_TYPE:STRING=Debug", "-S./", "-B./build", "-G Ninja"})
        os.cd("./build")
        print(os.curdir())
        os.execv("cmake --build ./")
    end)target_end()

target("microlua_esp32")
    on_build(function (target)
        print("--------- MicroLua Esp32 --------------")
        os.cd("./ports/esp32")
    print(os.curdir())
        os.execv("idf.py", {"build"})
    end)
target_end()
