-- ============================================================================
-- Rotator.lua
-- Simple rotating object script example
-- ============================================================================

-- This script rotates an object continuously around the Y axis

-- Local variables (private to this script instance)
local rotationSpeed = 90.0  -- degrees per second
local currentAngle = 0.0

-- Called once when the script is initialized
function OnInit()
    print("Rotator script initialized for entity: " .. self:GetTag())
    
    -- Get initial position
    local pos = self:GetPosition()
    print(string.format("Initial position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z))
end

-- Called every frame
function OnUpdate(deltaTime)
    -- Update rotation angle
    currentAngle = currentAngle + rotationSpeed * deltaTime
    
    -- Keep angle in [0, 360) range
    if currentAngle >= 360.0 then
        currentAngle = currentAngle - 360.0
    end
    
    -- Set rotation (Y-axis rotation)
    local rotation = Vec3(0, currentAngle, 0)
    self:SetRotation(rotation)
end

-- Called when the script is destroyed
function OnDestroy()
    print("Rotator script destroyed for entity: " .. self:GetTag())
end
