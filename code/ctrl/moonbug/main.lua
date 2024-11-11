local bugsy = require("bugsy")

local function dump(o)
    if type(o) == 'table' then
       local s = '{ '
       for k,v in pairs(o) do
          if type(k) ~= 'number' then k = '"'..k..'"' end
          s = s .. '['..k..'] = ' .. dump(v) .. ','
       end
       return s .. '} '
    else
       return tostring(o)
    end
 end

print("Opening port ... ")
local port, err = io.open("COM8", "wb")
print("done!")

if port then
    print(bugsy.Status.to_string(bugsy.cmd.get_status(port)))
    print(bugsy.cmd.is_trader_ready(port))
    print(bugsy.cmd.is_rpi_ready(port))

    -- print(dump(bugsy.get_status_summary(port)))
else 
    print(err)
end