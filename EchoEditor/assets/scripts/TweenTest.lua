exposedVars = {
    int_test    = 1
}

function Init()
    local duration = 10.0  -- Duración de la interpolación en segundos

    -- Llama a la función MoveEntityWithTween
    EntityManager:MoveEntityWithTween(entity, 10.0, 5.0, 0.0, duration)

    -- Llama a la función ScaleEntityWithTween
    EntityManager:ScaleEntityWithTween(entity, 10.0, 10.0, 2.0, duration)

    -- Llama a la función RotateEntityWithTween
    EntityManager:RotateEntityWithTween(entity, 45.0, 90.0, 0.0, duration)  -- Rotar a 45 grados en X y 90 grados en Y
end

-- Dejar vacío el Update por ahora
function Update(deltaTime)
end
