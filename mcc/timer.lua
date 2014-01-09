
Timer = Object:extend()

RCTimer = Object:extend { hz = 50 }

function RCTimer.new(hz)

   local tmr = Timer.new()
   local rct = RCTimer.__ctor { tmr = tmr }

   tmr:link("tick"):connect(function () rct:_restart() end)

   if hz then
      rct.hz = hz
   end

   return rct
end

function RCTimer:interval()
   return 1.0 / self.hz
end

function RCTimer:start()
   self.tmr:start(self:interval())
end

function RCTimer:_restart()
   self.tmr:restart(self:interval())
end

function RCTimer:stop()
   self.tmr:stop()
end

function RCTimer:link(change)
   return self.tmr:link(change)
end

function RCTimer:tostring()
   return ("RCTimer %f Hz"):format(self.hz)
end
