Command = {
    Test = 0x00,
    Status = 0x01,
    
    IsTraderReady = 0x02,
}

Status = {
    RUNNING = 0x00,

    to_string = function (status)
        for k, v in pairs(Status) do
            if status == v then
                return k
            end
        end

        return nil
    end
}

--- Write a new command to the interface
--- 
--- @param fp file*
--- @param cmd number
--- @return string?
local function write_cmd(fp, cmd)
    local _, err = fp:write(cmd)
    return err
end


-- COMMANDS
local cmd = {
    ---comment
    ---@param fp file*
    ---@return number|nil
    ---@return nil|string
    get_status = function (fp)
        local err = write_cmd(fp, Command.Status)

        if err then
            return nil, err
        end

        local status, err = fp:read(1)

        if err then
            return nil, err
        end

        return string.byte(status, 1), nil
    end,

    is_trader_ready = function (fp)
        local err = write_cmd(fp, Command.IsTraderReady)

        if err then
            return nil, err
        end
    end
}

-- Module export
return {
    Command = Command,
    Status = Status,

    write_cmd = write_cmd,

    cmd = cmd
}