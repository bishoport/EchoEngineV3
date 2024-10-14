-- Tabla especial para las variables expuestas a ImGui
exposedVars = {
    int_width    = 27,   -- Ancho de la matriz del suelo
    int_height   = 21,  -- Alto de la matriz del suelo
    model_Floor  = "", -- Modelo de losa      1
    model_Corner = "",  -- Modelo de  Esquina 2,3,4,5
    model_Wall   = ""     -- Modelo de  Pared   6,7,8,9
}

local modelEntities = {}

-- Matriz predefinida de 1s y 0s (20x20)
local heroQuestBoard = { 
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 2, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 1, 1, 2, 6, 6, 6, 6, 6, 6, 6, 6, 6, 3, 1, 1},
    {1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1},
    {1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1},
    {1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1},
    {1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 7, 7, 5, 1, 1, 4, 7, 7, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1},
    {1, 1, 8, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 8, 1, 1, 1, 1, 1, 1, 9, 1, 1},
    {1, 1, 8, 1, 1, 1, 1, 1, 1, 9, 1, 2, 6, 1, 1, 6, 3, 1, 8, 1, 1, 1, 1, 1, 1, 9, 1, 1},
    {1, 1, 4, 7, 7, 7, 7, 7, 7, 5, 1, 8, 1, 1, 1, 1, 9, 1, 4, 7, 7, 7, 7, 7, 7, 5, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 2, 6, 6, 6, 6, 6, 6, 3, 1, 8, 1, 1, 1, 1, 9, 1, 2, 6, 6, 6, 6, 6, 6, 3, 1, 1},
    {1, 1, 8, 1, 1, 1, 1, 1, 1, 9, 1, 4, 7, 1, 1, 7, 5, 1, 8, 1, 1, 1, 1, 1, 1, 9, 1, 1},
    {1, 1, 8, 1, 1, 1, 1, 1, 1, 9, 1, 1, 1, 1, 1, 1, 1, 1, 8, 1, 1, 1, 1, 1, 1, 9, 1, 1},
    {1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 6, 6, 3, 1, 1, 2, 6, 6, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1},
    {1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1},
    {1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1},
    {1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 1, 1},
    {1, 1, 4, 7, 7, 7, 7, 7, 7, 7, 7, 7, 5, 1, 1, 4, 7, 7, 7, 7, 7, 7, 7, 7, 7, 5, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
}





-- Inicializar el suelo en función de la matriz
function Init()
    -- Iterar sobre la matriz para instanciar objetos según el valor de cada celda
    for i = 1, exposedVars.int_height do
        for j = 1, exposedVars.int_width do

            --------------------------------------------------------------------------------------------------
            --_FLOOR
            -------------------------------------------------------------------------------------------------- 
            if heroQuestBoard[i][j] == 1 then -- Crear entidad para losas en las posiciones donde hay un 2
                local posX = j * 1.0  -- Escala de la posición X
                local posZ = i * 1.0  -- Escala de la posición Z

                local instancedEntity = EntityManager:CreateEntityFromModel(exposedVars.model_Floor)
                EntityManager:AddChild(entity, instancedEntity)
                EntityManager:SetPosition(instancedEntity, posX, 0.0, posZ) 

            --------------------------------------------------------------------------------------------------
            --_CORNERS
            -------------------------------------------------------------------------------------------------- 
            elseif heroQuestBoard[i][j] == 2 then
                local posX = j * 1.0
                local posZ = i * 1.0
            
                local instancedEntity = EntityManager:CreateEntityFromModel(exposedVars.model_Corner)
                EntityManager:AddChild(entity, instancedEntity)
                EntityManager:SetPosition(instancedEntity, posX, 0.0, posZ)

                local rx, ry, rz = EntityManager:GetRotation(instancedEntity)
                ry = ry + 180.0
                EntityManager:SetRotation(instancedEntity, rx, ry, rz)
            elseif heroQuestBoard[i][j] == 3 then -- Crear entidad para losas en las posiciones donde hay un 3
                local posX = j * 1.0
                local posZ = i * 1.0
            
                local instancedEntity = EntityManager:CreateEntityFromModel(exposedVars.model_Corner)
                EntityManager:AddChild(entity, instancedEntity)
                EntityManager:SetPosition(instancedEntity, posX, 0.0, posZ)
            
                local rx, ry, rz = EntityManager:GetRotation(instancedEntity)
                ry = ry + 90
                EntityManager:SetRotation(instancedEntity, rx, ry, rz)
            elseif heroQuestBoard[i][j] == 4 then -- Crear entidad para losas en las posiciones donde hay un 4
               
               local posX = j * 1.0
               local posZ = i * 1.0
            
               local instancedEntity = EntityManager:CreateEntityFromModel(exposedVars.model_Corner)
               EntityManager:AddChild(entity, instancedEntity)
               EntityManager:SetPosition(instancedEntity, posX, 0.0, posZ)
            
               local rx, ry, rz = EntityManager:GetRotation(instancedEntity)
               ry = ry + 270.0
               EntityManager:SetRotation(instancedEntity, rx, ry, rz)
            elseif heroQuestBoard[i][j] == 5 then -- Crear entidad para losas en las posiciones donde hay un 5
                
                local posX = j * 1.0
                local posZ = i * 1.0
            
                local instancedEntity = EntityManager:CreateEntityFromModel(exposedVars.model_Corner)
                EntityManager:AddChild(entity, instancedEntity)
                EntityManager:SetPosition(instancedEntity, posX, 0.0, posZ)
            
                local rx, ry, rz = EntityManager:GetRotation(instancedEntity)
                --ry = ry + 90
                EntityManager:SetRotation(instancedEntity, rx, ry, rz)

            --------------------------------------------------------------------------------------------------
            --_WALLS
            --------------------------------------------------------------------------------------------------
            elseif heroQuestBoard[i][j] == 6 then -- Crear entidad para losas en las posiciones donde hay un 6
                
                local posX = j * 1.0
                local posZ = i * 1.0
            
                local instancedEntity = EntityManager:CreateEntityFromModel(exposedVars.model_Wall)
                EntityManager:AddChild(entity, instancedEntity)
                EntityManager:SetPosition(instancedEntity, posX, 0.0, posZ)
            
                local rx, ry, rz = EntityManager:GetRotation(instancedEntity)
                ry = ry + 180.0
                EntityManager:SetRotation(instancedEntity, rx, ry, rz)
            elseif heroQuestBoard[i][j] == 7 then -- Crear entidad para losas en las posiciones donde hay un 7
                
                local posX = j * 1.0
                local posZ = i * 1.0
            
                local instancedEntity = EntityManager:CreateEntityFromModel(exposedVars.model_Wall)
                EntityManager:AddChild(entity, instancedEntity)
                EntityManager:SetPosition(instancedEntity, posX, 0.0, posZ)
            
                local rx, ry, rz = EntityManager:GetRotation(instancedEntity)
                --ry = ry + 180.0
                EntityManager:SetRotation(instancedEntity, rx, ry, rz)
            elseif heroQuestBoard[i][j] == 8 then -- Crear entidad para losas en las posiciones donde hay un 8
                
                local posX = j * 1.0
                local posZ = i * 1.0
            
                local instancedEntity = EntityManager:CreateEntityFromModel(exposedVars.model_Wall)
                EntityManager:AddChild(entity, instancedEntity)
                EntityManager:SetPosition(instancedEntity, posX, 0.0, posZ)
            
                local rx, ry, rz = EntityManager:GetRotation(instancedEntity)
                ry = ry + 270.0
                EntityManager:SetRotation(instancedEntity, rx, ry, rz)
            elseif heroQuestBoard[i][j] == 9 then -- Crear entidad para losas en las posiciones donde hay un 9
                
                local posX = j * 1.0
                local posZ = i * 1.0
            
                local instancedEntity = EntityManager:CreateEntityFromModel(exposedVars.model_Wall)
                EntityManager:AddChild(entity, instancedEntity)
                EntityManager:SetPosition(instancedEntity, posX, 0.0, posZ)
            
                local rx, ry, rz = EntityManager:GetRotation(instancedEntity)
                ry = ry + 90.0
                EntityManager:SetRotation(instancedEntity, rx, ry, rz)

            end
        end
    end
end



-- Dejar vacío el Update por ahora
function Update(deltaTime)
end
