Commands = {
    Test = 0x00,
    Status = 0x01
}

print("Opening file ... ")
local file = io.open("COM8", "wb")
print("done!")

print("Writing to file ... ")
file:write(Commands.Status)
print("done!");

print("Reading: ", string.byte(file:read(1))) 