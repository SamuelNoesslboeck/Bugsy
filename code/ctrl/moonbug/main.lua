local bugsy = require("bugsy")

print("Opening port ... ")
local port, err = io.open("COM8", "wb")
print("done!")

if port then
    print(bugsy.Status.to_string(bugsy.cmd.get_status(port)))
else 
    print(err)
end