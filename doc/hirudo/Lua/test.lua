--[[

lua
test
file

]]--

-- testing returning lua arguments.
function doStuff(n)

    -- tables
    f = {}
    k = {}

    f[1]               = "test"
    f["numberOfBeans"] = 2
    k["FKFKFKFK"]      = 3
    k[5]               = "b0x3n"
    k[6]               = f        -- nested table

    -- returning multiple arguments
    return k, "argument 2", 3, {3,4,5}

end

--[[
Test error handling :D
avdadlgbb98.233t mbg./...........
[apsd][asdpas]
'function
--]]
