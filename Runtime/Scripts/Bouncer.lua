-- ============================================================================
-- Bouncer.lua
-- Example script that makes an object bounce up and down
-- ============================================================================

-- Local state variables
local initialPosition = nil
local bounceHeight = 2.0
local bounceSpeed = 2.0
local time = 0.0

-- Called once when the script is initialized
function OnInit()
    print("Bouncer script initialized!")
    
    -- Store the initial position
    initialPosition = self:GetPosition()
    print(string.format("Starting at position: (%.2f, %.2f, %.2f)", 
        initialPosition.x, initialPosition.y, initialPosition.z))
end

-- Called every frame
function OnUpdate(deltaTime)
    -- Update time
    time = time + deltaTime
    
    -- Calculate bounce offset using sine wave
    local bounceOffset = math.sin(time * bounceSpeed) * bounceHeight
    
    -- Create new position (bounce on Y axis)
    local newPosition = Vec3(
        initialPosition.x,
        initialPosition.y + bounceOffset,
        initialPosition.z
    )
    
    -- Update entity position
    self:SetPosition(newPosition)
end

-- Called when the script is destroyed
function OnDestroy()
    print("Bouncer script destroyed")
    
    -- Restore original position
    if initialPosition then
        self:SetPosition(initialPosition)
    end
end

-- Custom function to change bounce parameters
function SetBounceParams(height, speed)
    bounceHeight = height
    bounceSpeed = speed
    print(string.format("Bounce parameters updated: height=%.2f, speed=%.2f", height, speed))
end
