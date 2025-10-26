-- example actor

function actor:spawn()
	-- replace tile indexes
	self.replaceSpriteTile(108, 111) -- low guard
	self.replaceSpriteTile(112, 115) -- low guard (patrol)
	self.replaceSpriteTile(116, 119) -- low guard (ambush)
	self.replaceSpriteTile(126, 129) -- hard mode low guard
	self.replaceSpriteTile(130, 133) -- hard mode low guard (patrol)
	self.replaceSpriteTile(134, 137) -- hard mode low guard (ambush)
end

function actor:think()

end
