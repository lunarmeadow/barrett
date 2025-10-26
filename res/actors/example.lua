-- example actor

function actor:spawn()
	-- replace tile indexes
	self.replaceSpriteByIndex(108, 111) -- low guard
	self.replaceSpriteByIndex(112, 115) -- low guard (patrol)
	self.replaceSpriteByIndex(116, 119) -- low guard (ambush)
	self.replaceSpriteByIndex(126, 129) -- hard mode low guard
	self.replaceSpriteByIndex(130, 133) -- hard mode low guard (patrol)
	self.replaceSpriteByIndex(134, 137) -- hard mode low guard (ambush)
end

function actor:think()

end
