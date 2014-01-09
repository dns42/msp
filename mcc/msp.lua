
MSP = Object:extend {}

MSP.Cmd = Object:extend {}

function MSP:vbox()
   local cmd_boxnames = self:boxnames()
   local cmd_box = self:box()
   local cmd = self.ParallelCmd.new(self,
				   { cmd_boxnames, cmd_box })

   cmd.sync = function ()
      local names = cmd_boxnames:sync()
      local boxes = cmd_box:sync()

      res = {}

      for i, val in pairs(boxes) do
	 name = names[i]
	 res[name] = val
      end

      return res
   end

   return cmd
end

function MSP:setvbox(set)
   local cmd_vbox = self:vbox()
   local cmd_boxnames = cmd_vbox.tab[1]
   local cmd = self.SerialCmd.new(self, cmd_vbox)

   local function update(_, err, vbox)
      if err ~= 0 then
	 cmd:fini(err, nil)
	 return
      end

      local names = cmd_boxnames:sync()
      local boxes = {}
      local change = false
      local result = {}

      for i, name in pairs(names) do
	 local box = vbox[name]
	 if set[name] and set[name] ~= box then
	    box = set[name]
	    change = true
	 end
	 boxes[i] = box
	 result[name] = box
      end

      if change then
	 local cmd_setbox = self:set_box(boxes)

	 cmd:next(cmd_setbox)

	 function fini(cur, err, res)
	    cmd:fini(err, err == 0 and result or nil)
	 end

	 cmd_setbox:async(fini)
      else
	 cmd:fini(0, result)
      end
   end

   cmd_vbox:async(update)

   return cmd;
end

MSP.SerialCmd = Object:extend()

getmetatable(MSP.SerialCmd).__objmt.__tostring =
   function(self)
      return ("MSP.SerialCmd, cur %s"):format(tostring(self.cur))
   end

function MSP.SerialCmd.new(mwc, cur)
   local obj = {
      mwc = mwc,
      cur = cur,
   }
   return MSP.SerialCmd.__ctor(obj)
end

function MSP.SerialCmd:next(cmd)
   assert(cmd ~= nil)
   self.cur = cmd
end

function MSP.SerialCmd:fini(err, res)
   self.cur = nil
   self.res = res

   if self.fun then
      self:fun(err, res)
   end
end

function MSP.SerialCmd:async(fun)
   self.fun = fun
end

function MSP.SerialCmd:sync(cmd)
   while self.cur do
      self.cur:sync()
   end
   return self.res
end

function MSP.SerialCmd:poll()
   return self.cur == nil
end

MSP.ParallelCmd = Object:extend()

getmetatable(MSP.ParallelCmd).__objmt.__tostring =
   function(self)
      return "MSP.ParallelCmd"
   end

function MSP.ParallelCmd.new(mwc, tab)
   
   local obj = {
      mwc = mwc,
      tab = tab,
   }

   local function ret(cmd, err, res)
      obj:__ret(cmd, err, res)
   end

   for k, cmd in pairs(tab) do
      cmd:async(ret)
   end

   return MSP.ParallelCmd.__ctor(obj)
end

function MSP.ParallelCmd:__ret(cmd, err, res)
   if self:poll() and self.fun then
      self:fun(err, self:sync())
   end
end

function MSP.ParallelCmd:async(fun)
   self.fun = fun
end

function MSP.ParallelCmd:poll()
   for k, cmd in pairs(self.tab) do
      if not cmd:poll() then
	 return false
      end
   end
   return true
end
