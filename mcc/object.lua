Object = {}

setmetatable(Object, { __class = Object })

function Object:extend(tab)
   class = tab or {}
   super = self:class()

   objmt = {
      __index = class,
      __class = class,
   }

   clsmt = {
      __index = super,
      __class = class,
      __objmt = objmt,
   }

   setmetatable(class, clsmt)

   function class.new(tab)
      obj = tab or {}
      setmetatable(obj, objmt)
      return obj
   end

   return class
end

function Object:class()
   return getmetatable(self).__class
end

function Object:super()
   class = self:class()
   return getmetatable(class).__index
end

function Object:extends(class)
   super = self:super()
   return super and super:isa(class)
end

function Object:isa(class)
   return self:class() == class or self:extends(class)
end

--[[
   Local variables:
   mode: Lua
   tab-width: 4
   indent-tabs-mode: nil
   End:
--]]
