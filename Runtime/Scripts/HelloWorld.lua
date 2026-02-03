-- ============================================================================
-- HelloWorld.lua
-- Simplest possible Lua script example
-- ============================================================================

-- This script demonstrates the basic structure of a Lua script in MyEngine

function OnInit()
    print("Hello from Lua!")
    print("My entity name is: " .. self:GetTag())
    
    -- Get and print current position
    local pos = self:GetPosition()
    print(string.format("I'm at position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z))
end

function OnUpdate(deltaTime)
    -- This runs every frame
    -- deltaTime is the time since last frame in seconds
    
    -- Simple example: move up slowly
    local pos = self:GetPosition()
    pos.y = pos.y + 0.5 * deltaTime  -- Move up at 0.5 units per second
    self:SetPosition(pos)
end

function OnDestroy()
    print("Goodbye from Lua!")
end
