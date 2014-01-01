
RCVec = Object:extend {}

function RCVec:update(tab)
   for k, v in pairs(tab) do
      self[k] = v
   end
end
