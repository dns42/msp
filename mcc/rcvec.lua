
RCVec = Object:extend {
   MIN = 1000,
   MAX = 2000
}

function RCVec:update(tab)
   for k, v in pairs(tab) do
      self[k] = v
   end
end
