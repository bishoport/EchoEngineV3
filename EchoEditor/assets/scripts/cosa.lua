function Init()
    print("Lua Init function called")
end

function Update(deltaTime)
    local currentValue = script:GetSimpleFloat()
    script:SetSimpleFloat(currentValue + (0.1 * deltaTime))
end