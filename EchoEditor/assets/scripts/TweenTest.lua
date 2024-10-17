exposedVars = {
    int_cosa    = 27,   -- Ancho de la matriz del suelo
}

function Init()
    -- Define la posición de destino como un vector glm
    
    local duration = 2.0  -- Duración de la interpolación en segundos

    -- Llama a la función MoveEntityWithTween
    EntityManager:MoveEntityWithTween(entity, 10.0, 5.0, 0.0, duration)
end

-- Dejar vacío el Update por ahora
function Update(deltaTime)
end
