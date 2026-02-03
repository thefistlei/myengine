-- ============================================================================
-- Orbiter.lua
-- Example script that orbits an object around a center point
-- Demonstrates Vec3 math operations
-- ============================================================================

-- Configuration
local centerPoint = Vec3(0, 0, 0)
local orbitRadius = 5.0
local orbitSpeed = 1.0
local orbitHeight = 2.0
local angle = 0.0

-- Called once when the script is initialized
function OnInit()
    print("=== Orbiter Script Initialized ===")
    print("Entity: " .. self:GetTag())
    
    -- Demonstrate Vec3 operations
    local pos = self:GetPosition()
    print(string.format("Starting position: %s", tostring(pos)))
    
    -- Vec3 math examples
    local v1 = Vec3(1, 2, 3)
    local v2 = Vec3(4, 5, 6)
    
    print("\n=== Vec3 Math Examples ===")
    print("v1 = " .. tostring(v1))
    print("v2 = " .. tostring(v2))
    
    local sum = v1 + v2
    print("v1 + v2 = " .. tostring(sum))
    
    local diff = v2 - v1
    print("v2 - v1 = " .. tostring(diff))
    
    local scaled = v1 * 2.0
    print("v1 * 2.0 = " .. tostring(scaled))
    
    print("v1:Length() = " .. v1:Length())
    
    local normalized = v1:Normalize()
    print("v1:Normalize() = " .. tostring(normalized))
    print("Normalized length: " .. normalized:Length())
    
    local dot = v1:Dot(v2)
    print("v1:Dot(v2) = " .. dot)
    
    local cross = v1:Cross(v2)
    print("v1:Cross(v2) = " .. tostring(cross))
    
    print("=== End Vec3 Examples ===\n")
    
    -- Set initial orbit position
    local startPos = Vec3(centerPoint.x + orbitRadius, centerPoint.y + orbitHeight, centerPoint.z)
    self:SetPosition(startPos)
end

-- Called every frame
function OnUpdate(deltaTime)
    -- Update angle
    angle = angle + orbitSpeed * deltaTime
    
    -- Keep angle in [0, 2*PI) range
    if angle >= 2 * math.pi then
        angle = angle - 2 * math.pi
    end
    
    -- Calculate new position using circular motion
    local x = centerPoint.x + orbitRadius * math.cos(angle)
    local y = centerPoint.y + orbitHeight
    local z = centerPoint.z + orbitRadius * math.sin(angle)
    
    local newPosition = Vec3(x, y, z)
    self:SetPosition(newPosition)
    
    -- Calculate direction to center
    local direction = centerPoint - newPosition
    local distance = direction:Length()
    
    -- Look towards center (would need rotation implementation)
    -- For now, just log periodically
    if math.floor(angle * 10) % 10 == 0 then
        print(string.format("Orbiting: angle=%.2f, distance=%.2f", angle, distance))
    end
end

-- Called when the script is destroyed
function OnDestroy()
    print("Orbiter script destroyed for " .. self:GetTag())
end

-- Custom function to change orbit parameters
function SetOrbitParams(radius, speed, height)
    orbitRadius = radius
    orbitSpeed = speed
    orbitHeight = height
    print(string.format("Orbit params: radius=%.2f, speed=%.2f, height=%.2f", 
        radius, speed, height))
end

-- Custom function to set orbit center
function SetOrbitCenter(x, y, z)
    centerPoint = Vec3(x, y, z)
    print("Orbit center set to: " .. tostring(centerPoint))
end
