Object = {}

setmetatable(Object, { __class = Object })

function Object:extend(tab)
   local class = tab or {}
   local super = self:class()

   local objmt = {
      __index = class,
      __class = class,
   }

   local clsmt = {
      __index = super,
      __super = super,
      __class = class,
      __objmt = objmt,
   }

   setmetatable(class, clsmt)

   function class.__ctor(tab)
      obj = tab or {}
      setmetatable(obj, objmt)
      return obj
   end

   class.new = class.__ctor

   return class
end

function Object:class()
   return getmetatable(self).__class
end

function Object:super()
   local class = self:class()
   return getmetatable(class).__index
end

function Object:extends(class)
   local super = self:super()
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
