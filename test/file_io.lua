function test1()
    local file = io.open("test1.txt", "w");
    for i = 1, 10, 1 do
        local str = "line" .. i .. "\n";
        file:write(str);
    end
    file:close()
    local file2 = io.open("test1.txt", "r")
    for i = 1, 10, 1 do
        print(file2:read());
    end
    file2:close();
end

function test2()
    local file = io.open("test1.txt", "a");
    for i = 10, 20, 1 do
        local str = "line" .. i .. "\n";
        file:write(str);
    end
    file:close();
    local file2 = io.open("test1.txt", "r")
    for i = 1, 20, 1 do
        print(file2:read());
    end
    file2:close();
end

function test3()
    local file = io.open("test1.txt", "w+");
    for i = 20, 30, 1 do
        local str = "line" .. i .. "\n";
        file:write(str);
    end
    file:close();
    local file2 = io.open("test1.txt", "r")
    for i = 1, 10, 1 do
        print(file2:read());
    end
    file2:close();
end

function test4()
    local file = io.open("test2.txt", "r+");
    for i = 30, 40, 1 do
        local str = "line" .. i .. "\n";
        file:write(str);
    end
    file:close();
    local file2 = io.open("test2.txt", "r")
    for i = 1, 10, 1 do
        print(file2:read());
    end
    file2:close();
end

function test5()
    local file = io.open("test2.txt", "a+");
    for i = 40, 50, 1 do
        local str = "line" .. i .. "\n";
        file:write(str);
    end
    file:close();
    local file2 = io.open("test2.txt", "r")
    for i = 1, 10, 1 do
        print(file2:read());
    end
    file2:close();
end

function test6()
    local file = io.open("test3.txt", "w");
    for i = 50, 60, 1 do
        local str = "line" .. i .. "\n";
        file:write(str);
    end
    file:close();
    local file2 = io.open("test3.txt", "r+")
    for i = 1, 10, 1 do
        print(file2:read());
    end
    file2:close();
end

function test7()
    local file = io.open("test4.txt", "w");
    for i = 60, 70, 1 do
        local str = "line" .. i .. "\n";
        file:write(str);
    end
    file:close();
    local file2 = io.open("test4.txt", "w+")
    for i = 1, 10, 1 do
        print(file2:read());
    end
    file2:close();
end

function test8()
    local file = io.open("test5.txt", "w");
    for i = 70, 80, 1 do
        local str = "line" .. i .. "\n";
        file:write(str);
    end
    file:close();
    local file2 = io.open("test5.txt", "a+")
    for i = 1, 10, 1 do
        print(file2:read());
    end
    file2:close();
end

function test9()
    local file = io.open("test1.txt", "w");
    io.output(file)
    for i = 1, 10, 1 do
        local str = "line" .. i .. "\n";
        io.write(str);
    end
    io.close(file);
    local file2 = io.open("test1.txt", "r")
    io.input(file2)
    for i = 1, 10, 1 do
        print(io.read());
    end
    io.close(file2);
end

function test10()
    local file = io.open("test1.txt", "w");
    for i = 1, 10, 1 do
        local str = "line" .. i .. "\n";
        file:write(str);
    end
    file:close()
    local file2 = io.open("test1.txt", "r")
    print(file2:read("*a"));
    file2:close();
end

function test11()
    local file = io.open("test1.txt", "w");
    for i = 1, 10, 1 do
        local str = "line: " .. i .. "\n";
        file:write(str);
    end
    local file2 = io.open("test1.txt", "r")
    file2:seek("end", -5)
    print(file:read("*a"))
    file2:close();
end

function test12()
    local file = io.open("test1.txt", "w");
    for i = 1, 10, 1 do
        local str = "line: " .. i .. "\n";
        file:write(str);
    end
    file:close()
    local file2 = io.open("test1.txt", "r")
    for c in file2:lines(1) do
        print(c)
    end
    file2:close();
end

function fsize(file)
    local current = file:seek()   -- get current position
    local size = file:seek("end") -- get file size
    file:seek("set", 1)           -- restore position
    current = file:seek()
    print("current: ", current)
    return size
end

function test13()
    local file = io.open("test1.txt", "w");
    for i = 1, 10, 1 do
        local str = "line: " .. i .. "\n";
        file:write(str);
    end
    print("size: ", fsize(file));
    file:close()
end

function test14()
    local file = io.open("test1.txt", "w");
    io.output(file)
    for i = 1, 10, 1 do
        local str = "line" .. i .. "\n";
        io.write(str);
    end
    io.close(file);
    local file2 = io.open("test1.txt", "r")
    io.input(file2)
    print(io.read("*a"));
    io.close(file2);
end

function test15()
    local file = io.open("test1.txt", "w");
    io.output(file)
    for i = 1, 10, 1 do
        local str = "line" .. i .. "\n";
        io.write(str);
    end
    io.close(file);
    io.read("*a");
end

function test16()
    local file = io.open("test1.txt", "w");
    io.output(file)
    for i = 1, 10, 1 do
        local str = "line" .. i .. "\n";
        io.write(str);
    end
    for c in io.lines(1) do
        print(c)
    end
    io.close(file);
end
