
--[[
this function will be called by the engine when an actor wants to spawn.
"self" is an already allocated actor table with the following read-only fields:
	self.id          actor tile index
	self.tilex       tilemap x position
	self.tiley       tilemap y position
	self.x           fine x position (tilex + 0..1 within tile)
	self.y           fine y position (tiley + 0..1 within tile)
	self.z           distance from the ground plane
the table also has the following editable fields:
	self.nextthink   time when self.think should be called next
	self.think       function called after self.nextthink has passed
	self.tick        function called every game tick
return "true" from this function to tell the engine that you want to take
control of the actor. return "false" to revert to default behaviour.
]]--
function CheckSpawn(self)
	return false
end
