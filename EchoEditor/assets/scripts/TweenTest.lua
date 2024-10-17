exposedVars = {
    int_cosa    = 27,   -- Ancho de la matriz del suelo
}

function Init()
    -- Define la posici�n de destino como un vector glm
    
    local duration = 2.0  -- Duraci�n de la interpolaci�n en segundos

    -- Llama a la funci�n MoveEntityWithTween
    EntityManager:MoveEntityWithTween(entity, 10.0, 5.0, 0.0, duration)
end

-- Dejar vac�o el Update por ahora
function Update(deltaTime)
end
